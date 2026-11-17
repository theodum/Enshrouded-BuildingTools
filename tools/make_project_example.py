#!/usr/bin/env python3
"""Create three small BTBP modules using a material from an existing blueprint.

Reads the source only. Refuses existing output files. Does not inspect/write saves.
Material identifiers are copied, never guessed; the DLL checks their native registry.
"""
import argparse
import json
from pathlib import Path
import struct


def material_from(path):
    data = path.read_bytes()
    if len(data) < 52 or data[:8] != b'BTBP\x01\x00\x00\x00':
        raise ValueError('Expected a BTBP v1 blueprint')
    count = struct.unpack_from('<H', data, 30)[0]
    if not 1 <= count <= 64 or len(data) < 36 + 16 * count:
        raise ValueError('Blueprint has no readable material table')
    item, mat = struct.unpack_from('<IB', data, 32)
    if not item or mat < 0x80:
        raise ValueError('First material has no building identifier')
    return item, mat


def blueprint(dims, step, occupied, material):
    nx, ny, nz = dims
    cells = [0 if occupied(x, y, z) else 255
             for y in range(ny) for z in range(nz) for x in range(nx)]
    runs = []
    for cell in cells:
        if runs and runs[-1][1] == cell and runs[-1][0] < 65535:
            runs[-1][0] += 1
        else:
            runs.append([1, cell])
    filled = cells.count(0)
    item, mat = material
    # Shape ID 0: composite placement uses the player's native 0.5 m cube.
    data = struct.pack('<4sHH3f3HIH', b'BTBP', 1, 0, step, step, step, nx, ny, nz, 0, 1)
    data += struct.pack('<IBBHII', item, mat, 0, 0, filled, filled)
    data += struct.pack('<I', len(runs))
    data += b''.join(struct.pack('<HBB', n, value, 0) for n, value in runs)
    return data + struct.pack('<I', 0)


def create(output, material):
    output.mkdir(parents=True, exist_ok=False)
    specs = [
        ('hall.btbp', (6, 3, 4), 1.0, lambda x, y, z: y == 0 or z == 3),
        ('tower.btbp', (2, 3, 2), 0.5, lambda x, y, z: True),
        ('bridge.btbp', (6, 1, 2), 0.5, lambda x, y, z: True),
    ]
    for name, dims, step, occupied in specs:
        (output / name).write_bytes(blueprint(dims, step, occupied, material))
    manifest = {
        'format': 'BuildingToolsExtendedProject', 'version': 1,
        'name': 'Trois modules - test', 'gridSize': 0.5, 'anchor': [0, 0, 0],
        'modules': [
            {'file': 'hall.btbp', 'offset': [0, 0, 0], 'rotation': 0},
            {'file': 'tower.btbp', 'offset': [8, 1, 0], 'rotation': 90},
            {'file': 'bridge.btbp', 'offset': [2, 2, 5], 'rotation': 270},
        ],
    }
    (output / 'Trois_modules_test.btproject').write_text(
        json.dumps(manifest, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--material-from', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True, help='A NEW directory')
    args = parser.parse_args()
    try:
        material = material_from(args.material_from)
        create(args.output, material)
    except (ValueError, OSError) as error:
        parser.exit(1, str(error) + '\n')
    print(f'Created {args.output}; copied item 0x{material[0]:08x}, material 0x{material[1]:02x}')


if __name__ == '__main__':
    main()
