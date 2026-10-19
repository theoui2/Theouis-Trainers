from pathlib import Path
import argparse
import hashlib
import json
import struct
import zipfile

ROOT = Path(__file__).resolve().parents[1]
PROJECT = json.loads((ROOT / 'release.json').read_text(encoding='utf-8'))

def main():
    parser = argparse.ArgumentParser(description="Create this project's SD installation ZIP, with corresponding source.")
    parser.add_argument('--binary', type=Path, default=ROOT / (PROJECT['target'] + '.ovl'))
    parser.add_argument('--output', type=Path, default=ROOT / 'dist' / PROJECT['archive'])
    args = parser.parse_args()
    binary = args.binary.read_bytes()
    if len(binary) < 0x80 or binary[0x10:0x14] != b'NRO0':
        parser.error('Expected a compiled NRO overlay.')
    size = struct.unpack_from('<I', binary, 0x18)[0]
    if size < 0x80 or size + 0x38 > len(binary) or binary[size:size+4] != b'ASET':
        parser.error('Missing or invalid overlay assets.')
    offset, length = struct.unpack_from('<QQ', binary, size+24)
    start = size + offset
    if offset < 0x38 or length < 0x3070 or start + length > len(binary):
        parser.error('Invalid NACP bounds.')
    version = binary[start+0x3060:start+0x3070].split(b'\0')[0].decode('utf-8')
    if version != PROJECT['version']:
        parser.error('Embedded version differs from the release configuration.')
    files = {'SD/' + PROJECT['install']: binary}
    if 'name' in PROJECT:
        files['SD/' + PROJECT['install'][:-4] + '.ini'] = ('[trainer]\nname=' + PROJECT['name'] + '\ntitle_id=' + PROJECT['title_id'] + '\n').encode()
    for name in ['README.md', 'INSTALLATION_FR.md', 'LICENSE', 'RELEASE_NOTES.md', 'VALIDATION.md']:
        p = ROOT / name
        if p.is_file():
            files[name] = p.read_bytes()
    roots = {'source', 'payload', 'libs', 'licenses', 'tests', 'tools'}
    extensions = {'.cpp', '.hpp', '.h', '.py', '.S', '.ld', '.md', '.json'}
    root_files = {'Makefile', 'LICENSE', '.gitignore', 'release.json'}
    for p in sorted(ROOT.rglob('*')):
        if not p.is_file():
            continue
        rel = p.relative_to(ROOT)
        if any(x in {'build', 'dist', '.git', '__pycache__'} for x in rel.parts):
            continue
        if (len(rel.parts) == 1 and (p.name in root_files or p.suffix == '.md')) or (rel.parts[0] in roots and (p.suffix in extensions or p.name in {'LICENSE', 'libnx-LICENSE'})):
            files['SOURCE/' + rel.as_posix()] = p.read_bytes()
    manifest = dict(PROJECT, binary_sha256=hashlib.sha256(binary).hexdigest(), binary_bytes=len(binary), console_tested_by_assistant=False)
    files['RELEASE_MANIFEST.json'] = (json.dumps(manifest, indent=2, ensure_ascii=False) + '\n').encode()
    files['CHECKSUMS.sha256'] = ''.join(hashlib.sha256(data).hexdigest() + '  ' + name + '\n' for name, data in sorted(files.items())).encode()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(args.output, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for name, data in sorted(files.items()):
            z.writestr(name, data)
    with zipfile.ZipFile(args.output) as z:
        assert z.testzip() is None
        for line in z.read('CHECKSUMS.sha256').decode().splitlines():
            digest, name = line.split('  ', 1)
            assert hashlib.sha256(z.read(name)).hexdigest() == digest, name
        overlays = [n for n in z.namelist() if n.startswith('SD/') and n.endswith('.ovl')]
        assert overlays == ['SD/' + PROJECT['install']]
        assert not any('.packages/' in n for n in z.namelist())
    print(str(args.output) + ': source, installation layout and checksums verified')

if __name__ == '__main__':
    main()
