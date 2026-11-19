// Read-only PlayerBases snapshot. BaseAt disassembly on the checked executable:
// RVA 0x470410: one mask bit -> bounding box; multiple bits -> intersections
// with the selected global sub-boxes. No altar is ever spawned or removed.
static extended::Box g_altarZones[256]; static int g_altarZoneCount = 0;
static extended::Proposal g_altarProposals[64]; static int g_altarProposalCount = 0;
static int g_altarScan = -1, g_altarUncovered = 0, g_altarUnassigned = 0, g_altarTemplate = 1;
static int g_altarTargets = 0;
static int g_altarDimensions[3] = {}, g_altarGeneration = -1, g_altarRotation = 0;
static bool g_altarAutoAdd = true;
static I3 g_altarOrigin;
static void altar_snapshot() {
    g_altarZoneCount = 0; uint8_t* sys = g_projectBaseSys;
    if (!sys || GetTickCount() - g_projectBaseSeen > 3000 || !readable(sys, 0x1340)) return;
    const int n = base_count(sys);
    if (!readable(base_rec(sys, 0), (size_t)n * 0x130)) return;
    const float* sub = *(const float**)(sys + 0x1330);
    for (int b = 0; b < n; ++b) {
        const uint8_t* rec = base_rec(sys, b); const float* box = (const float*)(rec + 0xf0);
        const uint32_t mask = *(const uint32_t*)(rec + 0x110);
        if (!mask) continue;
        bool single = (mask & (mask - 1)) == 0;
        if (!single && (!sub || !readable(sub, 0x200))) continue;
        for (int k = 0; k < (single ? 1 : 16) && g_altarZoneCount < 256; ++k) {
            if (!single && !(mask & (1u << k))) continue;
            extended::Box next{}; bool valid = true;
            for (int a = 0; a < 3; ++a) {
                double lo = box[a], hi = box[a + 4];
                if (!single) { if (lo < sub[k * 8 + a]) lo = sub[k * 8 + a]; if (hi > sub[k * 8 + a + 4]) hi = sub[k * 8 + a + 4]; }
                if (!extended::finite(lo) || !extended::finite(hi) || !(hi > lo)) { valid = false; break; }
                lo *= 16; hi *= 16; next.lo[a] = (int)lo; if (next.lo[a] < lo) ++next.lo[a];
                next.hi[a] = (int)hi; if (next.hi[a] > hi) --next.hi[a];
                if (next.hi[a] <= next.lo[a]) valid = false;
            }
            if (valid) g_altarZones[g_altarZoneCount++] = next;
        }
    }
}
static void altar_assist_begin(bool keep = false) {
    altar_snapshot();
    if (!g_projectLoaded || !g_altarZoneCount) { flash(T(S_PROJECT_UNAVAILABLE)); g_altarScan = -1; return; }
    if (g_altarTemplate > g_altarZoneCount) g_altarTemplate = 1;
    const auto& zone = g_altarZones[g_altarTemplate - 1];
    for (int a = 0; a < 3; ++a) g_altarDimensions[a] = zone.hi[a] - zone.lo[a];
    if (!keep) g_altarProposalCount = 0;
    g_altarScan = g_altarUncovered = g_altarUnassigned = 0; g_altarAutoAdd = !keep;
    g_altarTargets = g_projectPlan.count + 9 * (g_projectManifest.pieceCount + (g_projectUseObjects ? g_projectObjectCount : 0));
    g_altarOrigin = project_origin_now(); g_altarRotation = g_bpRot; g_altarGeneration = g_projectGeneration;
}
static int project_observed_zones(extended::Box* out,int capacity) {
    altar_snapshot();const int count=g_altarZoneCount<capacity ? g_altarZoneCount : capacity;
    for(int i=0;i<count;++i)out[i]=g_altarZones[i];return count;
}
static bool altar_target(int index, const I3& origin, int* low) {
    const int64_t world[] = {origin.x,origin.y,origin.z};
    if (index < g_projectPlan.count) {
        project::cell_low(g_projectPlan.cells[index],g_projectManifest.anchor,g_bpRot,low);
        for (int a=0;a<3;++a) low[a]+=(int)(world[a]/kProjectFixUnit); return true;
    }
    index-=g_projectPlan.count; const int part=index/9,sample=index%9; extended::Box box{};
    if (part < g_projectManifest.pieceCount) {
        box=g_projectPieceBoxes[part]; box.hi[1]+=g_projectManifest.pieces[part].clearance;
        for (int a=0;a<3;++a) { box.lo[a]-=g_projectManifest.anchor[a]; box.hi[a]-=g_projectManifest.anchor[a]; }
        box=extended::turn_box(box,g_bpRot);
        for (int a=0;a<3;++a) { box.lo[a]+=(int)(world[a]/kProjectFixUnit); box.hi[a]+=(int)(world[a]/kProjectFixUnit); }
    } else {
        const auto& object=g_projectObjects[part-g_projectManifest.pieceCount];
        double pos[3]; float rotation[4],mn[3],mx[3]; extended::object_world(object,g_projectManifest.anchor,g_bpRot,origin,pos,rotation);
        uint8_t* item=g_bpItemById ? g_bpItemById(object.item,g_bpItemReg) : nullptr;
        if (!bp_item_bounds(item,mn,mx) || !extended::object_box(pos,rotation,mn,mx,&box)) return false;
    }
    extended::box_sample(box,sample,low); return true;
}
static void altar_assist_update() {
    if (g_altarScan < 0 || !g_projectLoaded || !g_projectList || !g_exactOpen || g_bpPage != 1) return;
    I3 origin = project_origin_now();
    if (origin.x != g_altarOrigin.x || origin.y != g_altarOrigin.y || origin.z != g_altarOrigin.z || g_altarRotation != g_bpRot || g_altarGeneration != g_projectGeneration) altar_assist_begin();
    if (g_altarScan < 0) return;
    int end = g_altarScan + 256; if (end > g_altarTargets) end = g_altarTargets;
    for (; g_altarScan < end; ++g_altarScan) {
        int low[3]; if (!altar_target(g_altarScan,origin,low)) { ++g_altarUnassigned; continue; }
        bool covered = false;
        for (int z = 0; z < g_altarZoneCount && !covered; ++z) covered = extended::contains(g_altarZones[z], low);
        if (covered) continue;
        // Native BaseAt remains authoritative for cells straddling adjoining boxes.
        I3 center{(int64_t)(low[0] + 4) * kProjectFixUnit, (int64_t)(low[1] + 4) * kProjectFixUnit, (int64_t)(low[2] + 4) * kProjectFixUnit};
        if (project_covered(center)) continue;
        ++g_altarUncovered;
        for (int p = 0; p < g_altarProposalCount && !covered; ++p) covered = extended::contains(g_altarProposals[p].box, low);
        if (!covered && g_altarAutoAdd && g_altarProposalCount < 64) {
            if (extended::suggest(low, g_altarDimensions, &g_altarProposals[g_altarProposalCount])) { ++g_altarProposalCount; covered = true; }
        }
        if (!covered) ++g_altarUnassigned;
    }
}
