"""Real user blueprint bytes, file-only conversion; no live placement claim."""
from pathlib import Path
import hashlib
import importlib.util
import json
import struct
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]
EXAMPLE=ROOT/'examples/dev12'
spec=importlib.util.spec_from_file_location('classic_composite',ROOT/'tools/make_classic_composite.py')
converter=importlib.util.module_from_spec(spec);spec.loader.exec_module(converter)


class ClassicCompositeTests(unittest.TestCase):
    def source(self):return (EXAMPLE/'Dev12_Table_Classique_Source.btbp').read_bytes()

    def test_real_source_contains_five_objects_including_the_candle_and_floor(self):
        data=self.source();report=converter.inspect(data)
        self.assertEqual(hashlib.sha256(data).hexdigest(),'7147654d0e59e4a69a99b8cb2338a8f4ada128651a654561b0074d6a636e956b')
        self.assertEqual(len(data),224);self.assertEqual(report['sourceVoxelCells'],64)
        self.assertEqual([r['item'] for r in report['objects']],[0x58bbb7d6,0xaf3025e4,0x301dfbe1,0x301dfbe1,0xc672cf0d])
        self.assertEqual(report['objects'][-1]['position'],[.74169921875,.449951171875,2.930419921875])
        self.assertEqual(report['objects'][0]['recordOffset'],64)
        self.assertEqual(report['serializedProperties'],['inventoryID','position','quaternion'])

    def test_shipped_manifest_selects_objects_without_rewriting_source(self):
        m=json.loads((EXAMPLE/'Dev12_Table_Classique.btproject').read_text())
        self.assertEqual(m['anchor'],[0,.5,0]);self.assertEqual(m['pieces'],[])
        self.assertEqual(len(m['modules']),1);self.assertEqual(m['modules'][0]['content'],'objects')
        self.assertEqual(m['modules'][0]['objectPlacement'],'blueprint')
        report=json.loads((EXAMPLE/'Dev12_Table_Classique_Analysis.json').read_text())
        self.assertEqual(report['sourceSHA256'],hashlib.sha256(self.source()).hexdigest())
        self.assertEqual(report['buildVoxelCells'],0);self.assertFalse(report['compositePlacementValidatedInGame'])
        self.assertFalse(report['persistenceValidated'])

    def test_reusable_instances_preserve_source_bytes_and_refuse_overwrite(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);source=root/'input.btbp';source.write_bytes(self.source());out=root/'new'
            manifest=converter.generate(source,out,'TableSet',[0,.5,0],[[0,0,0,0],[10,0,-5,90]])
            m=json.loads(manifest.read_text());self.assertEqual(m['modules'][1]['offset'],[10,0,-5])
            self.assertEqual(m['modules'][1]['rotation'],90)
            self.assertEqual((out/'TableSet_Source.btbp').read_bytes(),self.source());self.assertEqual(source.read_bytes(),self.source())
            before={p.name:p.read_bytes() for p in out.iterdir()}
            with self.assertRaises(ValueError):converter.generate(source,out,'TableSet')
            self.assertEqual(before,{p.name:p.read_bytes() for p in out.iterdir()})

    def test_malformed_files_and_bad_transforms_publish_nothing(self):
        for change in ('cut','tail','zero_item','bad_quat','nan_position','bad_rotation','bad_grid'):
            data=bytearray(self.source())
            if change=='cut':data=data[:-1]
            if change=='tail':data+=b'bad'
            if change=='zero_item':struct.pack_into('<I',data,64,0)
            if change=='bad_quat':struct.pack_into('<4f',data,80,0,0,0,0)
            if change=='nan_position':struct.pack_into('<f',data,68,float('nan'))
            with self.subTest(change=change),tempfile.TemporaryDirectory() as tmp:
                root=Path(tmp);source=root/'input.btbp';source.write_bytes(data)
                instances=[[0,0,0,45]] if change=='bad_rotation' else [[0.25,0,0,0]] if change=='bad_grid' else None
                with self.assertRaises(ValueError):converter.generate(source,root/'output','Set',[0,.5,0],instances)
                self.assertFalse((root/'output').exists())


if __name__=='__main__':unittest.main()
