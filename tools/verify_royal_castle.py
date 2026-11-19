#!/usr/bin/env python3
"""Verify the castle import archive, dependency closure, hashes and host read-back."""
import hashlib
import json
from pathlib import Path
import sys
import zipfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
import make_royal_castle as castle


def verify():
    source = ROOT/'examples/royal-castle'
    archive_path = ROOT/'dist/Forteresse_Royale_Demo-dev7.zip'
    manifest = json.loads((source/castle.PROJECT).read_text(encoding='utf-8'))
    files = list(source.iterdir())
    dependencies = {module['file'] for module in manifest['modules']}
    assert dependencies == {p.name for p in files if p.suffix == '.btbp'}
    assert len(dependencies) <= 128
    assert len(list(source.glob('*.btproject'))) == 1
    sums = {}
    for line in (source/'SHA256SUMS.txt').read_text(encoding='ascii').splitlines():
        sha, name = line.split('  ', 1)
        assert name not in sums
        sums[name] = sha
    assert set(sums) == {p.name for p in files if p.name != 'SHA256SUMS.txt'}
    for name, sha in sums.items():
        assert hashlib.sha256((source/name).read_bytes()).hexdigest() == sha, name
    with zipfile.ZipFile(archive_path) as archive:
        names = archive.namelist()
        assert len(names) == len(set(names))
        assert archive.testzip() is None
        expected = {}
        for p in files:
            destination = 'BuildingTools-Projects/'+p.name if p.suffix in ('.btbp', '.btproject') else 'Forteresse-Royale-Guide/'+p.name
            expected[destination] = p.read_bytes()
        assert set(names) == set(expected)
        for name in names:
            assert '\\' not in name and '..' not in name.split('/') and not name.startswith('/')
            assert archive.read(name) == expected[name], name
        assert not any(name.endswith('.dll') for name in names)
    report = castle.host('--validate', 'examples/royal-castle/'+castle.PROJECT)
    assert report['result'] == 'PASS'
    assert report['cells'] == json.loads((source/'Validation.json').read_text(encoding='utf-8'))['cells']
    digest = hashlib.sha256(archive_path.read_bytes()).hexdigest()
    (ROOT/'dist/SHA256SUMS-Forteresse_Royale_Demo-dev7.txt').write_text(
        digest+'  '+archive_path.name+'\n', encoding='ascii')
    print(f'PASS {archive_path.name}: {len(dependencies)} BTBP, {report["cells"]} cells, all dependencies/hashes/ZIP bytes/native read-back verified')
    print('SHA256 '+digest)


if __name__ == '__main__':
    verify()
