#!/usr/bin/env python3
"""Generate new, non-overwriting dev3 fixtures from a real BTBP material only."""
import argparse
import json
from pathlib import Path
import struct
from make_project_example import blueprint, material_from


def create(output, material):
    output.mkdir(parents=True, exist_ok=False)
    def source(name, dims, step, occupied):
        data = bytearray(blueprint(dims, step, occupied, material))
        struct.pack_into('<I', data, 44, 0)  # price unknown: the engine recalculates it
        (output / name).write_bytes(data)
    def manifest(file, title, modules):
        value = dict(format='BuildingToolsExtendedProject', version=1, name=title,
                     gridSize=0.5, anchor=[0, 0, 0], modules=modules)
        (output / file).write_text(json.dumps(value, indent=2, ensure_ascii=False)+'\n', encoding='utf-8')
    def module(file, offset, rotation=0, category='general'):
        return dict(file=file, offset=offset, rotation=rotation, category=category)
    def house(x, y, z):
        shell = y in (0, 7) or x in (0, 7) or z in (0, 7)
        door = 1 <= y <= 5 and ((x in (0, 7) and z in (3, 4)) or (z in (0, 7) and x in (3, 4)))
        return shell and not door
    source('Dev3_Maison_05.btbp', (8, 8, 8), 0.5, house)
    source('Dev3_Maison_10.btbp', (8, 8, 8), 1.0, house)
    source('Dev3_Passage_05.btbp', (8, 1, 2), 0.5, lambda x,y,z: True)
    source('Dev3_Passage_large.btbp', (8, 1, 4), 0.5, lambda x,y,z: True)
    manifest('Dev3_Trois_batiments_relies.btproject', 'Dev3 - trois bâtiments reliés', [
        module('Dev3_Maison_05.btbp', [0,0,0], category='wall'),
        module('Dev3_Maison_05.btbp', [8,0,0], 180, 'wall'),
        module('Dev3_Maison_05.btbp', [16,0,0], 90, 'wall'),
        module('Dev3_Passage_05.btbp', [4,0,1.5], category='floor'),
        module('Dev3_Passage_05.btbp', [12,0,1.5], category='floor')])
    source('Dev3_Plancher_tremie.btbp', (12,1,12), 0.5, lambda x,y,z: not(4<=x<8 and 4<=z<8))
    source('Dev3_Murs_niveau.btbp', (12,8,12), 0.5,
           lambda x,y,z: (x in (0,11) or z in (0,11)) and not(z in (0,11) and 4<=x<8 and 1<=y<6))
    source('Dev3_Toiture.btbp', (16,6,16), 0.5, lambda x,y,z: y==min(x,15-x)*5//7)
    levels=[]
    for y in (0,4,8):
        levels += [module('Dev3_Plancher_tremie.btbp',[0,y,0],category='floor'), module('Dev3_Murs_niveau.btbp',[0,y,0],category='wall')]
    levels += [module('Dev3_Toiture.btbp',[-1,12,-1],category='roof')]
    manifest('Dev3_Batiment_etages.btproject', 'Dev3 - trois niveaux et trémies', levels)
    manifest('Dev3_Grille_vide_Y120.btproject', 'Dev3 - grille virtuelle Y120', [
        module('Dev3_Maison_05.btbp',[0,0,0],category='wall'), module('Dev3_Passage_05.btbp',[4,0,1.5],category='floor'), module('Dev3_Maison_05.btbp',[8,0,0],180,'wall')])
    city=[]
    for z in range(6):
        for x in range(6):
            city.append(module('Dev3_Maison_10.btbp',[x*12,0,z*12],90*((x+z)%4),'wall'))
            if x<5: city.append(module('Dev3_Passage_large.btbp',[x*12+8,0.5,z*12+3],0,'floor'))
            if z<5: city.append(module('Dev3_Passage_large.btbp',[x*12+3,0.5,z*12+8],90,'floor'))
    manifest('Dev3_Village_36_batiments.btproject', 'Dev3 - 36 bâtiments et 60 passages', city)


if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--material-from', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args=parser.parse_args()
    try:
        create(args.output,material_from(args.material_from))
    except (ValueError,OSError) as error:
        parser.exit(1,str(error)+'\n')
