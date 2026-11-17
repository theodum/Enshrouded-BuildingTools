// Native adapter. Included after blueprint.h; calls the existing game's dry-run,
// payment, commit and undo paths. No world/save-file writer lives here.
static bool key_down(int code);
static constexpr int64_t kProjectFixUnit = (int64_t)1 << 28;
struct ProjectStateGuard {
    ProjectStateGuard() { EnterCriticalSection(&g_stateLock); }
    ~ProjectStateGuard() { LeaveCriticalSection(&g_stateLock); }
};
static project::Manifest g_projectManifest;
static extended::Box g_projectModuleBoxes[project::MaxModules];
static extended::Object* g_projectObjects = nullptr;
static int g_projectObjectCount = 0, g_projectObjectsInJob = 0;
static constexpr int kProjectMaxObjects = project::MaxModules * core::BP_MAX_OBJS;
static project::Plan g_projectPlan{};
static project::Job g_projectJob;
static bool project_object_batch() { return g_projectJob.next >= g_projectPlan.count + g_projectManifest.pieceCount; }
static bool project_piece_batch() { return g_projectJob.next >= g_projectPlan.count && !project_object_batch(); }
static int g_projectRotation = 0, g_projectWaiting = 0;
static I3 g_projectOrigin;
static DWORD g_projectDue = 0, g_projectWaitSince = 0;
static uint32_t g_projectShape = 0;
static int g_projectGeneration = 0;
static bool g_projectModuleVisible[project::MaxModules];
static int g_projectSlice = 0, g_projectSliceY = 0, g_projectSliceHeight = 64;
static bool g_projectCreative = false;
static bool g_projectInspect = false;
static bool g_projectProblem = false;
static I3 g_projectProblemAt;
static const char* g_projectLastProblem = nullptr;
static DWORD g_projectReturnAt = 0;
static void project_preview_return() {
    g_projectRunning = 0; g_projectBatch = false; g_projectWaiting = 0; g_exactArmed = 0; g_exactFire = 0;
    g_bpRot = g_projectRotation; g_bpWin = BPW_PLACE; g_bpPage = 1; g_bpPlaced = 1;
    g_bpAnchor = I3{g_projectOrigin.x + ((int64_t)1 << 30), g_projectOrigin.y + ((int64_t)1 << 30), g_projectOrigin.z + ((int64_t)1 << 30)};
    for (int a = 0; a < 3; ++a) g_bpOff[a] = 0;
    g_projectReturnAt = GetTickCount() + 200;
}
static void project_return_poll(bool focus, bool inGame) {
    if (!g_projectReturnAt) return;
    DWORD now = GetTickCount();
    const int32_t elapsed = extended::elapsed(now, g_projectReturnAt);
    if (!focus || elapsed > 3000) { g_projectReturnAt = 0; return; }
    if (elapsed >= 0 && inGame && !(GetAsyncKeyState(VK_LBUTTON) & 0x8000) && !(GetAsyncKeyState(VK_RBUTTON) & 0x8000)) {
        g_projectReturnAt = 0; g_exactReq = 1;
    }
}
static int g_projectNeedN = 0;
static uint8_t* g_projectSourceMaterial = nullptr;
static extended::Box g_projectPieceBoxes[project::MaxPieces];
static bool project_check_pieces(void*, void*, const CursorXf&, bool);
static uint8_t project_queue_pieces(CostList*, void*, void*, uint8_t*, const CursorXf&);
static bool project_piece_bounds();
struct ProjectNeed { uint32_t item; uint64_t count; bool prop; };
static ProjectNeed g_projectNeed[128];

static bool project_journal(const char* event, const char* detail = "") {
    Buf b; b.str("project ").str(event).str(": ").str(g_projectManifest.name)
        .str(" next=").i(g_projectJob.next).str(" completed=").i(g_projectJob.completed)
        .str(" batch=").i(g_projectJob.batch).str(" turn=").i(g_projectRotation)
        .str(" origin=(").fix(g_projectOrigin.x).c(',').fix(g_projectOrigin.y).c(',').fix(g_projectOrigin.z)
        .str(") ").str(detail);
    log_line(b);
    wchar_t path[MAX_PATH];
    if (lstrlenW(g_dir) + 32 >= MAX_PATH) return false;
    lstrcpyW(path, g_dir); lstrcatW(path, L"BuildingTools-Projects.log");
    HANDLE h = CreateFileW(path, FILE_APPEND_DATA, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;
    SYSTEMTIME t; GetLocalTime(&t); Buf line;
    line.u(t.wYear).c('-').u(t.wMonth, 10, 2).c('-').u(t.wDay, 10, 2).c(' ')
        .u(t.wHour, 10, 2).c(':').u(t.wMinute, 10, 2).c(':').u(t.wSecond, 10, 2).c(' ').str(b.z()).str("\r\n");
    DWORD written = 0;
    const BOOL wrote = WriteFile(h, line.z(), line.n, &written, nullptr), flushed = FlushFileBuffers(h);
    CloseHandle(h); return wrote && flushed && written == (DWORD)line.n;
}
static void project_error(const project::Error& e) {
    Buf b; b.fmt(T(S_PROJECT_ERROR), e.message ? e.message : "Unknown error"); flash(b.z()); beep(MB_ICONHAND);
    LOG(.str("project load: ").str(e.message ? e.message : "unknown").str(" byte=").i(e.at).str(" module=").i(e.module));
}
static bool project_path(wchar_t* out, const wchar_t* leaf) {
    if (lstrlenW(g_dir) + 23 + lstrlenW(leaf) >= MAX_PATH) return false;
    lstrcpyW(out, g_dir); lstrcatW(out, L"BuildingTools-Projects\\"); lstrcatW(out, leaf); return true;
}
static bool project_load(const wchar_t* name) {
    ProjectStateGuard guard;
    if (g_projectRunning) return false;
    memset(g_projectModuleVisible, 1, sizeof g_projectModuleVisible);
    g_projectLoaded = false; g_bp.ok = false; g_bpBitsOk = 0;
    g_projectNeedN = 0; g_projectLastProblem = nullptr; g_projectProblem = false;
    if (g_projectObjects) VirtualFree(g_projectObjects, 0, MEM_RELEASE);
    g_projectObjectCount = 0;
    g_projectObjects = (extended::Object*)VirtualAlloc(nullptr, sizeof(extended::Object) * kProjectMaxObjects, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (g_projectPlan.cells) VirtualFree(g_projectPlan.cells, 0, MEM_RELEASE);
    g_projectPlan = project::Plan{};
    wchar_t path[MAX_PATH]; project::Error e;
    if (!project_path(path, name)) { e.message = "Project path too long"; project_error(e); return false; }
    DWORD size = 0; char* data = read_file(path, &size);
    if (!data) { e.message = "Cannot read project manifest"; project_error(e); return false; }
    bool ok = project::decode(data, (int)size, &g_projectManifest, &e);
    HeapFree(GetProcessHeap(), 0, data);
    if (!ok) { project_error(e); return false; }
    g_projectPlan.cells = (project::CellEntry*)VirtualAlloc(nullptr, sizeof(project::CellEntry) * project::MaxCells, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    g_projectPlan.capacity = project::MaxCells;
    if (!g_projectPlan.cells || !g_projectObjects) { e.message = "Cannot allocate project plan"; project_error(e); return false; }
    // Decode one source at a time: no dense bounding volume or array of 128 blueprints.
    static core::BpHeader h; static uint8_t cells[kBpMaxCells];
    for (int m = 0; m < g_projectManifest.count; ++m) {
        e.module = m;
        wchar_t file[192];
        if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, g_projectManifest.modules[m].file, -1, file, 192) || !project_path(path, file)) {
            e.message = "Invalid UTF-8 or project module path too long"; project_error(e); return false;
        }
        data = read_file(path, &size);
        if (!data) { e.message = "Referenced blueprint is missing or unreadable"; project_error(e); return false; }
        ok = core::bp_decode((const uint8_t*)data, (int)size, &h, cells, kBpMaxCells);
        HeapFree(GetProcessHeap(), 0, data);
        if (!ok) { e.message = "Referenced blueprint is invalid or exceeds 65536 source cells"; project_error(e); return false; }
        if (!project::add(&g_projectPlan, g_projectManifest.modules[m], m, h, cells, &e)) { project_error(e); return false; }
        const auto& module = g_projectManifest.modules[m];
        for (int k = 0; k < h.objects; ++k) {
            if (g_projectObjectCount == kProjectMaxObjects || !extended::transform_object(module, m, h, h.obj[k], &g_projectObjects[g_projectObjectCount])) {
                e.message = "Invalid object position or quaternion"; project_error(e); return false;
            }
            ++g_projectObjectCount;
        }
        for (int a = 0; a < 3; ++a) {
            const int axis = (module.rotation & 1) && a != 1 ? 2 - a : a;
            g_projectModuleBoxes[m].lo[a] = module.offset[a];
            g_projectModuleBoxes[m].hi[a] = module.offset[a] + (int)(h.n[axis] * h.step[axis] * 16);
        }
        LOG(.str("project module ").i(m).str(": ").str(g_projectManifest.modules[m].file).str(" cells so far=").i(g_projectPlan.count));
    }
    if (g_projectPlan.count) {
        if (!project::finish(&g_projectPlan, &e)) { project_error(e); return false; }
    } else if (!g_projectObjectCount && !g_projectManifest.pieceCount) { e.message = "Project is empty"; project_error(e); return false; }
    else for (int a = 0; a < 3; ++a) { g_projectPlan.lo[a] = 0; g_projectPlan.hi[a] = 8; }
    if (!project_piece_bounds()) { e.message = "Unknown native piece or invalid native bounds"; project_error(e); return false; }
    memset(&g_bp.h, 0, sizeof g_bp.h);
    for (int a = 0; a < 3; ++a) {
        g_bp.h.step[a] = 0.5f; g_bp.h.n[a] = (g_projectPlan.hi[a] - g_projectPlan.lo[a]) / project::Cell;
    }
    g_bp.h.nm = g_projectPlan.materialCount;
    for (int m = 0; m < g_bp.h.nm; ++m) g_bp.h.m[m] = g_projectPlan.materials[m];
    g_bp.filled = g_projectPlan.count; g_bp.ok = true; g_bpMirror = 0; g_projectLoaded = true; ++g_projectGeneration;
    LOG(.str("project loaded: ").str(g_projectManifest.name).str(" modules=").i(g_projectManifest.count)
        .str(" occupied native cells=").i(g_projectPlan.count).str(" duplicates removed=").i(g_projectPlan.duplicates)
        .str(" objects loaded=").i(g_projectObjectCount));
    return true;
}
static I3 project_origin_now() {
    I3 p = bp_anchor_now();
    const int64_t half = core::kFix / 4;
    p.x -= half; p.y -= half; p.z -= half; return p;
}
static I3 project_position(int index) {
    return project::position(g_projectPlan.cells[g_projectJob.next + index], g_projectManifest.anchor, g_projectRotation, g_projectOrigin);
}
static void project_bounds(int rotation, int* lo, int* hi) {
    for (int corner = 0; corner < 8; ++corner) {
        int p[3], turned[3];
        for (int a = 0; a < 3; ++a) p[a] = ((corner >> a) & 1 ? g_projectPlan.hi[a] : g_projectPlan.lo[a]) - g_projectManifest.anchor[a];
        project::turn(rotation, p, turned);
        for (int a = 0; a < 3; ++a) {
            if (!corner || turned[a] < lo[a]) lo[a] = turned[a];
            if (!corner || turned[a] > hi[a]) hi[a] = turned[a];
        }
    }
}
static bool project_box(int64_t* lo, int64_t* hi, float* step) {
    if (!g_projectLoaded || g_bpWin != BPW_PLACE || g_bpPage != 1) return false;
    int low[3], high[3]; project_bounds(g_projectRunning ? g_projectRotation : (int)g_bpRot, low, high);
    const I3 origin = g_projectRunning ? g_projectOrigin : project_origin_now();
    const int64_t xyz[3] = {origin.x, origin.y, origin.z};
    for (int a = 0; a < 3; ++a) { lo[a] = xyz[a] + (int64_t)low[a] * kProjectFixUnit; hi[a] = xyz[a] + (int64_t)high[a] * kProjectFixUnit; step[a] = 0.5f; }
    return true;
}
static void project_ghost_bits() {
    static int generation = -1, rotation = -1;
    if (generation == g_projectGeneration && rotation == g_bpRot) return;
    generation = g_projectGeneration; rotation = g_bpRot; g_bpBitsOk = 0;
    if (!g_projectLoaded) return;
    int lo[3], hi[3], n[3]; project_bounds(rotation, lo, hi); int64_t volume = 1;
    for (int a = 0; a < 3; ++a) { n[a] = (hi[a] - lo[a]) / project::Cell; volume *= n[a]; }
    if (volume > 40000) return; // existing renderer: bounded volume LOD for large projects
    memset(g_bpBits, 0, sizeof g_bpBits);
    for (int i = 0; i < g_projectPlan.count; ++i) {
        int p[3]; project::cell_low(g_projectPlan.cells[i], g_projectManifest.anchor, rotation, p);
        const auto& cell = g_projectPlan.cells[i];
        if (!g_projectModuleVisible[cell.module] || (g_projectSlice && (cell.p[1] < g_projectSliceY || cell.p[1] >= g_projectSliceY + g_projectSliceHeight))) continue;
        for (int a = 0; a < 3; ++a) p[a] = (p[a] - lo[a]) / project::Cell;
        const int bit = p[0] + n[0] * (p[1] + n[1] * p[2]);
        g_bpBits[bit >> 5] |= 1u << (bit & 31);
    }
    g_bpBitsOk = 1;
}
static void project_stop(const char* why) {
    if (!g_projectRunning) return;
    g_projectJob.phase = project::Phase::Stopped;
    project_journal("STOP", why);
    g_projectRunning = 0; g_projectWaiting = 0; g_projectBatch = false;
    g_exactArmed = 0; g_exactFire = 0; g_bpWin = BPW_NONE; g_bpGhost = 0;
    g_pend.n = 0; // preserve the project marker to suppress a late native commit
    g_bpRot = g_projectRotation;
    Buf b; b.fmt(T(S_EXT_JOB_STOP), num(g_projectJob.completed).s, why); flash(b.z()); beep(MB_ICONHAND);
    g_projectLastProblem = why;
    if (g_projectInspect) project_preview_return();
}
static bool project_start() {
    if (!g_projectLoaded || g_projectRunning || remote_session()) return false;
    for (int a = 0; a < 3; ++a) if (g_scStep[a] != 0.5f) { flash(T(S_PROJECT_SHAPE)); return false; }
    if (!g_remove || !g_caveForce || !g_baseAt || !g_projectBaseSys || GetTickCount() - g_projectBaseSeen > 10000) { flash(T(S_PROJECT_UNAVAILABLE)); return false; }
    g_projectOrigin = project_origin_now(); g_projectRotation = g_bpRot;
    g_projectProblem = false; g_projectLastProblem = nullptr;
    g_projectObjectsInJob = g_projectUseObjects ? g_projectObjectCount : 0;
    if (!g_projectPlan.count && !g_projectObjectsInJob && !g_projectManifest.pieceCount) { flash(T(S_BP_NOTHING)); return false; }
    g_projectJob.start(g_projectPlan.count + g_projectManifest.pieceCount + g_projectObjectsInJob); g_projectWaiting = 0; g_projectNeedN = 0;
    g_projectSourceMaterial = nullptr;
    g_projectShape = 0; g_projectCreative = creative_on();
    g_projectDue = GetTickCount() + 500; g_projectWaitSince = GetTickCount();
    g_projectRunning = 1;
    if (!project_journal("BEGIN")) { project_stop(T(S_PROJECT_JOURNAL)); g_bpWin = BPW_PLACE; return false; }
    flash(T(S_PROJECT_BEGIN));
    return true;
}
static bool project_cost(uint32_t item, uint32_t count, bool prop = false) {
    int n = 0; while (n < g_projectNeedN && g_projectNeed[n].item != item) ++n;
    if (n == 128) return false;
    if (n == g_projectNeedN) { g_projectNeed[n] = ProjectNeed{item, 0, prop}; ++g_projectNeedN; }
    g_projectNeed[n].count += count;
    return g_projectNeed[n].count <= 0x7fffffff;
}
// The engine's base_at handles the actual altar boxes and sub-boxes. Test every
// corner inside each 0.5 m voxel, in 3D; never infer coverage from a 2D footprint.
static bool project_covered(const I3& p) {
    uint8_t* sys = g_projectBaseSys;
    if (!sys || !g_baseAt || GetTickCount() - g_projectBaseSeen > 10000 || !readable(sys, 0x1340) || !base_count(sys)) return false;
    for (int c = 0; c < 8; ++c) {
        const int64_t xyz[3] = {p.x, p.y, p.z}; float v[3];
        for (int a = 0; a < 3; ++a) v[a] = (float)((double)xyz[a] / core::kFix + ((c >> a) & 1 ? 0.21875 : -0.21875));
        if (base_at(sys, v) < 0) return false;
    }
    return true;
}
static uint8_t* project_piece_item(const project::NativePiece& piece) {
    if (!g_nativePartsAvailable) return nullptr;
    uint8_t* item = g_bpItemById && g_bpItemReg ? g_bpItemById(piece.item, g_bpItemReg) : nullptr;
    const int kind = item_kind(item);
    return item && (kind == ITEM_PIECE || kind == ITEM_BIG_SHAPE) && readable(item + 0x427, 1) && !item[0x427] ? item : nullptr;
}
static CursorXf project_piece_transform(const project::NativePiece& piece, bool world = true) {
    int p[3]; for (int a = 0; a < 3; ++a) p[a] = piece.offset[a] - (world ? g_projectManifest.anchor[a] : 0);
    int turned[3]; project::turn(world ? g_projectRotation : 0, p, turned);
    CursorXf xf{}; const int64_t origin[] = {g_projectOrigin.x, g_projectOrigin.y, g_projectOrigin.z};
    for (int a = 0; a < 3; ++a) xf.pos[a] = (world ? origin[a] : 0) + (int64_t)turned[a] * kProjectFixUnit;
    const float identity[] = {0, 0, 0, 1}; core::bp_turn_rot(piece.rotation + (world ? g_projectRotation : 0), identity, xf.rot); xf.scale = 1.f;
    return xf;
}
static bool project_piece_bounds() {
    int kept = 0;
    for (int i = 0; i < g_projectManifest.pieceCount; ++i) {
        const auto p = g_projectManifest.pieces[i]; bool duplicate = false;
        for (int j = 0; j < kept; ++j) {
            auto& other = g_projectManifest.pieces[j];
            if (p.item == other.item && p.material == other.material && p.rotation == other.rotation && !memcmp(p.offset, other.offset, sizeof p.offset)) { if (p.clearance > other.clearance) other.clearance = p.clearance; duplicate = true; break; }
        }
        if (!duplicate) g_projectManifest.pieces[kept++] = p;
    }
    g_projectManifest.pieceCount = kept;
    for (int i = 0; i < g_projectManifest.pieceCount; ++i) {
        const auto& piece = g_projectManifest.pieces[i]; uint8_t* item = project_piece_item(piece); if (!item || !g_bounds) return false;
        CursorXf xf = project_piece_transform(piece, false); Aabb box{}; shape_bounds(item, &xf, &box);
        for (int a = 0; a < 3; ++a) {
            if (!extended::finite(box.mn[a]) || !extended::finite(box.mx[a]) || !(box.mx[a] > box.mn[a])) return false;
            int lo = (int)(box.mn[a] * 16), hi = (int)(box.mx[a] * 16);
            if (box.mn[a] * 16 < lo) --lo; if (box.mx[a] * 16 > hi) ++hi;
            g_projectPieceBoxes[i].lo[a] = lo; g_projectPieceBoxes[i].hi[a] = hi;
            const int gridLo = lo >= 0 ? lo / 8 * 8 : -((-lo + 7) / 8 * 8);
            const int gridHi = hi >= 0 ? (hi + 7) / 8 * 8 : -((-hi) / 8 * 8);
            if (gridLo < g_projectPlan.lo[a]) g_projectPlan.lo[a] = gridLo; if (gridHi > g_projectPlan.hi[a]) g_projectPlan.hi[a] = gridHi;
        }
    }
    return true;
}
static bool project_check_pieces(void* ctx, void* state, const CursorXf& target, bool costs) {
    Aabb templateBox; shape_bounds(state, &target, &templateBox); I3 tc = box_centre(templateBox);
    const int64_t off[] = {target.pos[0] - tc.x, target.pos[1] - tc.y, target.pos[2] - tc.z};
    static CostEntry entries[256];
    for (int i = 0; i < g_projectJob.batch; ++i) {
        const int index = g_projectJob.next - g_projectPlan.count + i;
        const auto& piece = g_projectManifest.pieces[index]; uint8_t* item = project_piece_item(piece);
        uint8_t* material = piece.material && g_bpItemById ? g_bpItemById(piece.material, g_bpItemReg) : g_projectSourceMaterial;
        if (!item || !bp_block_item(material)) { project_stop(T(S_PROJECT_MATERIAL)); return false; }
        CursorXf xf = project_piece_transform(piece); Aabb box{}; shape_bounds(item, &xf, &box);
        int low[3], high[3]; int64_t cells = 1;
        for (int a = 0; a < 3; ++a) {
            if (!extended::finite(box.mn[a]) || !extended::finite(box.mx[a]) || box.mx[a] <= box.mn[a]) { project_stop(T(S_PROJECT_RANGE)); return false; }
            low[a] = (int)(box.mn[a] * 2); if (low[a] > box.mn[a] * 2) --low[a];
            high[a] = (int)(box.mx[a] * 2); if (high[a] < box.mx[a] * 2) ++high[a];
            cells *= high[a] - low[a];
        }
        high[1] += (piece.clearance + 7) / 8;
        cells = (int64_t)(high[0] - low[0]) * (high[1] - low[1]) * (high[2] - low[2]);
        if (cells > 32768) { project_stop(T(S_PROJECT_RANGE)); return false; }
        if (costs) for (int c = 0; c < g_projectPlan.count; ++c) {
            const auto& cell = g_projectPlan.cells[c]; const auto& b = g_projectPieceBoxes[index]; bool overlaps = true;
            for (int a = 0; a < 3; ++a) if (cell.p[a] + 8 <= b.lo[a] || cell.p[a] >= b.hi[a] + (a == 1 ? piece.clearance : 0)) overlaps = false;
            if (overlaps) { project_stop(T(S_EXT_PIECE_OVERLAP)); return false; }
        }
        if (costs) for (int other = 0; other < index; ++other) {
            const bool overlap = extended::intersects(g_projectPieceBoxes[index], g_projectPieceBoxes[other], piece.clearance)
                || extended::intersects(g_projectPieceBoxes[other], g_projectPieceBoxes[index], g_projectManifest.pieces[other].clearance);
            if (overlap) { project_stop(T(S_EXT_PIECE_OVERLAP)); return false; }
        }
        for (int y = low[1]; y < high[1]; ++y) for (int z = low[2]; z < high[2]; ++z) for (int x = low[0]; x < high[0]; ++x) {
            I3 p{(int64_t)(x * 8 + 4) * kProjectFixUnit, (int64_t)(y * 8 + 4) * kProjectFixUnit, (int64_t)(z * 8 + 4) * kProjectFixUnit};
            if (!project_covered(p)) { g_projectProblem = true; g_projectProblemAt = p; project_stop(T(S_PROJECT_COVERAGE)); return false; }
            CursorXf probe = target; probe.pos[0] = p.x + off[0]; probe.pos[1] = p.y + off[1]; probe.pos[2] = p.z + off[2];
            CostList scratch{entries, 0, 256, nullptr}; memset(g_remVols, 0, sizeof g_remVols); price_removal(&scratch, ctx, state, &probe); g_refundN = 0; g_refundList = nullptr;
            for (int m = 0x80; m < 256; ++m) if (g_remVols[m] > 0) { g_projectProblem = true; g_projectProblemAt = p; project_stop(T(S_PROJECT_COLLISION)); return false; }
        }
        CostList price{entries, 0, 256, nullptr};
        if (!g_placeOrig(&price, ctx, item, material, &xf)) { project_stop(T(S_PROJECT_NATIVE)); return false; }
        if (costs) for (uint64_t q = 0; q < price.count; ++q) if (!project_cost(*(uint32_t*)(entries[q].b + 0x10), *(uint32_t*)(entries[q].b + 0x14))) { project_stop(T(S_PROJECT_MATERIAL)); return false; }
    }
    return true;
}
static uint8_t project_queue_pieces(CostList* list, void* ctx, void* state, uint8_t* pdata, const CursorXf& target) {
    uint64_t before = list->count; pend_begin(ctx, target, false, state ? *(uint32_t*)state : 0); g_pend.bp = g_pend.project = true;
    for (int i = 0; i < g_projectJob.batch; ++i) {
        const auto& piece = g_projectManifest.pieces[g_projectJob.next - g_projectPlan.count + i]; uint8_t* item = project_piece_item(piece);
        uint8_t* material = piece.material ? g_bpItemById(piece.material, g_bpItemReg) : g_projectSourceMaterial; CursorXf xf = project_piece_transform(piece);
        g_pendObj[i] = 0; g_pendMat[i] = 0xff; g_pendNativeShape[i] = piece.item; g_pendNativeMaterial[i] = *(uint32_t*)material; memcpy(g_pendNativeRotation[i], xf.rot, 16);
        if (!price_shape(list, ctx, item, material, &xf, false)) { list->count = before; project_stop(T(S_PROJECT_NATIVE)); return 0; }
    }
    g_pendCost[g_pend.n] = (uint32_t)list->count; g_pend.tick = GetTickCount(); g_actList = list; g_actShapes = g_pend.n;
    if (!project_journal("NATIVE_PIECE_BATCH_BEGIN")) { list->count = before; project_stop(T(S_PROJECT_JOURNAL)); return 0; }
    return 1;
}
static bool project_check(void* ctx, void* state, const CursorXf& target, bool costs) {
    Aabb box; shape_bounds(state, &target, &box);
    for (int a = 0; a < 3; ++a) if (box.mx[a] - box.mn[a] != 0.5f) { project_stop(T(S_PROJECT_SHAPE)); return false; }
    const I3 tc = box_centre(box);
    const int64_t off[3] = {target.pos[0] - tc.x, target.pos[1] - tc.y, target.pos[2] - tc.z};
    static CostEntry entries[256];
    uint8_t* material[core::BP_MAX_MATS] = {};
    if (!bp_item_registry(ctx, state)) { project_stop(T(S_PROJECT_UNAVAILABLE)); return false; }
    if (project_piece_batch()) return project_check_pieces(ctx, state, target, costs);
    if (project_object_batch()) {
        if (creative_on() && !g_creativeProps) { project_stop(T(S_EXT_OBJECT_CREATIVE)); return false; }
        for (int i = 0; i < g_projectJob.batch; ++i) {
            const auto& o = g_projectObjects[g_projectJob.next - g_projectPlan.count - g_projectManifest.pieceCount + i];
            uint8_t* it = g_bpItemById(o.item, g_bpItemReg);
            if (!extended_prop_item(it)) { project_stop(T(S_EXT_OBJECT_UNSAFE)); return false; }
            double pos[3]; float rot[4]; extended::object_world(o, g_projectManifest.anchor, g_projectRotation, g_projectOrigin, pos, rot);
            I3 center{(int64_t)(pos[0] * core::kFix), (int64_t)(pos[1] * core::kFix), (int64_t)(pos[2] * core::kFix)};
            if (!project_covered(center)) { project_stop(T(S_PROJECT_COVERAGE)); return false; }
            float low[3], high[3];
            if (!bp_item_bounds(it, low, high)) { project_stop(T(S_PROJECT_UNAVAILABLE)); return false; }
            const double x = rot[0], y = rot[1], z = rot[2], w = rot[3];
            for (int corner = 0; corner < 8; ++corner) {
                double p[3]; for (int a = 0; a < 3; ++a) { double eps = (high[a] - low[a]) * 0.001; p[a] = (corner >> a) & 1 ? high[a] - eps : low[a] + eps; }
                float v[] = {(float)(pos[0] + (1-2*(y*y+z*z))*p[0] + 2*(x*y-w*z)*p[1] + 2*(x*z+w*y)*p[2]),
                             (float)(pos[1] + 2*(x*y+w*z)*p[0] + (1-2*(x*x+z*z))*p[1] + 2*(y*z-w*x)*p[2]),
                             (float)(pos[2] + 2*(x*z-w*y)*p[0] + 2*(y*z+w*x)*p[1] + (1-2*(x*x+y*y))*p[2])};
                if (!extended::finite(v[0]) || !extended::finite(v[1]) || !extended::finite(v[2]) || base_at(g_projectBaseSys, v) < 0) { project_stop(T(S_PROJECT_COVERAGE)); return false; }
            }
            if (costs && !project_cost(o.item, 1, true)) { project_stop(T(S_PROJECT_MATERIAL)); return false; }
        }
        return true;
    }
    for (int m = 0; m < g_projectPlan.materialCount; ++m) {
        const auto& bm = g_projectPlan.materials[m];
        if (!bm.cells) continue;
        uint8_t* it = g_bpItemById(bm.item, g_bpItemReg);
        if (!bp_block_item(it)) { // same legacy #61 guard as classic blueprints
            uint8_t* blk = bp_block_of_material((uint8_t*)state, bm.mat); CostEntry refund;
            if (blk && (!bp_entry(ctx, bm.mat, &refund) || *(uint32_t*)(refund.b + 0x10) == bm.item)) it = blk;
        }
        if (!bp_block_item(it)) { project_stop(T(S_PROJECT_MATERIAL)); return false; }
        material[m] = it;
    }
    for (int i = 0; i < g_projectJob.batch; ++i) {
        const I3 p = project_position(i);
        const double xyz[3] = {(double)p.x / core::kFix, (double)p.y / core::kFix, (double)p.z / core::kFix};
        for (int a = 0; a < 3; ++a) if (xyz[a] < -65536.0 || xyz[a] > 65536.0) { project_stop(T(S_PROJECT_RANGE)); return false; }
        if (!project_covered(p)) { g_projectProblem = true; g_projectProblemAt = p; project_stop(T(S_PROJECT_COVERAGE)); return false; }
        CursorXf xf = target; xf.pos[0] = p.x + off[0]; xf.pos[1] = p.y + off[1]; xf.pos[2] = p.z + off[2];
        CostList scratch = {entries, 0, 256, nullptr};
        memset(g_remVols, 0, sizeof g_remVols);
        price_removal(&scratch, ctx, state, &xf);
        g_refundN = 0; g_refundList = nullptr;
        for (int m = 0x80; m < 256; ++m) if (g_remVols[m] > 0.f) {
            LOG(.str("project collision at cell ").i(g_projectJob.next + i).str(" module=").i(g_projectPlan.cells[g_projectJob.next + i].module));
            g_projectProblem = true; g_projectProblemAt = p;
            project_stop(T(S_PROJECT_COLLISION)); return false;
        }
        scratch.count = 0;
        const int mi = g_projectPlan.cells[g_projectJob.next + i].material;
        if (!g_placeOrig(&scratch, ctx, state, material[mi], &xf)) { project_stop(T(S_PROJECT_NATIVE)); return false; }
        if (costs) for (uint64_t q = 0; q < scratch.count; ++q)
            if (!project_cost(*(uint32_t*)(entries[q].b + 0x10), *(uint32_t*)(entries[q].b + 0x14))) { project_stop(T(S_PROJECT_MATERIAL)); return false; }
    }
    return true;
}
static uint8_t project_click(CostList* list, void* ctx, void* state, uint8_t* pdata, const CursorXf& target) {
    if (g_exactOpen || g_projectWaiting != 1) return 0;
    g_exactArmed = 0; g_exactFire = 0; g_projectWaiting = 2; g_projectWaitSince = GetTickCount();
    if (!g_projectShape) g_projectShape = state ? *(uint32_t*)state : 0;
    if (!g_projectSourceMaterial) g_projectSourceMaterial = pdata;
    if (!g_projectShape || !state || *(uint32_t*)state != g_projectShape || remote_session() || creative_on() != g_projectCreative) { project_stop(T(S_PROJECT_CHANGED)); return 0; }
    const bool checking = g_projectJob.phase == project::Phase::Checking;
    if (!project_check(ctx, state, target, checking)) return 0;
    if (checking) {
        g_projectJob.checked(g_projectJob.batch);
        if (g_projectJob.phase == project::Phase::Building && !creative_on()) {
            for (int q = 0; q < g_projectNeedN; ++q) {
                const int64_t have = stock_of(g_projectNeed[q].item);
                if (g_projectNeed[q].prop) continue; // native payment is authoritative for props, absent from the block stock cache
                if (have < 0 || (uint64_t)have < g_projectNeed[q].count) { project_stop(T(S_PROJECT_STOCK)); return 0; }
            }
        }
        project_journal(g_projectJob.phase == project::Phase::Checking ? "CHECK" : "CHECKED");
        if (g_projectInspect && g_projectJob.phase == project::Phase::Building) {
            project_journal("INSPECTED"); project_preview_return(); flash(T(S_EXT_CHECK_OK)); return 0;
        }
        g_projectWaiting = 0; g_projectDue = GetTickCount() + 150;
        return 0; // preflight clicks never return a buildable action to the game
    }
    // A small synthetic header lets bp_fill keep its proven material mapping,
    // native costs and commit queue. Positions come from the immutable sparse plan.
    g_bpRot = 0; g_bpMirror = 0; g_projectBatch = true;
    if (project_piece_batch()) return project_queue_pieces(list, ctx, state, pdata, target);
    if (project_object_batch()) {
        const uint64_t before = list->count;
        pend_begin(ctx, target, false, state ? *(uint32_t*)state : 0); g_pend.bp = g_pend.project = true; g_pend.list = list;
        for (int i = 0; i < g_projectJob.batch; ++i) {
            const auto& o = g_projectObjects[g_projectJob.next - g_projectPlan.count - g_projectManifest.pieceCount + i];
            uint8_t* it = g_bpItemById(o.item, g_bpItemReg); g_pendCost[i] = (uint32_t)list->count;
            CostEntry* e = list_push(list);
            if (!e) { list->count = before; project_stop(T(S_PROJECT_NATIVE)); return 0; }
            const uint32_t one = 1; memcpy(e->b, it + 4, 16); memcpy(e->b + 0x10, &o.item, 4); memcpy(e->b + 0x14, &one, 4);
            g_pendObj[i] = (uint16_t)(i + 1); g_pendMat[i] = 0xff; memset(g_pendPos[i], 0, 24); ++g_pend.n;
        }
        g_pendCost[g_pend.n] = (uint32_t)list->count;
        if (!project_journal("OBJECT_BATCH_BEGIN")) { list->count = before; project_stop(T(S_PROJECT_JOURNAL)); return 0; }
        g_pend.tick = GetTickCount(); g_actList = list; g_actShapes = g_pend.n; return 1;
    }
    g_bp.h.n[0] = g_projectJob.batch; g_bp.h.n[1] = g_bp.h.n[2] = 1;
    g_bp.h.objects = 0; g_bp.filled = g_projectJob.batch;
    for (int a = 0; a < 3; ++a) g_bp.h.step[a] = 0.5f;
    for (int i = 0; i < g_projectJob.batch; ++i) g_bp.cell[i] = g_projectPlan.cells[g_projectJob.next + i].material;
    g_bpMinArm = project_position(0);
    if (!project_journal("BATCH_BEGIN")) { project_stop(T(S_PROJECT_JOURNAL)); return 0; }
    uint8_t result = bp_fill(list, ctx, state, pdata, target);
    if (!result || g_pend.n != g_projectJob.batch) { project_stop(T(S_PROJECT_NATIVE)); return 0; }
    return result; // the game pays and invokes the existing commit hook
}
static bool project_commit_object(int index, void* ctx) {
    if (!project_object_batch()) return false;
    const auto& o = g_projectObjects[g_projectJob.next - g_projectPlan.count - g_projectManifest.pieceCount + index];
    double pos[3]; float rot[4]; extended::object_world(o, g_projectManifest.anchor, g_projectRotation, g_projectOrigin, pos, rot);
    return bp_place_object(ctx, o.item, pos, rot);
}
static void project_committed(int expected, int written) {
    if (!g_projectRunning) return;
    Buf counts; counts.str("expected=").i(expected).str(" written=").i(written);
    if (!project_journal("BATCH_RESULT", counts.z())) {
        if (written > 0 && written <= expected) g_projectJob.completed += written;
        project_stop(T(S_PROJECT_JOURNAL)); return;
    }
    if (expected != g_projectJob.batch || !g_projectJob.committed(written)) {
        if (written > 0 && written <= expected) g_projectJob.completed += written;
        project_stop(T(S_PROJECT_PARTIAL)); return;
    }
    g_projectBatch = false; g_projectWaiting = 0; g_projectDue = GetTickCount() + 250;
    if (g_projectJob.phase == project::Phase::Complete) {
        project_journal("COMPLETE"); g_projectRunning = 0; g_exactArmed = 0; g_exactFire = 0;
        g_bpWin = BPW_NONE; g_bpGhost = 0; g_bpRot = g_projectRotation;
        Buf b; b.fmt(T(S_EXT_JOB_DONE), num(g_projectJob.completed).s); flash(b.z());
    }
}
// Input thread calls this under g_stateLock. The caller sends one native click
// only when this returns true. Never retry a priced action or a partial commit.
static bool project_poll(bool focus, bool inGame, uint8_t* held) {
    if (!g_projectRunning) return false;
    if (!focus || g_modOff || g_mode != MODE_BLUEPRINT || !bp_tool() || remote_session() || creative_on() != g_projectCreative) { project_stop(T(S_PROJECT_CHANGED)); return false; }
    if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) || key_down(g_cfg.keyCancel) || key_down(g_cfg.keyMenu) || key_down(g_cfg.keyOff)) { project_stop(T(S_PROJECT_CANCEL)); return false; }
    DWORD now = GetTickCount();
    if (g_exactOpen) return false; // allow the confirmed window to finish closing
    if (!inGame || !held || (g_projectShape && *(uint32_t*)held != g_projectShape)) { project_stop(T(S_PROJECT_CHANGED)); return false; }
    if (g_projectWaiting) {
        if (now - g_projectWaitSince > 10000) project_stop(T(S_PROJECT_TIMEOUT));
        return false;
    }
    if ((LONG)(now - g_projectDue) < 0 || (GetAsyncKeyState(VK_LBUTTON) & 0x8000) || (GetAsyncKeyState(VK_RBUTTON) & 0x8000)) return false;
    int limit = g_cfg.maxShapes; if (limit > 256) limit = 256; if (limit < 1) limit = 1;
    if (g_projectJob.next < g_projectPlan.count && limit > g_projectPlan.count - g_projectJob.next) limit = g_projectPlan.count - g_projectJob.next;
    if (project_piece_batch() && limit > g_projectPlan.count + g_projectManifest.pieceCount - g_projectJob.next) limit = g_projectPlan.count + g_projectManifest.pieceCount - g_projectJob.next;
    if (project_piece_batch() && limit > 8) limit = 8;
    if (project_object_batch() && limit > 32) limit = 32;
    g_projectJob.prepare(limit); g_projectWaiting = 1; g_projectWaitSince = now;
    g_exactArmed = 1; g_exactFire = 0; g_bpWin = BPW_PLACE;
    Buf b; b.fmt(T(g_projectJob.phase == project::Phase::Checking ? S_EXT_JOB_CHECK : S_EXT_JOB_BUILD), num(g_projectJob.next).s, num(g_projectJob.total).s); flash(b.z());
    return true;
}
