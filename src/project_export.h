// Export into NEW BTBP files; publish the manifest last by atomic rename.
// Sparse plans are partitioned into 16 m tiles, never a giant dense blueprint.
#include "project_spaces.h"
static bool project_export(char* result, int capacity) {
    ProjectStateGuard guard;
    if (!g_projectLoaded || g_projectRunning) return false;
    if(spaces::validate(g_projectPlan,g_projectManifest)!=spaces::Issue::None)return false;
    struct Tile { int low[3]; uint16_t module; uint8_t category; };
    static Tile tiles[project::MaxModules]; int tileCount = 0;
    auto tile_low = [](int value) { return value >= 0 ? value / 256 * 256 : -((-value + 255) / 256 * 256); };
    for (int i = 0; i < g_projectPlan.count; ++i) {
        const auto& cell = g_projectPlan.cells[i]; Tile tile{}; tile.module = cell.module; tile.category = cell.category;
        for (int a = 0; a < 3; ++a) { if (cell.p[a] % 8) return false; tile.low[a] = tile_low(cell.p[a]); }
        int at = 0; while (at < tileCount && (tiles[at].module != tile.module || tiles[at].category != tile.category || memcmp(tiles[at].low, tile.low, sizeof tile.low))) ++at;
        if (at == tileCount) { if (tileCount == project::MaxModules) return false; tiles[tileCount++] = tile; }
    }
    const int objectFiles = (g_projectObjectCount + core::BP_MAX_OBJS - 1) / core::BP_MAX_OBJS;
    const int files = tileCount + objectFiles;
    if (files > project::MaxModules || (!files && !g_projectManifest.pieceCount)) return false;
    SYSTEMTIME time; GetLocalTime(&time); Buf base;
    base.str("Extended_").u(time.wYear).u(time.wMonth, 10, 2).u(time.wDay, 10, 2).c('_').u(time.wHour, 10, 2).u(time.wMinute, 10, 2).u(time.wSecond, 10, 2).c('_').u(GetTickCount());
    char prefix[96]; copy_text(prefix, base.z(), sizeof prefix);
    wchar_t folder[MAX_PATH]; if (!project_path(folder, L"")) return false; CreateDirectoryW(folder, nullptr);
    auto path = [&](const char* name, wchar_t* full) {
        wchar_t leaf[192]; if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, name, -1, leaf, 192)) return false;
        return project_path(full, leaf);
    };
    static core::BpHeader header; static uint8_t cells[32768], encoded[300000];
    static char names[project::MaxModules][128];
    for (int f = 0; f < files; ++f) {
        memset(&header, 0, sizeof header); memset(cells, core::BP_EMPTY, sizeof cells);
        for (int a = 0; a < 3; ++a) { header.step[a] = 0.5f; header.n[a] = f < tileCount ? 32 : 1; }
        if (f < tileCount) {
            const auto& tile = tiles[f]; header.nm = g_projectPlan.materialCount;
            for (int m = 0; m < header.nm; ++m) { header.m[m] = g_projectPlan.materials[m]; header.m[m].cells = header.m[m].items = 0; }
            for (int i = 0; i < g_projectPlan.count; ++i) {
                const auto& cell = g_projectPlan.cells[i];
                if (cell.module != tile.module || cell.category != tile.category) continue;
                int c[3]; bool inside = true;
                for (int a = 0; a < 3; ++a) { c[a] = (cell.p[a] - tile.low[a]) / 8; if (cell.p[a] < tile.low[a] || c[a] >= 32) inside = false; }
                if (inside) { cells[core::bp_index(header.n, c[0], c[1], c[2])] = cell.material; ++header.m[cell.material].cells; }
            }
        } else {
            int start = (f - tileCount) * core::BP_MAX_OBJS;
            for (int i = start; i < g_projectObjectCount && header.objects < core::BP_MAX_OBJS; ++i) {
                const auto& object = g_projectObjects[i]; auto& out = header.obj[header.objects++]; out.item = object.item;
                for (int a = 0; a < 3; ++a) out.pos[a] = (float)object.p[a];
                memcpy(out.rot, object.rotation, 16);
            }
        }
        const int size = core::bp_encode(header, cells, encoded, sizeof encoded); if (!size) return false;
        Buf name; name.str(prefix).str("_m").u(f, 10, 3).str(".btbp"); copy_text(names[f], name.z(), sizeof names[f]);
        wchar_t full[MAX_PATH]; if (!path(names[f], full)) return false;
        HANDLE file = CreateFileW(full, GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE) return false;
        DWORD written = 0; BOOL ok = WriteFile(file, encoded, size, &written, nullptr);
        if (ok && written == (DWORD)size) ok = FlushFileBuffers(file);
        CloseHandle(file);
        if (!ok || written != (DWORD)size) return false;
    }
    Buf manifestName; manifestName.str(prefix).str(".btproject"); char finalName[128]; copy_text(finalName, manifestName.z(), sizeof finalName);
    Buf temporary; temporary.str(finalName).str(".tmp"); wchar_t tempPath[MAX_PATH], finalPath[MAX_PATH];
    if (!path(temporary.z(), tempPath) || !path(finalName, finalPath)) return false;
    HANDLE file = CreateFileW(tempPath, GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return false;
    bool ok = true;
    auto write = [&](Buf& text) { DWORD written = 0; if (!WriteFile(file, text.z(), text.n, &written, nullptr) || written != (DWORD)text.n) ok = false; };
    Buf first; first.str("{\n  \"format\":\"BuildingToolsExtendedProject\",\n  \"version\":1,\n  \"name\":\"");
    for (const char* c = g_projectManifest.name; *c; ++c) { if (*c == '"' || *c == '\\') first.c('\\'); if ((uint8_t)*c >= 32) first.c(*c); }
    first.str("\",\n  \"gridSize\":0.5,\n  \"anchor\":[");
    for (int a = 0; a < 3; ++a) { if (a) first.c(','); first.f(g_projectManifest.anchor[a] / 16.0, 4); }
    first.str("],\n  \"modules\":[\n"); write(first);
    const char* categories[] = {"general","wall","floor","roof","tower","stairs","furniture"};
    for (int f = 0; f < files; ++f) {
        Buf row; if (f) row.str(",\n"); row.str("    {\"file\":\"").str(names[f]).str("\",\"offset\":[");
        for (int a = 0; a < 3; ++a) { if (a) row.c(','); row.f(f < tileCount ? tiles[f].low[a] / 16.0 : 0, 4); }
        const int category = f < tileCount ? (int)tiles[f].category : (int)extended::Furniture;
        row.str("],\"rotation\":0,\"category\":\"").str(categories[category >= 0 && category < 7 ? category : 0]).str("\"}"); write(row);
    }
    Buf middle; middle.str("\n  ],\n  \"pieces\":[\n"); write(middle);
    for (int i = 0; i < g_projectManifest.pieceCount; ++i) {
        const auto& piece = g_projectManifest.pieces[i]; Buf row;
        if (i) row.str(",\n");
        row.str("    {\"item\":").u(piece.item).str(",\"offset\":[");
        for (int a = 0; a < 3; ++a) { if (a) row.c(','); row.f(piece.offset[a] / 16.0, 4); }
        row.str("],\"rotation\":").i(piece.rotation * 90); if (piece.material) row.str(",\"material\":").u(piece.material);
        if (piece.clearance) row.str(",\"clearance\":").f(piece.clearance / 16.0, 4);
        row.c('}'); write(row);
    }
    Buf last;last.str("\n  ]");
    if(g_projectManifest.spaceCount){last.str(",\n  \"spaces\":[\n");write(last);
        for(int i=0;i<g_projectManifest.spaceCount;++i){const auto& s=g_projectManifest.spaces[i];Buf row;if(i)row.str(",\n");row.str("    {\"low\":[");
            for(int a=0;a<3;++a){if(a)row.c(',');row.f(s.lo[a]/16.0,4);}row.str("],\"high\":[");for(int a=0;a<3;++a){if(a)row.c(',');row.f(s.hi[a]/16.0,4);}row.str("]}");write(row);}
        Buf end;end.str("\n  ]\n}\n");write(end);
    }else{last.str("\n}\n");write(last);}
    if (!FlushFileBuffers(file)) ok = false; CloseHandle(file);
    if (!ok || !MoveFileExW(tempPath, finalPath, MOVEFILE_WRITE_THROUGH)) return false;
    copy_text(result, finalName, capacity); LOG(.str("project exported: ").str(finalName).str(" modules=").i(files).str(" native pieces=").i(g_projectManifest.pieceCount)); return true;
}
