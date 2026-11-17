#!/usr/bin/env python3
"""Verify both local packages against sources and emit versioned SHA-256 sums.

No installation, game access, deletion, extraction or network activity.
"""
from pathlib import Path
import hashlib
import json
import re
import struct
import zipfile

ROOT=Path(__file__).resolve().parents[1]
DOCS={'EXTENDED-TESTING.md':'TESTING','BTPROJECT.md':'FORMAT','EXTENDED-STATUS.md':'STATUS',
      'DEV3-EXAMPLES.md':'EXAMPLES','DEV3-AUDIT.md':'AUDIT','DEV3-VALIDATION.md':'VALIDATION'}


def packaged_doc(source):
    data=(ROOT/'docs'/source).read_bytes()
    for filename,label in DOCS.items():
        data=data.replace((']('+filename+')').encode(),('](BuildingToolsExtended-'+label+'.md)').encode())
    return data


def check_pe(data):
    assert data[:2]==b'MZ', 'missing DOS header'
    pe=struct.unpack_from('<I',data,0x3c)[0]
    assert data[pe:pe+4]==b'PE\0\0'
    machine,sections,timestamp=struct.unpack_from('<HHI',data,pe+4)
    assert machine==0x8664 and timestamp==0
    assert struct.unpack_from('<H',data,pe+22)[0]&0x2000, 'not a DLL'
    opt=pe+24
    assert struct.unpack_from('<H',data,opt)[0]==0x20b, 'not Windows PE32+'
    optsize=struct.unpack_from('<H',data,pe+20)[0]
    def rva(value):
        for i in range(sections):
            vs,va,rs,raw=struct.unpack_from('<IIII',data,opt+optsize+i*40+8)
            if va<=value<va+min(vs,rs): return raw+value-va
        raise AssertionError(f'unmapped PE RVA {value:x}')
    def text_at(offset):
        return data[offset:data.index(b'\0',offset)].decode('ascii')
    imports=[]
    table=rva(struct.unpack_from('<I',data,opt+112+8)[0])
    while any(data[table:table+20]):
        imports.append(text_at(rva(struct.unpack_from('<I',data,table+12)[0])))
        table+=20
    assert imports and set(s.upper() for s in imports)<={'KERNEL32.DLL','USER32.DLL','GDI32.DLL'}, imports
    export=rva(struct.unpack_from('<I',data,opt+112)[0])
    count=struct.unpack_from('<I',data,export+24)[0]
    names=rva(struct.unpack_from('<I',data,export+32)[0])
    assert 'DirectInput8Create' in [text_at(rva(struct.unpack_from('<I',data,names+4*i)[0])) for i in range(count)]
    return imports


def verify():
    version=re.search(r'^#define MOD_VERSION "([0-9A-Za-z.-]+)"', (ROOT/'src/main.cpp').read_text(encoding='utf-8'),re.M).group(1)
    dll=(ROOT/'build/dinput8.dll').read_bytes()
    imports=check_pe(dll)
    fixtures=[p for folder in ('three-modules','dev3') for p in (ROOT/'examples'/folder).iterdir() if p.suffix in ('.btbp','.btproject')]
    assert len(fixtures)==15 and len({p.name for p in fixtures})==15
    languages=list((ROOT/'lang').glob('*.txt'));assert len(languages)==15
    common={'BuildingTools-LICENSE.txt':(ROOT/'LICENSE').read_bytes(),
            'BuildingTools-Extended.ini.example':(ROOT/'package/BuildingTools-Extended.ini.example').read_bytes(),
            'BuildingToolsExtended-CHANGELOG.md':(ROOT/'CHANGELOG.md').read_bytes()}
    common.update({'BuildingTools-Languages/'+p.name:p.read_bytes() for p in languages})
    common.update({'BuildingTools-Projects/'+p.name:p.read_bytes() for p in fixtures})
    common.update({'BuildingToolsExtended-'+label+'.md':packaged_doc(source) for source,label in DOCS.items()})
    checksums=[]
    for sf in (False,True):
        filename=f'EnshroudedBuildingTools-{"Shroudforge-" if sf else ""}{version}.zip'
        path=ROOT/'dist'/filename
        with zipfile.ZipFile(path) as archive:
            names=archive.namelist();assert len(names)==len(set(names)) and archive.testzip() is None
            expected=set(common)|{'BuildingTools-README.txt'}
            dllname='mods/building-tools/BuildingTools.dll' if sf else 'dinput8.dll'
            expected.add(dllname)
            if sf: expected|={'mods/building-tools/'+n for n in ('mod.json','extended.mod.json','native-plugin.ini')}
            assert set(names)==expected, (set(names)-expected,expected-set(names))
            assert all(not n.startswith('/') and '..' not in n.split('/') and '\\' not in n for n in names)
            for name,data in common.items(): assert archive.read(name)==data,name
            assert archive.read(dllname)==dll
            assert archive.read('BuildingTools-README.txt').decode('utf-8').splitlines()[0]==f'Enshrouded Building Tools {version}'
            for name in names:
                if name.endswith('.md'):
                    for target in re.findall(r'\]\(([^)]+)\)',archive.read(name).decode('utf-8')):
                        if not re.match(r'https?://',target): assert target in names,(name,target)
            if sf:
                assert json.loads(archive.read('mods/building-tools/mod.json'))['version']==version
                assert json.loads(archive.read('mods/building-tools/extended.mod.json'))['schemaVersion']==1
                assert archive.read('mods/building-tools/native-plugin.ini')==(ROOT/'package/shroudforge/native-plugin.ini').read_bytes()
        checksums.append(hashlib.sha256(path.read_bytes()).hexdigest()+'  '+filename)
        print(f'OK {filename}: {path.stat().st_size} bytes; {len(names)} entries, PE/DLL, CRC, sources, links and loader layout')
    (ROOT/f'dist/SHA256SUMS-{version}.txt').write_text('\n'.join(checksums)+'\n',encoding='ascii')
    print('DLL SHA256 '+hashlib.sha256(dll).hexdigest()+'; imports '+', '.join(imports))


if __name__=='__main__':
    verify()
