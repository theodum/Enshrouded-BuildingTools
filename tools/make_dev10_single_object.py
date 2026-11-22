"""One-object stage-A fixture from an actual native catalogue export.

No ID/dimension guesses, no game or save writes. Uses the production BTBP exporter.
"""
from pathlib import Path
import argparse
import hashlib
import json
import math
import sys

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
import native_item_catalog
import make_furnishing_project


def generate(report_path,out,helper):
    raw=report_path.read_bytes();report=json.loads(raw)
    merged=native_item_catalog.merge(json.loads((ROOT/'catalog/native-items.json').read_text(encoding='utf-8')),report)
    item=0xd109e828
    native=next((r for r in report['items'] if r['item']==item),None)
    if not native or native.get('resolution')!='policy_compatible' or native.get('compatible') is not True:
        raise ValueError('The crude wooden table is not resolved by this native report')
    lo,hi=native['bounds']['low'],native['bounds']['high']
    if any(not math.isfinite(v) for v in lo+hi) or any(hi[a]<=lo[a] for a in range(3)):raise ValueError('Invalid native placement dimensions')
    position=[-(lo[0]+hi[0])/2,-lo[1],-(lo[2]+hi[2])/2]
    recipe={'format':'BuildingToolsFurnishingRecipe','version':1,'anchor':[0,0,0],
            'objects':[{'item':item,'position':position,'rotation':[0,0,0,1]}]}
    manifest=make_furnishing_project.generate(recipe,merged,out,helper)
    project=json.loads(manifest.read_text());source=out/project['modules'][0]['file']
    source.rename(out/'Dev10_A_Table_Unique.btbp');manifest.unlink()
    project['name']='Dev10 - Etape A - une table native verifiee'
    project['modules'][0]['file']='Dev10_A_Table_Unique.btbp'
    (out/'Dev10_A_Table_Unique.btproject').write_bytes((json.dumps(project,ensure_ascii=False,indent=2)+'\n').encode())
    provenance=json.loads((out/'furnishing-provenance.json').read_text());provenance['manifest']='Dev10_A_Table_Unique.btproject'
    (out/'furnishing-provenance.json').write_bytes((json.dumps(provenance,indent=2)+'\n').encode())
    (out/'furnishing-provenance.json').rename(out/'Dev10_A_Provenance.json')
    evidence={'format':'BuildingToolsSingleObjectEvidence','version':1,'sourceReport':report_path.name,
              'sourceReportSHA256':hashlib.sha256(raw).hexdigest(),'indexSHA256':report['indexSHA256'],
              'gameImage':report.get('gameImage'),'nativeEntry':native,'nativePlacementTested':False,
              'note':'Policy, template and native placement AABB observed in game. No placement/persistence claim.'}
    (out/'single-object-evidence.json').write_bytes((json.dumps(evidence,ensure_ascii=False,indent=2)+'\n').encode())
    (out/'single-object.recipe.json').write_bytes((json.dumps(recipe,indent=2)+'\n').encode())
    return out/'Dev10_A_Table_Unique.btproject'


def generate_stage_d(report_path, out, helper):
    """Gated five-object follow-up; use only after A/B/C pass in game."""
    raw = report_path.read_bytes(); report = json.loads(raw)
    merged = native_item_catalog.merge(json.loads((ROOT/'catalog/native-items.json').read_text(encoding='utf-8')), report)
    rows = {r['item']: r for r in report['items']}
    # Desired bottom-centres on a PRE-EXISTING floor. Torch back touches the
    # near face of a PRE-EXISTING wall at z=+2; no structure is exported.
    layout = [(0x566c8dfe, [-3,0,0], False), (0xd109e828, [2,0,0], False),
              (0xdd5f8bb9, [2,0,-1.5], False), (0xdd5f8bb9, [2,0,1.5], True),
              (0x66f057f6, [0,2,2], False)]
    objects = []
    for item, target, reverse in layout:
        row = rows.get(item)
        if not row or row.get('resolution') != 'policy_compatible' or row.get('compatible') is not True:
            raise ValueError(f'Stage D unresolved native item: {item:#x}')
        lo, hi = row['bounds']['low'], row['bounds']['high']
        if any(not math.isfinite(v) for v in lo+hi) or any(hi[a] <= lo[a] for a in range(3)):
            raise ValueError(f'Stage D invalid native placement bounds: {item:#x}')
        if reverse:
            lo, hi = [-hi[0],lo[1],-hi[2]], [-lo[0],hi[1],-lo[2]]
        position = [target[0]-(lo[0]+hi[0])/2, target[1]-lo[1], target[2]-(lo[2]+hi[2])/2]
        if item == 0x66f057f6:
            position = [0, 2, 2-hi[2]]  # vertical centre 2 m; back face z=2
        objects.append({'item': item, 'position': position, 'rotation': [0,1,0,0] if reverse else [0,0,0,1]})
    recipe = {'format':'BuildingToolsFurnishingRecipe','version':1,'anchor':[0,0,0], 'objects': objects}
    manifest = make_furnishing_project.generate(recipe, merged, out, helper)
    project = json.loads(manifest.read_text()); source = out/project['modules'][0]['file']
    source.rename(out/'Dev10_D_Cinq_Meubles.btbp'); manifest.unlink()
    project['name'] = 'Dev10 - Etape D APRES A B C - cinq meubles'
    project['modules'][0]['file'] = 'Dev10_D_Cinq_Meubles.btbp'
    (out/'Dev10_D_Cinq_Meubles.btproject').write_bytes((json.dumps(project,indent=2)+'\n').encode())
    provenance = json.loads((out/'furnishing-provenance.json').read_text())
    provenance['manifest'] = 'Dev10_D_Cinq_Meubles.btproject'
    (out/'furnishing-provenance.json').unlink()
    evidence = {'sourceReport': report_path.name, 'sourceReportSHA256': hashlib.sha256(raw).hexdigest(),
                'gameImage': report.get('gameImage'), 'nativePlacementTested': False,
                'nativeEntries': [rows[i] for i in sorted({o['item'] for o in objects})]}
    for name, data in [('Dev10_D_Provenance.json', provenance), ('Dev10_D_Measured.json', evidence),
                       ('Dev10_D.recipe.json', recipe)]:
        (out/name).write_bytes((json.dumps(data, ensure_ascii=False, indent=2)+'\n').encode())
    return out/'Dev10_D_Cinq_Meubles.btproject'


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--observed',type=Path,required=True);p.add_argument('--outdir',type=Path,required=True)
    p.add_argument('--helper',type=Path,default=ROOT/'build/native_furniture_host')
    p.add_argument('--stage', choices=('A', 'D'), default='A', help='D is the follow-up after successful A/B/C game tests')
    a=p.parse_args()
    try:print((generate_stage_d if a.stage=='D' else generate)(a.observed,a.outdir,a.helper))
    except (ValueError,KeyError,OSError) as e:p.exit(1,f'Refused: {e}\n')


if __name__=='__main__':main()
