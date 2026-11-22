// Item IDs come exclusively from the game's already-verified group iterator and
// item registry. No scan guesses, fabricated IDs or inventory mutation.
struct ExtendedItem { uint32_t item; int kind; };
static ExtendedItem g_catalog[8192]; static int g_catalogCount = 0;
static_assert(itemindex::Count<=8192,"Imported index exceeds the bounded native catalogue");
static uint32_t g_catalogMaterial = 0, g_catalogPiece = 0;
static bool g_catalogTruncated=false;
static bool catalog_refresh() {
    g_catalogCount = 0;g_catalogTruncated=true; // only a completed traversal may be exported
    // Seed every attributed source ID, including unresolved/non-placeable entries.
    for(const auto& source:itemindex::Entries){uint8_t* it=g_bpItemById&&g_bpItemReg?g_bpItemById(source.item,g_bpItemReg):nullptr;
        if(it&&(!readable(it,4)||*(uint32_t*)it!=source.item))it=nullptr;
        int kind=3;if(bp_block_item(it))kind=0;
        else if(native_item_resolution(source.item)==nativecatalog::Ready)kind=1;
        else if(it&&(item_kind(it)==ITEM_PIECE||item_kind(it)==ITEM_BIG_SHAPE)&&readable(it+0x427,1)&&!it[0x427])kind=2;
        g_catalog[g_catalogCount++]={source.item,kind};}
    uint8_t* reg = g_extendedItemGroups;
    if (!reg || !g_iterBegin || !g_iterNext || !g_bpItemReg || !g_bpItemById || !readable(reg, 0x48)) return false;
    uint64_t end = *(uint64_t*)(reg + 0x10); int guard = 0;
    uint64_t at = g_iterBegin(reg + 8);
    for (; at != end && guard++ < 200000; at = g_iterNext(reg + 8, at)) {
        auto groups = *(uint8_t***)(reg + 0x40);
        if (!groups || at > 2000000 || !readable(groups + at,sizeof(void*))) return false;
        uint8_t* group = groups[at]; if (!group || !readable(group, 16)) return false;
        const int32_t offset = *(int32_t*)(group + 8); const uint32_t n = *(uint32_t*)(group + 12);
        if(!n)continue;if (!offset || n > 20000) return false;
        const uint32_t* ids = (const uint32_t*)(group + 8 + offset); if (!readable(ids, (size_t)n * 4)) return false;
        for (uint32_t k = 0; k < n; ++k) {
            uint32_t id = ids[k]; if (!id) continue;
            bool duplicate = false; for (int j = 0; j < g_catalogCount; ++j) if (g_catalog[j].item == id) { duplicate = true; break; }
            if (duplicate) continue;
            uint8_t* item = g_bpItemById(id, g_bpItemReg); int kind = -1;
            if (bp_block_item(item)) kind = 0;
            else if (prop_item(item) && readable(item + 0x5d8, 4)) kind = project_prop_item(item) ? 1 : 3;
            else if (item && (item_kind(item) == ITEM_PIECE || item_kind(item) == ITEM_BIG_SHAPE) && readable(item + 0x427, 1) && !item[0x427]) kind = 2;
            if (kind >= 0) {if(g_catalogCount==8192){g_catalogTruncated=true;return false;}g_catalog[g_catalogCount++] = ExtendedItem{id, kind};}
        }
    }
    if(at!=end)return false;g_catalogTruncated=false;
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
    if(append&&project_classic_mode()){flash(T(S_EXT_CLASSIC_EDITED));return false;}
    uint8_t* item = g_bpItemById ? g_bpItemById(id, g_bpItemReg) : nullptr;
    if (!project_prop_item(item)) return false;
    double local[3]; for(int a=0;a<3;++a) local[a]=(offset ? offset[a] : 0)/16.0;
    float rotationQ[4],mn[3],mx[3]; const float identity[]={0,0,0,1}; core::bp_turn_rot(rotation,identity,rotationQ);
    extended::Box bounds{};
    if (!bp_item_bounds(item,mn,mx) || !extended::object_box(local,rotationQ,mn,mx,&bounds)) { LOG(.str("catalogue: invalid native object bounds item=").x(id)); return false; }
    if(append && g_projectLoaded)for(int i=0;i<g_projectManifest.spaceCount;++i)if(extended::intersects(bounds,spaces::box(g_projectManifest.spaces[i]))){flash(T(S_EXT_VOXEL_OBSTRUCTED));return false;}
    bool add = append && g_projectLoaded;
    if (!add && !project_generated_begin(bp_item_name(id) ? bp_item_name(id) : "Native object")) return false;
    if (g_projectObjectCount == kProjectMaxObjects) return false;
    auto& object = g_projectObjects[g_projectObjectCount++]; object = extended::Object{}; object.item = id;
    memcpy(object.rotation,rotationQ,sizeof rotationQ);
    if (!g_projectManifest.count) { g_projectManifest.count=1; memset(&g_projectManifest.modules[0],0,sizeof g_projectManifest.modules[0]); g_projectManifest.modules[0].category=extended::Furniture; }
    object.module = (uint16_t)(module >= 0 && module < g_projectManifest.count ? module : 0);
    g_projectPlan.objects = g_projectObjectCount; g_projectUseObjects = 1; g_projectNeedN = 0;
    for (int a = 0; a < 3; ++a) {
        object.p[a]=local[a];
        int low=bounds.lo[a]>=0 ? bounds.lo[a]/8*8 : -((-bounds.lo[a]+7)/8*8), high=bounds.hi[a]>=0 ? (bounds.hi[a]+7)/8*8 : -((-bounds.hi[a])/8*8);
        if (high<=low) high=low+8;
        if (!add || low<g_projectPlan.lo[a]) g_projectPlan.lo[a]=low;
        if (!add || high>g_projectPlan.hi[a]) g_projectPlan.hi[a]=high;
        auto& box=g_projectModuleBoxes[object.module];
        if (!add || low<box.lo[a]) box.lo[a]=low; if (!add || high>box.hi[a]) box.hi[a]=high;
    }
    project_generated_finish(); return true;
}
