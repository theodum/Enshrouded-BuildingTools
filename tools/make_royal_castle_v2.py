#!/usr/bin/env python3
"""Room-by-room spacious castle, with disjoint components and 0.5 m voxels.

V1 remains a reproducible fixture. V2 deliberately changes the stair structure:
production layouts, 1 m thick flights, a continuous supporting core and external
guards; no support cell is clipped to accommodate another flight. No game writes.
"""
from __future__ import annotations
import argparse
from collections import Counter, defaultdict, deque
import csv
import hashlib
import json
import math
from pathlib import Path
import sys
import zipfile

import make_royal_castle as v1

ROOT = v1.ROOT
NAME = 'Forteresse Royale V2 — Grands intérieurs'
PROJECT = 'Forteresse_Royale_Spacieuse_V2.btproject'
FOLDER = ROOT/'examples/royal-castle-v2'
ARCHIVE = ROOT/'dist/Forteresse_Royale_Spacieuse_V2-dev7.zip'


def u(metres):
    value = metres*2
    if int(value) != value:
        raise ValueError('Coordinate is not on the 0.5 m grid')
    return int(value)


def r(x0, z0, x1, z1):
    return v1.rect(u(x0), u(z0), u(x1), u(z1))


def box_points(low, high):
    return set(v1.cells_in(tuple(map(u, low)), tuple(map(u, high))))


def bbox(points):
    return (tuple(min(p[a] for p in points) for a in range(3)),
            tuple(max(p[a] for p in points)+1 for a in range(3)))


class SpaciousCastle:
    def __init__(self, profiles, palette=None):
        self.profiles = profiles
        self.palette = palette or v1.palette_from()
        self.cells = {}
        self.air = set()
        self.spaces = []
        self.space_keys = {}
        self.portals = []
        self.stairs = []
        self.rooms = []
        self.features = []
        self.lanes = []
        self.floor_planes = defaultdict(set)
        self.floor_candidates = defaultdict(set)
        self.networks = defaultdict(set)
        self.stair_cells = {}
        self.cages = []
        self.main = r(-14, -24, 14, 10)
        self.west = r(-28, -24, -14, 24)
        self.east = r(14, -24, 28, 24)
        self.gate = r(-6, -29, 6, -24)
        self.keep = r(-9, 19, 9, 37)
        self.tower_centres = {'SO': (-30, -26), 'SE': (30, -26), 'NO': (-30, 26), 'NE': (30, 26)}
        self.towers = {n: v1.octagon(u(cx), u(cz), 16) for n, (cx, cz) in self.tower_centres.items()}
        self.tower_all = set().union(*self.towers.values())
        self.court = r(-14, 10, 14, 24)-self.keep
        self.parvis = r(-9, -33, 9, -24)
        self.footprint = self.main | self.west | self.east | self.gate | self.keep | self.tower_all | self.court | self.parvis

    def put(self, points, role, category, owner):
        points = set(points)
        blocked = points & self.air
        if blocked:
            raise ValueError(f'{owner} empiète sur un dégagement: {min(blocked)}')
        occupied = points & self.cells.keys()
        if occupied:
            p = min(occupied)
            raise ValueError(f'Chevauchement de composants, même matériau inclus: {owner} / {self.cells[p][2]} à {p}')
        value = (role, category, owner)
        self.cells.update(dict.fromkeys(points, value))

    def space(self, low, high, label, native=True):
        low, high = tuple(low), tuple(high)
        if high[1]-low[1] not in (6, 7):
            raise ValueError('V2 clearance must reserve 3 or 3.5 m')
        key = (low, high)
        if native and key not in self.space_keys:
            self.space_keys[key] = len(self.spaces)
            self.spaces.append({'low': low, 'high': high, 'label': label})
        self.air.update(v1.cells_in(low, high))

    def lane(self, name, footprint, level, headroom=6):
        self.lanes.append({'name': name, 'footprint': set(footprint), 'level': level, 'height': headroom})
        self.air.update((x, y, z) for x, z in footprint for y in range(level, level+headroom))

    def portal(self, name, axis, plane, centre, level, width=3, depth=2, height=4):
        plane, centre, level, width, depth, height = map(u, (plane, centre, level, width, depth, height))
        if axis == 'z':
            low = (centre-width//2, level, plane-depth//2)
            high = (centre+width//2, level+6, plane+depth//2)
        else:
            low = (plane-depth//2, level, centre-width//2)
            high = (plane+depth//2, level+6, centre+width//2)
        self.space(low, high, name)
        self.portals.append({'name': name, 'axis': axis, 'plane': plane, 'centre': centre,
                             'level': level, 'width': width, 'depth': depth, 'height': height})

    def opening(self, x, y, z):
        for p in self.portals:
            cross, normal = (x, z) if p['axis'] == 'z' else (z, x)
            h = y-p['level']
            if abs(normal-p['plane']) <= p['depth']//2+1 and 0 <= h < p['height']:
                inset = max(0, h-max(6, p['height']-p['width']//2))
                if p['centre']-p['width']//2+inset <= cross < p['centre']+p['width']//2-inset:
                    return True
        return False

    def stair(self, name, kind, origin):
        origin = tuple(map(u, origin))
        profile = self.profiles[kind]
        base = origin[1]
        surfaces, physical = [], set()
        tops = {}
        for i, s in enumerate(profile['surfaces']):
            low = tuple(s['low'][a]+origin[a] for a in range(3))
            high = tuple(s['high'][a]+origin[a] for a in range(3))
            surfaces.append({'low': low, 'high': high, 'landing': s['landing']})
            self.space((low[0], high[1], low[2]), (high[0], high[1]+profile['headroom'], high[2]), f'{name}/{i}')
            bottom = base-1 if high[1] == base else max(base, high[1]-2)
            physical.update(v1.cells_in((low[0], bottom, low[2]), high))
            for x, z in v1.rect(low[0], low[2], high[0], high[2]):
                tops[x, z] = high[1]
        collision = physical & self.stair_cells.keys()
        if collision:
            p = min(collision)
            raise ValueError(f'Volées superposées: {name} / {self.stair_cells[p]} à {p}')
        self.stair_cells.update(dict.fromkeys(physical, name))
        # Guards sit OUTSIDE the 2.5 m walking width, never on a tread.
        for s in surfaces:
            top = s['high'][1]
            if top in (base, base+profile['height']):
                continue  # full floor landing/apron, guarded with that floor
            for x, z in v1.rect(s['low'][0], s['low'][2], s['high'][0], s['high'][2]):
                for dx, dz in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    q = (x+dx, z+dz)
                    if q in tops and abs(tops[q]-top) <= 1:
                        continue
                    self.networks['cage'].update((q[0], y, q[1]) for y in range(max(base, top-2), top+2))
        entry = surfaces[0]
        arrivals = [s for s in surfaces if s['landing'] and s['high'][1] == base+profile['height']]
        if kind == 'U':
            # A full-width cross landing in front of both flights, 2.5 m deep.
            z1 = origin[2]-profile['landing']
            for level in (base, base+profile['height']):
                low = (origin[0], level, z1-5)
                high = (origin[0]+2*profile['width']+1, level+6, z1)
                self.space(low, high, f'Palier transversal {name} / {level}')
        elif kind == 'L':
            self.space((entry['low'][0], base, entry['low'][2]-5),
                       (entry['high'][0], base+6, entry['low'][2]), f'Approche basse {name}')
            end = arrivals[0]
            self.space((end['high'][0], end['high'][1], end['low'][2]),
                       (end['high'][0]+5, end['high'][1]+6, end['high'][2]), f'Approche haute {name}')
        self.stairs.append({'name': name, 'kind': kind, 'origin': origin, 'surfaces': surfaces,
                            'physicalCellCount': len(physical), 'clearWidthMeters': 2.5,
                            'landingDepthMeters': 3, 'headroomMeters': 3.5})

    def prepare_circulation(self):
        self.stair('Escalier impérial', 'monumental', (-2.5, 0, -16))
        for floor in (0, 5):
            self.stair(f'Escalier ouest L {floor}', 'L', (-26, floor, -2))
            self.stair(f'Escalier est U {floor}', 'U', (18.5, floor, -2))
        for n, (cx, cz) in self.tower_centres.items():
            sx = cx-4 if cx < 0 else cx-1.5
            for floor in (0, 5, 10, 15):
                self.stair(f'Tour {n} U {floor}', 'U', (sx, floor, cz-1.5))
            self.networks['cage'].update(box_points((sx+2.5, 0, cz-1.5), (sx+3, 20, cz+1)))
            self.cages.append({'name': f'Cage tour {n}', 'low': (u(sx-.5), 0, u(cz-4.5)),
                               'high': (u(sx+6), 40, u(cz+4.5))})
        for floor in (0, 5, 10, 15, 20, 25):
            self.stair(f'Donjon U {floor}', 'U', (-3.5, floor, 26.5))
        self.networks['cage'].update(box_points((-1, 0, 26.5), (-.5, 30, 29)))
        self.networks['cage'].update(box_points((21, 0, -2), (21.5, 10, .5)))
        self.cages += [
            {'name': 'Cage du donjon', 'low': tuple(map(u, (-4, 0, 23.5))), 'high': tuple(map(u, (2.5, 30, 32.5)))},
            {'name': 'Cage ouest L', 'low': tuple(map(u, (-26.5, 0, -5))), 'high': tuple(map(u, (-17.5, 10, 3.5)))},
            {'name': 'Cage est U', 'low': tuple(map(u, (18, 0, -5))), 'high': tuple(map(u, (24.5, 10, 4)))},
        ]
        self.portal('Grand portail', 'z', -29, 0, 0, 6, 5, 5)
        self.portal('Vestibule', 'z', -24, 0, 0, 6, 2, 5)
        for level in (0, 5, 10):
            self.portal(f'Axe royal {level}', 'z', -4, 0, level, 6, 2, 4)
            for side in (-1, 1):
                for z in (-20, 2):
                    self.portal(f'Corps vers aile {side}/{level}/{z}', 'x', side*14, z, level, 4)
                for z in (-16, 16):
                    self.portal(f'Porte pièce aile {side}/{level}/{z}', 'x', side*18.5, z, level, 3)
            if level:
                self.portal(f'Loggia {level}', 'z', -24, 0, level, 4)
        self.portal('Réception vers cour', 'z', 9.5, 0, 0, 6)
        self.portal('Appartements communicants', 'x', 0, 4, 10, 3)
        for side in (-1, 1):
            for z in (14, 21):
                self.portal(f'Arcade de cour {side}/{z}', 'x', side*14, z, 0, 4)
        for level in (0, 10, 15):
            self.portal(f'Entrée donjon {level}', 'z', 19.5, 0, level, 4, 3)

    def floor_plan(self):
        full = self.main | self.west | self.east | self.gate | self.tower_all | self.keep
        for level in (0, 10, 20):
            footprint = full.copy()
            if level == 0:
                footprint |= self.court | self.parvis
            if level == 10:
                footprint -= r(-8, -20, 8, -8)
            if level == 20:
                footprint |= r(-28, 18, 28, 24) | r(-14, 15, 14, 24)
            self.floor_candidates[level] |= footprint
        walk = (r(-28, -24, -14, -18) | r(-19, -24, -14, 24) |
                r(14, -24, 28, -18) | r(14, -24, 19, 24) | r(-28, 18, 28, 24) | r(-14, 15, 14, 24))
        self.floor_candidates[30] |= walk | self.tower_all | self.keep
        self.floor_candidates[40] |= self.tower_all | self.keep
        self.floor_candidates[50] |= self.keep
        self.floor_candidates[60] |= self.keep

    def window(self, x, y, z):
        lancet = v1.Castle.lancet
        for level in (0, 10, 20):
            if z in (u(-24), u(-23.5), u(9), u(9.5)):
                if any(lancet(x, y, u(c), level+3, 4, 5) for c in (-10.5, -6.5, 6.5, 10.5)):
                    return True
            if x in (u(-28), u(-27.5), u(27), u(27.5)):
                if any(lancet(z, y, u(c), level+3, 4, 5) for c in (-19, -12, -4, 4, 12, 19)):
                    return True
        for cx, cz in self.tower_centres.values():
            for level in (0, 10, 20, 30):
                if x in (u(cx-8), u(cx-7.5), u(cx+7), u(cx+7.5)) and any(
                        lancet(z, y, u(cz+c), level+3, 3, 5) for c in (-3, 3)):
                    return True
                if z in (u(cz-8), u(cz-7.5), u(cz+7), u(cz+7.5)) and any(
                        lancet(x, y, u(cx+c), level+3, 3, 5) for c in (-3, 3)):
                    return True
        for level in (0, 10, 20, 30, 40, 50):
            if x in (-18, -17, 16, 17) and any(lancet(z, y, u(c), level+3, 4, 5) for c in (23, 28, 33)):
                return True
            if z in (38, 39, 72, 73) and any(lancet(x, y, u(c), level+3, 4, 5) for c in (-5, 5)):
                return True
        return False

    def plan_shell(self):
        for y in range(60):
            footprint = self.keep.copy()
            if y < 40:
                footprint |= self.tower_all
            if y < 30:
                footprint |= self.main | self.west | self.east
            if y < 20:
                footprint |= self.gate
            for x, z in v1.border(footprint, 2):
                if not self.opening(x, y, z) and not self.window(x, y, z):
                    self.networks['shell'].add((x, y, z))
        for level in (0, 10, 20):
            lines = set()
            for side in (-1, 1):
                x = u(side*14)-(side > 0)
                lines |= {(x, z) for z in range(-48, 20)}
                # No partition through the middle stair hall: its corridor is
                # a 3.5 m open gallery beside the dedicated cage.
                x = u(side*18.5)-(side > 0)
                lines |= {(x, z) for z in range(-48, -16)}
                lines |= {(x, z) for z in range(16, 48)}
                for z in (-16, 16):
                    xs = range(-56, -37) if side < 0 else range(37, 56)
                    lines |= {(x, z) for x in xs}
            lines |= {(x, -8) for x in range(-28, 28)}
            if level == 20:
                lines |= {(0, z) for z in range(-8, 20)}
            for y in range(level, level+10):
                self.networks['shell'].update((x, y, z) for x, z in lines if not self.opening(x, y, z))

    def room(self, name, footprint, level, seed, kind='room', programme=''):
        self.rooms.append({'name': name, 'footprint': set(footprint), 'seed': tuple(map(u, (seed[0], level, seed[1]))),
                           'kind': kind, 'programme': programme, 'level': u(level)})

    def plan_rooms(self):
        self.room('Parvis royal', self.parvis, 0, (0, -31), 'entry')
        self.room('Grand hall impérial', r(-14, -24, 14, -4), 0, (0, -21), programme='Escalier monumental, deux arcades latérales')
        self.room('Réception et antichambre', r(-14, -4, 14, 10), 0, (0, 0), programme='Accueil, banquettes et niches')
        self.room('Cour des arcades', self.court, 0, (11, 13), 'court')
        for level in (5,):
            self.room('Galerie impériale ouest', r(-14, -24, -8, -4), level, (-11, -17), 'gallery')
            self.room('Galerie impériale est', r(8, -24, 14, -4), level, (11, -17), 'gallery')
        self.room('Grande salle du trône', r(-14, -4, 14, 10), 5, (0, 0), programme='Axe de 6 m, colonnes, estrade et trône')
        self.room('Estrade royale', r(-3.5, 7, 3.5, 9), 6.5, (2.5, 7.5), 'dais', 'Deux accès latéraux et trois degrés larges')
        self.room('Grande bibliothèque des archives', r(-14, -24, 14, -4), 10, (0, -16), programme='Rayonnages latéraux, tables et allées transversales')
        for side, suffix in ((-1, 'ouest'), (1, 'est')):
            self.room(f'Appartement royal {suffix}', r(-14 if side < 0 else .5, -4, -.5 if side < 0 else 14, 10),
                      10, (side*6, 1), programme='Alcôve privée, penderies et cabinet')
        names = {0: ('Salle des gardes', 'Salle des banquets', 'Salle des armes', 'Cuisine et offices'),
                 5: ('Salon noble', 'Salle du conseil', 'Salon des hôtes', 'Galerie des portraits'),
                 10: ('Chambre des hôtes ouest', 'Bibliothèque de lecture', 'Chambre des hôtes est', 'Salle des trophées')}
        for level, labels in names.items():
            for side, offset in ((-1, 0), (1, 2)):
                x0, x1 = (-28, -18.5) if side < 0 else (18.5, 28)
                for j, (z0, z1, z) in enumerate(((-24, -8, -16), (8, 24, 16))):
                    self.room(labels[offset+j], r(x0, z0, x1, z1), level, (side*22.5, z))
                x0, x1 = (-28, -14) if side < 0 else (14, 28)
                self.room(f'Grand vestibule escalier {side}/{level}', r(x0, -8, x1, 8), level,
                          (side*16, 5.5), 'stairhall')
                # Protected 3 m axes through the rooms and their doors.
                for z in (-16, 16):
                    lane = r(-25, z-1.5, -17, z+1.5) if side < 0 else r(17, z-1.5, 25, z+1.5)
                    self.lane(f'Allée pièce {side}/{level}/{z}', lane, u(level))
                lane = r(-18, -23, -15, 23) if side < 0 else r(15, -23, 18, 23)
                self.lane(f'Galerie aile {side}/{level}', lane, u(level))
        for n, (cx, cz) in self.tower_centres.items():
            for level in (0, 5, 10, 15, 20):
                # The usable room beside the cage lies on the inward side.
                x = cx+4.5 if cx < 0 else cx-4.5
                z = cz-4.5 if cz < 0 else cz+4.5
                self.room(f'Tour {n} — salle {level:g} m', self.towers[n]-(self.west | self.east),
                          level, (x, z), 'tower', {0: 'Salle de garde', 5: 'Salle de guet', 10: 'Cabinet de tour',
                                                15: 'Salon de lecture', 20: 'Terrasse de couronnement'}[level])
        for level in (0, 5, 10, 15, 20, 25, 30):
            self.room(f'Donjon — chambre {level:g} m', self.keep, level, (5, 27), 'keep')
            self.lane(f'Allée du donjon {level}', r(3.5, 21, 6.5, 35), u(level))
        for level in (10, 15):
            self.room(f'Terrasse arrière {level:g} m', (r(-28, 18, 28, 24) | r(-14, 15, 14, 24))-self.keep, level, (-12, 21), 'terrace')
            self.lane(f'Passage devant donjon {level}', r(-13.5, 15.5, 13.5, 18.5), u(level))
        for side in (-1, 1):
            self.room(f'Chemin de ronde {side}', r(-19, -24, -14, 24) if side < 0 else r(14, -24, 19, 24),
                      15, (side*16, 13), 'roofwalk')
        for level in (5, 10):
            self.room(f'Loggia royale {level:g} m', self.gate, level, (0, -26), 'balcony')
        self.lane('Axe du trône et de la réception', r(-3, -4, 3, 4), 0)
        self.lane('Nef de la salle du trône', r(-3, -4, 3, 3.5), 10)
        self.lane('Axe des archives', r(-2, -23, 2, -4), 20)
        self.lane('Galerie appartement ouest', r(-7.5, -.5, -.5, 2.5), 20)
        self.lane('Galerie appartement est', r(.5, -.5, 7.5, 2.5), 20)

    def materialize_structure(self):
        protected = self.air
        for key in ('shell', 'cage', 'relief'):
            blocked = self.networks[key] & protected
            if blocked:
                raise ValueError(f'Le réseau {key} coupe un passage: {min(blocked)}')
        for a, b in (('shell', 'cage'), ('shell', 'relief'), ('cage', 'relief')):
            collision = self.networks[a] & self.networks[b]
            if collision:
                raise ValueError(f'Réseaux {a}/{b} occupent la même cellule: {min(collision)}')
        physical = set(self.stair_cells)
        for key in ('shell', 'cage', 'relief'):
            collision = physical & self.networks[key]
            if collision:
                p = min(collision)
                raise ValueError(f'Structure {key} et escalier se chevauchent: {self.stair_cells[p]} à {p}')
        structural = self.networks['shell'] | self.networks['cage'] | self.networks['relief'] | physical
        floor_points = set()
        for level, footprint in self.floor_candidates.items():
            for x, z in footprint:
                p = (x, level-1, z)
                if p not in protected:
                    self.floor_planes[level].add((x, z))
                    if p not in structural:
                        floor_points.add(p)
        self.put(floor_points, 'masonry', 'floor', 'Planchers communs')
        self.put(self.networks['shell'], 'masonry', 'wall', 'Murs et partitions')
        self.put(self.networks['cage'], 'trim', 'wall', 'Noyaux porteurs et garde-corps extérieurs aux marches')
        self.put(self.networks['relief'], 'trim', 'wall', 'Corniches, contreforts et piédroits')
        for stair in self.stairs:
            points = {p for p, name in self.stair_cells.items() if name == stair['name']}
            self.put(points, 'masonry', 'stairs', stair['name'])

    def feature(self, name, low, high, role='trim', room=''):
        points = box_points(low, high)
        lo, hi = tuple(map(u, low)), tuple(map(u, high))
        for cage in self.cages:
            if all(lo[a] < cage['high'][a] and hi[a] > cage['low'][a] for a in range(3)):
                raise ValueError(f'Le mobilier {name} empiète sur la cage réservée {cage["name"]}')
        for p in points:
            if p in self.air:
                raise ValueError(f'Le mobilier {name} coupe une allée ou un escalier: {p}')
        self.put(points, role, 'furniture', name)
        self.features.append({'name': name, 'room': room, 'low': tuple(map(u, low)), 'high': tuple(map(u, high))})

    def interiors(self):
        # Distinct room-specific layouts keep the protected 3 m axes empty.
        for level in (0, 5):
            for x in (-12, 12):
                for z in (-21, -6):
                    self.feature(f'Base arcade hall {x}/{z}/{level}', (x-1, level, z-1), (x+1, level+.5, z+1))
                    self.feature(f'Pilier arcade hall {x}/{z}/{level}', (x-.5, level+.5, z-.5), (x+.5, level+4, z+.5))
                    self.feature(f'Chapiteau arcade hall {x}/{z}/{level}', (x-1, level+4, z-1), (x+1, level+4.5, z+1))
        for x in (-10, 10):
            for z in (-1, 5):
                self.feature(f'Base trône {x}/{z}', (x-1, 5, z-1), (x+1, 5.5, z+1), room='Grande salle du trône')
                self.feature(f'Colonne trône {x}/{z}', (x-.5, 5.5, z-.5), (x+.5, 9, z+.5), room='Grande salle du trône')
                self.feature(f'Chapiteau trône {x}/{z}', (x-1, 9, z-1), (x+1, 9.5, z+1), room='Grande salle du trône')
        # Each tier is a disjoint solid; the next starts directly above it.
        self.feature('Premier degré estrade', (-4.5, 5, 4), (4.5, 5.5, 9), 'throne', 'Grande salle du trône')
        self.feature('Deuxième degré estrade', (-4, 5.5, 5.5), (4, 6, 9), 'throne', 'Grande salle du trône')
        self.feature('Plateau de l’estrade', (-3.5, 6, 7), (3.5, 6.5, 9), 'throne', 'Grande salle du trône')
        self.feature('Siège du trône', (-1, 6.5, 7.5), (1, 7, 9), 'throne', 'Grande salle du trône')
        self.feature('Dossier du trône', (-1, 7, 8.5), (1, 8.5, 9), 'throne', 'Grande salle du trône')
        for x in (-1.5, 1):
            self.feature(f'Accoudoir du trône {x}', (x, 6.5, 7.5), (x+.5, 7.5, 9), room='Grande salle du trône')
        for level in (0, 5, 10):
            for side in (-1, 1):
                a, b = (-27, -26) if side < 0 else (26, 27)
                for z0, z1 in ((-22, -18), (-14, -10), (10, 14), (18, 22)):
                    self.feature(f'Banquette/présentoir mural {side}/{level}/{z0}', (a, level, z0), (b, level+.5, z1))
        # Council/banquet tables are offset from the doors' transverse 3 m aisle.
        for level, name in ((0, 'Table des banquets'), (5, 'Table du conseil')):
            self.feature(name, (-24.5, level, 18.5), (-21.5, level+1, 21.5), room='Salle des banquets' if level == 0 else 'Salle du conseil')
        for z in (-22, -17, -12, -7):
            for x in (-12.5, 11.5):
                for yy in (10.5, 12, 13.5):
                    self.feature(f'Rayon archives {x}/{z}/{yy}', (x, yy, z), (x+1, yy+.5, z+3), room='Grande bibliothèque des archives')
        for z in (10, 18):
            for yy in (10.5, 12, 13.5):
                self.feature(f'Rayonnage lecture {z}/{yy}', (-27, yy, z), (-26, yy+.5, z+4), room='Bibliothèque de lecture')
        for z in (10.5, 20):
            for x in (20, 24):
                self.feature(f'Socle de trophée {x}/{z}', (x, 10, z), (x+1.5, 11, z+1.5), 'throne', 'Salle des trophées')
        for side in (-1, 1):
            x0, x1 = (-12, -9) if side < 0 else (9, 12)
            self.feature(f'Alcôve royale {side}', (x0, 10, 6), (x1, 10.5, 9), room=f'Appartement royal {side}')
            self.feature(f'Cabinet royal {side}', (x0, 10.5, 8.5), (x1, 12, 9), room=f'Appartement royal {side}')
        for side in (-1, 1):
            x0, x1 = (-25.5, -22.5) if side < 0 else (22.5, 25.5)
            self.feature(f'Alcôve des hôtes {side}', (x0, 10, -22), (x1, 10.5, -19), room='Chambre des hôtes')
        for side in (-1, 1):
            x0, x1 = (-25, -22) if side < 0 else (22, 25)
            self.feature(f'Table basse du salon {side}', (x0, 5, -22), (x1, 5.5, -20), room='Salon noble' if side < 0 else 'Salon des hôtes')
        self.feature('Four architectural des offices', (22, 0, 20), (24.5, 1.5, 22), room='Cuisine et offices')
        for side, name in ((-1, 'Râtelier des gardes'), (1, 'Présentoir de la salle des armes')):
            x0, x1 = (-27, -26.5) if side < 0 else (26.5, 27)
            self.feature(name, (x0, .5, -22), (x1, 2.5, -18), room='Salle des gardes' if side < 0 else 'Salle des armes')
        for n, (cx, cz) in self.tower_centres.items():
            x0, x1 = (cx+5, cx+6) if cx < 0 else (cx-6, cx-5)
            for level in (0, 5, 10, 15):
                self.feature(f'Banquette de tour {n}/{level}', (x0, level, cz-1.5), (x1, level+.5, cz+1.5), room=f'Tour {n}')
        for level in (0, 5, 10, 15, 20, 25):
            for z in (22, 31):
                self.feature(f'Banquette du donjon {level}/{z}', (7, level, z), (8, level+.5, z+3), room=f'Donjon — chambre {level:g} m')
        # Two low basins leave the gate-to-keep axis entirely open.
        for cx in (-8, 8):
            footprint = r(cx-2, 12, cx+2, 15)
            self.put({(x, 0, z) for x, z in v1.border(footprint)}, 'trim', 'furniture', f'Bassin sec {cx}')

    def plan_exterior(self):
        relief = set()
        for y in (9, 19, 29, 39, 49, 59):
            footprint = self.keep.copy()
            if y < 40:
                footprint |= self.tower_all
            if y < 30:
                footprint |= self.main | self.west | self.east
            if y < 20:
                footprint |= self.gate
            for x, z in v1.border(footprint):
                if (x, y, z) not in self.networks['shell']:
                    continue
                for dx, dz in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    if (x+dx, z+dz) not in footprint:
                        relief.add((x+dx, y, z+dz))
        for side in (-1, 1):
            for z in (-16, -8, 8, 16):
                for y in range(29):
                    extension = 3 if y < 4 else 2 if y < 20 else 1
                    for dx in range(extension):
                        x = side*(57+dx)-(side > 0)
                        for dz in range(2):
                            relief.add((x, y, u(z)+dz))
        for x in (-26, -25, -17, -16, 15, 16, 24, 25):
            for y in range(29):
                for z in range(-51 if y < 4 else -50, -48):
                    relief.add((x, y, z))
        # Gate piers stay outside the clear 6 m arch and the upper loggia.
        for x in (-15, -14, 13, 14):
            relief.update((x, y, z) for y in range(20) for z in (-60, -59))
        self.networks['relief'] = relief

    def roofs_and_guards(self):
        roofs = set()
        gables = set()
        for x in range(-29, 29):
            top = 30+min(x+29, 28-x)//2
            roofs.update((x, y, z) for z in range(-49, 21) for y in range(max(30, top-1), top+1))
            for z in (-48, 19):
                gables.update((x, y, z) for y in range(30, top-1) if (2*x+1)**2+(2*y-70)**2 > 36)
        for x0, x1 in ((-57, -39), (39, 57)):
            for x in range(x0, x1):
                top = 30+min(x-x0, x1-1-x)
                roofs.update((x, y, z) for z in range(-36, 36) for y in range(max(30, top-1), top+1))
                for z in (-36, 35):
                    gables.update((x, y, z) for y in range(30, top-1))
        crowns = set()
        for footprint, level in [(fp, 40) for fp in self.towers.values()] + [(self.keep, 60)]:
            for x, z in v1.border(footprint):
                for y in range(level, level+4):
                    if y < level+2 or (x+z)//2 % 2 == 0:
                        crowns.add((x, y, z))
        for cx in (-30, 30):
            for dx in (-6, 5.5):
                for dz in (-6, 5.5):
                    crowns.update(box_points((cx+dx, 20, -26+dz), (cx+dx+.5, 22.5, -26+dz+.5)))
            for x, z in self.towers['SO' if cx < 0 else 'SE']:
                ring = max(abs(2*x+1-u(2*cx)), abs(2*z+1-u(-52)))//2
                top = 46+(15-ring)//2
                roofs.update((x, y, z) for y in range(top-1, top+1))
        # Whole-floor edges provide parapets. Boundaries that intentionally open
        # into a stair reservation are its entrances, not walls to be clipped.
        guards = set()
        for level in (10, 20, 30):
            for x, z in v1.border(self.floor_planes[level]):
                for y in range(level, level+2):
                    p = (x, y, z)
                    if p in self.air or p in self.cells or p in roofs or p in gables:
                        continue
                    guards.add(p)
        self.put(gables, 'masonry', 'roof', 'Pignons ajourés')
        self.put(roofs, 'roof', 'roof', 'Combles et coiffes')
        self.put(crowns, 'trim', 'tower', 'Couronnes et supports des belvédères')
        self.put(guards, 'trim', 'wall', 'Parapets des galeries et terrasses')

    def build(self):
        self.prepare_circulation()
        self.floor_plan()
        self.plan_shell()
        self.plan_rooms()
        self.plan_exterior()
        self.materialize_structure()
        self.interiors()
        self.roofs_and_guards()
        return self

    def validate(self):
        if len(self.cells) > 1048576 or len(self.spaces) > 512:
            raise ValueError('Budget composite dépassé')
        columns = 0
        for s in self.spaces:
            lo, hi = s['low'], s['high']
            area = (hi[0]-lo[0])*(hi[2]-lo[2])
            columns += area
            if area > 4096:
                raise ValueError('Réservation trop large: '+s['label'])
            blocked = set(v1.cells_in(lo, hi)) & self.cells.keys()
            if blocked:
                raise ValueError(f"Dégagement obstrué {s['label']}: {min(blocked)}")
            for x, z in v1.rect(lo[0], lo[2], hi[0], hi[2]):
                if (x, lo[1]-1, z) not in self.cells:
                    raise ValueError(f"Palier sans soutien {s['label']}: {(x, lo[1], z)}")
        if columns > 65536:
            raise ValueError('Budget cumulé de dégagement dépassé')
        for lane in self.lanes:
            for x, z in lane['footprint']:
                for y in range(lane['level'], lane['level']+lane['height']):
                    if (x, y, z) in self.cells:
                        raise ValueError(f"Allée encombrée {lane['name']}: {(x, y, z)}")
                if (x, lane['level']-1, z) not in self.cells:
                    raise ValueError(f"Sol discontinu dans l'allée {lane['name']}: {(x, lane['level'], z)}")
        authored = {(x, level, z) for level, points in self.floor_planes.items() for x, z in points
                    if (x, level-1, z) not in self.networks['cage']}
        for stair in self.stairs:
            for s in stair['surfaces']:
                authored |= {(x, s['high'][1], z) for x, z in v1.rect(s['low'][0], s['low'][2], s['high'][0], s['high'][2])}
        for feature in self.features:
            if feature['name'] in ('Premier degré estrade', 'Deuxième degré estrade', 'Plateau de l’estrade'):
                lo, hi = feature['low'], feature['high']
                authored |= {(x, hi[1], z) for x, z in v1.rect(lo[0], lo[2], hi[0], hi[2])}
        # A low basin rim is an intentional 0.5 m change in the ground floor.
        authored |= {(x, y+1, z) for (x, y, z), value in self.cells.items() if value[2].startswith('Bassin sec')}
        surfaces = {p for p in authored if (p[0], p[1]-1, p[2]) in self.cells
                    and all((p[0], p[1]+h, p[2]) not in self.cells for h in range(6))}
        entry = self.rooms[0]['seed']
        if entry not in surfaces:
            raise ValueError('Entrée sans surface libre')
        reached = {entry}
        queue = deque([entry])
        while queue:
            x, y, z = queue.popleft()
            for dx, dz in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                for dy in (0, 1, -1):
                    q = x+dx, y+dy, z+dz
                    if q in surfaces and q not in reached:
                        if dy == 1 and (x, y+6, z) in self.cells:
                            continue
                        if dy == -1 and (q[0], q[1]+6, q[2]) in self.cells:
                            continue
                        reached.add(q)
                        queue.append(q)
        missing = [room['name'] for room in self.rooms if room['seed'] not in reached]
        if missing:
            raise ValueError('Pièces déconnectées du graphe de surfaces: '+', '.join(missing))
        for stair in self.stairs:
            for s in stair['surfaces']:
                for x, z in v1.rect(s['low'][0], s['low'][2], s['high'][0], s['high'][2]):
                    if (x, s['high'][1], z) not in reached:
                        raise ValueError(f"Marche/palier déconnecté: {stair['name']} {(x, s['high'][1], z)}")
        isolated = surfaces-reached
        if isolated:
            raise ValueError(f'Surfaces isolées: {len(isolated)}; premières {sorted(isolated)[:8]}')
        # Independent flat-floor audit with a 1.5 x 1.5 m footprint. No window
        # sill, cornice or single 0.5 m ledge can count as a room connection.
        wide = {p for p in surfaces if all((p[0]+dx, p[1], p[2]+dz) in surfaces
                                         for dx in range(3) for dz in range(3))}
        components = {}
        component_count = 0
        for p in sorted(wide):
            if p in components:
                continue
            components[p] = component_count
            queue = deque([p])
            while queue:
                x, y, z = queue.popleft()
                for dx, dz in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    q = x+dx, y, z+dz
                    if q in wide and q not in components:
                        components[q] = component_count
                        queue.append(q)
            component_count += 1
        edges = defaultdict(set)
        for stair in self.stairs:
            base = stair['origin'][1]
            endpoints = [s for s in stair['surfaces'] if s['landing']]
            ids = []
            for s in endpoints:
                p = s['low'][0]+1, s['high'][1], s['low'][2]+1
                if p not in components:
                    raise ValueError(f"Palier trop étroit pour le contrôle 1,5 m: {stair['name']} {p}")
                ids.append(components[p])
            for a in ids:
                edges[a].update(ids)
        # The three tiers of the dais are separately checked by the voxel graph.
        dais_entry = (u(2), u(5), u(2.5))
        dais_top = (u(2), u(6.5), u(7))
        if dais_entry not in components or dais_top not in components:
            raise ValueError('Allée de l’estrade trop étroite')
        dais_nodes = [dais_entry, (u(2), u(5.5), u(4)), (u(2), u(6), u(5.5)), dais_top, (u(-3.5), u(6.5), u(7))]
        if any(p not in components for p in dais_nodes):
            raise ValueError('Degré de l’estrade trop étroit')
        for p in dais_nodes:
            edges[components[p]].update(components[q] for q in dais_nodes)
        root = (entry[0]-1, entry[1], entry[2]-1)
        if root not in components:
            raise ValueError('Parvis trop étroit')
        joined = {components[root]}
        queue = deque(joined)
        while queue:
            a = queue.popleft()
            for b in edges[a]:
                if b not in joined:
                    joined.add(b)
                    queue.append(b)
        for room in self.rooms:
            p = room['seed']
            q = p[0]-1, p[1], p[2]-1
            if q not in components or components[q] not in joined:
                raise ValueError(f"Accès de largeur 1,5 m insuffisant: {room['name']} {q}; surface={q in components}")
            room['wideFloorComponent'] = components[q]
            room['clearFloorAreaM2'] = sum((x, room['level'], z) in surfaces for x, z in room['footprint'])/4
            room['nominalAreaM2'] = len(room['footprint'])/4
            room['features'] = []
            for f in self.features:
                if f['low'][1] < room['level'] or f['low'][1] >= room['level']+10:
                    continue
                lo, hi = f['low'], f['high']
                if any((x, z) in room['footprint'] for x, z in v1.rect(lo[0], lo[2], hi[0], hi[2])):
                    room['features'].append(f['name'])
        unused = set(components.values())-joined
        unused_patches = []
        if unused:
            for component in sorted(unused):
                points = [p for p, c in components.items() if c == component]
                if not all(p[1] == 0 and any((p[0], p[2]) in r(cx-1.5, 12.5, cx+1.5, 14.5) for cx in (-8, 8)) for p in points):
                    raise ValueError(f'Plancher large isolé hors bassin décoratif: {component} {points[:4]}')
                lo, hi = bbox(points)
                unused_patches.append({'nodes': len(points), 'low': lo, 'high': hi})
        low, high = bbox(self.cells)
        clo = [min([low[a]]+[s['low'][a] for s in self.spaces]) for a in range(3)]
        chi = [max([high[a]]+[s['high'][a] for s in self.spaces]) for a in range(3)]
        return {'result': 'PASS', 'revision': 2, 'cells': len(self.cells),
                'stairs': len(self.stairs), 'stairKinds': dict(Counter(s['kind'] for s in self.stairs)),
                'reservedSpaces': len(self.spaces), 'reservedColumns': columns,
                'roomsAndDestinations': len(self.rooms), 'reachableRoomsAndDestinations': len(self.rooms),
                'features': len(self.features), 'protectedRoomAisles': len(self.lanes),
                'surfaceGraphNodes': len(surfaces), 'entryComponentNodes': len(reached),
                'wideFootprintMeters': [1.5, 1.5], 'wideFloorNodes': len(wide),
                'wideFloorComponents': component_count, 'connectedWideFloorComponents': len(joined),
                'smallUnlinkedWidePatches': unused_patches,
                'isolatedFloorSurfaces': 0, 'componentVoxelOverlaps': 0,
                'intersectingStairBodies': 0, 'clippedStairSupportCells': 0,
                'boundsMeters': [[v/2 for v in low], [v/2 for v in high]],
                'dimensionsMeters': [(high[a]-low[a])/2 for a in range(3)],
                'requiredClearanceBoundsMeters': [[v/2 for v in clo], [v/2 for v in chi]],
                'minimumValidationVolumeMeters': [(chi[a]-clo[a])/2 for a in range(3)],
                'nominalMainFloorSurfacesMeters': [0, 5, 10], 'headroomMeters': 3,
                'stairHeadroomMeters': 3.5,
                'stairClearWidthMeters': 2.5, 'monumentalCentralWidthMeters': 5,
                'landingDepthMeters': 3, 'towerDiameterMeters': 16, 'mainAisleWidthMeters': 3,
                'geometricGraphMaximumRiseMeters': .5,
                'occupiedGroundAreaM2': len({(x, z) for x, y, z in self.cells if y == -1})/4,
                'nativePieces': 0, 'objects': 0, 'roleCells': dict(Counter(value[0] for value in self.cells.values())),
                'gameplayTested': False, 'smoothStairTraversalCertified': False,
                'altarCoverage': 'UNKNOWN — native in-game validation required', 'palette': self.palette}


def export(castle, output):
    origin = tuple(min(p[a] for p in castle.cells) for a in range(3))
    groups = defaultdict(dict)
    # 40^3 = 64,000 dense cells, below the unchanged 65,536 limit. General
    # spatial modules retain every palette entry; room boundaries are unrelated
    # to file boundaries. This avoids increasing the 128-source engine limit.
    for p, value in castle.cells.items():
        tile = tuple((p[a]-origin[a])//40 for a in range(3))
        groups[tile][p] = value
    if len(groups) > 128:
        raise ValueError('Trop de sources BTBP')
    modules = []
    for index, (tile, points) in enumerate(sorted(groups.items())):
        low, high = bbox(points)
        name = f'FR2_{index:03d}_Secteur_{tile[0]}_{tile[1]}_{tile[2]}.btbp'
        (output/name).write_bytes(v1.encode_blueprint(points, low, high, castle.palette))
        modules.append({'file': name, 'offset': [v/2 for v in low], 'rotation': 0, 'category': 'general'})
    return {'format': 'BuildingToolsExtendedProject', 'version': 1, 'name': NAME,
            'gridSize': .5, 'anchor': [0, 0, 0], 'modules': modules,
            'spaces': [{'low': [v/2 for v in s['low']], 'high': [v/2 for v in s['high']]} for s in castle.spaces]}


def create(output, palette=None, render=False):
    if output.exists():
        raise FileExistsError('Refus de remplacer le dossier existant: '+str(output))
    castle = SpaciousCastle(v1.host('--profiles-spacious')['profiles'], v1.palette_from(palette)).build()
    validation = castle.validate()
    output.mkdir(parents=True, exist_ok=False)
    manifest = export(castle, output)
    pending = output/(PROJECT+'.pending')
    v1.write_json(pending, manifest)
    relative = str(pending.resolve().relative_to(ROOT)).replace('\\', '/')
    native = v1.host('--validate', relative)
    if native['cells'] != validation['cells']:
        raise ValueError('Relecture différente de la géométrie générée')
    validation['productionReader'] = native
    validation['modules'] = len(manifest['modules'])
    pending.rename(output/PROJECT)
    v1.write_json(output/'Validation.json', validation)
    v1.write_json(output/'Circulation.json', {'rooms': [{k: value for k, value in room.items() if k != 'footprint'} for room in castle.rooms],
                                             'stairs': castle.stairs, 'features': castle.features, 'cages': castle.cages})
    reports(castle, validation, output)
    plans(castle, output)
    if render:
        render_views(castle, output)
    checksums(output)
    print(json.dumps({'output': str(output), **validation}, ensure_ascii=False, indent=2))
    return castle, validation


def checksums(output):
    data = ''.join(f'{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.name}\n' for p in sorted(output.iterdir())
                   if p.is_file() and p.name != 'SHA256SUMS.txt')
    (output/'SHA256SUMS.txt').write_text(data, encoding='ascii')


def package(output, archive):
    archive.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(archive, 'x', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for file in sorted(output.iterdir()):
            folder = 'BuildingTools-Projects' if file.suffix in ('.btbp', '.btproject') else 'Forteresse-Royale-V2-Guide'
            z.write(file, folder+'/'+file.name)


def reports(castle, report, output):
    dimensions = ' × '.join(f'{v:g}' for v in report['dimensionsMeters'])
    volume = ' × '.join(f'{v:g}' for v in report['minimumValidationVolumeMeters'])
    rows = []
    for index, room in enumerate(castle.rooms, 1):
        rows.append(f'| {index} | {room["name"]} | {room["level"]/2:g} | {room["nominalAreaM2"]:g} | {room["clearFloorAreaM2"]:g} | {len(room["features"])} |')
    table = '\n'.join(rows)
    (output/'Architecture.md').write_text(f'''# Forteresse Royale V2 — grands intérieurs

La V1 a été parcourue en jeu par le joueur : l'extérieur est apprécié, mais les
tours, les escaliers et les circulations intérieures sont trop serrés. Cette V2
reprend la composition royale et la redessine pièce par pièce. **La V2 n'a pas
encore été testée en jeu.** Tous les blocs restent des cubes de **0,5 m**.

## Dimensions et confort prévu

| Élément | V1 | V2 |
|---|---|---|
| Emprise extérieure | 45 × 42,5 m | 77 × 72 m |
| Corps royal et ailes | 36 × 30 m | 56 × 48 m |
| Tours d'angle octogonales | 8 m | 16 m |
| Donjon | 10 × 10 m | 18 × 18 m |
| Hall central | 18 × 12 m | 28 × 20 m |
| Salle du trône | 18 × 10 m | 28 × 14 m |
| Hauteur d'étage | 4,5 m | 5 m |
| Largeur des volées secondaires | 1,5 m | 2,5 m utiles, garde-corps à l'extérieur |
| Profondeur des paliers | 1,5 m | 3 m, avec approches de 2,5 m |
| Hauteur libre réservée aux escaliers | 2,5 m | 3,5 m |

Dimensions construites X × Y × Z : **{dimensions} m**. Les sols principaux sont
à 0 / 5 / 10 m. Les galeries de toiture sont à 15 m, les couronnes des tours à
20 m, et le donjon possède des sols jusqu'à 30 m. Son couronnement monte à 32 m.
Le volume à contrôler, air compris, mesure **{volume} m**.

## Implantation des pièces

Le hall à double hauteur reçoit la volée impériale de 5 m, ses deux branches de
2,5 m et une galerie large sur les côtés. Les arcades et colonnes sont placées
hors des débouchés. La réception et le trône occupent les grandes salles arrière.
La salle du trône conserve une nef libre de 6 m, des colonnes séparées de
l'estrade et trois degrés de 1,5 m de giron avant le plateau. Deux accès latéraux
au plateau sont contrôlés.

Les ailes possèdent des galeries de circulation avec **3 m d'allée protégée**.
Les pièces avant et arrière mesurent 9,5 × 16 m nominaux ; les vestibules
d'escalier centraux mesurent 14 × 16 m. Les portes ordinaires font 3 m, les
principales 4 à 6 m. Chaque salle a son axe d'accès, ses zones d'aménagement et
un point de contrôle situé dans son propre volume. Les bibliothèques gardent
leurs rayonnages sur les côtés, les salles de conseil et de banquet placent
leurs tables après l'allée transversale, et les trophées restent en périphérie.
Les appartements disposent d'alcôves et de cabinets en retrait.

Les cages des tours sont décalées vers l'extérieur du corps royal : leurs volées
n'occupent pas les salles des ailes. Les tours possèdent des pièces sur le côté,
des banquettes et des paliers transversaux. Le donjon sépare sa cage de sa grande
allée de 3 m et de ses banquettes. La terrasse a été avancée pour dégager le
passage devant le donjon. La cour conserve son axe central entre deux bassins secs.

## Structure des escaliers

{report['stairs']} parcours : un monumental, deux L, vingt-quatre U. Les profils
proviennent de `voxel::prepare` en production : dix contremarches de 0,5 m,
giron de 0,5 m. Les volées sont des bandes de pierre de 1 m d'épaisseur, reliées
aux paliers ; les U ont un noyau porteur continu et des garde-corps extérieurs.
Les soutiens pleins empilés de la V1 ont été remplacés. **Aucun soutien n'est
découpé et aucun tablier n'est supprimé pour loger une autre volée.** Les cages,
les corps physiques des escaliers et leurs dégagements sont vérifiés séparément.

Les marches de 0,5 m peuvent toujours demander des sauts dans Enshrouded. La
grille, les dégagements et les graphes ne certifient pas la marche du personnage.

## Aménagement et limites

{report['features']} éléments intérieurs sont construits en voxels : bases,
colonnes, chapiteaux, rayonnages, tables, alcôves, banquettes, présentoirs, estrade
et trône. Ils sont décoratifs : aucun lit utilisable, atelier fonctionnel, feu,
livre, trophée natif ou porte interactive n'est prétendu placé. Le mobilier en
blocs peut servir de base à un ameublement ultérieur dans le jeu.

La Pierre des Anciens confirmée localement reste la substitution explicite pour
la maçonnerie, la taille, les toitures et le décor sombre. Une vraie palette peut
être fournie via `--palette` et des petits BTBP exportés du jeu. Les couleurs
des vues distinguent les rôles architecturaux, elles ne simulent pas les textures.

## Inventaire vérifié pièce par pièce

La surface libre ci-dessous est l'aire des colonnes disposant d'un sol et de 3 m
d'air ; elle ne constitue pas une mesure de surface habitable réglementaire.
Tours et donjon incluent leurs circulations ; l'estrade est une destination dans
la salle du trône. `Pieces.csv` et `Circulation.json` donnent les accès et éléments.

| N° | Pièce / destination | Sol (m) | Surface nominale (m²) | Sol libre avec 3 m d'air (m²) | Éléments |
|---|---|---|---|---|---|
{table}
''', encoding='utf-8', newline='\n')
    (output/'Validation.md').write_text(f'''# Validation technique V2

**PASS hors jeu ; recette V2 en jeu non effectuée.** Le retour du joueur porte
sur la V1 et a motivé le redessin des intérieurs.

| Contrôle exécuté | Résultat |
|---|---|
| Cellules / modules BTBP | {report['cells']} / {report['modules']} |
| Espaces réservés / colonnes | {report['reservedSpaces']} / {report['reservedColumns']} |
| Pièces et destinations reliées | {report['reachableRoomsAndDestinations']} / {report['roomsAndDestinations']} |
| Éléments intérieurs / allées protégées | {report['features']} / {report['protectedRoomAisles']} |
| Graphe des surfaces / composante depuis l'entrée | {report['surfaceGraphNodes']} / {report['entryComponentNodes']} |
| Empreinte du second contrôle de circulation | 1,5 × 1,5 m, 3 m de hauteur |
| Chevauchements physiques entre composants | 0, même matériau inclus |
| Corps d'escaliers intersectés / soutiens découpés | 0 / 0 |
| Pièces natives expérimentales / objets | 0 / 0 |
| Lots théoriques à 128 cellules | {report['productionReader']['batches128']} |

Le premier contrôle vérifie les volumes occupés, les réservations d'air, chaque
colonne de soutien, les allées de 3 m et les cages protégées. Le placement d'un
meuble dans une cage est refusé même s'il n'occupe pas encore la cellule d'une
marche. Tout chevauchement avec un autre composant est refusé, sans exception
fondée sur l'identité de matériau. Les jonctions de maçonnerie et de dalles sont
attribuées une seule fois lors du calcul de la structure.

Le graphe des surfaces vérifie toutes les marches et tous les paliers. Le second
contrôle utilise des empreintes **1,5 × 1,5 m** sur les sols plats, puis raccorde
les composantes par les escaliers et leurs paliers vérifiés. Il contrôle les
accès des pièces, les arrivées et les changements de direction. Les corniches,
toitures et appuis de fenêtres ne servent pas de chemins. Les intérieurs des
deux bassins décoratifs sont exclus du réseau large, et sont explicitement
signalés dans `Validation.json`. Aucune pièce ni aucun palier n'est exclu.

La relecture C++ emploie `core::bp_decode`, `project::add/finish` et
`spaces::validate`, puis vérifie les quatre rotations et les lots 1/32/128/256
avec interruptions de la machine d'état. Chaque BTBP conserve une grille de
0,5 m et au plus 64 000 cellules denses. Le plafond 128 modules reste inchangé.
Les modules d'export sont des secteurs spatiaux de catégorie `general` ; les
rôles de matériaux restent configurables au moment de la génération.

Ces contrôles ne reproduisent pas la capsule, le pas maximal ou la physique du
personnage. La pratique des marches, la stabilité, les collisions du monde,
les ressources et la couverture des autels restent à tester en jeu. La reprise
automatique et le rollback complet restent indisponibles dans dev7.
''', encoding='utf-8', newline='\n')
    (output/'Placement.md').write_text(f'''# Placement de la forteresse spacieuse V2

Projet à sélectionner : **Forteresse_Royale_Spacieuse_V2**.
Compatible avec la DLL **0.34.0-dev7** existante ; aucune nouvelle DLL nécessaire.

1. Installer tous les `FR2_*.btbp` et `{PROJECT}` directement dans
   `BuildingTools-Projects`, à côté de `enshrouded.exe`. Le ZIP possède déjà cette
   arborescence. La V1 et ses fichiers `FR_*.btbp` peuvent être conservés.
2. Prévoir un site de test libre d'environ **80 × 75 m**, avec **36 m** de
   hauteur libre et couverte. Emprise construite : 77 × 72 m ; volume d'air à
   valider : {volume} m (X × Y × Z). Pivot au sol central, entrée vers -Z.
   Bornes occupées : {report['boundsMeters']}. Avec l'air :
   {report['requiredClearanceBoundsMeters']}. Le sol inférieur est à Y=-0,5 m.
3. Prendre le cube de **0,5 m**, ouvrir le sélecteur de matériaux dans la base,
   puis Blueprint → Placer → Projets composites. Choisir le projet V2 et
   positionner l'ensemble. Le placement natif expérimental reste inutile.
4. Vérifier les compteurs : **{report['cells']} cellules**, {report['modules']}
   BTBP, {report['reservedSpaces']} dégagements, zéro pièce native et zéro objet.
   Examiner `Plans.svg`, les coupes du hall et des tours et `Pieces.csv`.
5. Analyser la couverture des autels, notamment jusqu'à Y=33,5 m au-dessus du
   pivot pour les dégagements du donjon. Les contraintes natives sont conservées.
   Le nombre exact d'autels dépend de leurs volumes réellement disponibles.
6. Cliquer **Vérifier sans construire** et attendre la fin. Résoudre tout refus
   de collision, couverture, matériau ou ressources avant la construction.
7. Cliquer **Construire une seule fois**. Tous les secteurs sont assemblés
   automatiquement ; à 128 cellules, {report['productionReader']['batches128']}
   lots théoriques. Le moteur peut réduire la taille des lots.

## Recette intérieure prioritaire

Tester les deux branches impériales, les L et U des ailes, puis chaque tour.
Vérifier les 2,5 m entre garde-corps, les paliers de 3 m, les arrivées à chaque
sol et la possibilité de tourner sans se coincer. Parcourir chaque pièce des
trois niveaux, la bibliothèque, les chambres, les trophées et les deux côtés
de l'estrade. Rejoindre les terrasses, les couronnes et le sommet du donjon.
Noter les sauts nécessaires, accrochages, angles étroits et éventuels obstacles.
Le confort réel de cette V2 reste à confirmer par ce test.

Échap, perte de focus ou erreur peuvent laisser un chantier partiel. Les lots
acceptés restent ; aucune reprise automatique garantie ni restauration totale
n'est fournie. Conserver les journaux et utiliser une copie de monde de test.
Le générateur et les tests hors jeu ne modifient aucune sauvegarde.
''', encoding='utf-8', newline='\n')
    with (output/'Pieces.csv').open('w', encoding='utf-8-sig', newline='') as file:
        writer = csv.writer(file, delimiter=';')
        writer.writerow(['Numero', 'Piece', 'Sol_m', 'Surface_nominale_m2', 'Surface_libre_3m_m2', 'Acces_controle_1_5m', 'Elements_constructibles', 'Programme'])
        for index, room in enumerate(castle.rooms, 1):
            writer.writerow([index, room['name'], room['level']/2, room['nominalAreaM2'], room['clearFloorAreaM2'],
                             'PASS', ' | '.join(room['features']), room['programme']])
    (output/'README.txt').write_text(f'''FORTERESSE ROYALE SPACIEUSE V2
Projet unique : {PROJECT}
{report['cells']} cellules ; {report['modules']} dépendances FR2_*.btbp ; 27 escaliers.
Tous les fichiers de construction vont directement dans BuildingTools-Projects.
Le projet V1 peut être conservé. DLL 0.34.0-dev7 inchangée, cubes de 0,5 m.
Lire Placement.md, Architecture.md, Validation.md et Pieces.csv avant le test.
Vue_Sud.png et Vue_Nord.png : extérieur. Vue_Hall_coupe.png et Vue_Tour_coupe.png :
coupes des voxels réels, avec murs/planchers masqués pour inspecter l'intérieur.
Les couleurs des images indiquent les rôles, pas les textures réelles du jeu.
Le passage du personnage sur les marches de 0,5 m reste à valider dans Enshrouded.
''', encoding='utf-8', newline='\n')


def plans(castle, output):
    import html
    colors = {'masonry': '#697b89', 'trim': '#bba87d', 'throne': '#87649e', 'roof': '#45667c'}
    parts = ['<svg xmlns="http://www.w3.org/2000/svg" width="1600" height="2200" viewBox="0 0 1600 2200">',
             '<rect width="1600" height="2200" fill="#101b28"/>',
             '<style>text{font-family:Segoe UI,Arial,sans-serif;fill:#e5e0d3}</style>',
             '<text x="40" y="40" font-size="25">Forteresse Royale V2 — plans des voxels réels</text>',
             '<text x="40" y="70" font-size="16">Nord = +Z · or : escaliers · vert : allées protégées · numéros : Pieces.csv · grille 0,5 m</text>']
    for index, level in enumerate((0, 10, 20, 30, 40, 60)):
        ox, oy, scale = 50+(index % 2)*790, 110+(index//2)*680, 4.25
        def pos(x, z):
            return ox+(x+79)*scale, oy+(77-z)*scale
        parts.append(f'<text x="{ox}" y="{oy}" font-size="23">Sol à {level/2:g} m</text>')
        for (x, y, z), (role, category, owner) in sorted(castle.cells.items()):
            if y not in (level-1, level+1):
                continue
            px, py = pos(x, z+1)
            color = '#c7a465' if category == 'stairs' else colors[role] if y == level+1 else '#293d4c'
            parts.append(f'<rect x="{px:.1f}" y="{py:.1f}" width="{scale}" height="{scale}" fill="{color}"/>')
        for lane in castle.lanes:
            if lane['level'] != level:
                continue
            for x, z in sorted(lane['footprint']):
                px, py = pos(x, z+1)
                parts.append(f'<rect x="{px:.1f}" y="{py:.1f}" width="{scale}" height="{scale}" fill="#56b58f" opacity=".45"/>')
        for number, room in enumerate(castle.rooms, 1):
            x, y, z = room['seed']
            if y == level:
                px, py = pos(x+.5, z+.5)
                parts.append(f'<circle cx="{px}" cy="{py}" r="8" fill="#e0d9c4"/><text x="{px}" y="{py+4}" text-anchor="middle" font-size="11" style="fill:#101b28">{number}</text>')
        parts.append(f'<path d="M{ox},{oy+642}h{20*scale}" stroke="#d0b783" stroke-width="3"/><text x="{ox}" y="{oy+665}" font-size="14">10 m · largeur de circulation auditée séparément</text>')
    parts.append('</svg>')
    (output/'Plans.svg').write_text('\n'.join(parts), encoding='utf-8', newline='\n')


def render_views(castle, output):
    hall = {p: value for p, value in castle.cells.items()
            if -28 <= p[0] < 26 and -46 <= p[2] < 20 and p[1] < 19
            and not (value[1] == 'floor' and p[1] == 9 and p[2] < -37)
            and not (value[2] == 'Parapets des galeries et terrasses' and p[2] < -37)}
    tower = {p: value for p, value in castle.cells.items()
             if (p[0], p[2]) in castle.towers['SO'] and -75 < p[0] < -46 and -65 < p[2] < -36 and p[1] < 10
             and not (value[1] == 'floor' and p[1] == 9)
             and value[2] != 'Tour SO U 5'}
    throne = {p: value for p, value in castle.cells.items()
              if -28 <= p[0] < 26 and -7 <= p[2] < 20 and 9 <= p[1] < 19}
    castle.render_title = 'FORTERESSE ROYALE V2'
    castle.render_views = [('Sud', 1, -1, castle.cells), ('Nord', -1, 1, castle.cells),
                           ('Hall_coupe', 1, -1, hall), ('Tour_coupe', 1, -1, tower), ('Trone_coupe', 1, -1, throne)]
    v1.render_castle(castle, output)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True, help='NEW directory inside the repo')
    parser.add_argument('--palette', type=Path)
    parser.add_argument('--render', action='store_true')
    parser.add_argument('--zip', type=Path)
    args = parser.parse_args()
    try:
        if args.zip and args.zip.exists():
            raise FileExistsError('Archive existante: '+str(args.zip))
        create(args.output, args.palette, args.render)
        if args.zip:
            package(args.output, args.zip)
    except (OSError, ValueError) as error:
        parser.exit(1, str(error)+'\n')


if __name__ == '__main__':
    main()
