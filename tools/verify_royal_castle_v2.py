#!/usr/bin/env python3
"""Check V2 delivery closure, byte hashes and the actual production reader."""
import hashlib
import json
import zipfile
from pathlib import Path
import make_royal_castle_v2 as v2


def verify():
    source, archive = v2.FOLDER, v2.ARCHIVE
    manifest = json.loads((source/v2.PROJECT).read_text(encoding='utf-8'))
    files = list(source.iterdir())
    dependencies = {module['file'] for module in manifest['modules']}
    assert dependencies == {p.name for p in files if p.suffix == '.btbp'}
    assert len(dependencies) <= 128 and len(list(source.glob('*.btproject'))) == 1
    sums = {}
    for line in (source/'SHA256SUMS.txt').read_text(encoding='ascii').splitlines():
        digest, name = line.split('  ', 1)
        assert name not in sums
        sums[name] = digest
    assert set(sums) == {p.name for p in files if p.name != 'SHA256SUMS.txt'}
    for name, digest in sums.items():
        assert hashlib.sha256((source/name).read_bytes()).hexdigest() == digest, name
    with zipfile.ZipFile(archive) as z:
        names = z.namelist()
        assert len(names) == len(set(names)) and z.testzip() is None
        expected = {}
        for p in files:
            folder = 'BuildingTools-Projects' if p.suffix in ('.btbp', '.btproject') else 'Forteresse-Royale-V2-Guide'
            expected[folder+'/'+p.name] = p.read_bytes()
        assert set(names) == set(expected)
        for name, data in expected.items():
            assert z.read(name) == data, name
            assert not name.startswith('/') and '..' not in name.split('/') and '\\' not in name
        assert not any(name.endswith('.dll') for name in names)
    native = v2.v1.host('--validate', 'examples/royal-castle-v2/'+v2.PROJECT)
    report = json.loads((source/'Validation.json').read_text(encoding='utf-8'))
    assert native == report['productionReader']
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    (v2.ROOT/'dist/SHA256SUMS-Forteresse_Royale_Spacieuse_V2-dev7.txt').write_text(digest+'  '+archive.name+'\n', encoding='ascii')
    print(f'PASS {archive.name}: {native["cells"]} cells, {len(dependencies)} BTBP, CRC, SHA256, exact sources, dependencies and native reader')
    print('SHA256 '+digest)


if __name__ == '__main__':
    verify()
