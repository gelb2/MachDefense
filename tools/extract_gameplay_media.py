#!/usr/bin/env python3
"""Export documented gameplay scenes from local originals; never upload originals."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEST = ROOT / 'docs/public/media/gameplay-20261107'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(args):
    subprocess.run(args, check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--originals', required=True, type=Path)
    args = parser.parse_args()
    manifest = DEST / 'scenes.json'
    data = json.loads(manifest.read_text())
    sources = {s['id']: s for s in data['sources']}
    for source in sources.values():
        path = args.originals / source['filename']
        assert sha(path) == source['sha256'], path.name
    for scene in data['scenes']:
        source = args.originals / sources[scene['source']]['filename']
        outputs = []
        for filename in scene['files']:
            path = DEST / filename
            if not path.exists():
                cmd = ['ffmpeg', '-v', 'error', '-n']
                if source.suffix.lower() == '.mov':
                    cmd += ['-ss', str(scene['poster'] if path.suffix == '.jpg' else scene['start'])]
                cmd += ['-i', str(source), '-map_metadata', '-1']
                if path.suffix == '.jpg':
                    cmd += ['-frames:v', '1', '-q:v', '2', str(path)]
                elif path.suffix == '.mp4':
                    cmd += ['-t', str(scene['end'] - scene['start']), '-map', '0:v:0', '-map', '0:a:0',
                            '-vf', 'scale=630:1368:flags=lanczos,setsar=1', '-r', '30',
                            '-c:v', 'libx264', '-crf', '21', '-preset', 'fast', '-threads', '4',
                            '-pix_fmt', 'yuv420p', '-c:a', 'aac', '-b:a', '128k', '-movflags', '+faststart', str(path)]
                else:
                    cmd += ['-t', str(scene['gif_end'] - scene['gif_start'])]
                    # GIFs begin at the same point as their MP4, with a shorter end.
                    assert scene['gif_start'] == scene['start']
                    cmd += ['-filter_complex',
                            'fps=12,scale=420:912:flags=lanczos,split[a][b];[a]palettegen=max_colors=128:stats_mode=diff[p];[b][p]paletteuse=dither=bayer:bayer_scale=3:diff_mode=rectangle',
                            '-an', '-loop', '0', str(path)]
                run(cmd)
            probe = json.loads(subprocess.check_output(['ffprobe', '-v', 'error', '-show_streams', '-show_format', '-of', 'json', str(path)]))
            v = next(s for s in probe['streams'] if s['codec_type'] == 'video')
            src = sources[scene['source']]
            assert abs(v['width'] / v['height'] - src['width'] / src['height']) < .001
            meta = dict(path=filename, sha256=sha(path), bytes=path.stat().st_size,
                        width=v['width'], height=v['height'], codec=v['codec_name'])
            if path.suffix != '.jpg':
                expected = scene['end'] - scene['start'] if path.suffix == '.mp4' else scene['gif_end'] - scene['gif_start']
                duration = float(probe['format']['duration'])
                assert abs(duration - expected) < .15, (filename, duration, expected)
                if path.suffix == '.mp4':
                    assert any(s['codec_type'] == 'audio' for s in probe['streams'])
                run(['ffmpeg', '-v', 'error', '-i', str(path), '-f', 'null', '-'])
                meta.update(duration=duration, full_decode='passed')
            outputs.append(meta)
        scene['output_metadata'] = outputs
        print(scene['id'], 'verified', flush=True)
    manifest.write_text(json.dumps(data, ensure_ascii=False, indent=2) + '\n')


if __name__ == '__main__':
    main()
