"""Import/search the public ID CSV and merge separately observed native evidence.

CSV membership never authorizes placement. All imported records start unverified.
The DLL resolves every ID against its current registry before loading a project.
"""
from pathlib import Path
import argparse
import collections
import csv
import hashlib
import io
import json
import math
import re
import urllib.request

ROOT = Path(__file__).resolve().parents[1]
URL = 'https://enshroudedsaveeditor.com/items/item-ids.csv'
CATEGORIES = ('other', 'beds', 'tables', 'seating', 'wall_lights', 'lights', 'doors_windows', 'statues', 'storage', 'wall_decor', 'decorations', 'workshops', 'plants', 'special_review')


def category(name, source):
    n = name.casefold()
    if source != 'BuildTools': return 'other'
    # Hints for browsing only, never a compatibility decision.
    if re.search(r'\b(altar|npc|summon|seedling|farmbed)\b', n): return 'special_review'
    if re.search(r'\b(door|gate|window)\b', n): return 'doors_windows'
    if re.search(r'\b(statue|sculpture)\b', n): return 'statues'
    if re.search(r'\b(bed|mattress)\b', n) and 'bedside' not in n: return 'beds'
    if re.search(r'\b(table|counter|desk)\b', n) and 'saw' not in n: return 'tables'
    if re.search(r'\b(chair|armchair|stool|bench|throne|sofa)\b', n): return 'seating'
    if 'wall' in n and re.search(r'torch|candle|lamp|light|lantern|brazier', n): return 'wall_lights'
    if re.search(r'torch|candle|lamp|light|lantern|chandelier|brazier', n): return 'lights'
    if re.search(r'\b(chest|cabinet|shelf|shelves|cupboard|sideboard|closet|container)\b', n): return 'storage'
    if name.startswith('"') or re.search(r'\b(painting|tapestry|banner|emblem)\b', n): return 'wall_decor'
    if re.search(r'\b(forge|kiln|station|workbench|oven|smelter|spinning|sawmill|loom)\b', n): return 'workshops'
    if re.search(r'\b(plant|flower|rose|bush|tree)\b', n) and 'vase' not in n: return 'plants'
    return 'decorations'


def parse(raw):
    reader = csv.DictReader(io.StringIO(raw.decode('utf-8-sig'), newline=''))
    if reader.fieldnames != ['name', 'item_id_hex', 'item_id_decimal', 'category', 'stack_size']:
        raise ValueError('Unexpected source CSV schema')
    items = []; seen = set()
    for row in reader:
        item = int(row['item_id_decimal']); hexa = row['item_id_hex']
        if not 0 < item <= 0xffffffff or not re.fullmatch('[0-9a-fA-F]{8}', hexa) or int(hexa, 16) != item or item in seen:
            raise ValueError('Invalid/duplicate ID or inconsistent hex/decimal columns')
        name = row['name']; stack = int(row['stack_size'])
        if not name or len(name.encode()) > 512 or any(ord(c) < 32 for c in name) or stack < 0:
            raise ValueError('Invalid item name or stack size')
        seen.add(item)
        items.append({'item': item, 'hex': f'{item:08x}', 'name': name, 'sourceCategory': row['category'],
                      'category': category(name, row['category']), 'categoryEvidence': 'name_hint', 'stackSize': stack,
                      'compatibility': 'source_only_unverified', 'bounds': None, 'nativeCallAccepted': 0,
                      'interactionValidated': False})
    if not items or len(items)>8192: raise ValueError('CSV must contain 1..8192 unique IDs')
    return {'format': 'BuildingToolsItemIndex', 'version': 1, 'sourceURL': URL,
            'sourceSHA256': hashlib.sha256(raw).hexdigest(), 'items': items}


def merge(index, observed):
    if observed.get('format') != 'BuildingToolsNativeCatalog' or observed.get('source') != 'live_registry' or observed.get('complete') is not True:
        raise ValueError('A complete native registry export is required')
    if observed.get('indexSHA256') != index['sourceSHA256']:
        raise ValueError('Registry export does not match this source index; refresh the DLL/index together')
    native = {r['item']: r for r in observed['items']}
    if len(native) != len(observed['items']): raise ValueError('Duplicate native evidence ID')
    for r in native.values():
        if type(r['item']) is not int or not 0 < r['item'] <= 0xffffffff: raise ValueError('Invalid native ID')
        if r.get('resolution') == 'policy_compatible':
            b=r.get('bounds'); guid=r.get('placedEntity')
            if r.get('compatible') is not True or not isinstance(b,dict) or not isinstance(guid,str) or not re.fullmatch('[0-9a-fA-F]{32}',guid) or int(guid,16)==0:
                raise ValueError('Incomplete native placement metadata')
            lo,hi=b.get('low',[]),b.get('high',[])
            if len(lo)!=3 or len(hi)!=3 or any(type(v) not in (int,float) or not math.isfinite(v) or abs(v)>65536 for v in lo+hi) or any(hi[a]<=lo[a] for a in range(3)):
                raise ValueError('Invalid native model bounds')
        for value in r.get('placementCalls',{}).values():
            if type(value) is not int or not 0<=value<=0xffffffff: raise ValueError('Invalid native call counters')
    indexed={r['item'] for r in index['items']}
    for item,r in native.items():
        if item not in indexed:
            name=r.get('name') or f'Native {item:08x}'
            index['items'].append({'item':item,'hex':f'{item:08x}','name':name,'sourceCategory':'native_registry',
                                   'category':category(name,'BuildTools'),'categoryEvidence':'name_hint',
                                   'compatibility':'source_only_unverified','interactionValidated':False})
    for row in index['items']:
        # Every merge is a fresh observation, never a union of stale approvals.
        row.update(compatibility='source_only_unverified',bounds=None,nativeCallAccepted=0,nativeCallFailed=0,interactionValidated=False)
        row.pop('placedEntity',None);row.pop('flags',None)
        r = native.get(row['item'])
        # Evidence absent from a scan stays unverified, never inferred missing.
        if not r: continue
        status = r.get('resolution', 'source_only_unverified')
        if status not in ('registry_unobserved', 'unknown_id', 'not_prop', 'altar_slot', 'special_flags', 'logic_disabled', 'invalid_bounds', 'abi_unavailable', 'policy_compatible'):
            raise ValueError('Unknown native resolution status')
        row['compatibility'] = status; row['bounds'] = r.get('bounds'); row['flags'] = r.get('flags')
        row['placedEntity'] = r.get('placedEntity'); row['nativeCallAccepted'] = r.get('placementCalls', {}).get('accepted', 0)
        row['nativeCallFailed'] = r.get('placementCalls', {}).get('failed', 0)
    index['nativeEvidence'] = {k: observed.get(k) for k in ('mod', 'gameImage', 'indexSHA256')}
    return index


def header(index):
    rows = sorted(index['items'], key=lambda r: r['item'])
    out = ['// Generated from the attributed CSV. Discovery hints ONLY; native resolution is mandatory.',
           '#pragma once', 'namespace itemindex {', 'struct Entry {uint32_t item;const char* name;uint8_t category;};',
           f'constexpr const char* SHA256="{index["sourceSHA256"]}";',
           'static const char* const Categories[]={' + ','.join(json.dumps(c) for c in CATEGORIES) + '};',
           'static const Entry Entries[]={']
    out += [f'{{0x{r["hex"]}u,{json.dumps(r["name"], ensure_ascii=False)},{CATEGORIES.index(r["category"])}}},' for r in rows]
    out += ['};', 'constexpr int Count=sizeof(Entries)/sizeof(Entries[0]);',
            'inline const Entry* find(uint32_t id){int lo=0,hi=Count;while(lo<hi){int m=(lo+hi)/2;if(Entries[m].item<id)lo=m+1;else hi=m;}return lo<Count&&Entries[lo].item==id?Entries+lo:nullptr;}',
            'inline bool contains(const char* text,const char* query){if(!query||!*query)return true;if(!text)return false;for(;*text;++text){int k=0;while(query[k]&&text[k]){char a=text[k],b=query[k];if(a>=\'A\'&&a<=\'Z\')a+=32;if(b>=\'A\'&&b<=\'Z\')b+=32;if(a!=b)break;++k;}if(!query[k])return true;}return false;}', '}']
    return '\n'.join(out) + '\n'


def write_new(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open('xb') as f: f.write(data)


def main():
    p = argparse.ArgumentParser(description=__doc__); s = p.add_subparsers(dest='command', required=True)
    imp = s.add_parser('import'); imp.add_argument('--csv', type=Path, required=True); imp.add_argument('--out', type=Path, required=True); imp.add_argument('--header', type=Path)
    up = s.add_parser('download'); up.add_argument('--out', type=Path, required=True)
    q = s.add_parser('query'); q.add_argument('text', nargs='?', default=''); q.add_argument('--catalog', type=Path, default=ROOT/'catalog/native-items.json'); q.add_argument('--category', choices=CATEGORIES); q.add_argument('--status')
    m = s.add_parser('merge'); m.add_argument('--catalog', type=Path, required=True); m.add_argument('--observed', type=Path, required=True); m.add_argument('--out', type=Path, required=True)
    a = p.parse_args()
    try:
        if a.command == 'download':
            with urllib.request.urlopen(URL, timeout=30) as r: raw = r.read(4_000_001)
            if len(raw) > 4_000_000: raise ValueError('Source exceeds size budget')
            c = parse(raw); write_new(a.out, raw); print(f'{len(c["items"])} unique IDs; SHA256 {c["sourceSHA256"]}')
        if a.command == 'import':
            c = parse(a.csv.read_bytes()); write_new(a.out, (json.dumps(c, ensure_ascii=False, indent=2)+'\n').encode())
            if a.header: write_new(a.header, header(c).encode())
            print(json.dumps(collections.Counter(r['category'] for r in c['items']), sort_keys=True))
        if a.command == 'query':
            c = json.loads(a.catalog.read_text(encoding='utf-8'))
            for r in c['items']:
                if a.text.casefold() not in (r['name']+' '+r['hex']+' '+str(r['item'])).casefold(): continue
                if a.category and r['category'] != a.category or a.status and r['compatibility'] != a.status: continue
                print(f'{r["hex"]} {r["item"]:10} {r["category"]:15} {r["compatibility"]:24} {r["name"]}')
        if a.command == 'merge':
            c = merge(json.loads(a.catalog.read_text(encoding='utf-8')), json.loads(a.observed.read_text(encoding='utf-8')))
            write_new(a.out, (json.dumps(c, ensure_ascii=False, indent=2)+'\n').encode())
    except (ValueError, KeyError, TypeError, UnicodeError, OSError) as e: p.exit(1, f'Refused: {e}\n')


if __name__ == '__main__': main()
