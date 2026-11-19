"""Golden fixture reproducibility and non-overwrite guarantees of the generator."""
import importlib.util
from pathlib import Path
import tempfile
import struct
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('make_project_example', ROOT / 'tools/make_project_example.py')
generator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(generator)
import sys
sys.path.insert(0,str(ROOT/'tools'))
import make_dev3_examples as dev3


class ExampleTests(unittest.TestCase):
    def test_shipped_material_is_the_registered_block_seen_in_dev4_log(self):
        files = [p for folder in ('three-modules', 'dev3') for p in (ROOT/'examples'/folder).glob('*.btbp')]
        self.assertEqual(len(files), 10)
        for path in files:
            data = path.read_bytes()
            self.assertEqual(struct.unpack_from('<H', data, 30)[0], 1, path.name)
            self.assertEqual(struct.unpack_from('<I', data, 32)[0], 0x7ad0559c, path.name)
            self.assertEqual(data[36], 0x9d, path.name)

    def test_dev3_fixtures_are_reproducible_and_non_overwriting(self):
        material=generator.material_from(ROOT/'examples/three-modules/hall.btbp')
        with tempfile.TemporaryDirectory() as tmp:
            out=Path(tmp)/'dev3'
            dev3.create(out,material)
            self.assertEqual(len(list(out.iterdir())),11)
            for file in out.iterdir():
                self.assertEqual(file.read_bytes(),(ROOT/'examples/dev3'/file.name).read_bytes(),file.name)
            before={p.name:p.read_bytes() for p in out.iterdir()}
            with self.assertRaises(FileExistsError): dev3.create(out,material)
            self.assertEqual(before,{p.name:p.read_bytes() for p in out.iterdir()})

    def test_reproduces_all_shipped_modules_and_manifest(self):
        fixtures = ROOT / 'examples/three-modules'
        material = generator.material_from(fixtures / 'hall.btbp')
        with tempfile.TemporaryDirectory() as tmp:
            out = Path(tmp) / 'new-example'
            generator.create(out, material)
            for file in out.iterdir():
                self.assertEqual(file.read_bytes(), (fixtures / file.name).read_bytes(), file.name)
            before = {p.name: p.read_bytes() for p in out.iterdir()}
            with self.assertRaises(FileExistsError):
                generator.create(out, material)
            self.assertEqual(before, {p.name: p.read_bytes() for p in out.iterdir()})

    def test_source_is_read_only_and_invalid_input_refused(self):
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / 'source.btbp'
            source.write_bytes((ROOT / 'examples/three-modules/hall.btbp').read_bytes())
            original = source.read_bytes()
            generator.material_from(source)
            self.assertEqual(original, source.read_bytes())
            for data in [b'', b'BTBP', b'wrong magic' * 10, original[:31], original[:8] + bytes(50)]:
                source.write_bytes(data)
                with self.assertRaises(ValueError):
                    generator.material_from(source)


if __name__ == '__main__':
    unittest.main()
