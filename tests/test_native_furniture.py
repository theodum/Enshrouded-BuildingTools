"""Temporary synthetic registry for file-only integration; no real-ID claims."""
from pathlib import Path
import copy
import importlib.util
import json
import struct
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('native_furniture', ROOT / 'tools/make_native_furniture_project.py')
generator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(generator)


def catalog():
    dims = [(2, 1, 3), (2, 1, 1), (.6, 1, .6), (.3, 1, .3), (2, 1.2, .1), (1, .8, 1), (.2, .3, .2), (.5, 1, .5)]
    return {'format': 'BuildingToolsNativeCatalog', 'version': 1, 'source': 'live_registry', 'complete': True,
            'mod': 'SYNTHETIC TEST ONLY', 'structureMaterial': {'item': 0xf1000000, 'byte': 0x9d},
            'items': [{'item': 0xf0000000 + i, 'name': 'SYNTHETIC ' + role, 'compatible': True, 'flags': 0,
                       'bounds': {'low': [-w / 2, 0, -d / 2], 'high': [w / 2, h, d / 2]}}
                      for i, (role, (w, h, d)) in enumerate(zip(generator.ROLES, dims))],
            'demoSelection': {role: 0xf0000000 + i for i, role in enumerate(generator.ROLES)}}


class NativeFurnitureTests(unittest.TestCase):
    def test_rejects_incomplete_unknown_or_refused_items(self):
        for change in ('complete', 'unknown', 'bounds', 'refused', 'duplicate', 'logic', 'material'):
            c = catalog()
            if change == 'complete': c['complete'] = False
            if change == 'unknown': c['demoSelection']['bed'] = 0xdeadbeef
            if change == 'bounds': c['items'][0]['bounds']['low'][0] = float('nan')
            if change == 'refused': c['items'][0]['compatible'] = False
            if change == 'duplicate': c['items'].append(copy.deepcopy(c['items'][0]))
            if change == 'logic': c['items'][0]['flags'] = 0x80000
            if change == 'material': c['structureMaterial'] = None
            with self.subTest(change=change), self.assertRaises(ValueError): generator.definitions(c)

    def test_complete_export_contains_11_exact_native_references(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp); source = root / 'catalog.json'; source.write_text(json.dumps(catalog()))
            before = source.read_bytes(); out = root / 'room'
            manifest = generator.generate(source, out, ROOT / 'build/native_furniture_host')
            project = json.loads(manifest.read_text()); objects = []; voxel_cells = 0
            for module in project['modules']:
                bp = (out / module['file']).read_bytes(); self.assertEqual(bp[:4], b'BTBP')
                material_count = struct.unpack_from('<H', bp, 30)[0]; offset = 32 + material_count * 16
                runs = struct.unpack_from('<I', bp, offset)[0]; offset += 4
                for r in range(runs):
                    n, material = struct.unpack_from('<HB', bp, offset + 4 * r)
                    if material != 255: voxel_cells += n
                offset += runs * 4; count = struct.unpack_from('<I', bp, offset)[0]; offset += 4
                objects.extend(struct.unpack_from('<I7f', bp, offset + i * 32) for i in range(count))
                self.assertEqual(len(bp), offset + count * 32)
                if count: self.assertEqual(material_count, 0)
            self.assertEqual(len(objects), 11); self.assertEqual(voxel_cells, 1692)
            ids = [row[0] for row in objects]
            self.assertEqual(ids.count(0xf0000002), 2); self.assertEqual(ids.count(0xf0000003), 2)
            self.assertEqual(set(ids), {r['item'] for r in catalog()['items']})
            self.assertEqual(source.read_bytes(), before)
            provenance = json.loads((out / 'native-furniture-provenance.json').read_text())
            self.assertIs(provenance['inGameValidated'], False)
            files = {p.name: p.read_bytes() for p in out.iterdir()}
            with self.assertRaises(ValueError): generator.generate(source, out, ROOT / 'build/native_furniture_host')
            self.assertEqual(files, {p.name: p.read_bytes() for p in out.iterdir()})

    def test_unfitting_selection_publishes_no_project(self):
        import subprocess
        c = catalog(); c['items'][0]['bounds']['high'][0] = 50
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp); source = root / 'catalog.json'; source.write_text(json.dumps(c))
            with self.assertRaises(subprocess.CalledProcessError): generator.generate(source, root / 'room', ROOT / 'build/native_furniture_host')
            self.assertFalse((root / 'room').exists())


if __name__ == '__main__': unittest.main()
