#!/bin/sh
# Host-only regression suite. Does not rebuild/install the game DLL.
set -eu
cd "$(dirname "$0")/.."
mkdir -p build
compiler=${HOSTCXX:-c++}
"$compiler" -std=c++17 -O2 tools/castle_host.cpp -o build/castle_host
for name in core project project_actions materials storeys voxel_stairs extended extended_config project_export; do
    "$compiler" -std=c++17 -O1 "-DSRC_DIR=\"$(pwd)/tests\"" "tests/test_$name.cpp" -o "build/test_$name"
    if [ "$name" = core ] && [ -n "${ENSHROUDED_EXE:-}" ]; then
        "./build/test_$name" "$ENSHROUDED_EXE"
    else
        "./build/test_$name"
    fi
done
"$compiler" -std=c++17 -O1 tools/make_voxel_example.cpp -o build/make_voxel_example
./build/make_voxel_example --material-from examples/three-modules/hall.btbp --verify examples/dev7
python3 -m unittest discover -s tests -p 'test_project_example.py'
python3 -m unittest discover -s tests -p 'test_royal_castle.py'
python3 tools/verify_royal_castle.py
python3 -m unittest discover -s tests -p 'test_royal_castle_v2.py'
python3 tools/verify_royal_castle_v2.py
