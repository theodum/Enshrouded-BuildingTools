"""Generate a complete file-only demo from a real exported native catalogue.

No item substitution, game/save access or placement. Uses the production C++
layout, BTBP codec and project exporter via native_furniture_host.
"""
from pathlib import Path
import argparse
import hashlib
import json
import math
import subprocess

ROOT = Path(__file__).resolve().parents[1]
ROLES = ('bed', 'table', 'chair', 'wall_torch', 'painting', 'chest', 'small_decoration', 'floor_decoration')


def definitions(catalog):
    if (catalog.get('format'), catalog.get('version'), catalog.get('source'), catalog.get('complete')) != (
            'BuildingToolsNativeCatalog', 1, 'live_registry', True):
        raise ValueError('A complete live-registry catalogue exported by dev8 is required')
    material = catalog.get('structureMaterial')
    if not isinstance(material, dict) or type(material.get('item')) is not int or not 0 < material['item'] <= 0xffffffff:
        raise ValueError('Select a real construction material before exporting the catalogue')
    if type(material.get('byte')) is not int or not 1 <= material['byte'] <= 255:
        raise ValueError('Invalid native material byte')
    items = {}
    for row in catalog['items']:
        if type(row.get('item')) is not int or not 0 < row['item'] <= 0xffffffff or row['item'] in items:
            raise ValueError('Duplicate or invalid native catalogue ID')
        items[row['item']] = row
    rows = [f"{material['item']} {material['byte']}"]
    selected = {}
    for role in ROLES:
        item = catalog.get('demoSelection', {}).get(role)
        row = items.get(item) if type(item) is int else None
        if not row or row.get('compatible') is not True or not row.get('name') or not row.get('bounds'):
            raise ValueError(f'{role}: unavailable, refused or unidentified ID {item}; no replacement made')
        if row.get('flags', 0) & 0x80000 and catalog.get('logicCopyOptIn') is not True:
            raise ValueError(f'{role}: original blueprint logic copy not explicitly enabled')
        low, high = row['bounds']['low'], row['bounds']['high']
        if len(low) != 3 or len(high) != 3 or any(type(v) not in (int, float) or not math.isfinite(v) or abs(v) > 65536 for v in low + high):
            raise ValueError(f'{role}: invalid native bounds')
        if any(high[a] <= low[a] for a in range(3)):
            raise ValueError(f'{role}: empty native bounds')
        rows.append(' '.join(str(v) for v in [item] + low + high))
        selected[role] = {'item': item, 'name': row['name']}
    return '\n'.join(rows) + '\n', selected


def generate(catalog_path, output, helper):
    raw = catalog_path.read_bytes()
    data, selected = definitions(json.loads(raw))
    if output.exists():
        raise ValueError('Output directory already exists; no files overwritten')
    run = subprocess.run([str(helper.resolve()), str(output.resolve())], input=data, text=True, capture_output=True, check=True)
    manifest = run.stdout.strip()
    if Path(manifest).name != manifest or not (output / manifest).is_file():
        raise ValueError('No complete project manifest returned by exporter')
    provenance = {'format': 'BuildingToolsFurnitureDemoProvenance', 'version': 1,
                  'catalogSHA256': hashlib.sha256(raw).hexdigest(), 'catalogMod': json.loads(raw).get('mod'),
                  'objects': 11, 'voxelCells': 1692, 'selected': selected,
                  'inGameValidated': False, 'manifest': manifest}
    (output / 'native-furniture-provenance.json').write_text(json.dumps(provenance, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    return output / manifest


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--catalog', type=Path, required=True)
    p.add_argument('--outdir', type=Path, required=True)
    p.add_argument('--helper', type=Path, default=ROOT / 'build/native_furniture_host')
    args = p.parse_args()
    try:
        print(generate(args.catalog, args.outdir, args.helper))
    except (ValueError, KeyError, TypeError, OSError, subprocess.CalledProcessError) as e:
        p.exit(1, f'Refused: {e}\n')


if __name__ == '__main__':
    main()
