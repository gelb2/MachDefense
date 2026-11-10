#!/usr/bin/env python3
"""Import a user-notified local editor file without rewriting its text or running Git."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import stat
import sys
import tempfile

DOCUMENTS = {
    "article-ko": "docs/public/unease-and-typewriters.md",
    "article-en": "docs/en/public/unease-and-typewriters.md",
    "materials-ko": "docs/editorial/helicopter-writing-inventory-20261110.md",
    "materials-en": "docs/en/editorial/helicopter-writing-inventory-20261110.md",
}
SITE_ID = "appgprj_6addbf55aea081918d3d182d7e496e88"
STATE_PATH = "docs/editorial/editor-sync-state.json"


def digest(data):
    return hashlib.sha256(data).hexdigest()


def atomic_write(path, data):
    mode = stat.S_IMODE(path.stat().st_mode) if path.exists() else 0o644
    fd, name = tempfile.mkstemp(prefix=".editor-import-", dir=path.parent)
    try:
        with os.fdopen(fd, "wb") as stream:
            stream.write(data)
        os.chmod(name, mode)
        os.replace(name, path)
    finally:
        if os.path.exists(name):
            os.unlink(name)


def import_revision(payload, root, apply=False):
    root = Path(root).resolve()
    document = payload.get("documentId")
    relative = DOCUMENTS.get(document)
    if not relative or payload.get("path") != relative or payload.get("repository") != "gelb2/MachDefense":
        raise ValueError("등록된 원고·저장소·파일 경로가 아닙니다.")
    version = payload.get("version")
    if type(version) is not int or version <= 0:
        raise ValueError("사용자가 저장한 수정본 번호가 필요합니다.")
    content = payload.get("content")
    if not isinstance(content, str):
        raise ValueError("원고 본문이 필요합니다.")
    data = content.encode("utf-8")
    if digest(data) != payload.get("contentSha256"):
        raise ValueError("저장본의 본문과 SHA-256이 다릅니다.")
    if not re.fullmatch(r"[0-9a-f]{64}", payload.get("sourceSha256", "")) or not re.fullmatch(r"[0-9a-f]{40}", payload.get("sourceCommit", "")):
        raise ValueError("기준 원고 해시와 커밋을 확인해야 합니다.")
    target = root / relative
    state_path = root / STATE_PATH
    for path in [target, state_path]:
        if path.is_symlink() or not path.resolve().is_relative_to(root):
            raise ValueError("저장소 밖이나 심볼릭 링크에 쓸 수 없습니다.")
    state = json.loads(state_path.read_text()) if state_path.exists() else {"schema": 1, "documents": {}}
    previous = state["documents"].get(document)
    if previous and version < previous["version"]:
        raise ValueError("이미 반영한 수정본보다 오래된 저장본입니다.")
    if previous and version == previous["version"] and previous["contentSha256"] != payload["contentSha256"]:
        raise ValueError("같은 저장본 번호의 본문 해시가 바뀌었습니다.")
    current = digest(target.read_bytes())
    expected = previous["contentSha256"] if previous else payload["sourceSha256"]
    if current not in [expected, payload["contentSha256"]]:
        raise ValueError("원고가 에디터의 기준본 이후 다른 곳에서 변경됐습니다. 차이를 검토해야 합니다.")
    record = {"version": version, "contentSha256": payload["contentSha256"], "sourceSha256": payload["sourceSha256"], "sourceCommit": payload["sourceCommit"], "savedAt": payload.get("savedAt"), "siteProjectId": SITE_ID}
    unchanged = previous == record and current == payload["contentSha256"]
    result = {"documentId": document, "path": relative, "version": version, "beforeSha256": current, "afterSha256": payload["contentSha256"], "status": "unchanged" if unchanged else "ready", "gitExecuted": False}
    if apply and not unchanged:
        state["documents"][document] = record
        # A retry can repair the receipt if a previous write stopped between these files.
        if current != payload["contentSha256"]:
            atomic_write(target, data)
        atomic_write(state_path, (json.dumps(state, ensure_ascii=False, indent=2) + "\n").encode("utf-8"))
        result["status"] = "applied"
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", required=True, help="Local editor export JSON file; '-' reads stdin")
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--apply", action="store_true", help="Write the exact saved revision after the user notifies the agent")
    args = parser.parse_args()
    try:
        payload = json.loads(sys.stdin.read() if args.input == "-" else Path(args.input).read_text())
        print(json.dumps(import_revision(payload, args.root, args.apply), ensure_ascii=False))
    except (ValueError, OSError, KeyError, TypeError) as error:
        parser.exit(1, str(error) + "\n")


if __name__ == "__main__":
    main()
