"""Regression tests for the in-game feedback: room widths and component clashes."""
import copy
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
import make_royal_castle_v2 as v2


class SpaciousCastleTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.profiles = v2.v1.host('--profiles-spacious')['profiles']
        cls.model = v2.SpaciousCastle(cls.profiles).build()
        cls.report = cls.model.validate()

    def altered(self):
        model = copy.copy(self.model)
        model.cells = self.model.cells.copy()
        model.rooms = copy.deepcopy(self.model.rooms)
        model.floor_planes = copy.deepcopy(self.model.floor_planes)
        model.lanes = self.model.lanes.copy()
        return model

    def test_larger_rooms_and_clear_stair_widths(self):
        report = self.report
        self.assertEqual(report['towerDiameterMeters'], 16)
        self.assertEqual(report['nominalMainFloorSurfacesMeters'], [0, 5, 10])
        self.assertEqual(report['stairClearWidthMeters'], 2.5)
        self.assertEqual(report['landingDepthMeters'], 3)
        self.assertEqual(report['stairHeadroomMeters'], 3.5)
        self.assertEqual(report['stairKinds'], {'monumental': 1, 'L': 2, 'U': 24})
        self.assertEqual(report['clippedStairSupportCells'], 0)
        self.assertGreater(report['occupiedGroundAreaM2'], 2*1384)
        self.assertEqual(report['surfaceGraphNodes'], report['entryComponentNodes'])
        self.assertFalse(report['gameplayTested'])
        self.assertFalse(report['smoothStairTraversalCertified'])

    def test_every_room_seed_belongs_to_its_room_and_has_a_wide_access(self):
        for room in self.model.rooms:
            x, y, z = room['seed']
            self.assertIn((x, z), room['footprint'], room['name'])
            self.assertIn('wideFloorComponent', room, room['name'])
            self.assertGreater(room['clearFloorAreaM2'], 0, room['name'])
        self.assertEqual(self.report['reachableRoomsAndDestinations'], len(self.model.rooms))

    def test_identical_material_does_not_allow_a_component_overlap(self):
        model = self.altered()
        p, value = next(iter(model.cells.items()))
        with self.assertRaisesRegex(ValueError, 'Chevauchement de composants'):
            model.put({p}, value[0], value[1], 'Second component with same material')

    def test_furniture_cannot_use_empty_space_inside_a_stair_cage(self):
        model = self.altered()
        cage = model.cages[0]
        p = next(p for p in v2.v1.cells_in(cage['low'], cage['high']) if p not in model.cells)
        with self.assertRaisesRegex(ValueError, 'cage réservée'):
            model.feature('Test cabinet', tuple(v/2 for v in p), tuple((v+1)/2 for v in p))

    def test_room_furniture_boxes_are_disjoint(self):
        features = self.model.features
        for index, a in enumerate(features):
            for b in features[index+1:]:
                overlap = all(a['low'][axis] < b['high'][axis] and a['high'][axis] > b['low'][axis] for axis in range(3))
                self.assertFalse(overlap, (a['name'], b['name']))

    def test_three_metre_room_aisle_must_stay_clear(self):
        model = self.altered()
        model.cells[-33, 0, -20] = ('masonry', 'furniture', 'Injected obstacle')
        with self.assertRaisesRegex(ValueError, 'Allée encombrée'):
            model.validate()

    def test_removed_tread_and_low_ceiling_are_refused(self):
        s = next(s for s in self.model.spaces if s['label'] == 'Tour NO U 5/3')
        p = s['low']
        model = self.altered()
        del model.cells[p[0], p[1]-1, p[2]]
        with self.assertRaisesRegex(ValueError, 'Palier sans soutien'):
            model.validate()
        model = self.altered()
        model.cells[p] = ('masonry', 'wall', 'Injected ceiling')
        with self.assertRaisesRegex(ValueError, 'Dégagement obstrué'):
            model.validate()

    def test_old_narrow_terrace_turn_fails_the_wide_route_check(self):
        # Reproduce the concrete 0.5 m turning strip found during V2 design.
        # The small surface graph can still reach it; the wide audit must refuse.
        model = self.altered()
        model.lanes = [lane for lane in model.lanes if not lane['name'].startswith('Passage devant donjon')]
        old_front = v2.r(-14, 15, 14, 18)
        for level in (20, 30):
            for x, z in old_front:
                model.cells.pop((x, level-1, z), None)
                if model.cells.get((x, level, z), (None, None, None))[2] == 'Parapets des galeries et terrasses':
                    model.cells.pop((x, level, z), None)
                    model.cells.pop((x, level+1, z), None)
            for x in range(-28, 28):
                for y in range(level, level+2):
                    p = x, y, 36
                    if p not in model.air and p not in model.cells:
                        model.cells[p] = ('trim', 'wall', 'Restored narrow terrace parapet')
        with self.assertRaisesRegex(ValueError, 'Accès de largeur 1,5 m insuffisant'):
            model.validate()

    def test_four_distinct_material_roles_have_no_hidden_overlap(self):
        palette = copy.deepcopy(self.model.palette)
        for index, role in enumerate(v2.v1.ROLES):
            palette[role]['pair'] = (100+index, 0x80+index)  # test-only host fixtures
        model = v2.SpaciousCastle(self.profiles, palette).build()
        self.assertEqual(model.validate()['cells'], self.report['cells'])

    def test_existing_directory_is_preserved(self):
        with tempfile.TemporaryDirectory(dir=ROOT/'build') as folder:
            marker = Path(folder)/'existing.txt'
            marker.write_bytes(b'preserve')
            with self.assertRaises(FileExistsError):
                v2.create(Path(folder))
            self.assertEqual(marker.read_bytes(), b'preserve')

    def test_exact_shipped_btbp_bytes_and_native_readback(self):
        source = ROOT/'examples/royal-castle-v2'
        manifest = json.loads((source/v2.PROJECT).read_text(encoding='utf-8'))
        self.assertLessEqual(len(manifest['modules']), 128)
        with tempfile.TemporaryDirectory(dir=ROOT/'build') as folder:
            exported = v2.export(self.model, Path(folder))
            self.assertEqual(exported, manifest)
            for module in exported['modules']:
                name = module['file']
                data = (Path(folder)/name).read_bytes()
                self.assertEqual(data, (source/name).read_bytes(), name)
                self.assertEqual(struct.unpack_from('<3f', data, 8), (.5, .5, .5))
                dims = struct.unpack_from('<3H', data, 20)
                self.assertLessEqual(dims[0]*dims[1]*dims[2], 65536)
        native = v2.v1.host('--validate', 'examples/royal-castle-v2/'+v2.PROJECT)
        self.assertEqual(native['result'], 'PASS')
        self.assertEqual(native['cells'], self.report['cells'])
        self.assertEqual(native['duplicates'], 0)


if __name__ == '__main__':
    unittest.main()
