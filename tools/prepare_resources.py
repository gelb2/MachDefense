#!/usr/bin/env python3
"""Prepare MachDefense WeMade 1.0.3 game assets for source-only builds.

The input archive stays outside this repository. Only allowlisted game resource
members are read; executable, signatures, NIBs and package metadata are never
copied. Running the game or compiling it does not invoke this preparation tool.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import plistlib
import struct
import zipfile
import zlib
from collections import Counter
from pathlib import Path, PurePosixPath

ROOT = Path(__file__).resolve().parents[1]
SOURCE_SHA256 = "882fe8efa9a0c3d51c23da190ee3d30376a1b6766e16cc6b87daf09b98dc92b1"
BUNDLE = "com.wemadeent.HeavyMachDefence"
PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"


def digest(blob: bytes) -> str:
    return hashlib.sha256(blob).hexdigest()


def write(path: Path, data: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists() and path.read_bytes() == data:
        return
    path.write_bytes(data)


def dump(path: Path, value: object) -> None:
    write(path, (json.dumps(value, ensure_ascii=False, indent=2) + "\n").encode())


def random_bytes(seed: int, count: int) -> bytes:
    """BSD/Darwin TYPE_3 random(), isolated from host libc global RNG state.

    MachDefense QTable uses srandom(0xb27f7), then random()%227 per byte.
    Warm-up and additive 31-word state match Apple Libc random.c.
    """
    seed = seed & 0xFFFFFFFF or 1
    state = [seed]
    word = seed if seed < 0x80000000 else seed - 0x100000000
    for _ in range(1, 31):
        hi = abs(word) // 127773 * (-1 if word < 0 else 1)
        lo = word - hi * 127773
        word = 16807 * lo - 2836 * hi
        if word < 0:
            word += 2147483647
        state.append(word & 0xFFFFFFFF)
    front, rear = 3, 0
    result = bytearray()
    for index in range(count + 310):
        state[front] = (state[front] + state[rear]) & 0xFFFFFFFF
        value = (state[front] >> 1) & 0x7FFFFFFF
        front, rear = (front + 1) % 31, (rear + 1) % 31
        if index >= 310:
            result.append(value % 227)
    return bytes(result)


def decode_table(blob: bytes) -> bytes:
    if not blob.startswith(b"EncTbl"):
        raise ValueError("missing EncTbl header")
    payload = blob[6:]
    result = bytes(x ^ y for x, y in zip(payload, random_bytes(0xB27F7, len(payload))))
    return result


def png_pixels(blob: bytes) -> tuple[dict, bytes]:
    """Decode 8-bit RGB(A), including CgBI and Adam7, to RGBA.

    Returned CgBI RGB remains premultiplied; PNG filtering runs on original
    BGRA bytes before channel conversion. Each Adam7 pass resets its prior row.
    """
    if not blob.startswith(PNG_SIGNATURE):
        raise ValueError("invalid PNG signature")
    chunks, cursor = [], 8
    while cursor < len(blob):
        if cursor + 12 > len(blob):
            raise ValueError("truncated PNG chunk")
        size = struct.unpack_from(">I", blob, cursor)[0]
        kind = blob[cursor + 4:cursor + 8]
        payload = blob[cursor + 8:cursor + 8 + size]
        if len(payload) != size or cursor + size + 12 > len(blob):
            raise ValueError("truncated PNG payload")
        crc = struct.unpack_from(">I", blob, cursor + 8 + size)[0]
        if zlib.crc32(kind + payload) & 0xFFFFFFFF != crc:
            raise ValueError("PNG CRC mismatch")
        chunks.append((kind, payload))
        cursor += size + 12
    width, height, bits, color, compression, filtering, interlace = struct.unpack(
        ">IIBBBBB", next(data for kind, data in chunks if kind == b"IHDR"))
    if bits != 8 or color not in (2, 6) or compression or filtering or interlace not in (0, 1):
        raise ValueError(f"unsupported PNG format {(bits, color, compression, filtering, interlace)}")
    if not (0 < width <= 16384 and 0 < height <= 16384):
        raise ValueError("invalid PNG dimensions")
    cgbi = any(kind == b"CgBI" for kind, _ in chunks)
    raw = zlib.decompress(b"".join(data for kind, data in chunks if kind == b"IDAT"), -15 if cgbi else 15)
    channels = 4 if color == 6 else 3
    pixels = bytearray(width * height * 4)
    passes = [(0, 0, 1, 1)] if not interlace else [
        (0, 0, 8, 8), (4, 0, 8, 8), (0, 4, 4, 8), (2, 0, 4, 4),
        (0, 2, 2, 4), (1, 0, 2, 2), (0, 1, 1, 2)]
    cursor = 0
    for x0, y0, dx, dy in passes:
        pw, ph = max(0, (width - x0 + dx - 1) // dx), max(0, (height - y0 + dy - 1) // dy)
        if not pw or not ph:
            continue
        stride = pw * channels
        previous = bytearray(stride)
        for row in range(ph):
            if cursor + stride + 1 > len(raw):
                raise ValueError("truncated PNG scanline")
            kind = raw[cursor]
            current = bytearray(raw[cursor + 1:cursor + stride + 1])
            cursor += stride + 1
            if kind > 4:
                raise ValueError("invalid PNG filter")
            for i in range(stride):
                left = current[i - channels] if i >= channels else 0
                above = previous[i]
                upper_left = previous[i - channels] if i >= channels else 0
                if kind == 1:
                    predictor = left
                elif kind == 2:
                    predictor = above
                elif kind == 3:
                    predictor = (left + above) // 2
                elif kind == 4:
                    p = left + above - upper_left
                    pa, pb, pc = abs(p - left), abs(p - above), abs(p - upper_left)
                    predictor = left if pa <= pb and pa <= pc else above if pb <= pc else upper_left
                else:
                    predictor = 0
                current[i] = (current[i] + predictor) & 255
            for col in range(pw):
                source, target = col * channels, ((y0 + row * dy) * width + x0 + col * dx) * 4
                red, green, blue = current[source:source + 3]
                if cgbi:
                    red, blue = blue, red
                pixels[target:target + 4] = bytes((red, green, blue, current[source + 3] if channels == 4 else 255))
            previous = current
    if cursor != len(raw):
        raise ValueError("unexpected PNG scanline bytes")
    return {"width": width, "height": height, "cgbi": cgbi, "interlaced": bool(interlace)}, bytes(pixels)


def png_encode(width: int, height: int, pixels: bytes) -> bytes:
    def chunk(kind: bytes, data: bytes) -> bytes:
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)
    rows = b"".join(b"\0" + pixels[y * width * 4:(y + 1) * width * 4] for y in range(height))
    return PNG_SIGNATURE + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)) + chunk(b"IDAT", zlib.compress(rows, 9)) + chunk(b"IEND", b"")


def jpeg_size(blob: bytes) -> tuple[int, int]:
    if blob[:2] != b"\xff\xd8":
        raise ValueError("invalid JPEG signature")
    cursor = 2
    while cursor < len(blob):
        if blob[cursor] != 255:
            raise ValueError("invalid JPEG marker")
        while cursor < len(blob) and blob[cursor] == 255:
            cursor += 1
        marker = blob[cursor]
        cursor += 1
        if marker in (0xD8, 0xD9) or 0xD0 <= marker <= 0xD7:
            continue
        size = struct.unpack_from(">H", blob, cursor)[0]
        if marker in (0xC0, 0xC1, 0xC2):
            height, width = struct.unpack_from(">HH", blob, cursor + 3)
            return width, height
        cursor += size
    raise ValueError("JPEG frame header absent")


def binary_node(blob: bytes, offset: int = 0, depth: int = 0) -> tuple[dict, int]:
    if depth > 30 or offset + 16 > len(blob):
        raise ValueError("invalid MCB node")
    size, kind, length = struct.unpack_from("<III", blob, offset)
    end = offset + 12 + length
    if end + 4 > len(blob):
        raise ValueError("truncated MCB node")
    count = struct.unpack_from("<I", blob, end)[0]
    if count > 50000:
        raise ValueError("invalid MCB child count")
    payload = blob[offset + 12:end]
    result = {"offset": offset, "declared_size": size, "type": kind,
              "data_size": length, "data_hex": payload.hex(), "children": []}
    end += 4
    for _ in range(count):
        child, end = binary_node(blob, end, depth + 1)
        result["children"].append(child)
    result["consumed_size"] = end - offset
    return result, end


def table_summary(data: bytes) -> dict:
    lines = [line.strip("\r") for line in data.decode("utf-8-sig").splitlines()
             if line.strip() and not line.lstrip().startswith("//")]
    return {"columns": [s.strip() for s in lines[0].split("\t")], "rows": len(lines) - 1}


def prepare(source: Path, output: Path) -> dict:
    source, output = source.resolve(), output.resolve()
    if source == ROOT or ROOT in source.parents:
        raise ValueError("source archive must stay outside the repository")
    archive = source.read_bytes()
    if digest(archive) != SOURCE_SHA256:
        raise ValueError("source SHA-256 differs from selected WeMade build")
    output.mkdir(parents=True, exist_ok=True)
    files, images, tables, excluded = [], {}, {}, []
    prepared_tables = {}
    with zipfile.ZipFile(source) as package:
        metadata_name = "Payload/MachDefense.app/Info.plist"
        metadata = plistlib.loads(package.read(metadata_name))
        if metadata["CFBundleIdentifier"] != BUNDLE or metadata["CFBundleVersion"] != "1.0.3":
            raise ValueError("source bundle identity differs")
        prefix = "Payload/MachDefense.app/"
        for member in sorted(package.namelist()):
            if not member.startswith(prefix) or member.endswith("/"):
                continue
            name = member[len(prefix):]
            relative = PurePosixPath(name)
            # The original game resources are direct bundle children. Never
            # traverse a package directory or copy any executable or NIB.
            if len(relative.parts) != 1 or relative.name != name or relative.suffix.lower() not in {".png", ".jpg", ".wav", ".mp3", ".mcb", ".tble", ".tbl"}:
                excluded.append(name)
                continue
            blob = package.read(member)
            extension = relative.suffix.lower()
            entry = {"source_member": name, "source_size": len(blob), "source_sha256": digest(blob)}
            if extension in (".png", ".jpg"):
                original_path, path = "OriginalImages/" + name, "Images/" + name
                write(output / original_path, blob)
                if blob.startswith(PNG_SIGNATURE):
                    meta, pixels = png_pixels(blob)
                    entry["decoded_source_rgba_sha256"] = digest(pixels)
                    straight = bytearray(pixels)
                    if meta["cgbi"]:
                        for i in range(0, len(straight), 4):
                            alpha = straight[i + 3]
                            for channel in range(3):
                                straight[i + channel] = min(255, (straight[i + channel] * 255 + alpha // 2) // alpha) if alpha else 0
                    prepared = png_encode(meta["width"], meta["height"], bytes(straight))
                    check, decoded = png_pixels(prepared)
                    if decoded != straight or check["cgbi"] or check["interlaced"]:
                        raise ValueError("prepared PNG pixel verification failed")
                    entry.update(meta)
                    entry["prepared_pixel_sha256"] = digest(decoded)
                    entry["alpha"] = "straight RGBA"
                elif blob.startswith(b"\xff\xd8"):
                    width, height = jpeg_size(blob)
                    entry.update(width=width, height=height, alpha="opaque JPEG")
                    prepared = blob
                else:
                    raise ValueError(f"unknown image encoding: {name}")
                write(output / path, prepared)
                entry.update(category="image", path=path, sha256=digest(prepared), original_path=original_path)
                images[name] = {k: v for k, v in entry.items() if k not in ("category", "source_member")}
            elif extension in (".wav", ".mp3"):
                path = "Audio/" + name
                write(output / path, blob)
                entry.update(category="audio", path=path, sha256=digest(blob))
            else:
                path = "RawData/" + name
                write(output / path, blob)
                entry.update(category="data", path=path, sha256=digest(blob))
                if extension in (".tble", ".tbl"):
                    decoded_bytes = decode_table(blob) if extension == ".tble" else blob
                    try:
                        decoded_text = decoded_bytes.decode("utf-8", errors="strict")
                        encoding = "utf-8"
                    except UnicodeDecodeError:
                        # Spawn-table comments use Korean Windows codepage;
                        # the description table itself uses UTF-8.
                        decoded_text = decoded_bytes.decode("cp949", errors="strict")
                        encoding = "cp949"
                    plaintext = decoded_text.encode("utf-8")
                    table_path = "Tables/" + relative.stem + ".tsv"
                    if table_path in prepared_tables and prepared_tables[table_path] != digest(plaintext):
                        raise ValueError(f"different original tables map to {table_path}")
                    prepared_tables[table_path] = digest(plaintext)
                    write(output / table_path, plaintext)
                    entry.update(table_path=table_path, table_sha256=digest(plaintext), source_text_encoding=encoding, decoded_source_sha256=digest(decoded_bytes), **table_summary(plaintext))
                    tables[name] = {k: entry[k] for k in ("table_path", "table_sha256", "columns", "rows")}
                else:
                    node, end = binary_node(blob)
                    if end != len(blob):
                        raise ValueError(f"unconsumed MCB data in {name}: {end}/{len(blob)}")
                    node_path = "Data/" + relative.stem + ".json"
                    dump(output / node_path, {"source": name, "source_sha256": digest(blob), "root": node})
                    entry.update(node_path=node_path, root_children=len(node["children"]))
            files.append(entry)
    if digest(source.read_bytes()) != SOURCE_SHA256:
        raise ValueError("source changed while preparing resources")
    result = {"source_bundle": BUNDLE, "source_version": "1.0.3", "source_ipa_sha256": SOURCE_SHA256,
              "files": files, "counts": dict(Counter(e["category"] for e in files)),
              "excluded_members": excluded, "source_unchanged": True,
              "image_conversion": "CgBI BGRA/premultiplied to standard straight RGBA PNG; Adam7 flattened; JPEG copied unchanged",
              "alpha_limit": "Partial-alpha CgBI contains quantized premultiplied RGB; straight conversion cannot recover pre-quantization original RGB.",
              "game_build_inputs": ["Images", "Audio", "RawData", "Tables"],
              "evidence_only": ["OriginalImages", "Data"]}
    dump(output / "images.json", images)
    dump(output / "tables.json", tables)
    dump(output / "inventory.json", result)
    print(json.dumps({"counts": result["counts"], "source_tables": len(tables), "unique_tables": len(prepared_tables), "mission_rows": tables["MapList.tble"]["rows"],
                      "cgbi": sum(bool(e.get("cgbi")) for e in files), "adam7": sum(bool(e.get("interlaced")) for e in files),
                      "excluded": len(excluded), "source_unchanged": True}))
    return result


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--output", type=Path, default=ROOT / "Resources")
    options = parser.parse_args()
    prepare(options.source, options.output)
