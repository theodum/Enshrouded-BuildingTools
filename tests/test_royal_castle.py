"""Castle-specific acceptance checks against final geometry and shipped bytes."""
from pathlib import Path
import copy
import json
import struct
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
import make_royal_castle as castle


class RoyalCastleTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.profiles = castle.host('--profiles')['profiles']
        cls.model = castle.Castle(cls.profiles, castle.palette_from()).build()
        cls.validation = cls.model.validate()

    def altered(self):
        model = copy.copy(self.model)
        model.cells = self.model.cells.copy()
        model.rooms = copy.deepcopy(self.model.rooms)
        return model

    def test_connected_compact_castle_and_all_requested_stair_types(self):
        report = self.validation
        self.assertEqual(report['result'], 'PASS')
        self.assertEqual(report['stairs'], 25)
        self.assertEqual(report['stairKinds'], {'monumental': 1, 'L': 2, 'U': 22})
        self.assertEqual(report['roomsAndDestinations'], report['reachableRoomsAndDestinations'])
        self.assertEqual(report['isolatedFloorSurfaces'], 0)
        x, y, z = report['dimensionsMeters']
        self.assertLess(max(x, z)/min(x, z), 1.15)
        self.assertGreaterEqual(y, 23)
        self.assertFalse(report['smoothStairTraversalCertified'])
        self.assertFalse(report['gameplayTested'])

    def test_obstructed_door_and_stair_ceiling_are_refused(self):
        for label in ('Porte monumentale', 'Escalier impérial du hall/3'):
            model = self.altered()
            reservation = next(s for s in model.spaces if s['label'] == label)
            model.cells[reservation['low']] = ('masonry', 'wall', 'Injected obstruction')
            with self.assertRaisesRegex(ValueError, 'Obstructed opening'):
                model.validate()

    def test_removed_step_is_refused(self):
        model = self.altered()
        s = next(s for s in model.spaces if s['label'] == 'Tour NO U 2/3')
        x, y, z = s['low']
        del model.cells[x, y-1, z]
        with self.assertRaisesRegex(ValueError, 'Missing passage surface'):
            model.validate()

    def test_room_names_do_not_override_voxel_connectivity(self):
        model = self.altered()
        seed = next(r['seed'] for r in model.rooms if r['name'] == 'Bibliothèque')
        model.cells[seed] = ('masonry', 'wall', 'Injected room obstruction')
        with self.assertRaisesRegex(ValueError, 'Unreachable destinations'):
            model.validate()

    def test_different_material_overlap_is_refused(self):
        model = self.altered()
        model.palette = copy.deepcopy(model.palette)
        # Test-only numeric IDs are never exported as a real palette.
        model.palette['trim']['pair'] = (101, 0x81)
        p = next(p for p, value in model.cells.items() if value[0] == 'masonry')
        with self.assertRaisesRegex(ValueError, 'Contradictory overlap'):
            model.put(p, 'trim')

    def test_four_distinct_roles_can_be_authored_without_overlaps(self):
        palette = copy.deepcopy(self.model.palette)
        for index, role in enumerate(castle.ROLES):
            palette[role]['pair'] = (100+index, 0x80+index)  # host fixtures only
        model = castle.Castle(self.profiles, palette).build()
        self.assertEqual(model.validate()['cells'], self.validation['cells'])

    def test_existing_output_is_never_overwritten(self):
        with tempfile.TemporaryDirectory(dir=ROOT/'build') as temp:
            output = Path(temp)
            marker = output/'marker.txt'
            marker.write_bytes(b'preserve')
            with self.assertRaises(FileExistsError):
                castle.create(output)
            self.assertEqual(marker.read_bytes(), b'preserve')

    def test_shipped_geometry_is_reproducible_and_native_readable(self):
        fixture = ROOT/'examples/royal-castle'
        manifest = json.loads((fixture/castle.PROJECT).read_text(encoding='utf-8'))
        validation = json.loads((fixture/'Validation.json').read_text(encoding='utf-8'))
        self.assertEqual(validation['cells'], self.validation['cells'])
        self.assertEqual(validation['stairs'], self.validation['stairs'])
        with tempfile.TemporaryDirectory(dir=ROOT/'build') as temp:
            generated = castle.export(self.model, Path(temp))
            self.assertEqual(generated, manifest)
            for module in manifest['modules']:
                file = module['file']
                data = (Path(temp)/file).read_bytes()
                self.assertEqual(data, (fixture/file).read_bytes(), file)
                self.assertLessEqual(struct.unpack_from('<3H', data, 20)[0] *
                                     struct.unpack_from('<3H', data, 20)[1] *
                                     struct.unpack_from('<3H', data, 20)[2], 65536)
        result = castle.host('--validate', 'examples/royal-castle/'+castle.PROJECT)
        self.assertEqual(result['cells'], self.validation['cells'])
        self.assertEqual(result['result'], 'PASS')
        self.assertEqual(result['interruptionStateMachine'], 'PASS')


if __name__ == '__main__':
    unittest.main()
