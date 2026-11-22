"""Real attributed CSV integrity + simulated native evidence, kept separate."""
from pathlib import Path
import copy
import importlib.util
import json
import struct
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]
def module(name):
    spec=importlib.util.spec_from_file_location(name,ROOT/'tools'/f'{name}.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
c=module('native_item_catalog');f=module('make_furnishing_project')


class CatalogTests(unittest.TestCase):
    def setUp(self): self.index=c.parse((ROOT/'catalog/item-ids.source.csv').read_bytes())
    def test_real_snapshot_reproduces_index_and_header(self):
        self.assertEqual(len(self.index['items']),3526)
        self.assertEqual(sum(r['sourceCategory']=='BuildTools' for r in self.index['items']),1303)
        self.assertTrue(all(r['compatibility']=='source_only_unverified' for r in self.index['items']))
        self.assertEqual(json.loads((ROOT/'catalog/native-items.json').read_text(encoding='utf-8')),self.index)
        self.assertEqual((ROOT/'src/native_item_index.h').read_text(encoding='utf-8'),c.header(self.index))
    def test_bad_csv_is_rejected(self):
        raw=(ROOT/'catalog/item-ids.source.csv').read_bytes()
        for data in (b'bad schema',raw.replace(b'3201054228',b'42',1),raw+raw.decode('utf-8-sig').splitlines()[1].encode()+b'\n'):
            with self.assertRaises((ValueError,TypeError)): c.parse(data)
    def evidence(self):
        return {'format':'BuildingToolsNativeCatalog','source':'live_registry','complete':True,
                'indexSHA256':self.index['sourceSHA256'],'gameImage':{'synthetic':True},'items':[
                    {'item':0x566c8dfe,'name':'SYNTHETIC lookup for real CSV ID','resolution':'policy_compatible','compatible':True,
                     'bounds':{'low':[-1,0,-1],'high':[1,1,1]},'placedEntity':'a'*32,
                     'placementCalls':{'accepted':0,'failed':0}}]}
    def test_evidence_is_not_interaction_validation_and_stale_rows_reset(self):
        merged=c.merge(self.index,self.evidence());bed=next(r for r in merged['items'] if r['item']==0x566c8dfe)
        self.assertEqual(bed['compatibility'],'policy_compatible');self.assertFalse(bed['interactionValidated']);self.assertEqual(bed['nativeCallAccepted'],0)
        e=self.evidence();e['items']=[];again=c.merge(merged,e);bed=next(r for r in again['items'] if r['item']==0x566c8dfe)
        self.assertEqual(bed['compatibility'],'source_only_unverified');self.assertIsNone(bed['bounds'])
    def test_incomplete_wrong_index_or_metadata_is_refused(self):
        for change in ('complete','hash','guid','bounds','contradiction'):
            e=self.evidence()
            if change=='complete':e['complete']=False
            if change=='hash':e['indexSHA256']='old'
            if change=='guid':e['items'][0]['placedEntity']='12345678'
            if change=='bounds':e['items'][0]['bounds']['high'][0]=float('nan')
            if change=='contradiction':e['items'][0]['compatible']=False
            with self.subTest(change=change),self.assertRaises(ValueError):c.merge(copy.deepcopy(self.index),e)
    def test_demo_has_exact_csv_ids_and_preserves_precision(self):
        recipe=json.loads((ROOT/'examples/dev9/furnishing-demo.recipe.json').read_text())
        with self.assertRaises(ValueError):f.recipe_objects(recipe,self.index)
        objects=f.recipe_objects(recipe,self.index,True);self.assertEqual(len(objects),11)
        names={r['item']:r['name'] for r in self.index['items']}
        self.assertEqual(names[objects[0]['item']],'Crude Wooden Bed');self.assertEqual(names[objects[4]['item']],'Wall Torch')
        precise=copy.deepcopy(recipe);precise['objects'][0]['position']=[2.125,0.0625,3.375];precise['objects'][0].pop('yawDegrees');precise['objects'][0]['rotation']=[.70710678,0,0,.70710678]
        self.assertEqual(f.recipe_objects(precise,self.index,True)[0]['position'],[2.125,.0625,3.375])
        precise['objects'][0]['rotation']=[0,0,0,0]
        with self.assertRaises(ValueError):f.recipe_objects(precise,self.index,True)
        recipe['objects'][0]['item']=0xdeadbeef
        with self.assertRaises(ValueError):f.recipe_objects(recipe,self.index,True)
    def test_actual_exporter_writes_zero_structure_and_no_overwrite(self):
        recipe=json.loads((ROOT/'examples/dev9/furnishing-demo.recipe.json').read_text())
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);out=root/'new';helper=ROOT/'build/native_furniture_host'
            manifest=f.generate(recipe,self.index,out,helper,True);m=json.loads(manifest.read_text());self.assertEqual(m['pieces'],[]);self.assertNotIn('spaces',m)
            total=0
            for source in m['modules']:
                bp=(out/source['file']).read_bytes();self.assertEqual(struct.unpack_from('<H',bp,30)[0],0)
                runs=struct.unpack_from('<I',bp,32)[0];self.assertEqual(bp[38],255);total+=struct.unpack_from('<I',bp,36+runs*4)[0]
            self.assertEqual(total,11);self.assertEqual(json.loads((out/'furnishing-provenance.json').read_text())['voxelCells'],0)
            old=manifest.read_bytes()
            with self.assertRaises(ValueError):f.generate(recipe,self.index,out,helper,True)
            self.assertEqual(old,manifest.read_bytes())
    def test_reference_requires_world_transform_and_keeps_structure_bytes(self):
        recipe=json.loads((ROOT/'examples/dev9/furnishing-demo.recipe.json').read_text())
        ref=ROOT/'examples/royal-castle-v2/Forteresse_Royale_Spacieuse_V2.btproject';before=f.reference_data(ref)
        with tempfile.TemporaryDirectory() as tmp:
            out=Path(tmp)/'aligned'
            with self.assertRaises(ValueError):f.generate(recipe,self.index,out,ROOT/'build/native_furniture_host',True,ref)
            manifest=f.generate(recipe,self.index,out,ROOT/'build/native_furniture_host',True,ref,[100,50,200],90)
            self.assertEqual(json.loads(manifest.read_text())['anchor'],before[0]);self.assertEqual(f.reference_data(ref),before)
            p=json.loads((out/'furnishing-provenance.json').read_text());self.assertEqual(p['requiredWorldRotation'],90)
            self.assertEqual(len(p['referenceHashes']),31)
    def test_shipped_draft_exactly_matches_recipe(self):
        root=ROOT/'examples/dev9';m=json.loads((root/'Dev9_Demo_Mobilier_A_VALIDER.btproject').read_text())
        self.assertEqual(m['pieces'],[]);self.assertEqual(len(m['modules']),1)
        bp=(root/m['modules'][0]['file']).read_bytes();self.assertEqual(struct.unpack_from('<H',bp,30)[0],0)
        runs=struct.unpack_from('<I',bp,32)[0];start=36+runs*4;count=struct.unpack_from('<I',bp,start)[0]
        recipe=json.loads((root/'furnishing-demo.recipe.json').read_text());expected=f.recipe_objects(recipe,self.index,True)
        self.assertEqual(count,11);self.assertEqual(len(bp),start+4+32*count)
        for i,o in enumerate(expected):
            row=struct.unpack_from('<I7f',bp,start+4+32*i);self.assertEqual(row[0],o['item'])
            for got,want in zip(row[1:],o['position']+o['rotation']):self.assertAlmostEqual(got,want,places=6)
        p=json.loads((root/'furnishing-provenance.json').read_text());self.assertTrue(p['unverifiedDraft']);self.assertFalse(p['inGameValidated'])
    def test_native_only_discovery_is_added_without_fabricating_csv_membership(self):
        e=self.evidence();extra=copy.deepcopy(e['items'][0]);extra['item']=0xf0001234;extra['name']='SYNTHETIC live-only item';e['items'].append(extra)
        merged=c.merge(self.index,e);r=next(r for r in merged['items'] if r['item']==extra['item'])
        self.assertEqual(r['sourceCategory'],'native_registry');self.assertEqual(r['compatibility'],'policy_compatible');self.assertFalse(r['interactionValidated'])


if __name__=='__main__': unittest.main()
