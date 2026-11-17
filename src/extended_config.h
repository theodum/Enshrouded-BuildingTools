// Separate, additive configuration. Never reads or writes game saves.
static extended::Grid g_grid;
static int g_gridLevels[extended::Levels] = {0, 320, 640, 960, 1280, 1600, 1920, 2240};
static int g_gridLevel = 0, g_creativeProps = 0, g_projectUseObjects = 1;
static int g_showAltars = 1, g_showModules = 1, g_showObjects = 1;
static uint8_t* g_extendedItemGroups = nullptr;
static bool g_nativePartsAvailable = false;
static bool g_nativeObjectAbi = false;
static int g_nativeObjectTrace = 0;
static int g_experimentalNative = 0, g_projectBatchLimit = 128;
static int g_projectSlice = 0, g_projectSliceY = 0, g_projectSliceHeight = 64;
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
    g_experimentalNative = extended_read(L"ExperimentalNativePlacement", 0) != 0;
    g_projectBatchLimit = extended::batch_limit(extended_read(L"ProjectBatchCells", 128));
    g_showAltars = extended_read(L"ShowAltars", 1) != 0; g_showModules = extended_read(L"ShowModules", 1) != 0; g_showObjects = extended_read(L"ShowObjects", 1) != 0;
    g_gridLevel=extended_read(L"SelectedLevel",0); if (g_gridLevel<0 || g_gridLevel>=extended::Levels) g_gridLevel=0;
    g_projectSlice=extended_read(L"PreviewSlice",0)!=0; g_projectSliceY=extended_read(L"SliceY16",0); g_projectSliceHeight=extended_read(L"SliceHeight16",64);
    if (g_projectSliceY < -project::MaxCoord || g_projectSliceY > project::MaxCoord) g_projectSliceY=0;
    if (g_projectSliceHeight<8 || g_projectSliceHeight>4096) g_projectSliceHeight=64;
    g_projectSliceY=extended::snap(g_projectSliceY,8); g_projectSliceHeight=extended::snap(g_projectSliceHeight,8);
    for (int i = 0; i < extended::Levels; ++i) { wchar_t key[] = L"Level0"; key[5] += i; int v = extended_read(key, i * 320); if (v >= -project::MaxCoord && v <= project::MaxCoord && v % 8 == 0) g_gridLevels[i] = v; }
}
static void extended_save() {
    wchar_t path[MAX_PATH]; extended_ini_path(path);
    auto write = [&](const wchar_t* key, int value) { Buf b; b.i(value); wchar_t number[32]; for (int i = 0; i <= b.n; ++i) number[i] = (wchar_t)b.z()[i]; WritePrivateProfileStringW(L"Extended", key, number, path); };
    write(L"Grid", g_grid.enabled); write(L"Height16", g_grid.height); write(L"OriginX16", g_grid.x); write(L"OriginZ16", g_grid.z); write(L"GridStep16", g_grid.spacing); write(L"GridRadius", g_grid.extent); write(L"Axes", g_grid.axes);
    write(L"CreativeObjects", g_creativeProps); write(L"ProjectObjects", g_projectUseObjects);
    write(L"ExperimentalNativePlacement", g_experimentalNative); write(L"ProjectBatchCells", g_projectBatchLimit);
    write(L"ShowAltars", g_showAltars); write(L"ShowModules", g_showModules); write(L"ShowObjects", g_showObjects); write(L"NativeObjectTrace", g_nativeObjectTrace);
    write(L"SelectedLevel",g_gridLevel); write(L"PreviewSlice",g_projectSlice); write(L"SliceY16",g_projectSliceY); write(L"SliceHeight16",g_projectSliceHeight);
    for (int i = 0; i < extended::Levels; ++i) { wchar_t key[] = L"Level0"; key[5] += i; write(key, g_gridLevels[i]); }
}
