// Item IDs come exclusively from the game's already-verified group iterator and
// item registry. No scan guesses, fabricated IDs or inventory mutation.
struct ExtendedItem { uint32_t item; int kind; };
static ExtendedItem g_catalog[4096]; static int g_catalogCount = 0;
static uint32_t g_catalogMaterial = 0, g_catalogPiece = 0;
static bool catalog_refresh() {
    g_catalogCount = 0;
    uint8_t* reg = g_extendedItemGroups;
    if (!reg || !g_iterBegin || !g_iterNext || !g_bpItemReg || !g_bpItemById || !readable(reg, 0x48)) return false;
    uint64_t end = *(uint64_t*)(reg + 0x10); int guard = 0;
    for (uint64_t at = g_iterBegin(reg + 8); at != end && guard++ < 200000; at = g_iterNext(reg + 8, at)) {
        uint8_t* group = (*(uint8_t***)(reg + 0x40))[at]; if (!group || !readable(group, 16)) continue;
        const int32_t offset = *(int32_t*)(group + 8); const uint32_t n = *(uint32_t*)(group + 12);
        if (!offset || n > 20000) continue;
        const uint32_t* ids = (const uint32_t*)(group + 8 + offset); if (!readable(ids, (size_t)n * 4)) continue;
        for (uint32_t k = 0; k < n && g_catalogCount < 4096; ++k) {
            uint32_t id = ids[k]; if (!id) continue;
            bool duplicate = false; for (int j = 0; j < g_catalogCount; ++j) if (g_catalog[j].item == id) { duplicate = true; break; }
            if (duplicate) continue;
            uint8_t* item = g_bpItemById(id, g_bpItemReg); int kind = -1;
            if (bp_block_item(item)) kind = 0;
            else if (prop_item(item) && readable(item + 0x5d8, 4)) kind = extended_prop_item(item) ? 1 : 3;
            else if (item && (item_kind(item) == ITEM_PIECE || item_kind(item) == ITEM_BIG_SHAPE) && readable(item + 0x427, 1) && !item[0x427]) kind = 2;
            if (kind >= 0) g_catalog[g_catalogCount++] = ExtendedItem{id, kind};
        }
    }
    // Stable numeric IDs keep paging independent of the engine hash-table order.
    for (int i = 1; i < g_catalogCount; ++i) { ExtendedItem item = g_catalog[i]; int j = i; while (j && g_catalog[j - 1].item > item.item) { g_catalog[j] = g_catalog[j - 1]; --j; } g_catalog[j] = item; }
    return g_catalogCount != 0;
}
static bool project_generated_begin(const char* name) {
    if (g_projectRunning) return false;
    if (!g_projectPlan.cells) g_projectPlan.cells = (project::CellEntry*)VirtualAlloc(nullptr, sizeof(project::CellEntry) * project::MaxCells, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!g_projectObjects) g_projectObjects = (extended::Object*)VirtualAlloc(nullptr, sizeof(extended::Object) * kProjectMaxObjects, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!g_projectPlan.cells || !g_projectObjects) return false;
    auto cells = g_projectPlan.cells; g_projectPlan = project::Plan{}; g_projectPlan.cells = cells; g_projectPlan.capacity = project::MaxCells;
    g_projectObjectCount = 0; memset(&g_projectManifest, 0, sizeof g_projectManifest); memset(&g_bp.h, 0, sizeof g_bp.h);
    g_projectNeedN = 0; g_projectLastProblem = nullptr; g_projectProblem = false;
    memset(g_projectModuleVisible, 1, sizeof g_projectModuleVisible);
    copy_text(g_projectManifest.name, name, sizeof g_projectManifest.name); g_projectManifest.grid = 8; g_projectManifest.count = 1;
    g_projectLoaded = false; g_bp.ok = false; return true;
}
static void project_generated_finish() {
    g_projectNeedN = 0; g_projectProblem = false; g_projectLastProblem = nullptr;
    for (int m = 0; m < g_projectPlan.materialCount; ++m) g_projectPlan.materials[m].cells = 0;
    for (int i = 0; i < g_projectPlan.count; ++i) ++g_projectPlan.materials[g_projectPlan.cells[i].material].cells;
    for (int a = 0; a < 3; ++a) { g_bp.h.n[a] = (g_projectPlan.hi[a] - g_projectPlan.lo[a]) / 8; if (g_bp.h.n[a] < 1) g_bp.h.n[a] = 1; g_bp.h.step[a] = 0.5f; }
    g_bp.h.nm = g_projectPlan.materialCount; for (int m = 0; m < g_bp.h.nm; ++m) g_bp.h.m[m] = g_projectPlan.materials[m];
    g_bp.filled = g_projectPlan.count; g_bp.ok = true; g_projectLoaded = true; g_projectList = true; g_bpPage = 1; g_bpMirror = 0; g_bpBitsOk = 0; ++g_projectGeneration;
}
static bool catalog_object(uint32_t id, const int* offset = nullptr, int rotation = 0, bool append = false, int module = 0) {
    ProjectStateGuard guard;
    uint8_t* item = g_bpItemById ? g_bpItemById(id, g_bpItemReg) : nullptr;
    if (!extended_prop_item(item)) return false;
    bool add = append && g_projectLoaded;
    if (!add && !project_generated_begin(bp_item_name(id) ? bp_item_name(id) : "Native object")) return false;
    if (g_projectObjectCount == kProjectMaxObjects) return false;
    auto& object = g_projectObjects[g_projectObjectCount++]; object = extended::Object{}; object.item = id;
    const float identity[] = {0,0,0,1}; core::bp_turn_rot(rotation, identity, object.rotation);
    object.module = (uint16_t)(module >= 0 && module < g_projectManifest.count ? module : 0);
    g_projectPlan.objects = g_projectObjectCount; g_projectUseObjects = 1; g_projectNeedN = 0;
    for (int a = 0; a < 3; ++a) {
        int p = offset ? offset[a] : 0; object.p[a] = p / 16.0;
        if (!add || p < g_projectPlan.lo[a]) g_projectPlan.lo[a] = p;
        if (!add || p + 8 > g_projectPlan.hi[a]) g_projectPlan.hi[a] = p + 8;
        if (!add) { g_projectModuleBoxes[0].lo[a] = p; g_projectModuleBoxes[0].hi[a] = p + 8; }
    }
    project_generated_finish(); return true;
}
