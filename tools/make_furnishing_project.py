"""Export precise native objects using the existing C++ BTBP/project exporter.

Requires native policy evidence unless --unverified-draft is explicitly requested.
No game calls or save access. Never writes structure cells or native pieces.
"""
from pathlib import Path
import argparse
import hashlib
import json
import math
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def vector(value, count):
    if not isinstance(value, list) or len(value) != count or any(type(x) not in (int, float) or not math.isfinite(x) or abs(x) > 65536 for x in value):
        raise ValueError('Invalid finite vector')
    return value


def recipe_objects(recipe, catalog, draft=False):
    if recipe.get('format') != 'BuildingToolsFurnishingRecipe' or recipe.get('version') != 1: raise ValueError('Unknown recipe format')
    items = {r['item']: r for r in catalog['items']}; result = []
    for entry in recipe['objects']:
        if set(entry) - {'item', 'position', 'rotation', 'yawDegrees', 'role'}: raise ValueError('Unsupported object property')
        item = entry['item']; row = items.get(item) if type(item) is int else None
        if not row: raise ValueError(f'ID {item} is absent from the index; no substitution')
        state = row.get('compatibility')
        if state != 'policy_compatible' and not (draft and state in ('source_only_unverified', 'registry_unobserved')):
            raise ValueError(f'{item} {row["name"]}: {state}; resolve the current native registry first')
        position = vector(entry['position'], 3)
        if 'rotation' in entry and 'yawDegrees' in entry: raise ValueError('Specify quaternion or yaw, not both')
        if 'rotation' in entry: q = vector(entry['rotation'], 4)
        else:
            yaw = entry.get('yawDegrees', 0)
            if type(yaw) not in (int, float) or not math.isfinite(yaw): raise ValueError('Invalid yaw')
            angle = math.radians(yaw % 360) / 2; q = [0, math.sin(angle), 0, math.cos(angle)]
        if any(abs(x) > 1.001 for x in q) or not .99 <= sum(x*x for x in q) <= 1.01: raise ValueError('Invalid quaternion; no implicit correction')
        result.append({'item': item, 'position': position, 'rotation': q})
    if not 1 <= len(result) <= 512: raise ValueError('File-only authoring budget is 1..512 objects per export')
    return result


def reference_data(path):
    data = json.loads(path.read_text(encoding='utf-8-sig'))
    if data.get('format') != 'BuildingToolsExtendedProject' or data.get('version') != 1: raise ValueError('Invalid reference project')
    hashes = {path.name: hashlib.sha256(path.read_bytes()).hexdigest()}
    for module in data['modules']:
        name = module['file']
        if Path(name).name != name or '/' in name or '\\' in name or ':' in name: raise ValueError('Unsafe module path')
        hashes[name] = hashlib.sha256((path.parent/name).read_bytes()).hexdigest()
    return vector(data.get('anchor', [0, 0, 0]), 3), hashes


def generate(recipe, catalog, out, helper, draft=False, reference=None, world_origin=None, world_turn=None):
    objects = recipe_objects(recipe, catalog, draft)
    if out.exists(): raise ValueError('Output exists; no overwrite')
    anchor, hashes = reference_data(reference) if reference else (vector(recipe.get('anchor', [0, 0, 0]), 3), {})
    if any(abs(x*2-round(x*2)) > 1e-7 for x in anchor): raise ValueError('Reference anchor must use the 0.5 m grid')
    if reference and (world_origin is None or world_turn is None): raise ValueError('Exact world origin and turn of the existing castle are required; anchor alone is insufficient')
    if world_origin is not None: vector(world_origin, 3)
    if world_turn is not None and world_turn not in (0, 90, 180, 270): raise ValueError('World rotation must be a quarter turn')
    data = [str(len(objects)), ' '.join(str(round(x*16)) for x in anchor)]
    data += [' '.join(str(x) for x in [o['item']]+o['position']+o['rotation']) for o in objects]
    args = [str(helper.resolve()), '--objects', str(out.resolve())]
    # The CLI also runs directly under Windows with the existing WSL compiler.
    if sys.platform == 'win32' and helper.suffix.lower() != '.exe':
        rel = helper.resolve().relative_to(ROOT)
        linux_out = subprocess.check_output(['wsl','-d','Ubuntu-24.04','--','wslpath','-a',out.resolve().as_posix()],text=True).strip()
        args = ['wsl','-d','Ubuntu-24.04','--cd',str(ROOT),'--','./'+rel.as_posix(),'--objects',linux_out]
    run = subprocess.run(args, input='\n'.join(data)+'\n', text=True, capture_output=True, check=True)
    filename = run.stdout.strip()
    if Path(filename).name != filename or not (out/filename).is_file(): raise ValueError('Exporter did not publish a manifest')
    if reference and reference_data(reference)[1] != hashes: raise ValueError('Reference structure changed concurrently')
    report = {'format':'BuildingToolsFurnishingProvenance','version':1,'manifest':filename,'objects':len(objects),
              'voxelCells':0,'nativePieces':0,'inGameValidated':False,'unverifiedDraft':draft,
              'indexSHA256':catalog.get('sourceSHA256'),'referenceHashes':hashes,
              'anchor':anchor,'requiredWorldOrigin':world_origin,'requiredWorldRotation':world_turn}
    (out/'furnishing-provenance.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    return out/filename


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--recipe',type=Path,required=True);p.add_argument('--catalog',type=Path,default=ROOT/'catalog/native-items.json')
    p.add_argument('--outdir',type=Path,required=True);p.add_argument('--helper',type=Path,default=ROOT/'build/native_furniture_host')
    p.add_argument('--unverified-draft',action='store_true');p.add_argument('--reference',type=Path)
    p.add_argument('--world-origin',type=float,nargs=3);p.add_argument('--world-turn',type=int)
    a=p.parse_args()
    try: print(generate(json.loads(a.recipe.read_text(encoding='utf-8')),json.loads(a.catalog.read_text(encoding='utf-8')),a.outdir,a.helper,a.unverified_draft,a.reference,a.world_origin,a.world_turn))
    except (ValueError,KeyError,TypeError,OSError,subprocess.CalledProcessError) as e: p.exit(1,f'Refused: {e}\n')


if __name__=='__main__': main()
