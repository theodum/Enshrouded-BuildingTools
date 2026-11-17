// Separate, additive configuration. Never reads or writes game saves.
static extended::Grid g_grid;
static int g_gridLevels[extended::Levels] = {0, 320, 640, 960, 1280, 1600, 1920, 2240};
static int g_gridLevel = 0, g_creativeProps = 0, g_projectUseObjects = 1;
static int g_showAltars = 1, g_showModules = 1, g_showObjects = 1;
static uint8_t* g_extendedItemGroups = nullptr;
static bool g_nativePartsAvailable = false;
static int g_nativeObjectTrace = 0;
static void extended_ini_path(wchar_t* path) { lstrcpynW(path, g_dir, MAX_PATH - 32); lstrcatW(path, L"BuildingTools-Extended.ini"); }
static int extended_read(const wchar_t* key, int fallback) {
    wchar_t path[MAX_PATH]; extended_ini_path(path);
    return (int)GetPrivateProfileIntW(L"Extended", key, fallback, path);
}
static void extended_load() {
    g_grid.enabled = extended_read(L"Grid", 0) != 0;
    g_grid.height = extended_read(L"Height16", 1920); g_grid.x = extended_read(L"OriginX16", 0); g_grid.z = extended_read(L"OriginZ16", 0);
    if (g_grid.height < -project::MaxCoord || g_grid.height > project::MaxCoord) g_grid.height = 1920;
    if (g_grid.x < -project::MaxCoord || g_grid.x > project::MaxCoord) g_grid.x = 0;
    if (g_grid.z < -project::MaxCoord || g_grid.z > project::MaxCoord) g_grid.z = 0;
    g_grid.height = extended::snap(g_grid.height, 8); g_grid.x = extended::snap(g_grid.x, 8); g_grid.z = extended::snap(g_grid.z, 8);
    g_grid.spacing = extended_read(L"GridStep16", 8) == 16 ? 16 : 8;
    g_grid.extent = extended_read(L"GridRadius", 32); if (g_grid.extent < 4 || g_grid.extent > 64) g_grid.extent = 32;
    g_grid.axes = extended_read(L"Axes", 1) != 0;
    g_creativeProps = extended_read(L"CreativeObjects", 0) != 0;
    g_projectUseObjects = extended_read(L"ProjectObjects", 1) != 0;
    g_nativeObjectTrace = extended_read(L"NativeObjectTrace", 0) != 0;
    for (int i = 0; i < extended::Levels; ++i) { wchar_t key[] = L"Level0"; key[5] += i; int v = extended_read(key, i * 320); if (v >= -project::MaxCoord && v <= project::MaxCoord && v % 8 == 0) g_gridLevels[i] = v; }
}
static void extended_save() {
    wchar_t path[MAX_PATH]; extended_ini_path(path);
    auto write = [&](const wchar_t* key, int value) { Buf b; b.i(value); wchar_t number[32]; for (int i = 0; i <= b.n; ++i) number[i] = (wchar_t)b.z()[i]; WritePrivateProfileStringW(L"Extended", key, number, path); };
    write(L"Grid", g_grid.enabled); write(L"Height16", g_grid.height); write(L"OriginX16", g_grid.x); write(L"OriginZ16", g_grid.z); write(L"GridStep16", g_grid.spacing); write(L"GridRadius", g_grid.extent); write(L"Axes", g_grid.axes);
    write(L"CreativeObjects", g_creativeProps); write(L"ProjectObjects", g_projectUseObjects);
    for (int i = 0; i < extended::Levels; ++i) { wchar_t key[] = L"Level0"; key[5] += i; write(key, g_gridLevels[i]); }
}
