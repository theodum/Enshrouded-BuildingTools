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
static bool g_projectCreative = false;
static bool g_projectInspect = false;
static bool g_projectProblem = false;
static I3 g_projectProblemAt;
static const char* g_projectLastProblem = nullptr;
static DWORD g_projectReturnAt = 0;
static project::Launch g_projectLaunch;
static char g_projectActionMessage[512] = {};
static bool g_projectPublishedBatch = false;
static void project_preview_return() {
    g_projectRunning = 0; g_projectBatch = false; g_projectWaiting = 0; g_exactArmed = 0; g_exactFire = 0;
    g_projectLaunch.reset();
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
using ProjectNeed = materials::Need;
static ProjectNeed g_projectNeed[materials::MaxNeeds], g_projectBuildNeed[materials::MaxNeeds];
static int g_projectBuildNeedN = 0;
static char g_projectMaterialError[1024] = {};
static uint64_t g_projectOperation = 0, g_projectFingerprint = 0;
static int g_projectRunCells = 0, g_projectRunPieces = 0, g_projectRunObjects = 0;
static char g_projectRunName[192] = {};
static bool g_projectRunInspect = false;
static const char* g_projectRunError = nullptr;
static char g_projectRunErrorText[1024]={};
static DWORD g_projectWorstCheck = 0;
static int g_projectActiveLimit = 128;
static diagnostics::Validation g_projectValidation;
struct ProjectRunDiagnostics {
    int modules=0,materials=0,costs=0,zones=0,spaces=0;bool fullQuote=false;
    int dimensions[3]{};core::BpMat palette[core::BP_MAX_MATS]{};
    ProjectNeed needs[materials::MaxNeeds]{};extended::Box altarZones[256]{};
};
static ProjectRunDiagnostics g_projectRunDiagnostics;
static char g_projectPieceError[512] = {},g_projectSpatialError[1024] = {},g_projectLoadError[1024] = {};
static int project_observed_zones(extended::Box*,int);
static void project_log_summary(const char*);
static bool project_validate_spaces();
static char g_projectSpaceError[1024]={};
static void project_snapshot_costs() {
    auto& d=g_projectRunDiagnostics;d.costs=g_projectNeedN;d.fullQuote=g_projectValidation.fullQuote;
    for(int i=0;i<d.costs;++i)d.needs[i]=g_projectNeed[i];
}
static uint64_t project_fingerprint() {
    uint64_t hash = 14695981039346656037ull;
    auto word = [&](uint64_t v) { hash = extended::fingerprint_word(hash,v); };
    word(g_projectPlan.count); word(g_projectManifest.pieceCount); word(g_projectObjectsInJob);
    for (int a = 0; a < 3; ++a) word(g_projectManifest.anchor[a]);
    for (int i = 0; i < g_projectPlan.count; ++i) {
        const auto& c = g_projectPlan.cells[i]; for (int a = 0; a < 3; ++a) word(c.p[a]); word(g_projectPlan.materials[c.material].item);
    }
    for (int i = 0; i < g_projectManifest.pieceCount; ++i) {
        const auto& p = g_projectManifest.pieces[i]; word(p.item); word(p.material); word(p.rotation); word(p.clearance); for (int a = 0; a < 3; ++a) word(p.offset[a]);
    }
    for (int i = 0; i < g_projectObjectsInJob; ++i) {
        const auto& o = g_projectObjects[i]; word(o.item);
        for (int a = 0; a < 3; ++a) { uint64_t bits; memcpy(&bits,&o.p[a],8); word(bits); }
        for (int a = 0; a < 4; ++a) { uint32_t bits; memcpy(&bits,&o.rotation[a],4); word(bits); }
    }
    if(g_projectManifest.spaceCount){word(0x5350414345ull);word(g_projectManifest.spaceCount);
        for(int i=0;i<g_projectManifest.spaceCount;++i)for(int a=0;a<3;++a){word(g_projectManifest.spaces[i].lo[a]);word(g_projectManifest.spaces[i].hi[a]);}}
    return hash;
}

static bool project_journal(const char* event, const char* detail = "") {
    Buf b; b.str("project ").str(event).str(" op=").x(g_projectOperation).str(" hash=").x(g_projectFingerprint).str(": ").str(g_projectManifest.name)
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
    copy_text(g_projectLoadError,b.z(),sizeof g_projectLoadError);g_projectLastProblem=g_projectLoadError;
    LOG(.str("project load: ").str(e.message ? e.message : "unknown").str(" byte=").i(e.at).str(" module=").i(e.module));
}
static bool project_path(wchar_t* out, const wchar_t* leaf) {
    if (lstrlenW(g_dir) + 23 + lstrlenW(leaf) >= MAX_PATH) return false;
    lstrcpyW(out, g_dir); lstrcatW(out, L"BuildingTools-Projects\\"); lstrcatW(out, leaf); return true;
}
static bool project_report(char* name, int capacity) {
    ProjectStateGuard guard;
    if (!g_projectOperation || g_projectRunning) return false;
    Buf leaf; leaf.str("ProjectDiagnostic_").x(g_projectOperation).c('_').u(GetTickCount()).str(".txt");
    wchar_t wide[128], path[MAX_PATH], folder[MAX_PATH];
    if (!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,leaf.z(),-1,wide,128) || !project_path(path,wide) || !project_path(folder,L"")) return false;
    CreateDirectoryW(folder,nullptr); HANDLE file = CreateFileW(path,GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
    if (file == INVALID_HANDLE_VALUE) return false;
    Buf report; report.str("Building Tools Extended " MOD_VERSION "\r\nOperation: ").x(g_projectOperation).str("\r\nPlan fingerprint (FNV64 diagnostic): ").x(g_projectFingerprint)
        .str("\r\nProject: ").str(g_projectRunName).str("\r\nMode: ").str(g_projectRunInspect ? "INSPECTION" : "BUILD")
        .str("\r\nAnchor: ").fix(g_projectOrigin.x).c(',').fix(g_projectOrigin.y).c(',').fix(g_projectOrigin.z).str("\r\nQuarter turns: ").i(g_projectRotation)
        .str("\r\nPlanned blocks/pieces/objects: ").i(g_projectRunCells).c('/').i(g_projectRunPieces).c('/').i(g_projectRunObjects)
        .str("\r\nAcknowledged prefix [0,N): N=").i(g_projectJob.completed).str("\r\nLast checked/queued index: ").i(g_projectJob.next)
        .str("\r\nWorst check duration (ms): ").u(g_projectWorstCheck).str("\r\nAdaptive batch ceiling: ").i(g_projectActiveLimit).str("\r\n");
    bool ok=true; auto write=[&](Buf& b) { DWORD written=0; if (b.n >= (int)sizeof(b.s)-1 || !WriteFile(file,b.z(),b.n,&written,nullptr) || written!=(DWORD)b.n) ok=false; };
    write(report); Buf result; result.str("Result: ").str(g_projectRunError ? g_projectRunError : g_projectRunInspect ? "inspection completed" : g_projectJob.phase == project::Phase::Complete ? "completed" : "stopped").str("\r\n"); write(result);
    const auto& d=g_projectRunDiagnostics;
    Buf dimensions;dimensions.str("Modules: ").i(d.modules).str("\r\nRotated dimensions (m): ");
    for(int a=0;a<3;++a){if(a)dimensions.c('x');dimensions.f(d.dimensions[a]/16.0);}
    dimensions.str("\r\nValidation checked/total: ").i(g_projectValidation.checked).c('/').i(g_projectValidation.total)
        .str("\r\nReserved voxel surfaces: ").i(d.spaces)
        .str("\r\nQuote: ").str(d.fullQuote ? "FULL PREFLIGHT (historical; Build rechecks)" : "PARTIAL / UNAVAILABLE").str("\r\n");write(dimensions);
    for(int m=0;m<d.materials;++m)if(d.palette[m].cells){Buf row;row.str("Material slot=").i(m).str(" item=").x(d.palette[m].item).str(" byte=").x(d.palette[m].mat).str(" cells=").u(d.palette[m].cells).str("\r\n");write(row);}
    for(int i=0;i<d.costs;++i){Buf row;row.str("Native cost item=").x(d.needs[i].item).str(" count=").u(d.needs[i].count).str(" prop=").i(d.needs[i].prop).str("\r\n");write(row);}
    for(int i=0;i<d.zones;++i){Buf row;row.str("Observed altar zone=").i(i);for(int a=0;a<3;++a)row.c(' ').f(d.altarZones[i].lo[a]/16.0).c(':').f(d.altarZones[i].hi[a]/16.0);row.str("\r\n");write(row);}
    Buf help; help.str("See BuildingTools-Projects.log for CHECK/BATCH_BEGIN/BATCH_RESULT with this operation ID.\r\nNative acknowledgements are not a saved-world audit. A paid or failed batch may be incomplete.\r\nNo automatic resume, refund or removal is safe after subsequent world edits.\r\n"); write(help);
    if (!FlushFileBuffers(file)) ok=false; CloseHandle(file);
    if (!ok) return false;
    copy_text(name,leaf.z(),capacity); return true;
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
    for (int i = 0; i < g_projectObjectCount; ++i) {
        const auto& o = g_projectObjects[i]; float lo[3],hi[3]; extended::Box box{};
        uint8_t* item = g_bpItemById ? g_bpItemById(o.item,g_bpItemReg) : nullptr;
        if (bp_item_bounds(item,lo,hi) && extended::object_box(o.p,o.rotation,lo,hi,&box)) for (int a = 0; a < 3; ++a) {
            int low = box.lo[a]>=0 ? box.lo[a]/8*8 : -((-box.lo[a]+7)/8*8), high = box.hi[a]>=0 ? (box.hi[a]+7)/8*8 : -((-box.hi[a])/8*8);
            if (low<g_projectPlan.lo[a]) g_projectPlan.lo[a]=low; if (high>g_projectPlan.hi[a]) g_projectPlan.hi[a]=high;
        }
    }
    if (!project_piece_bounds()) { e.message = g_projectPieceError[0] ? g_projectPieceError : "Unknown native piece or invalid native bounds"; project_error(e); return false; }
    if(!project_validate_spaces()){e.message=g_projectSpaceError;project_error(e);return false;}
    memset(&g_bp.h, 0, sizeof g_bp.h);
    for (int a = 0; a < 3; ++a) {
        g_bp.h.step[a] = 0.5f; g_bp.h.n[a] = (g_projectPlan.hi[a] - g_projectPlan.lo[a]) / project::Cell;
    }
    g_bp.h.nm = g_projectPlan.materialCount;
    for (int m = 0; m < g_bp.h.nm; ++m) g_bp.h.m[m] = g_projectPlan.materials[m];
    g_bp.filled = g_projectPlan.count; g_bp.ok = true; g_bpMirror = 0; g_projectLoaded = true; ++g_projectGeneration;
    LOG(.str("project loaded: ").str(g_projectManifest.name).str(" modules=").i(g_projectManifest.count)
        .str(" occupied native cells=").i(g_projectPlan.count).str(" duplicates removed=").i(g_projectPlan.duplicates)
        .str(" objects loaded=").i(g_projectObjectCount).str(" captured shape size=(").f(g_scStep[0],4).c(',').f(g_scStep[1],4).c(',').f(g_scStep[2],4).str(")m"));
    project_log_summary("LOADED");
    return true;
}
static I3 project_origin_now() {
    I3 p = bp_anchor_now();
    const int64_t half = core::kFix / 4;
    p.x -= half; p.y -= half; p.z -= half; return p;
}
static diagnostics::Context project_diagnostic_context() {
    return {g_projectGeneration,g_bpRot,project_origin_now(),creative_on(),g_projectUseObjects!=0};
}
static void project_log_summary(const char* stage) {
    LOG(.str("project SUMMARY stage=").str(stage).str(" name=").str(g_projectManifest.name).str(" modules=").i(g_projectManifest.count).str(" cells=").i(g_projectPlan.count)
        .str(" pieces=").i(g_projectManifest.pieceCount).str(" objects=").i(g_projectUseObjects ? g_projectObjectCount : 0)
        .str(" dimensions_m=(").f((g_projectPlan.hi[g_bpRot&1?2:0]-g_projectPlan.lo[g_bpRot&1?2:0])/16.0).c(',')
        .f((g_projectPlan.hi[1]-g_projectPlan.lo[1])/16.0).c(',').f((g_projectPlan.hi[g_bpRot&1?0:2]-g_projectPlan.lo[g_bpRot&1?0:2])/16.0).str(")"));
    for(int m=0;m<g_projectPlan.materialCount;++m)if(g_projectPlan.materials[m].cells)
        LOG(.str("project MATERIAL slot=").i(m).str(" item=").x(g_projectPlan.materials[m].item).str(" byte=").x(g_projectPlan.materials[m].mat).str(" cells=").u(g_projectPlan.materials[m].cells));
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
    if(g_projectJob.phase==project::Phase::Checking)LOG(.str("project validation RESULT op=").x(g_projectOperation).str(" valid=0 checked=").i(g_projectValidation.checked)
        .str(" total=").i(g_projectValidation.total).str(" reason=").str(why));
    g_projectValidation.fail();project_snapshot_costs();
    const bool returnPreview=g_projectInspect || !g_projectPublishedBatch;
    LOG(.str("project execution ERROR op=").x(g_projectOperation).str(" phase=").i((int)g_projectJob.phase).str(" waiting=").i(g_projectWaiting).str(" published=").i(g_projectPublishedBatch).str(" reason=").str(why));
    g_projectJob.phase = project::Phase::Stopped;
    project_journal("STOP", why);
    g_projectRunning = 0; g_projectWaiting = 0; g_projectBatch = false;
    g_projectLaunch.reset();
    g_exactArmed = 0; g_exactFire = 0; g_bpWin = BPW_NONE; g_bpGhost = 0;
    g_pend.n = 0; // preserve the project marker to suppress a late native commit
    g_bpRot = g_projectRotation;
    Buf b; b.fmt(T(S_EXT_JOB_STOP), num(g_projectJob.completed).s, why); flash(b.z()); beep(MB_ICONHAND);
    g_projectLastProblem = why;
    copy_text(g_projectRunErrorText,why,sizeof g_projectRunErrorText);g_projectRunError=g_projectRunErrorText;
    if (returnPreview) project_preview_return();
}
static project::ActionContext project_action_context() {
    project::ActionContext c;
    c.running=g_projectRunning!=0; c.window=g_exactOpen && !g_exactHidden && g_bpWin==BPW_PLACE && g_bpPage==1;
    c.armed=g_exactArmed!=0; c.loaded=g_projectList && g_projectLoaded && g_bp.ok;
    c.local=!g_dedicated && !remote_session(); c.enabled=!g_modOff; c.mode=g_mode==MODE_BLUEPRINT && bp_tool();
    c.systems=g_remove && g_caveForce && g_baseAt && g_placeOrig && g_placeCommitOrig;
    c.bases=g_projectBaseSys!=nullptr; c.baseAge=GetTickCount()-g_projectBaseSeen; c.native=g_experimentalNative!=0;
    for(int a=0;a<3;++a)c.step[a]=g_scStep[a];
    c.cells=g_projectPlan.count;c.pieces=g_projectManifest.pieceCount;c.objects=g_projectUseObjects ? g_projectObjectCount : 0;
    return c;
}
static void project_shape_dimensions(Buf* sizes,const float* step) {
    for(int a=0;a<3;++a) { if(extended::finite(step[a]))sizes[a].f(step[a],4);else sizes[a].str("?"); }
}
static void project_log_action(const char* event,project::Action action,const char* source,const project::ActionContext& c) {
    Buf dimensions[3];project_shape_dimensions(dimensions,c.step);
    LOG(.str("project action ").str(event).str(" action=").str(project::action_name(action)).str(" source=").str(source)
        .str(" window=").i(c.window).str(" armed=").i(c.armed).str(" loaded=").i(c.loaded).str(" running=").i(c.running)
        .str(" exactOpen=").i(g_exactOpen).str(" hidden=").i(g_exactHidden).str(" bpWin=").i(g_bpWin).str(" page=").i(g_bpPage)
        .str(" local=").i(c.local).str(" mode=").i(g_mode).str(" enabled=").i(c.enabled).str(" systems=").i(c.systems)
        .str(" bases=").i(c.bases).str(" baseAgeMs=").u(c.baseAge).str(" step=(").str(dimensions[0].z()).c(',').str(dimensions[1].z()).c(',').str(dimensions[2].z())
        .str(") cells=").i(c.cells).str(" pieces=").i(c.pieces).str(" objects=").i(c.objects));
}
static void project_refuse(project::Action action,project::ActionIssue issue,const project::ActionContext& context) {
    Str key=S_PROJECT_NATIVE;
    switch(issue) {
    case project::ActionIssue::Busy:key=S_EXT_ACTION_BUSY;break;
    case project::ActionIssue::Window:key=S_EXT_ACTION_WINDOW;break;
    case project::ActionIssue::Armed:key=S_EXT_ACTION_ARMED;break;
    case project::ActionIssue::NoProject:key=S_EXT_ACTION_NO_PROJECT;break;
    case project::ActionIssue::Remote:key=S_BP_REMOTE;break;
    case project::ActionIssue::Disabled:key=S_EXT_ACTION_DISABLED;break;
    case project::ActionIssue::Mode:key=S_EXT_ACTION_MODE;break;
    case project::ActionIssue::Shape:key=S_EXT_ACTION_SHAPE;break;
    case project::ActionIssue::Systems:key=S_EXT_ACTION_SYSTEMS;break;
    case project::ActionIssue::NoBases:key=S_EXT_ACTION_NO_BASES;break;
    case project::ActionIssue::StaleBases:key=S_EXT_ACTION_STALE_BASES;break;
    case project::ActionIssue::NativeDisabled:key=S_EXT_NATIVE_DISABLED;break;
    case project::ActionIssue::Empty:key=S_BP_NOTHING;break;
    case project::ActionIssue::Limits:key=S_PROJECT_RANGE;break;
    case project::ActionIssue::Journal:key=S_PROJECT_JOURNAL;break;
    default:break;
    }
    Buf reason;
    if(issue==project::ActionIssue::Shape) {
        Buf sizes[3];project_shape_dimensions(sizes,context.step);
        reason.fmt(T(key),sizes[0].z(),sizes[1].z(),sizes[2].z());
    } else reason.str(T(key));
    copy_text(g_projectActionMessage,reason.z(),sizeof g_projectActionMessage);g_projectLastProblem=g_projectActionMessage;
    flash(reason.z());beep(MB_ICONHAND);
    LOG(.str("project action REFUSED action=").str(project::action_name(action)).str(" code=").str(project::issue_code(issue)).str(" reason=").str(reason.z()));
}
static project::ActionIssue project_start(project::Action action) {
    const auto issue=project::action_issue(project_action_context()); if(issue!=project::ActionIssue::None)return issue;
    g_projectInspect=action==project::Action::Inspect;
    g_projectPublishedBatch=false;
    g_projectOrigin = project_origin_now(); g_projectRotation = g_bpRot;
    g_projectProblem = false; g_projectLastProblem = nullptr;
    g_projectObjectsInJob = g_projectUseObjects ? g_projectObjectCount : 0;
    g_projectJob.start(g_projectPlan.count + g_projectManifest.pieceCount + g_projectObjectsInJob); g_projectWaiting = 0; g_projectNeedN = 0;
    g_projectSourceMaterial = nullptr;
    g_projectShape = 0; g_projectCreative = creative_on();
    g_projectValidation.begin(project_diagnostic_context(),g_projectJob.total);
    auto& diagnostic=g_projectRunDiagnostics;diagnostic=ProjectRunDiagnostics{};
    diagnostic.modules=g_projectManifest.count;diagnostic.materials=g_projectPlan.materialCount;
    diagnostic.spaces=g_projectManifest.spaceCount;
    for(int a=0;a<3;++a){const int axis=(g_bpRot&1) && a!=1 ? 2-a : a;diagnostic.dimensions[a]=g_projectPlan.hi[axis]-g_projectPlan.lo[axis];}
    for(int m=0;m<diagnostic.materials;++m)diagnostic.palette[m]=g_projectPlan.materials[m];
    diagnostic.zones=project_observed_zones(diagnostic.altarZones,256);
    project_log_summary("VALIDATION_QUEUED");
    FILETIME time; GetSystemTimeAsFileTime(&time); g_projectOperation = ((uint64_t)time.dwHighDateTime << 32) | time.dwLowDateTime;
    g_projectFingerprint = project_fingerprint(); g_projectRunCells = g_projectPlan.count; g_projectRunPieces = g_projectManifest.pieceCount; g_projectRunObjects = g_projectObjectsInJob;
    copy_text(g_projectRunName,g_projectManifest.name,sizeof g_projectRunName);
    g_projectRunInspect = g_projectInspect; g_projectRunError = nullptr;
    g_projectWorstCheck=0; g_projectActiveLimit=g_projectBatchLimit;
    g_projectReturnAt = 0;
    g_projectDue = GetTickCount() + 500; g_projectWaitSince = GetTickCount();
    if (!project_journal("BEGIN",project::action_name(action))) { g_projectJob.phase=project::Phase::Stopped;g_projectRunError=T(S_PROJECT_JOURNAL);g_projectValidation.fail();return project::ActionIssue::Journal; }
    g_projectRunning = 1;
    LOG(.str("project command START action=").str(project::action_name(action)).str(" op=").x(g_projectOperation).str(" validation=queued total=").i(g_projectJob.total));
    flash(T(S_PROJECT_BEGIN));
    return project::ActionIssue::None;
}
static void project_window_closed(bool gameScreen,const char* why) {
    ProjectStateGuard guard;
    if(!g_projectRunning)return;
    LOG(.str("project window CLOSED op=").x(g_projectOperation).str(" gameScreen=").i(gameScreen).str(" armed=").i(g_exactArmed).str(" reason=").str(why));
    if(!g_projectLaunch.window_closed(gameScreen))project_stop(T(S_EXT_ACTION_WINDOW_CHANGED));
}
struct ProjectMaterialContext {
    int module = -1, slot = -1, cell = -1, piece = -1;
    uint32_t saved = 0, native = 0;
    uint8_t byte = 0;
};
static const char* project_material_file(const ProjectMaterialContext& c) {
    return c.module >= 0 && c.module < g_projectManifest.count && g_projectManifest.modules[c.module].file[0]
        ? g_projectManifest.modules[c.module].file : "(generated/native)";
}
static ProjectMaterialContext project_material_context(int slot, int cell = -1) {
    ProjectMaterialContext c; c.slot = slot;
    if (cell < 0) for (int i = 0; i < g_projectPlan.count; ++i) if (g_projectPlan.cells[i].material == slot) { cell = i; break; }
    c.cell = cell;
    if (cell >= 0) c.module = g_projectPlan.cells[cell].module;
    if (slot >= 0) { c.saved = g_projectPlan.materials[slot].item; c.byte = g_projectPlan.materials[slot].mat; }
    return c;
}
static void project_material_context_log(const ProjectMaterialContext& c) {
    const char* name = bp_item_name_cached(c.saved);
    LOG(.str("project material CONTEXT op=").x(g_projectOperation).str(" module=").i(c.module).str(" file=").str(project_material_file(c))
        .str(" slot=").i(c.slot).str(" cell=").i(c.cell).str(" piece=").i(c.piece).str(" saved_item=").x(c.saved).str(" saved_byte=").x(c.byte)
        .str(" native_item=").x(c.native).str(" cached_name=").str(name ? name : "(not cached)"));
}
static bool project_material_stop(Str key, const ProjectMaterialContext& c, Buf& detail) {
    project_material_context_log(c);
    Buf reason, module, id;
    module.i(c.module >= 0 ? c.module + 1 : 0); id.x(c.saved);
    reason.str(T(key)).c(' ').fmt(T(S_EXT_MATERIAL_CONTEXT), module.z(), project_material_file(c), id.z()).c(' ').str(detail.z());
    copy_text(g_projectMaterialError, reason.z(), sizeof g_projectMaterialError);
    project_stop(g_projectMaterialError); return false;
}
static bool project_cost_result(const materials::CostResult& r, const ProjectMaterialContext& c) {
    if (r.issue == materials::CostIssue::None) return true;
    LOG(.str("project cost REJECT op=").x(g_projectOperation).str(" issue=").str(materials::name(r.issue)).str(" cost_item=").x(r.item)
        .str(" previous=").u(r.previous).str(" requested=").u(r.requested).str(" attempted=").u(r.attempted).str(" limit=").u(r.limit)
        .str(" creative=").i(creative_on()));
    Str key = r.issue == materials::CostIssue::UnknownItem ? S_EXT_COST_ITEM : r.issue == materials::CostIssue::TooManyItems ? S_EXT_COST_ITEMS
        : r.issue == materials::CostIssue::InvalidList ? S_EXT_COST_LIST : S_EXT_COST_EXCESSIVE;
    Buf detail, id, attempted, limit; id.x(r.item); attempted.u(r.attempted); limit.u(r.limit);
    detail.fmt(T(S_EXT_COST_VALUES), id.z(), attempted.z(), limit.z());
    return project_material_stop(key, c, detail);
}
#include "project_material_cost.h"
static bool project_spatial_stop(Str key,const I3& point,int cell=-1,int module=-1,int piece=-1) {
    LOG(.str("project spatial REJECT op=").x(g_projectOperation).str(" cell=").i(cell).str(" module=").i(module).str(" piece=").i(piece)
        .str(" world_m=(").fix(point.x).c(',').fix(point.y).c(',').fix(point.z).str(") reason=").str(T(key)));
    Buf reason,position,owner;position.fix(point.x).c(',').fix(point.y).c(',').fix(point.z);
    owner.str("module=").i(module).str(" cell=").i(cell).str(" piece=").i(piece);
    reason.str(T(key)).c(' ').fmt(T(S_EXT_SPATIAL_DETAIL),position.z(),owner.z());
    copy_text(g_projectSpatialError,reason.z(),sizeof g_projectSpatialError);
    g_projectProblem=true;g_projectProblemAt=point;project_stop(g_projectSpatialError);return false;
}
#include "project_space_runtime.h"
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
    g_projectPieceError[0]=0;
    auto reject=[&](int index,const char* reason) {
        Buf b,id,material;id.x(g_projectManifest.pieces[index].item);material.x(g_projectManifest.pieces[index].material);
        b.fmt(T(S_EXT_NATIVE_MISSING),num(index).s,id.z(),material.z(),reason);copy_text(g_projectPieceError,b.z(),sizeof g_projectPieceError);
        LOG(.str("project native UNAVAILABLE ").str(b.z()));return false;
    };
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
        const auto& piece = g_projectManifest.pieces[i]; uint8_t* item = project_piece_item(piece);
        if (!item || !g_bounds) return reject(i,!g_nativePartsAvailable ? "ABI unavailable" : "unknown/unsafe item or bounds unavailable");
        CursorXf xf = project_piece_transform(piece, false); Aabb box{}; shape_bounds(item, &xf, &box);
        for (int a = 0; a < 3; ++a) {
            if (!extended::finite(box.mn[a]) || !extended::finite(box.mx[a]) || !(box.mx[a] > box.mn[a])) return reject(i,"invalid native bounds");
            const double min16=extended::native_bound16(box.mn[a]),max16=extended::native_bound16(box.mx[a]);
            int lo = (int)min16, hi = (int)max16;
            if (min16 < lo) --lo; if (max16 > hi) ++hi;
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
        ProjectMaterialContext mc; mc.piece = index; mc.saved = piece.material; mc.native = bp_block_item(material) ? *(uint32_t*)material : 0;
        if (!item || !bp_block_item(material)) {
            Buf detail; detail.str("piece_item=").x(piece.item).str(" cost=not_computed");
            return project_material_stop(S_EXT_MATERIAL_UNKNOWN, mc, detail);
        }
        CursorXf xf = project_piece_transform(piece); Aabb box{}; shape_bounds(item, &xf, &box);
        int low[3], high[3]; int64_t cells = 1;
        for (int a = 0; a < 3; ++a) {
            if (!extended::finite(box.mn[a]) || !extended::finite(box.mx[a]) || box.mx[a] <= box.mn[a]) { project_stop(T(S_PROJECT_RANGE)); return false; }
            const double minCell=extended::native_bound16(box.mn[a])/8,maxCell=extended::native_bound16(box.mx[a])/8;
            low[a] = (int)minCell; if (low[a] > minCell) --low[a];
            high[a] = (int)maxCell; if (high[a] < maxCell) ++high[a];
            cells *= high[a] - low[a];
        }
        high[1] += (piece.clearance + 7) / 8;
        cells = (int64_t)(high[0] - low[0]) * (high[1] - low[1]) * (high[2] - low[2]);
        if (cells > 4096) { project_stop(T(S_EXT_PIECE_BUDGET)); return false; }
        if (costs) { auto bounds = g_projectPieceBoxes[index]; bounds.hi[1] += piece.clearance;
            if (extended::plan_intersects(g_projectPlan,bounds)) return project_spatial_stop(S_EXT_PIECE_OVERLAP,box_centre(box),-1,-1,index);
        }
        if (costs) for (int other = 0; other < index; ++other) {
            const bool overlap = extended::intersects(g_projectPieceBoxes[index], g_projectPieceBoxes[other], piece.clearance)
                || extended::intersects(g_projectPieceBoxes[other], g_projectPieceBoxes[index], g_projectManifest.pieces[other].clearance);
            if (overlap) return project_spatial_stop(S_EXT_PIECE_OVERLAP,box_centre(box),-1,-1,index);
        }
        for (int y = low[1]; y < high[1]; ++y) for (int z = low[2]; z < high[2]; ++z) for (int x = low[0]; x < high[0]; ++x) {
            I3 p{(int64_t)(x * 8 + 4) * kProjectFixUnit, (int64_t)(y * 8 + 4) * kProjectFixUnit, (int64_t)(z * 8 + 4) * kProjectFixUnit};
            if (!project_covered(p)) return project_spatial_stop(S_PROJECT_COVERAGE,p,-1,-1,index);
            CursorXf probe = target; probe.pos[0] = p.x + off[0]; probe.pos[1] = p.y + off[1]; probe.pos[2] = p.z + off[2];
            CostList scratch{entries, 0, 256, nullptr}; memset(g_remVols, 0, sizeof g_remVols); price_removal(&scratch, ctx, state, &probe); g_refundN = 0; g_refundList = nullptr;
            for (int m = 0x80; m < 256; ++m) if (g_remVols[m] > 0) return project_spatial_stop(S_PROJECT_COLLISION,p,-1,-1,index);
        }
        CostList price{entries, 0, 256, nullptr};
        if (!g_placeOrig(&price, ctx, item, material, &xf)) { project_stop(T(S_PROJECT_NATIVE)); return false; }
        if (!project_native_cost(price, entries, mc, costs)) return false;
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
    g_projectPublishedBatch=true;
    return 1;
}
static bool project_check(void* ctx, void* state, uint8_t* pdata, const CursorXf& target, bool costs) {
    g_projectBuildNeedN = 0;
    Aabb box; shape_bounds(state, &target, &box);
    for (int a = 0; a < 3; ++a) if (box.mx[a] - box.mn[a] != 0.5f) { project_stop(T(S_PROJECT_SHAPE)); return false; }
    const I3 tc = box_centre(box);
    const int64_t off[3] = {target.pos[0] - tc.x, target.pos[1] - tc.y, target.pos[2] - tc.z};
    static CostEntry entries[256];
    uint8_t* material[core::BP_MAX_MATS] = {};
    if (!bp_item_registry(ctx, state)) { project_stop(T(S_PROJECT_UNAVAILABLE)); return false; }
    if(costs && !g_projectJob.next && !project_validate_spaces()){project_stop(g_projectSpaceError);return false;}
    if (project_piece_batch()) return project_check_pieces(ctx, state, target, costs);
    if (project_object_batch()) {
        if (creative_on() && !g_creativeProps) { project_stop(T(S_EXT_OBJECT_CREATIVE)); return false; }
        for (int i = 0; i < g_projectJob.batch; ++i) {
            const auto& o = g_projectObjects[g_projectJob.next - g_projectPlan.count - g_projectManifest.pieceCount + i];
            uint8_t* it = g_bpItemById(o.item, g_bpItemReg);
            if (!extended_prop_item(it)) { project_stop(T(S_EXT_OBJECT_UNSAFE)); return false; }
            double pos[3]; float rot[4]; extended::object_world(o, g_projectManifest.anchor, g_projectRotation, g_projectOrigin, pos, rot);
            I3 center{(int64_t)(pos[0] * core::kFix), (int64_t)(pos[1] * core::kFix), (int64_t)(pos[2] * core::kFix)};
            if (!project_covered(center)) return project_spatial_stop(S_PROJECT_COVERAGE,center,-1,o.module);
            float low[3], high[3];
            if (!bp_item_bounds(it, low, high)) { project_stop(T(S_PROJECT_UNAVAILABLE)); return false; }
            const double x = rot[0], y = rot[1], z = rot[2], w = rot[3];
            for (int corner = 0; corner < 8; ++corner) {
                double p[3]; for (int a = 0; a < 3; ++a) { double eps = (high[a] - low[a]) * 0.001; p[a] = (corner >> a) & 1 ? high[a] - eps : low[a] + eps; }
                float v[] = {(float)(pos[0] + (1-2*(y*y+z*z))*p[0] + 2*(x*y-w*z)*p[1] + 2*(x*z+w*y)*p[2]),
                             (float)(pos[1] + 2*(x*y+w*z)*p[0] + (1-2*(x*x+z*z))*p[1] + 2*(y*z-w*x)*p[2]),
                             (float)(pos[2] + 2*(x*z-w*y)*p[0] + 2*(y*z+w*x)*p[1] + (1-2*(x*x+y*y))*p[2])};
                if (!extended::finite(v[0]) || !extended::finite(v[1]) || !extended::finite(v[2])) { project_stop(T(S_PROJECT_RANGE)); return false; }
                if (base_at(g_projectBaseSys, v) < 0) {I3 corner{(int64_t)(v[0]*core::kFix),(int64_t)(v[1]*core::kFix),(int64_t)(v[2]*core::kFix)};return project_spatial_stop(S_PROJECT_COVERAGE,corner,-1,o.module);}
            }
            extended::Box worldBox{},localBox{};
            if (!extended::object_box(pos,rot,low,high,&worldBox) || !extended::object_box(o.p,o.rotation,low,high,&localBox)) { project_stop(T(S_PROJECT_RANGE)); return false; }
            int first[3],end[3]; int64_t probes=1;
            for (int a=0;a<3;++a) { first[a]=worldBox.lo[a]>=0 ? worldBox.lo[a]/8 : -((-worldBox.lo[a]+7)/8); end[a]=worldBox.hi[a]>=0 ? (worldBox.hi[a]+7)/8 : -((-worldBox.hi[a])/8); probes*=end[a]-first[a]; }
            if (probes>512) { project_stop(T(S_EXT_OBJECT_BUDGET)); return false; }
            if (costs && extended::plan_intersects(g_projectPlan,localBox)) { project_stop(T(S_EXT_PIECE_OVERLAP)); return false; }
            for (int y=first[1];y<end[1];++y) for (int z=first[2];z<end[2];++z) for (int x=first[0];x<end[0];++x) {
                I3 p{(int64_t)(x*8+4)*kProjectFixUnit,(int64_t)(y*8+4)*kProjectFixUnit,(int64_t)(z*8+4)*kProjectFixUnit};
                CursorXf probe=target; probe.pos[0]=p.x+off[0]; probe.pos[1]=p.y+off[1]; probe.pos[2]=p.z+off[2];
                CostList scratch{entries,0,256,nullptr}; memset(g_remVols,0,sizeof g_remVols); price_removal(&scratch,ctx,state,&probe); g_refundN=0; g_refundList=nullptr;
                for (int m=0x80;m<256;++m) if (g_remVols[m]>0) return project_spatial_stop(S_PROJECT_COLLISION,p,-1,o.module);
            }
            ProjectMaterialContext mc; mc.module = o.module; mc.saved = mc.native = o.item;
            if (!project_cost(o.item, 1, mc, costs, true)) return false;
        }
        return true;
    }
    for (int m = 0; m < g_projectPlan.materialCount; ++m) {
        const auto& bm = g_projectPlan.materials[m];
        if (!bm.cells) continue;
        const auto r = bp_resolve_material(ctx, pdata, bm);
        if (!g_projectJob.next || !r.item) {
            const auto mc = project_material_context(m);
            project_material_context_log(mc);
            LOG(.str("project material RESOLVE op=").x(g_projectOperation).str(" slot=").i(m).str(" saved_item=").x(r.saved).str(" saved_byte=").x(r.savedByte)
                .str(" block_item=").x(r.candidate).str(" native_byte=").x(r.nativeByte).str(" refund_known=").i(r.refundKnown).str(" refund_item=").x(r.refund)
                .str(" lookup_found=").i(r.lookupFound).str(" route=").str(materials::name(r.route)).str(" issue=").str(materials::name(r.issue)).str(" anchor=held_material cost=not_computed"));
        }
        if (!r.item) {
            Buf detail, block, refund; block.x(r.candidate); refund.x(r.refund);
            detail.fmt(T(S_EXT_MATERIAL_IDS), block.z(), r.refundKnown ? refund.z() : "?", "not_computed");
            return project_material_stop(r.issue == materials::Issue::RefundMismatch ? S_EXT_MATERIAL_MISMATCH : S_EXT_MATERIAL_UNKNOWN, project_material_context(m), detail);
        }
        material[m] = r.item;
    }
    for (int i = 0; i < g_projectJob.batch; ++i) {
        const I3 p = project_position(i);
        const double xyz[3] = {(double)p.x / core::kFix, (double)p.y / core::kFix, (double)p.z / core::kFix};
        for (int a = 0; a < 3; ++a) if (xyz[a] < -65536.0 || xyz[a] > 65536.0) { project_stop(T(S_PROJECT_RANGE)); return false; }
        if (!project_covered(p)) {const auto& cell=g_projectPlan.cells[g_projectJob.next+i];return project_spatial_stop(S_PROJECT_COVERAGE,p,g_projectJob.next+i,cell.module);}
        CursorXf xf = target; xf.pos[0] = p.x + off[0]; xf.pos[1] = p.y + off[1]; xf.pos[2] = p.z + off[2];
        CostList scratch = {entries, 0, 256, nullptr};
        memset(g_remVols, 0, sizeof g_remVols);
        price_removal(&scratch, ctx, state, &xf);
        g_refundN = 0; g_refundList = nullptr;
        for (int m = 0x80; m < 256; ++m) if (g_remVols[m] > 0.f) {
            LOG(.str("project collision at cell ").i(g_projectJob.next + i).str(" module=").i(g_projectPlan.cells[g_projectJob.next + i].module));
            return project_spatial_stop(S_PROJECT_COLLISION,p,g_projectJob.next+i,g_projectPlan.cells[g_projectJob.next+i].module);
        }
        scratch.count = 0;
        const auto& source=g_projectPlan.cells[g_projectJob.next+i];
        if(!spaces::visit(source,g_projectManifest,g_projectRotation,g_projectOrigin,[&](const I3& air){
            if(!project_covered(air))return project_spatial_stop(S_PROJECT_COVERAGE,air,g_projectJob.next+i,source.module);
            CursorXf probe=target;probe.pos[0]=air.x+off[0];probe.pos[1]=air.y+off[1];probe.pos[2]=air.z+off[2];
            static CostEntry airEntries[256];CostList airCost{airEntries,0,256,nullptr};memset(g_remVols,0,sizeof g_remVols);
            price_removal(&airCost,ctx,state,&probe);g_refundN=0;g_refundList=nullptr;
            for(int m=0x80;m<256;++m)if(g_remVols[m]>0)return project_spatial_stop(S_EXT_VOXEL_AIR,air,g_projectJob.next+i,source.module);
            return true;
        }))return false;
        const int mi = g_projectPlan.cells[g_projectJob.next + i].material;
        if (!g_placeOrig(&scratch, ctx, state, material[mi], &xf)) { project_stop(T(S_PROJECT_NATIVE)); return false; }
        auto mc = project_material_context(mi, g_projectJob.next + i); mc.native = *(uint32_t*)material[mi];
        if (!project_native_cost(scratch, entries, mc, costs)) return false;
    }
    return true;
}
static uint8_t project_click(CostList* list, void* ctx, void* state, uint8_t* pdata, const CursorXf& target) {
    if (g_exactOpen || g_projectWaiting != 1) { LOG(.str("project native click IGNORED window=").i(g_exactOpen).str(" waiting=").i(g_projectWaiting)); return 0; }
    LOG(.str("project native click RECEIVED op=").x(g_projectOperation).str(" phase=").i((int)g_projectJob.phase).str(" next=").i(g_projectJob.next).str(" batch=").i(g_projectJob.batch));
    g_exactArmed = 0; g_exactFire = 0; g_projectWaiting = 2; g_projectWaitSince = GetTickCount();
    if (!g_projectShape) g_projectShape = state ? *(uint32_t*)state : 0;
    if (!g_projectSourceMaterial) g_projectSourceMaterial = pdata;
    if (!g_projectShape || !state || *(uint32_t*)state != g_projectShape || remote_session() || creative_on() != g_projectCreative) { project_stop(T(S_PROJECT_CHANGED)); return 0; }
    const bool checking = g_projectJob.phase == project::Phase::Checking;
    if(checking && !g_projectJob.next)LOG(.str("project validation START op=").x(g_projectOperation).str(" inspect=").i(g_projectInspect));
    const DWORD checkAt=GetTickCount(); const bool valid=project_check(ctx,state,pdata,target,checking);
    const DWORD checkMs=GetTickCount()-checkAt;
    if (checkMs>g_projectWorstCheck) g_projectWorstCheck=checkMs;
    g_projectActiveLimit=extended::adapt_batch(g_projectActiveLimit,checkMs);
    if (!valid) return 0;
    if (checking) {
        if(!g_projectJob.checked(g_projectJob.batch)){project_stop(T(S_PROJECT_PARTIAL));return 0;}
        const int checked=g_projectJob.phase==project::Phase::Building ? g_projectJob.total : g_projectJob.next;
        if(!g_projectValidation.progress(checked) || (g_projectJob.phase==project::Phase::Building && !g_projectValidation.pass())) {project_stop(T(S_PROJECT_PARTIAL));return 0;}
        project_snapshot_costs();
        if (g_projectJob.phase == project::Phase::Building) for (int q = 0; q < g_projectNeedN; ++q)
            LOG(.str("project cost TOTAL item=").x(g_projectNeed[q].item).str(" count=").u(g_projectNeed[q].count).str(" limit=").u(materials::MaxCost)
                .str(" prop=").i(g_projectNeed[q].prop).str(" creative=").i(creative_on()));
        if (g_projectJob.phase == project::Phase::Building && !creative_on()) {
            for (int q = 0; q < g_projectNeedN; ++q) {
                const int64_t have = stock_of(g_projectNeed[q].item);
                if (g_projectNeed[q].prop) continue; // native payment is authoritative for props, absent from the block stock cache
                if (have < 0 || (uint64_t)have < g_projectNeed[q].count) { project_stop(T(S_PROJECT_STOCK)); return 0; }
            }
        }
        if(!project_journal(g_projectJob.phase == project::Phase::Checking ? "CHECK" : "CHECKED")){project_stop(T(S_PROJECT_JOURNAL));return 0;}
        if(g_projectJob.phase==project::Phase::Building)LOG(.str("project validation RESULT op=").x(g_projectOperation).str(" valid=1 placements=").i(g_projectJob.total).str(" inspect=").i(g_projectInspect));
        if (g_projectInspect && g_projectJob.phase == project::Phase::Building) {
            if(!project_journal("INSPECTED")){project_stop(T(S_PROJECT_JOURNAL));return 0;}
            project_preview_return();g_projectLastProblem=T(S_EXT_CHECK_OK);flash(T(S_EXT_CHECK_OK));return 0;
        }
        if(g_projectJob.phase==project::Phase::Building && !project_journal("BUILD_STARTED")){project_stop(T(S_PROJECT_JOURNAL));return 0;}
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
        g_projectPublishedBatch=true;
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
    g_projectPublishedBatch=true;
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
        g_projectLaunch.reset();
        g_bpWin = BPW_NONE; g_bpGhost = 0; g_bpRot = g_projectRotation;
        Buf b; b.fmt(T(S_EXT_JOB_DONE), num(g_projectJob.completed).s); flash(b.z());
    }
}
// Input thread calls this under g_stateLock. The caller sends one native click
// only when this returns true. Never retry a priced action or a partial commit.
static bool project_poll(bool focus, bool inGame, uint8_t* held) {
    if (!g_projectRunning) return false;
    if(!focus){project_stop(T(S_EXT_ACTION_FOCUS));return false;}
    if(g_modOff || g_mode!=MODE_BLUEPRINT || !bp_tool()){project_stop(T(S_EXT_ACTION_MODE));return false;}
    if(remote_session()){project_stop(T(S_BP_REMOTE));return false;}
    if(creative_on()!=g_projectCreative){project_stop(T(S_EXT_ACTION_CREATIVE));return false;}
    if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) || key_down(g_cfg.keyCancel) || key_down(g_cfg.keyMenu) || key_down(g_cfg.keyOff)) { project_stop(T(S_PROJECT_CANCEL)); return false; }
    DWORD now = GetTickCount();
    const auto launch=g_projectLaunch.poll(now,g_exactOpen!=0,inGame);
    if(launch==project::LaunchPoll::Wait)return false;
    if(launch==project::LaunchPoll::Timeout){project_stop(T(S_EXT_ACTION_CLOSE_TIMEOUT));return false;}
    if(launch==project::LaunchPoll::Interrupted){project_stop(T(S_EXT_ACTION_WINDOW_CHANGED));return false;}
    if (!inGame || !held || (g_projectShape && *(uint32_t*)held != g_projectShape)) { project_stop(T(S_PROJECT_CHANGED)); return false; }
    if (g_projectWaiting) {
        if (now - g_projectWaitSince > 10000) project_stop(T(S_PROJECT_TIMEOUT));
        return false;
    }
    if ((LONG)(now - g_projectDue) < 0 || (GetAsyncKeyState(VK_LBUTTON) & 0x8000) || (GetAsyncKeyState(VK_RBUTTON) & 0x8000)) return false;
    int limit = g_cfg.maxShapes; if (limit > g_projectActiveLimit) limit = g_projectActiveLimit; if (limit < 1) limit = 1;
    if (g_projectJob.next < g_projectPlan.count && limit > g_projectPlan.count - g_projectJob.next) limit = g_projectPlan.count - g_projectJob.next;
    if (project_piece_batch() && limit > g_projectPlan.count + g_projectManifest.pieceCount - g_projectJob.next) limit = g_projectPlan.count + g_projectManifest.pieceCount - g_projectJob.next;
    if (project_piece_batch() && limit > 1) limit = 1;
    if (project_object_batch() && limit > 4) limit = 4;
    g_projectJob.prepare(limit); g_projectWaiting = 1; g_projectWaitSince = now;
    LOG(.str("project native click SCHEDULED op=").x(g_projectOperation).str(" phase=").i((int)g_projectJob.phase).str(" next=").i(g_projectJob.next).str(" batch=").i(g_projectJob.batch));
    g_exactArmed = 1; g_exactFire = 0; g_bpWin = BPW_PLACE;
    Buf b; b.fmt(T(g_projectJob.phase == project::Phase::Checking ? S_EXT_JOB_CHECK : S_EXT_JOB_BUILD), num(g_projectJob.next).s, num(g_projectJob.total).s); flash(b.z());
    return true;
}
