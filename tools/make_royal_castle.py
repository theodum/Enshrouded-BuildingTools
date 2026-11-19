#!/usr/bin/env python3
"""Rule-based royal castle for the unchanged dev7 composite backend.

All authoring coordinates are integer 0.5 m voxels. Stair layouts come from
castle_host, compiled against src/voxel_stairs.h. No game or save writes.
Output must be a new directory; the manifest is published after validation.
"""
from __future__ import annotations

import argparse
from collections import Counter, defaultdict, deque
import hashlib
import json
import math
from pathlib import Path
import struct
import subprocess
import sys
import zipfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from make_project_example import material_from

NAME = 'Forteresse Royale — Couronne des Anciens'
PROJECT = 'Forteresse_Royale_Demo.btproject'
ROLES = ('masonry', 'trim', 'roof', 'throne')
CATEGORIES = ('general', 'wall', 'floor', 'roof', 'tower', 'stairs', 'furniture')


def host_command():
    if sys.platform == 'win32':
        return ['wsl', '-d', 'Ubuntu-24.04', '--cd', str(ROOT), '--', './build/castle_host']
    return [str(ROOT / 'build/castle_host')]


def host(*args):
    result = subprocess.run(host_command() + list(args), cwd=ROOT, capture_output=True, text=True)
    if result.returncode:
        raise ValueError('Production host validation: ' + result.stderr.strip())
    return json.loads(result.stdout)


def cells_in(low, high):
    return ((x, y, z) for y in range(low[1], high[1])
            for z in range(low[2], high[2]) for x in range(low[0], high[0]))


def rect(x0, z0, x1, z1):
    return {(x, z) for x in range(x0, x1) for z in range(z0, z1)}


def octagon(cx, cz, radius=8):
    # Cell centres, symmetric about the exact global tower centre.
    return {(x, z) for x in range(cx-radius, cx+radius) for z in range(cz-radius, cz+radius)
            if abs(2*x+1-2*cx) + abs(2*z+1-2*cz) <= 3*radius}


def border(footprint, thickness=1):
    current = footprint
    for _ in range(thickness):
        current = {(x, z) for x, z in current
                   if all((x+dx, z+dz) in current for dx, dz in ((1, 0), (-1, 0), (0, 1), (0, -1)))}
    return footprint - current


class Castle:
    def __init__(self, profiles, palette):
        self.profiles = profiles
        self.palette = palette
        self.cells = {}  # (x,y,z) -> (material role, category, architectural owner)
        self.air = set()
        self.spaces = []
        self.stairs = []
        self.portals = []
        self.rooms = []
        self.unions = 0
        self.support_arch_cells = 0
        self.floor_planes = defaultdict(set)
        self.main = rect(-18, -30, 18, 14)
        self.west = rect(-36, -30, -18, 30)
        self.east = rect(18, -30, 36, 30)
        self.gate = rect(-8, -35, 8, -30)
        self.keep = rect(-10, 24, 10, 44)
        self.towers = {n: octagon(x, z) for n, x, z in (
            ('SO', -36, -30), ('SE', 36, -30), ('NO', -36, 30), ('NE', 36, 30))}
        self.tower_all = set().union(*self.towers.values())
        self.body = self.main | self.west | self.east | self.gate
        self.court = rect(-18, 14, 18, 30) - self.keep
        self.footprint = self.body | self.keep | self.tower_all | self.court | rect(-12, -40, 12, -30)

    def put(self, p, role='masonry', category='wall', owner='shell'):
        if p in self.air:
            raise ValueError(f'{owner} obstructs reserved air at {p}')
        value = (role, category, owner)
        previous = self.cells.get(p)
        if previous:
            # Only identical real materials may be geometrically united. Export
            # emits a single cell, and retains its first semantic owner.
            if self.palette[previous[0]]['pair'] != self.palette[role]['pair']:
                raise ValueError(f'Contradictory overlap at {p}: {previous} / {value}')
            self.unions += 1
            return
        self.cells[p] = value

    def box(self, low, high, role='masonry', category='wall', owner='shell'):
        for p in cells_in(low, high):
            self.put(p, role, category, owner)

    def space(self, low, high, label):
        low, high = tuple(low), tuple(high)
        if not 4 <= high[1] - low[1] <= 16:
            raise ValueError('Invalid reserved headroom: ' + label)
        self.air.update(cells_in(low, high))
        self.spaces.append({'low': low, 'high': high, 'label': label})

    def stair(self, name, kind, origin):
        profile = self.profiles[kind]
        surfaces = []
        for i, surface in enumerate(profile['surfaces']):
            low = tuple(surface['low'][a] + origin[a] for a in range(3))
            high = tuple(surface['high'][a] + origin[a] for a in range(3))
            self.space((low[0], high[1], low[2]), (high[0], high[1]+profile['headroom'], high[2]), f'{name}/{i}')
            surfaces.append({'low': low, 'high': high, 'landing': surface['landing']})
        self.stairs.append({'name': name, 'kind': kind, 'origin': origin, 'surfaces': surfaces})

    def portal(self, name, axis, plane, centre, level, width=4, height=6, depth=3):
        # Full rectangular 2 m air core; pointed arch above it is a real void.
        if axis == 'z':
            low = (centre-width//2, level, plane-depth//2)
            high = (centre+width//2, level+4, plane+(depth+1)//2)
        else:
            low = (plane-depth//2, level, centre-width//2)
            high = (plane+(depth+1)//2, level+4, centre+width//2)
        self.space(low, high, name)
        self.portals.append({'name': name, 'axis': axis, 'plane': plane, 'centre': centre,
                             'level': level, 'width': width, 'height': height, 'depth': depth})

    def opening(self, x, y, z):
        for p in self.portals:
            cross, normal = (x, z) if p['axis'] == 'z' else (z, x)
            h = y - p['level']
            if abs(normal-p['plane']) <= p['depth']//2+1 and 0 <= h < p['height']:
                half = p['width']//2
                inset = max(0, h - (p['height']-half))
                if p['centre']-half+inset <= cross < p['centre']+half-inset:
                    return True
        return False

    def room(self, name, x, level, z, kind='room'):
        self.rooms.append({'name': name, 'seed': (x, level, z), 'kind': kind})

    def prepare_circulation(self):
        self.stair('Escalier impérial du hall', 'monumental', (-3, 0, -20))
        for i in (1, 2):
            self.stair(f'Aile ouest L {i}', 'L', (-32, i*9, -2))
        self.stair('Aile est U 1', 'U', (25, 9, -2))
        for name, cx, cz in (('SO', -36, -30), ('SE', 36, -30), ('NO', -36, 30), ('NE', 36, 30)):
            for i in range(4):
                self.stair(f'Tour {name} U {i}', 'U', (cx-4, i*9, cz-1))
        for i in range(5):
            self.stair(f'Donjon U {i}', 'U', (1, i*9, 33))
        self.portal('Porte monumentale', 'z', -35, 0, 0, width=8, height=9, depth=6)
        self.portal('Vestibule vers hall', 'z', -30, 0, 0, width=8, height=9)
        self.portal('Hall vers réception', 'z', -6, 0, 0, width=8, height=7)
        self.portal('Galerie vers salle du trône', 'z', -6, 0, 9, width=8, height=7)
        self.portal('Archives vers appartements', 'z', -6, 0, 18, width=6)
        self.portal('Appartements communicants', 'x', 0, 7, 18)
        self.portal('Réception vers cour', 'z', 14, 0, 0, width=8, height=8)
        for level in (0, 9, 18):
            for side in (-1, 1):
                for z in (-25, 2):
                    self.portal(f'Aile {side}/{level}/{z}', 'x', side*18, z, level)
            if level in (9, 18):
                self.portal(f'Loggia entrée {level}', 'z', -30, 0, level, width=6)
        for side in (-1, 1):
            for z in (18, 26):
                self.portal(f'Arcade cour {side}/{z}', 'x', side*18, z, 0, width=4, height=7)
            for level in (0, 9, 18):
                for z in (-10, 10):
                    self.portal(f'Couloir aile {side}/{level}/{z}', 'z', z, side*21, level, width=4)
        for level in (0, 18, 27):
            self.portal(f'Donjon entrée {level}', 'z', 24, 0, level, width=4, height=6)

    def floors(self):
        all_floor = self.main | self.west | self.east | self.tower_all | self.keep
        for level in (0, 9, 18):
            footprint = all_floor | (self.gate if level <= 18 else set())
            if level == 0:
                footprint |= self.court | rect(-12, -40, 12, -30)
            if level == 9:
                # Gallery around the double-height entrance hall. The void is
                # independent of the automatically carved stair headroom.
                footprint -= rect(-12, -25, 12, -11)
            if level == 18:
                footprint |= rect(-36, 22, 36, 30)
            self.floor(footprint, level, f'Plancher {level/2:g} m')
        # Roof walks: a U around the open courtyard and the main roof.
        walk = (rect(-36, -30, -18, -23) | rect(-23, -30, -18, 30) |
                rect(18, -30, 36, -23) | rect(18, -30, 23, 30) | rect(-36, 22, 36, 30))
        self.floor(walk | self.tower_all | self.keep, 27, 'Chemin de ronde et terrasse')
        self.floor(self.tower_all | self.keep, 36, 'Couronnes des tours')
        self.floor(self.keep, 45, 'Sommet panoramique du donjon')

    def floor(self, footprint, level, owner):
        for x, z in sorted(footprint):
            p = (x, level-1, z)
            if p in self.air:
                continue
            if level == 9 and (x, z) in self.main and (x, z) in rect(-12, -25, 12, -11):
                continue
            self.put(p, 'masonry', 'floor', owner)
            self.floor_planes[level].add((x, z))

    @staticmethod
    def lancet(cross, y, centre, base, width=3, height=5):
        h = y-base
        if not 0 <= h < height:
            return False
        inset = max(0, h-height+2)
        return centre-width//2+inset <= cross < centre+(width+1)//2-inset

    def window(self, x, y, z):
        # Windows follow occupied storeys and never straddle their floor bands.
        for level in (0, 9, 18):
            if z in (-30, 13) and any(self.lancet(x, y, c, level+2) for c in (-13, -7, 7, 13)):
                return True
            if x in (-36, 35) and any(self.lancet(z, y, c, level+2) for c in (-20, -3, 14, 22)):
                return True
            if x in (-19, 18) and z >= 14 and any(self.lancet(z, y, c, level+2) for c in (18, 26)):
                return True
        for cx, cz in ((-36, -30), (36, -30), (-36, 30), (36, 30)):
            for level in (0, 9, 18, 27):
                if (x in (cx-8, cx-7, cx+6, cx+7) and self.lancet(z, y, cz, level+3, 2, 4)) or (
                        z in (cz-8, cz-7, cz+6, cz+7) and self.lancet(x, y, cx, level+3, 2, 4)):
                    return True
        for level in (0, 9, 18, 27, 36):
            if x in (-10, -9, 8, 9) and any(self.lancet(z, y, c, level+2, 3, 5) for c in (29, 39)):
                return True
            if z in (24, 25, 42, 43) and any(self.lancet(x, y, c, level+2, 3, 5) for c in (-5, 5)):
                return True
        return False

    def shell(self):
        tower_footprint = self.tower_all | self.keep
        for y in range(45):
            footprint = self.keep.copy()
            if y < 36:
                footprint |= self.tower_all
            if y < 27:
                footprint |= self.main | self.west | self.east
            if y < 18:
                footprint |= self.gate
            # Union before erosion removes hidden walls at tower/wing junctions.
            thin, thick = border(footprint), border(footprint, 2)
            walls = thin | (thick & (self.tower_all | self.keep))
            for x, z in sorted(walls):
                if self.opening(x, y, z) or self.window(x, y, z):
                    continue
                category = 'tower' if (x, z) in tower_footprint else 'wall'
                self.put((x, y, z), 'masonry', category)
        # Internal walls keep a shared global frame and use the same portals.
        for level in (0, 9, 18):
            lines = []
            for x in (-18, 17):
                lines += [(x, z) for z in range(-30, 14)]
            lines += [(x, -6) for x in range(-18, 18)]
            for z in (-10, 10):
                lines += [(x, z) for x in list(range(-36, -18)) + list(range(18, 36))]
            if level == 18:
                lines += [(0, z) for z in range(-6, 14)]
            for y in range(level, level+8):
                for x, z in lines:
                    if not self.opening(x, y, z):
                        self.put((x, y, z), 'masonry', 'wall', 'Partitions et arcades')

    def emit_stairs(self):
        for stair in self.stairs:
            origin = stair['origin']
            decks = set()
            for surface in stair['surfaces']:
                lo, hi = surface['low'], surface['high']
                decks.update(cells_in((lo[0], hi[1]-1, lo[2]), hi))
            for cell in self.profiles[stair['kind']]['cells']:
                p = tuple(cell[a] + origin[a] for a in range(3))
                if p in self.air:
                    if p in decks:
                        raise ValueError('Stair deck obstructs another stair: ' + stair['name'])
                    # Superimposed storeys have open support arches below
                    # their decks. Only support infill is omitted, never a tread.
                    self.support_arch_cells += 1
                    continue
                self.put(p, 'masonry', 'stairs', stair['name'])

    def safe_detail(self, p, role='trim', category='wall', owner='Façades'):
        if p in self.air:
            return
        previous = self.cells.get(p)
        if previous:
            # Planned surface cladding. It replaces the single existing cell,
            # so no contradictory materials are exported at module junctions.
            if previous[0] in ('masonry', 'trim'):
                self.cells[p] = (role, previous[1], owner)
            return
        self.put(p, role, category, owner)

    def details(self):
        # Cornices follow the union boundary; portals and lancets remain open.
        for y in (7, 16, 25, 34, 43):
            footprint = self.keep | (self.tower_all if y < 36 else set())
            if y < 27:
                footprint |= self.main | self.west | self.east
            if y < 18:
                footprint |= self.gate
            for x, z in sorted(border(footprint)):
                if (x, y, z) not in self.cells:
                    continue
                for dx, dz in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    if (x+dx, z+dz) not in footprint:
                        self.safe_detail((x+dx, y, z+dz))
        for side in (-1, 1):
            # Stepped buttresses on lateral facades, clear of the windows.
            for z in (-24, -13, 5, 19, 27):
                for y in range(26):
                    projection = 3 if y < 3 else 2 if y < 17 else 1
                    for dx in range(projection):
                        for dz in range(2):
                            self.safe_detail((side*(36+dx) - (side > 0), y, z+dz))
            for x in (side*16, side*9):
                for y in range(26):
                    for dx in (0, 1):
                        for z in range(-32 if y < 4 else -31, -30):
                            self.safe_detail((x+dx, y, z))
        # Dressed-stone surrounds frame real lancet voids.
        for level in (0, 9, 18):
            for centre in (-13, -7, 7, 13):
                for y in range(level+1, level+8):
                    for x in range(centre-2, centre+3):
                        if self.window(x, y, -30):
                            continue
                        if x in (centre-2, centre+2) or y in (level+1, level+7):
                            self.safe_detail((x, y, -31), owner='Encadrements des lancettes')
        # Gate: paired jambs, stepped archivolt, flanking piers and royal lozenge.
        for x in (-10, -9, 8, 9):
            for y in range(17):
                for z in (-36, -35):
                    self.safe_detail((x, y, z), owner='Portail royal')
        for h in range(5):
            for side in (-1, 1):
                self.safe_detail((side*(5-h) - (side > 0), 5+h, -36), owner='Archivolte')
        for y in range(11, 16):
            for x in range(-3, 3):
                if abs(2*x+1)+abs(2*y-26) <= 6:
                    self.safe_detail((x, y, -36), 'throne', owner='Blason minéral')
        # Throne room: clear axial nave, paired piers, rear stepped dais and
        # a stone throne. No unverified prop IDs or decorative fake furniture.
        for x in (-13, 12):
            for z in (-1, 7):
                self.box((x, 9, z), (x+2, 10, z+2), 'trim', 'wall', 'Base des colonnes')
                self.box((x, 10, z), (x+2, 16, z+2), 'trim', 'wall', 'Colonnes du trône')
                self.box((x-1, 16, z-1), (x+3, 17, z+3), 'trim', 'wall', 'Chapiteaux')
        # Three shallow platform tiers leave the central entrance fully clear.
        for y, x0, x1, z0 in ((9, -7, 7, 9), (10, -6, 6, 10), (11, -5, 5, 11)):
            self.box((x0, y, z0), (x1, y+1, 13), 'throne', 'furniture', 'Estrade royale')
        self.box((-2, 12, 11), (2, 13, 13), 'throne', 'furniture', 'Trône minéral')
        self.box((-2, 13, 12), (2, 16, 13), 'throne', 'furniture', 'Dossier du trône')
        for x in (-3, 2):
            self.box((x, 12, 11), (x+1, 14, 13), 'trim', 'furniture', 'Accoudoirs')
        # Recessed library shelves and niches are constructed from voxels.
        for z in (12, 17, 21):
            for y in (19, 22, 25):
                self.box((-35, y, z), (-33, y+1, z+4), 'trim', 'furniture', 'Rayonnages de bibliothèque')
        # Court: low four-pier basin with a dry centre, no water/object dependency.
        for x, z in sorted(border(rect(-4, 17, 4, 23))):
            self.safe_detail((x, 0, z), 'trim', 'furniture', 'Bassin de cour')

    def parapet(self, footprint, level, name):
        for x, z in sorted(border(footprint)):
            for y in range(level, level+4):
                if y < level+2 or (x+z)//2 % 2 == 0:
                    self.safe_detail((x, y, z), 'trim', 'tower', name)

    def roofs(self):
        # Main gable: 1 m horizontal run per 0.5 m rise, real hollow roof.
        for x in range(-19, 19):
            y = 27 + min(x+19, 18-x)//2
            for z in range(-31, 15):
                self.safe_detail((x, y, z), 'roof', 'roof', 'Grand comble royal')
                # Vertical riser closes a one-cell gap between roof steps.
                if min(x+19, 18-x) % 2 == 0:
                    self.safe_detail((x, y-1, z), 'roof', 'roof', 'Grand comble royal')
        for z in (-30, 13):
            for x in range(-18, 18):
                top = 27 + min(x+19, 18-x)//2
                for y in range(27, top):
                    # A small circular oculus remains genuinely open.
                    if (2*x+1)**2 + (2*y-61)**2 <= 16:
                        continue
                    self.safe_detail((x, y, z), 'masonry', 'roof', 'Pignons ajourés')
        # Two lower, narrower gables leave a 2 m gallery on the courtyard side.
        for x0, x1 in ((-37, -24), (24, 37)):
            for x in range(x0, x1):
                top = 27 + min(x-x0, x1-1-x)
                for z in range(-23, 22):
                    self.safe_detail((x, top, z), 'roof', 'roof', 'Comble des ailes')
                    if top > 27:
                        self.safe_detail((x, top-1, z), 'roof', 'roof', 'Raccords des pans des ailes')
            for z in (-23, 21):
                for x in range(x0+1, x1-1):
                    for y in range(27, 27+min(x-x0, x1-1-x)):
                        self.safe_detail((x, y, z), 'masonry', 'roof', 'Pignons des ailes')
        for n, footprint in self.towers.items():
            self.parapet(footprint, 36, f'Couronne crénelée {n}')
        self.parapet(self.keep, 45, 'Couronne du donjon')
        # Southern towers carry open belvederes. The roof starts 2 m above the
        # summit deck; the stepped cap is never a traversable decorative floor.
        for cx in (-36, 36):
            for x, z in ((cx-5, -35), (cx+4, -35), (cx-5, -26), (cx+4, -26)):
                for y in range(36, 40):
                    self.safe_detail((x, y, z), 'trim', 'tower', 'Piliers du belvédère')
            for x, z in sorted(octagon(cx, -30, 8)):
                ring = max(abs(2*x+1-2*cx), abs(2*z+61))//2
                y = 40 + (7-ring)
                self.safe_detail((x, y, z), 'roof', 'roof', 'Coiffe du belvédère')
                if ring < 7:
                    self.safe_detail((x, y-1, z), 'roof', 'roof', 'Raccords de la coiffe')
        # Derive guards from actual floor boundaries, including the back of the
        # hall void, both sides of the loggia and the north side of each terrace.
        # An intact enclosing wall already provides the guard. Stair openings
        # remain clear under their explicit reservations.
        for level in (9, 18, 27):
            for x, z in sorted(border(self.floor_planes[level])):
                if (x, level, z) in self.cells:
                    continue
                for y in range(level, level+2):
                    self.safe_detail((x, y, z), owner='Garde-corps des bords exposés')

    def register_rooms(self):
        self.room('Parvis et entrée', 0, 0, -38, 'entry')
        self.room('Grand hall à double hauteur', 0, 0, -26)
        self.room('Salle de réception', 0, 0, 1)
        self.room('Cour des arcades', 10, 0, 19, 'court')
        self.room('Galerie du hall ouest', -15, 9, -21, 'gallery')
        self.room('Galerie du hall est', 14, 9, -21, 'gallery')
        self.room('Salle du trône', 0, 9, 1)
        self.room('Estrade du trône', 3, 12, 11, 'dais')
        self.room('Archives hautes', 0, 18, -20)
        self.room('Appartement royal ouest', -7, 18, 4)
        self.room('Appartement royal est', 7, 18, 4)
        labels = {0: ('Salle des gardes', 'Office ouest', 'Salle de réception ouest',
                      'Quartier de garde', 'Office est', 'Salle des banquets'),
                  9: ('Salon noble', 'Vestibule ouest', 'Cabinet du conseil',
                      'Salon des hôtes', 'Vestibule est', 'Galerie des portraits'),
                  18: ('Chambre ouest', 'Antichambre ouest', 'Bibliothèque',
                       'Chambre est', 'Antichambre est', 'Salle des trophées')}
        for level, names in labels.items():
            for side, offset in ((-1, 0), (1, 3)):
                for i, z in enumerate((-19, 7, 17)):
                    self.room(names[offset+i], side*26, level, z)
        for n, cx, cz in (('SO', -36, -30), ('SE', 36, -30), ('NO', -36, 30), ('NE', 36, 30)):
            for i in range(5):
                self.room(f'Tour {n} niveau {i}', cx+4, i*9, cz+2, 'tower')
        for i in range(6):
            self.room(f'Donjon niveau {i}', -5, i*9, 32, 'keep')
        for level in (18, 27):
            self.room(f'Terrasse arrière {level/2:g} m', -14, level, 26, 'terrace')
        self.room('Chemin de ronde ouest', -21, 27, 16, 'roofwalk')
        self.room('Chemin de ronde est', 20, 27, 16, 'roofwalk')
        for level in (9, 18):
            self.room(f'Loggia royale {level/2:g} m', 0, level, -33, 'balcony')

    def build(self):
        self.prepare_circulation()
        self.floors()
        self.shell()
        self.emit_stairs()
        self.details()
        self.roofs()
        self.register_rooms()
        return self

    def validate(self):
        if len(self.spaces) > 512 or len(self.cells) > 1048576:
            raise ValueError('Production budget exceeded')
        columns = 0
        for space in self.spaces:
            lo, hi = space['low'], space['high']
            area = (hi[0]-lo[0])*(hi[2]-lo[2])
            columns += area
            if area > 4096:
                raise ValueError('Space column budget exceeded')
            for p in cells_in(lo, hi):
                if p in self.cells:
                    raise ValueError(f"Obstructed opening {space['label']}: {p}")
            for x, z in rect(lo[0], lo[2], hi[0], hi[2]):
                if (x, lo[1]-1, z) not in self.cells:
                    raise ValueError(f"Missing passage surface {space['label']}: {(x, lo[1], z)}")
        if columns > 65536:
            raise ValueError('Cumulative space column budget exceeded')
        # Independently derive a headroom/surface graph from the final occupied
        # voxels. No declared room edge can make a blocked passage pass this test.
        surfaces = set()
        for (x, y, z), (_, category, owner) in self.cells.items():
            # Cornices, crenellations and window sills cannot substitute for an
            # intended room connection. Only authored floors, stair decks and
            # the royal dais participate in this geometric circulation graph.
            if category not in ('floor', 'stairs') and owner not in ('Estrade royale', 'Bassin de cour'):
                continue
            if all((x, y+h, z) not in self.cells for h in range(1, 5)):
                surfaces.add((x, y+1, z))
        entry = self.rooms[0]['seed']
        if entry not in surfaces:
            raise ValueError('Entry seed has no clear surface')
        reached = {entry: 0}
        queue = deque([entry])
        while queue:
            x, y, z = queue.popleft()
            for dx, dz in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                for dy in (0, 1, -1):
                    p = (x+dx, y+dy, z+dz)
                    if p in surfaces and p not in reached:
                        # Extra ceiling clearance for a 0.5 m change in level.
                        if dy == 1 and (x, y+4, z) in self.cells:
                            continue
                        if dy == -1 and (p[0], p[1]+4, p[2]) in self.cells:
                            continue
                        reached[p] = reached[(x, y, z)]+1
                        queue.append(p)
        disconnected = []
        for room in self.rooms:
            p = room['seed']
            if p not in reached:
                disconnected.append(f"{room['name']}: {p}; surface={p in surfaces}")
            else:
                room['surfaceGraphDistance'] = reached[p]
        if disconnected:
            for stair in self.stairs:
                counts = []
                for s in stair['surfaces']:
                    lo, hi = s['low'], s['high']
                    counts.append(sum((x, hi[1], z) in reached for x, z in rect(lo[0], lo[2], hi[0], hi[2])))
                disconnected.append(f"{stair['name']}: reached surface columns {counts}")
            raise ValueError('Unreachable destinations:\n' + '\n'.join(disconnected))
        # Every declared tread/landing must join the same geometric component.
        for stair in self.stairs:
            for surface in stair['surfaces']:
                lo, hi = surface['low'], surface['high']
                for x, z in rect(lo[0], lo[2], hi[0], hi[2]):
                    if (x, hi[1], z) not in reached:
                        raise ValueError(f"Disconnected stair surface: {stair['name']} {(x, hi[1], z)}")
        # Detect stranded occupied floors, rather than only checking room labels.
        orphan_floors = [p for p in surfaces if (p[0], p[1]-1, p[2]) in self.cells
                         and self.cells[(p[0], p[1]-1, p[2])][1] == 'floor' and p not in reached]
        if orphan_floors:
            raise ValueError(f'Isolated floor surfaces: {len(orphan_floors)}; first={orphan_floors[:6]}')
        low = [min(p[a] for p in self.cells) for a in range(3)]
        high = [max(p[a] for p in self.cells)+1 for a in range(3)]
        clearance_low = [min([low[a]] + [s['low'][a] for s in self.spaces]) for a in range(3)]
        clearance_high = [max([high[a]] + [s['high'][a] for s in self.spaces]) for a in range(3)]
        return {'result': 'PASS', 'cells': len(self.cells), 'stairs': len(self.stairs),
                'stairKinds': dict(Counter(s['kind'] for s in self.stairs)),
                'reservedSpaces': len(self.spaces), 'reservedColumns': columns,
                'roomsAndDestinations': len(self.rooms), 'reachableRoomsAndDestinations': len(self.rooms),
                'surfaceGraphNodes': len(surfaces), 'entryComponentNodes': len(reached),
                'isolatedFloorSurfaces': 0, 'contradictoryOverlaps': 0,
                'intentionalMaterialUnions': self.unions, 'omittedSupportInfillCells': self.support_arch_cells,
                'boundsMeters': [[v/2 for v in low], [v/2 for v in high]],
                'dimensionsMeters': [(high[a]-low[a])/2 for a in range(3)],
                'requiredClearanceBoundsMeters': [[v/2 for v in clearance_low], [v/2 for v in clearance_high]],
                'minimumValidationVolumeMeters': [(clearance_high[a]-clearance_low[a])/2 for a in range(3)],
                'occupiedGroundAreaM2': len({(x, z) for x, y, z in self.cells if y == -1})/4,
                'nominalMainFloorSurfacesMeters': [0, 4.5, 9],
                'geometricGraphMaximumRiseMeters': .5, 'headroomMeters': 2,
                'stairHeadroomMeters': 2.5,
                'gameplayTested': False, 'smoothStairTraversalCertified': False,
                'altarCoverage': 'UNKNOWN — native in-game validation required',
                'nativePieces': 0, 'objects': 0,
                'roleCells': dict(Counter(v[0] for v in self.cells.values()))}


def encode_blueprint(cells, low, high, palette):
    dims = [high[a]-low[a] for a in range(3)]
    total = math.prod(dims)
    if not 1 <= total <= 65536:
        raise ValueError(f'BTBP dense cell budget exceeded: {dims}')
    roles = sorted({v[0] for v in cells.values()})
    pairs = sorted({tuple(palette[r]['pair']) for r in roles})
    indices = {pair: i for i, pair in enumerate(pairs)}
    dense = bytearray([255])*total
    counts = Counter()
    for p, value in cells.items():
        x, y, z = (p[a]-low[a] for a in range(3))
        index = indices[tuple(palette[value[0]]['pair'])]
        dense[x+dims[0]*(z+dims[2]*y)] = index
        counts[index] += 1
    data = struct.pack('<4sHH3f3HIH', b'BTBP', 1, 0, .5, .5, .5, *dims, 0, len(pairs))
    for i, (item, material) in enumerate(pairs):
        data += struct.pack('<IBBHII', item, material, 0, 0, counts[i], counts[i])
    runs = []
    for value in dense:
        if runs and runs[-1][1] == value and runs[-1][0] < 65535:
            runs[-1][0] += 1
        else:
            runs.append([1, value])
    return data + struct.pack('<I', len(runs)) + b''.join(
        struct.pack('<HBB', count, value, 0) for count, value in runs) + struct.pack('<I', 0)


def export(castle, output):
    # Spatial tiles crossed with actual engine categories. A category can be
    # selected for material substitution in the unchanged dev7 UI.
    groups = defaultdict(dict)
    origin = tuple(min(p[a] for p in castle.cells) for a in range(3))
    for p, value in castle.cells.items():
        key = (value[1], *((p[a]-origin[a])//32 for a in range(3)))
        groups[key][p] = value
    if len(groups) > 128:
        raise ValueError(f'Too many source modules: {len(groups)}')
    modules = []
    labels = {'wall': 'Facades', 'floor': 'Planchers', 'roof': 'Toitures',
              'tower': 'Tours', 'stairs': 'Escaliers', 'furniture': 'Decors', 'general': 'Structure'}
    for i, (key, cells) in enumerate(sorted(groups.items())):
        category = key[0]
        low = tuple(min(p[a] for p in cells) for a in range(3))
        high = tuple(max(p[a] for p in cells)+1 for a in range(3))
        name = f'FR_{i:03d}_{labels[category]}.btbp'
        (output / name).write_bytes(encode_blueprint(cells, low, high, castle.palette))
        modules.append({'file': name, 'offset': [v/2 for v in low], 'rotation': 0, 'category': category})
    return {'format': 'BuildingToolsExtendedProject', 'version': 1, 'name': NAME,
            'gridSize': .5, 'anchor': [0, 0, 0], 'modules': modules,
            'spaces': [{'low': [v/2 for v in s['low']], 'high': [v/2 for v in s['high']]}
                       for s in castle.spaces]}


def write_json(path, value):
    path.write_text(json.dumps(value, indent=2, ensure_ascii=False)+'\n', encoding='utf-8', newline='\n')


def palette_from(config=None):
    material = material_from(ROOT / 'examples/three-modules/hall.btbp')
    palette = {role: {'pair': material, 'name': 'Bloc de pierre des Anciens',
                      'status': 'explicit-substitution', 'source': 'dev5 registered fixture; dev7 local log'}
               for role in ROLES}
    if config:
        data = json.loads(config.read_text(encoding='utf-8-sig'))
        if set(data)-set(ROLES):
            raise ValueError('Unknown material role')
        for role, settings in data.items():
            if not isinstance(settings, dict) or set(settings) != {'blueprint', 'name'}:
                raise ValueError('Each palette role needs blueprint and name')
            if not all(isinstance(settings[k], str) and settings[k] for k in ('blueprint', 'name')):
                raise ValueError('Palette blueprint and name must be nonempty strings')
            source = (config.parent / settings['blueprint']).resolve()
            palette[role] = {'pair': material_from(source), 'name': settings['name'],
                             'source': source.name, 'status': 'copied-from-user-blueprint; native revalidation required'}
    return palette


def create(output, config=None, render=False):
    if output.exists():
        raise FileExistsError('Refusing existing output directory: ' + str(output))
    profiles = host('--profiles')['profiles']
    castle = Castle(profiles, palette_from(config)).build()
    validation = castle.validate()
    output.mkdir(parents=True, exist_ok=False)
    manifest = export(castle, output)
    temporary = output / (PROJECT + '.pending')
    write_json(temporary, manifest)
    # Host runs in WSL on Windows; give it a path relative to its repo cwd.
    relative = str(temporary.resolve().relative_to(ROOT)).replace('\\', '/') if temporary.resolve().is_relative_to(ROOT) else str(temporary)
    if sys.platform == 'win32' and ':' in relative:
        raise ValueError('On Windows, output must be inside the repository for the WSL host')
    native = host('--validate', relative)
    if native['cells'] != validation['cells'] or native['spaces'] != validation['reservedSpaces']:
        raise ValueError('Export read-back differs from authoring geometry')
    validation['productionReader'] = native
    validation['modules'] = len(manifest['modules'])
    validation['palette'] = castle.palette
    temporary.rename(output / PROJECT)
    write_json(output / 'Validation.json', validation)
    write_json(output / 'Circulation.json', {'model': '0.5 m geometric surface graph, 2 m air; does not model player physics',
                                            'rooms': castle.rooms, 'stairs': castle.stairs, 'portals': castle.portals})
    write_json(output / 'Materiaux.json', castle.palette)
    reports(castle, validation, output)
    plans(castle, output)
    if render:
        render_castle(castle, output)
    checksums = ''.join(f'{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.name}\n'
                        for p in sorted(output.iterdir()) if p.is_file())
    (output / 'SHA256SUMS.txt').write_text(checksums, encoding='ascii')
    print(json.dumps({'output': str(output), **validation}, ensure_ascii=False, indent=2))
    return castle, validation


def reports(castle, v, output):
    dimensions = ' × '.join(f'{x:g}' for x in v['dimensionsMeters'])
    output.joinpath('README.txt').write_text(f'''FORTERESSE ROYALE — COURONNE DES ANCIENS
Projet unique : {PROJECT}
Compatible Building Tools Extended 0.34.0-dev7, déjà installé et testé par le joueur.
Aucune nouvelle DLL nécessaire. Aucun escalier natif expérimental.

Copier TOUS les FR_*.btbp et {PROJECT} directement dans BuildingTools-Projects,
à côté de enshrouded.exe. Ne pas copier un sous-dossier contenant ces fichiers.
Charger seulement Forteresse_Royale_Demo dans la liste des projets composites.
Consulter Placement.md, Architecture.md, Validation.md et Plans.svg avant construction.
Vue_Sud.png et Vue_Nord.png montrent l'extérieur ; Vue_Hall_coupe.png et
Vue_Trone_coupe.png sont des coupes avec murs/planchers masqués pour l'inspection.
Dimensions X × Y × Z : {dimensions} m. {v['cells']:,} cellules. {v['stairs']} escaliers.
{v['modules']} sources BTBP, assemblées automatiquement dans un même repère.
Les rapports et images sont facultatifs pour le mod, mais utiles pour la recette.
La pierre des Anciens remplace explicitement les matériaux nobles non identifiés.
Les contrôles géométriques réussis ne certifient pas la marche sans saut en jeu.
''', encoding='utf-8', newline='\n')
    output.joinpath('Architecture.md').write_text(f'''# Couronne des Anciens — rapport architectural

Forteresse compacte de **{dimensions} m** (X × Y × Z), corps principal nominal
36 × 30 m, avec quatre tours octogonales de 8 m et un donjon de 10 × 10 m.
Le nord du plan correspond à +Z ; l'entrée est au sud (-Z). Le pivot du projet
est le sol du hall : X=0, Y=0, Z=0. Une cellule vaut 0,5 m.

## Composition et silhouette

Le hall central mesure 18 × 12 m et la salle arrière 18 × 10 m. Les ailes
secondaires de 9 m encadrent le corps royal et une cour ouverte de 18 × 8 m,
dont le donjon occupe partiellement l'extrémité. Les tours sont intégrées à
l'union des murs ; leurs jonctions intérieures restent ouvertes. Le parvis,
le portail en saillie et les loggias donnent de la profondeur à la façade sud.

Le grand comble à deux pans culmine à 18 m environ. Deux combles secondaires
laissent des galeries latérales de 2 m de largeur utile. Les tours nord portent
des couronnes crénelées ; les tours sud des belvédères couverts au-dessus de
leurs terrasses. Le donjon termine la silhouette par une couronne panoramique.
Tous les toits sont des peaux de voxels fermées, à redents de 0,5 m. Les combles
ne sont pas présentés comme des étages supplémentaires habitables.

## Niveaux

| Surface du sol | Programme |
|---|---|
| 0 m | Parvis, entrée traversante, grand hall, réception, offices, gardes, banquets, cour et accès au donjon |
| 4,5 m | Galerie partielle sur le hall, grande salle du trône, salons nobles, conseil, portraits, loggia et tours |
| 9 m | Archives, appartements communicants, bibliothèque ouest, trophées est, chambres, terrasse arrière et donjon |
| 13,5 m | Chemins de ronde des ailes, terrasse arrière haute, niveaux des quatre tours et donjon |
| 18 m | Sommet des quatre tours et niveau haut du donjon |
| 22,5 m | Terrasse panoramique du donjon |

## Escaliers et raccords

Le hall reçoit un escalier impérial : volée centrale de 3 m, palier de 6 m,
deux branches de 1,5 m, arrivée sur les deux côtés de la galerie à 4,5 m.
L'aile ouest possède deux escaliers en L (4,5→9→13,5 m), l'aile est un U
(4,5→9 m). Chaque tour possède quatre U superposés ; le donjon cinq.
Total : **{v['stairs']} escaliers**, {v['stairKinds']['U']} U, {v['stairKinds']['L']} L et un monumental.
Les U font 3,5 × 5,5 m, les L 5,5 × 5 m. Les marches montent de 0,5 m et ont
0,5 m de giron. Les paliers ont au moins 1,5 m de profondeur. Le profil exact
vient de `src/voxel_stairs.h`, compilé dans l'outil hors jeu.

La montée par cubes de 0,5 m peut demander des sauts : **la franchissabilité
reste à tester en jeu**. Aucun profil à 0,25 m ni forme native non prouvée
n'est prétendu disponible. Les trémies sont découpées sur les volumes d'air
réels, avec 2,5 m de hauteur libre pour les escaliers. Les soutiens massifs superposés sont ajourés
uniquement sous les tabliers, là où ils rencontreraient un autre dégagement.
Aucune marche ni aucun palier n'est supprimé pour faire passer un contrôle.

## Façades et intérieurs

Lancettes réellement ouvertes, meurtrières dans les tours, oculi de pignons,
contreforts à retraits, corniches, piédroits, archivolte et blason minéral.
Les sols ne bouchent ni les trémies ni les baies. La salle du trône conserve
un axe central dégagé, quatre colonnes à bases et chapiteaux, une estrade à
trois degrés et un trône construit en pierre. La bibliothèque possède des
rayonnages de blocs, à meubler si souhaité. Le bassin de cour est sec.
Les portes sont des ouvertures architecturales ; aucune porte interactive,
lumière, tapisserie, eau ou meuble natif n'est inclus.

## Matériaux

Les rôles maçonnerie, pierre de taille, toiture et décor du trône sont séparés
dans le générateur. Par défaut ils utilisent tous le **Bloc de pierre des Anciens**,
ID `0x7ad0559c`, octet `0x9d`, reconnu dans le journal natif dev7 local et déjà
construit par les exemples validés. Bloc de château, pierre raffinée et
obsidienne restent **non identifiés**, donc substitués explicitement.
L'aperçu coloré distingue les rôles, il ne représente pas les textures du jeu.
`Materiaux.json` donne les choix effectivement exportés. Pour une vraie palette,
fournir au générateur `--palette CONFIG.json`, avec pour chaque rôle voulu
un `blueprint` BTBP exporté du jeu et son `name`. Voir la configuration d'exemple
dans le dépôt. Le jeu revalide toujours les ID avant toute construction.
''', encoding='utf-8', newline='\n')
    output.joinpath('Validation.md').write_text(f'''# Validation technique — Couronne des Anciens

**Résultat hors jeu : PASS. Test de ce château en jeu : non effectué.**
Le joueur a confirmé la génération multiétage avec escaliers de la démo dev7.
Ce retour ne valide pas encore les profils ni l'architecture de ce château.

| Contrôle exécuté sur les fichiers livrés | Résultat |
|---|---|
| Cellules occupées / placements prévus | {v['cells']:,} |
| BTBP v1, grille 0,5 m, modules / maximum | {v['modules']} / 128 |
| Réservations d'air / maximum | {v['reservedSpaces']} / 512 |
| Colonnes réservées / maximum | {v['reservedColumns']} / 65 536 |
| Doublons dans l'export / chevauchements contradictoires | 0 / 0 |
| Escaliers / types | {v['stairs']} / monumental, L, U |
| Pièces et destinations géométriquement reliées | {v['reachableRoomsAndDestinations']} / {v['roomsAndDestinations']} |
| Surfaces de plancher candidates du graphe, isolées | 0 |
| Graphe de surfaces, nœuds / composante depuis l'entrée | {v['surfaceGraphNodes']} / {v['entryComponentNodes']} |
| Pièces natives expérimentales / objets | 0 / 0 |
| Lots théoriques de 128 cellules | {v['productionReader']['batches128']} |
| Surface au sol construite | {v['occupiedGroundAreaM2']:g} m² |
| Dimensions X × Y × Z | {dimensions} m |

Chaque BTBP est relu par `core::bp_decode`, chaque module ajouté par
`project::add`, puis le projet finalisé par `project::finish`. Le véritable
`spaces::validate` vérifie toutes les réservations et leurs soutiens.
Les quatre rotations globales sont contrôlées cellule par cellule. La machine
`project::Job` effectue des passes avec lots 1, 32, 128 et 256 : validation sans
écriture simulée, acquittements complets, arrêt à plusieurs préfixes et refus
d'un acquittement partiel. Aucun placement natif n'est simulé ou appelé.

La vérification de circulation est dérivée **des voxels finaux**, avec surface
de soutien, 2 m d'air et adjacency horizontale limitée à une différence de
0,5 m. Toutes les pièces enregistrées, toutes les marches/paliers et les
planchers exposés doivent rejoindre l'entrée. Les noms de pièces n'ajoutent
aucune arête artificielle au graphe. Les corniches, toits, appuis de fenêtres et
créneaux sont exclus des chemins autorisés : un détour par une façade ne peut
pas remplacer une porte. Les escaliers réservent 2,5 m d'air. `Circulation.json` fournit les positions,
altitudes et distances dans ce graphe. Ce modèle ne calcule ni la capsule du
joueur, ni la physique, ni la hauteur de pas native : il ne certifie pas un
escalier utilisable sans saut, ni la stabilité structurelle dans le jeu.

## Limites et vérifications encore nécessaires

- Couverture des autels : inconnue hors jeu, contrôlée en 3D par le moteur natif.
  Il faut couvrir la boîte complète, les corniches et tous les dégagements.
  `Placement.md` donne une estimation paramétrique ; aucun quota n'est contourné.
- Coût et stock : calcul natif avant exécution ; nombre de cellules ≠ devis réel.
- Collision terrain, entités, soutien et stabilité : inspection et test en jeu.
- Reprise automatique et rollback complet : indisponibles dans dev7. Une
  interruption conserve les lots déjà acceptés. Le journal reste diagnostique.
- Palette noble : substitutions explicites, aucune identité déduite d'un nom.
- Aperçu Vulkan dev7 : boîte volumineuse donc affichage par volumes de modules ;
  les plans et vues joints représentent la géométrie réelle hors jeu.

La DLL et le format restent ceux de dev7. Ce projet utilise uniquement les
blocs composites, leurs catégories, les réservations d'air et les lots existants.
Les sommes SHA-256 permettent de vérifier le contenu distribué.
''', encoding='utf-8', newline='\n')
    output.joinpath('Placement.md').write_text(f'''# Placement en jeu

1. Conserver Building Tools Extended **0.34.0-dev7** déjà testé. Aucune mise à jour
   de DLL n'est nécessaire. Jouer dans un monde local de test dont vous possédez
   une copie. Le générateur n'a touché ni installation ni sauvegarde.
2. Copier tous les `FR_*.btbp` et `{PROJECT}` directement dans
   `BuildingTools-Projects`, à côté de `enshrouded.exe`. Les rapports et images
   peuvent rester dans leur dossier. Le ZIP fourni possède cette arborescence.
3. Préparer au moins **{math.ceil(v['dimensionsMeters'][0])+2} × {math.ceil(v['dimensionsMeters'][2])+2} m**
   horizontaux, et **{math.ceil(v['dimensionsMeters'][1])+2} m** verticaux libres et
   couverts. Sols à Y=-0,5 m et surface d'entrée à Y=0 relativement à l'ancrage.
   Placer la base du projet hors terrain et hors constructions existantes ;
   le moteur ne terrasse ni ne retire de blocs automatiquement.
4. Prendre le **cube de 0,5 m**, ouvrir une fois le sélecteur de matériaux dans
   la base, puis passer en Blueprint → Placer → Projets composites. Sélectionner
   uniquement `Forteresse_Royale_Demo`. Conserver le placement natif expérimental
   désactivé. Le créatif original peut servir au premier essai.
5. Vérifier **{v['cells']:,} cellules**, {v['modules']} modules, {v['reservedSpaces']}
   dégagements, zéro pièce native et zéro objet. Le nom affiché est « {NAME} ».
   Le pivot est au sol du corps central ; le portail regarde -Z à rotation 0°.
   Utiliser Placer ici et les réglages de hauteur/rotation pour tout le château.
6. Analyser les autels. Les bornes locales occupées sont
   `{v['boundsMeters'][0]}` à `{v['boundsMeters'][1]}` m ; les réservations d'air
   portent le volume complet à **{' × '.join(f'{x:g}' for x in v['minimumValidationVolumeMeters'])} m** (X × Y × Z), jusqu'à
   Y={v['requiredClearanceBoundsMeters'][1][1]:g} m au-dessus de l'ancrage. Une couverture réelle W×D×H permet au mieux
   ceil(X/W)×ceil(Z/D)×ceil(Y/H) boîtes théoriques, avant les contraintes natives,
   les recouvrements, quotas et terrain. Par exemple avec 40×40 m horizontaux
   et assez de hauteur : au moins quatre boîtes géométriques pour cette emprise.
   Ce n'est pas une promesse que quatre autels de votre niveau suffisent.
7. Cliquer **Vérifier sans construire**, attendre la fin et résoudre tout refus
   précis (couverture, collision, matériau, stock). Un aperçu visible ne suffit
   pas. Ne pas supprimer `spaces` ni désactiver les contrôles pour forcer le projet.
8. Après validation, cliquer **Construire une seule fois**. Garder la fenêtre
   active et le même outil. Le moteur refait ses contrôles, puis place tous les
   modules par lots. Avec un plafond 128 : {v['productionReader']['batches128']}
   lots ; le plafond adaptatif peut les rendre plus nombreux.
9. En cas d'Échap, perte de focus ou erreur, conserver les journaux et le rapport
   Diagnostics. Les lots écrits restent en place. Il n'existe pas de reprise
   automatique garantie ; ne pas rejouer aveuglément sur un chantier partiel.

## Recette à consigner après la construction

Parcourir parvis → hall → deux branches impériales → galerie → trône ; puis les
deux ailes, bibliothèque, trophées et appartements à 9 m. Vérifier les L/U dans
les deux sens, chaque palier et les trémies ; noter tout saut nécessaire. Visiter
chaque niveau des quatre tours, les chemins de ronde à 13,5 m, les terrasses
arrière, le donjon et son sommet à 22,5 m. Vérifier les parapets, les garde-corps,
les accès latéraux et l'absence de chute imposée. Inspecter aussi façades et toits.
Sauvegarder/recharger le monde de test puis refaire les parcours. Consigner
version jeu/mod, ancrage, rotation, créatif, durée, obstacles et journaux.

Une réussite des tests hors jeu prouve la géométrie et la compatibilité du fichier.
**Elle ne valide pas la circulation réelle du personnage.**
''', encoding='utf-8', newline='\n')


def plans(castle, output):
    # Six actual voxel sections, not a hand-drawn diagram of intended rooms.
    colors = {'masonry': '#596471', 'trim': '#d2b683', 'roof': '#6b8496', 'throne': '#80579c'}
    parts = ['<svg xmlns="http://www.w3.org/2000/svg" width="1560" height="1380" viewBox="0 0 1560 1380">',
             '<rect width="1560" height="1380" fill="#101924"/>',
             '<style>text{font-family:Segoe UI,Arial,sans-serif;fill:#e6e1d5}.small{font-size:12px}.title{font-size:22px}</style>',
             '<text x="35" y="38" class="title">Couronne des Anciens — sections des voxels construits</text>',
             '<text x="35" y="62" class="small">Nord = +Z (haut) · pas 0,5 m · murs en coupe · escaliers en or · repères des destinations vérifiées</text>']
    for index, level in enumerate((0, 9, 18, 27, 36, 45)):
        ox = 45+(index % 3)*510
        oy = 100+(index//3)*640
        scale = 4.9
        def point(x, z):
            return ox+(x+46)*scale, oy+(46-z)*scale
        parts.append(f'<text x="{ox}" y="{oy}" class="title">Sol à {level/2:g} m</text>')
        for (x, y, z), (role, category, owner) in sorted(castle.cells.items()):
            if y not in (level-1, level+1):
                continue
            px, py = point(x, z+1)
            color = '#bd995a' if category == 'stairs' else colors[role] if y == level+1 else '#263846'
            parts.append(f'<rect x="{px:.1f}" y="{py:.1f}" width="{scale}" height="{scale}" fill="{color}"/>')
        for room in castle.rooms:
            x, y, z = room['seed']
            if y == level:
                px, py = point(x+.5, z+.5)
                label = str(castle.rooms.index(room)+1)
                parts.append(f'<circle cx="{px:.1f}" cy="{py:.1f}" r="6" fill="#dfdbc8"/><text x="{px:.1f}" y="{py+3:.1f}" text-anchor="middle" style="fill:#111;font-size:8px">{label}</text>')
        parts.append(f'<path d="M{ox+8},{oy+540}h{20*scale}" stroke="#d2b683" stroke-width="3"/><text x="{ox+8}" y="{oy+560}" class="small">10 m</text>')
    parts.append('<text x="35" y="1350" class="small">Les numéros renvoient à Circulation.json dans l’ordre des pièces. Modèle géométrique : la marche du personnage reste à tester.</text></svg>')
    (output/'Plans.svg').write_text('\n'.join(parts), encoding='utf-8', newline='\n')


def render_castle(castle, output):
    # Optional Pillow, isolated under build/. This is a voxel plot of the actual
    # exported geometry, with schematic role colours (no invented game textures).
    sys.path.insert(0, str(ROOT/'build/castle-render-libs'))
    from PIL import Image, ImageDraw, ImageFont
    shades = {'masonry': (126, 137, 148), 'trim': (188, 171, 137),
              'roof': (68, 90, 111), 'throne': (83, 58, 101)}
    font_path = Path('C:/Windows/Fonts/segoeui.ttf')
    font = ImageFont.truetype(str(font_path), 32) if font_path.exists() else ImageFont.load_default()
    small = ImageFont.truetype(str(font_path), 20) if font_path.exists() else font
    views = [('Sud', 1, -1, castle.cells), ('Nord', -1, 1, castle.cells)]
    hall = {p: v for p, v in castle.cells.items()
            if -18 <= p[0] < 17 and -29 <= p[2] < 14 and p[1] < 17
            and not (v[1] == 'floor' and p[1] == 8 and p[2] < -11)
            and not (v[2] == 'Garde-corps des bords exposés' and p[2] < -11)}
    throne = {p: v for p, v in castle.cells.items()
              if -18 <= p[0] < 17 and -5 <= p[2] < 14 and 8 <= p[1] < 17}
    views += [('Hall_coupe', 1, -1, hall), ('Trone_coupe', 1, -1, throne)]
    views = getattr(castle, 'render_views', views)
    for label, signx, signz, display in views:
        im = Image.new('RGB', (1900, 1550), (19, 28, 40))
        draw = ImageDraw.Draw(im)
        def raw(x, y, z):
            return ((signx*x-signz*z)*11, (signx*x+signz*z)*5-y*15)
        vertices = [raw(x+dx, y+dy, z+dz) for x, y, z in display
                    for dx, dy, dz in ((0, 0, 0), (1, 1, 1), (0, 1, 0), (1, 0, 1))]
        low = [min(p[a] for p in vertices) for a in range(2)]
        high = [max(p[a] for p in vertices) for a in range(2)]
        scale = min(1750/(high[0]-low[0]), 1250/(high[1]-low[1]))
        offset = (950-(low[0]+high[0])*scale/2, 185-low[1]*scale)
        def point(x, y, z):
            px, py = raw(x, y, z)
            return (px*scale+offset[0], py*scale+offset[1])
        faces = []
        for (x, y, z), value in display.items():
            X, Z = x+(signx < 0), z+(signz < 0)
            role, cat, owner = value
            color = shades[role]
            if (x, y+1, z) not in display:
                poly = [point(x, y+1, z), point(x+1, y+1, z), point(x+1, y+1, z+1), point(x, y+1, z+1)]
                faces.append((signx*(x+.5)+signz*(z+.5)+y+1, poly, tuple(min(255, int(v*1.12)) for v in color)))
            for dx, dz, factor in ((signx, 0, .82), (0, signz, .63)):
                if (x+dx, y, z+dz) in display:
                    continue
                if dx:
                    xx = x+(dx > 0)
                    poly = [point(xx, y, z), point(xx, y, z+1), point(xx, y+1, z+1), point(xx, y+1, z)]
                else:
                    zz = z+(dz > 0)
                    poly = [point(x, y, zz), point(x+1, y, zz), point(x+1, y+1, zz), point(x, y+1, zz)]
                faces.append((signx*(x+.5)+signz*(z+.5)+y, poly, tuple(int(v*factor) for v in color)))
        for _, poly, color in sorted(faces, key=lambda f: f[0]):
            draw.polygon(poly, fill=color)
        title = getattr(castle, 'render_title', 'COURONNE DES ANCIENS')
        draw.text((55, 40), f'{title} — vue {label.lower().replace("_", " ")}', font=font, fill=(235, 226, 205))
        draw.text((55, 91), 'Géométrie réelle des voxels · couleurs des rôles architecturaux · textures et parcours en jeu à valider', font=small, fill=(161, 178, 194))
        im.save(output/f'Vue_{label}.png')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True, help='NEW output directory inside the repository')
    parser.add_argument('--palette', type=Path)
    parser.add_argument('--render', action='store_true')
    parser.add_argument('--zip', type=Path, help='NEW import archive with BuildingTools-Projects at its root')
    args = parser.parse_args()
    try:
        if args.zip and args.zip.exists():
            raise FileExistsError('Refusing existing ZIP: ' + str(args.zip))
        create(args.output, args.palette, args.render)
        if args.zip:
            args.zip.parent.mkdir(parents=True, exist_ok=True)
            with zipfile.ZipFile(args.zip, 'x', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
                for file in sorted(args.output.iterdir()):
                    destination = 'BuildingTools-Projects/'+file.name if file.suffix in ('.btbp', '.btproject') else 'Forteresse-Royale-Guide/'+file.name
                    archive.write(file, destination)
    except (ValueError, OSError) as error:
        parser.exit(1, str(error)+'\n')


if __name__ == '__main__':
    main()
