"""Real captured table metadata, file-only exporter and strict refusal regression.

Nothing here places an entity or proves game persistence.
"""
from pathlib import Path
import copy
import importlib.util
import json
import struct
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('single_object', ROOT / 'tools/make_dev10_single_object.py')
generator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(generator)
EXAMPLE = ROOT / 'examples/dev10'


def fixture():
    evidence = json.loads((EXAMPLE / 'single-object-evidence.json').read_text(encoding='utf-8'))
    # Reconstruct a report subset from recorded metadata, not a new live observation.
    return {'format': 'BuildingToolsNativeCatalog', 'version': 1, 'source': 'live_registry',
            'complete': True, 'indexSHA256': evidence['indexSHA256'],
            'gameImage': evidence['gameImage'], 'items': [evidence['nativeEntry']]}


def read_objects(folder, name='Dev10_A_Table_Unique', count=1):
    manifest = json.loads((folder / (name+'.btproject')).read_text(encoding='utf-8'))
    assert manifest['pieces'] == [] and len(manifest['modules']) == 1
    bp = (folder / manifest['modules'][0]['file']).read_bytes()
    assert bp[:4] == b'BTBP' and struct.unpack_from('<H', bp, 30)[0] == 0
    runs = struct.unpack_from('<I', bp, 32)[0]
    for n in range(runs):
        assert bp[36 + 4*n + 2] == 255  # air only; no structure cells
    start = 36 + 4*runs
    assert struct.unpack_from('<I', bp, start)[0] == count and len(bp) == start + 4 + 32*count
    return [struct.unpack_from('<I7f', bp, start+4+32*n) for n in range(count)]


def read_object(folder):
    return read_objects(folder)[0]


class SingleObjectTests(unittest.TestCase):
    def test_shipped_one_object_has_measured_bottom_center_and_no_structure(self):
        row = read_object(EXAMPLE)
        self.assertEqual(row[0], 0xd109e828)
        for actual, expected in zip(row[1:], [-.003659, 0, .000846, 0, 0, 0, 1]):
            self.assertAlmostEqual(actual, expected, places=6)
        provenance = json.loads((EXAMPLE / 'Dev10_A_Provenance.json').read_text())
        self.assertEqual(provenance['voxelCells'], 0)
        self.assertFalse(provenance['inGameValidated'])

    def test_recorded_native_metadata_never_claims_placement(self):
        evidence = json.loads((EXAMPLE / 'single-object-evidence.json').read_text(encoding='utf-8'))
        self.assertFalse(evidence['nativePlacementTested'])
        entry = evidence['nativeEntry']
        self.assertEqual(entry['resolution'], 'policy_compatible')
        self.assertEqual(entry['placedEntity'], 'ddae6400a2e77345961e551beb051aa0')
        self.assertEqual(entry['placementCalls'], {'accepted': 0, 'failed': 0})
        self.assertFalse(entry['interactionValidated'])

    def test_actual_exporter_reproduces_binary_and_preserves_existing_files(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp); source = root / 'subset.json'; source.write_text(json.dumps(fixture()))
            out = root / 'output'
            generator.generate(source, out, ROOT / 'build/native_furniture_host')
            self.assertEqual(read_object(out), read_object(EXAMPLE))
            before = {p.name: p.read_bytes() for p in out.iterdir()}
            with self.assertRaises(ValueError):
                generator.generate(source, out, ROOT / 'build/native_furniture_host')
            self.assertEqual(before, {p.name: p.read_bytes() for p in out.iterdir()})

    def test_bad_resolution_and_bad_bounds_do_not_publish(self):
        for change in ('missing', 'special_flags', 'zero', 'nan', 'incomplete', 'wrong_index'):
            report = copy.deepcopy(fixture())
            if change == 'missing': report['items'] = []
            if change == 'special_flags':
                report['items'][0].update(resolution='special_flags', compatible=False, bounds=None)
            if change == 'zero': report['items'][0]['bounds']['high'][0] = report['items'][0]['bounds']['low'][0]
            if change == 'nan': report['items'][0]['bounds']['high'][0] = float('nan')
            if change == 'incomplete': report['complete'] = False
            if change == 'wrong_index': report['indexSHA256'] = 'stale'
            with self.subTest(change=change), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp); source = root / 'report.json'; source.write_text(json.dumps(report))
                with self.assertRaises(ValueError):
                    generator.generate(source, root / 'output', ROOT / 'build/native_furniture_host')
                self.assertFalse((root / 'output').exists())

    def test_failure_evidence_distinguishes_world_entity_and_target(self):
        evidence = json.loads((EXAMPLE / 'Dev10_Failure_Evidence.json').read_text(encoding='utf-8'))
        rows = {r['item']: r for r in evidence['objects']}
        self.assertEqual(evidence['observedFailures'], 4)
        self.assertEqual(rows[0xd1d489cb]['resolution'], 'invalid_bounds')
        self.assertIsNone(rows[0xd1d489cb]['bounds'])  # dev9 lost raw values; never invent them
        self.assertEqual(rows[0xd01eb1b8]['resolution'], 'policy_compatible')
        self.assertEqual(rows[0xaf3025e4]['resolution'], 'special_flags')
        self.assertEqual(rows[0xaf3025e4]['flags'], 0x80001)

    def test_stage_d_is_five_measured_props_without_the_refused_candle(self):
        rows = read_objects(EXAMPLE, 'Dev10_D_Cinq_Meubles', 5)
        ids = [r[0] for r in rows]
        self.assertEqual(ids, [0x566c8dfe, 0xd109e828, 0xdd5f8bb9, 0xdd5f8bb9, 0x66f057f6])
        self.assertAlmostEqual(rows[2][2], -.0171, places=6)
        self.assertEqual(rows[3][4:], (0,1,0,0))
        self.assertAlmostEqual(rows[4][3], 1.594609, places=6)
        evidence = json.loads((EXAMPLE/'Dev10_D_Measured.json').read_text(encoding='utf-8'))
        self.assertFalse(evidence['nativePlacementTested'])
        self.assertTrue(all(r['resolution']=='policy_compatible' for r in evidence['nativeEntries']))
        self.assertEqual({r['item']:r['flags'] for r in evidence['nativeEntries']},
                         {0x566c8dfe:0, 0x66f057f6:0x80000, 0xd109e828:0, 0xdd5f8bb9:0})

    def test_stage_d_export_is_reproducible_and_missing_wall_prop_is_refused(self):
        report = fixture()
        report['items'] = json.loads((EXAMPLE/'Dev10_D_Measured.json').read_text(encoding='utf-8'))['nativeEntries']
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp); source = root/'subset.json'; source.write_text(json.dumps(report))
            generator.generate_stage_d(source, root/'output', ROOT/'build/native_furniture_host')
            self.assertEqual(read_objects(root/'output', 'Dev10_D_Cinq_Meubles', 5),
                             read_objects(EXAMPLE, 'Dev10_D_Cinq_Meubles', 5))
            report['items'] = [r for r in report['items'] if r['item'] != 0x66f057f6]
            source.write_text(json.dumps(report))
            with self.assertRaises(ValueError):
                generator.generate_stage_d(source, root/'refused', ROOT/'build/native_furniture_host')
            self.assertFalse((root/'refused').exists())


if __name__ == '__main__':
    unittest.main()
