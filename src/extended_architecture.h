// Component authoring modifies the in-memory preview only. Engine writes still
// go through the common project's complete preflight and native batch path.
static uint32_t g_componentFrameMaterial = 0;
static int g_stairModelTurn = 0;
// Preserve the previous draft on any failed generation, including failed
// landings, allocation errors and intersecting materials. No world rollback.
struct ArchitectureEdit {
    struct Snapshot {
        project::Plan plan; project::Manifest manifest; core::BpHeader header;
        extended::Box modules[project::MaxModules], pieces[project::MaxPieces];
        bool visible[project::MaxModules], loaded, list, ok, problem;
        int objectCount, filled, generation, page, mirror, needCount;
        const char* lastProblem; extended::Object* objects;
    };
    Snapshot* before = nullptr; bool committed = false;
    ArchitectureEdit() {
        before = (Snapshot*)HeapAlloc(GetProcessHeap(), 0, sizeof(Snapshot)); if (!before) return;
        auto cells = (project::CellEntry*)VirtualAlloc(nullptr, sizeof(project::CellEntry) * project::MaxCells, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (!cells) { HeapFree(GetProcessHeap(), 0, before); before = nullptr; return; }
        before->plan = g_projectPlan; before->manifest = g_projectManifest; before->header = g_bp.h;
        memcpy(before->modules, g_projectModuleBoxes, sizeof before->modules); memcpy(before->pieces, g_projectPieceBoxes, sizeof before->pieces); memcpy(before->visible, g_projectModuleVisible, sizeof before->visible);
        before->loaded = g_projectLoaded; before->list = g_projectList; before->ok = g_bp.ok; before->problem = g_projectProblem;
        before->objectCount = g_projectObjectCount; before->objects = g_projectObjects; before->filled = g_bp.filled; before->generation = g_projectGeneration;
        before->page = g_bpPage; before->mirror = g_bpMirror; before->needCount = g_projectNeedN; before->lastProblem = g_projectLastProblem;
        if (g_projectPlan.count) memcpy(cells, g_projectPlan.cells, (size_t)g_projectPlan.count * sizeof(*cells));
        g_projectPlan.cells = cells; g_projectPlan.capacity = project::MaxCells;
    }
    ~ArchitectureEdit() {
        if (!before) return;
        if (committed) { if (before->plan.cells) VirtualFree(before->plan.cells, 0, MEM_RELEASE); }
        else {
            if (g_projectPlan.cells && g_projectPlan.cells != before->plan.cells) VirtualFree(g_projectPlan.cells, 0, MEM_RELEASE);
            if (g_projectObjects != before->objects) { if (g_projectObjects) VirtualFree(g_projectObjects, 0, MEM_RELEASE); g_projectObjects = before->objects; }
            g_projectPlan = before->plan; g_projectManifest = before->manifest; g_bp.h = before->header;
            memcpy(g_projectModuleBoxes, before->modules, sizeof before->modules); memcpy(g_projectPieceBoxes, before->pieces, sizeof before->pieces); memcpy(g_projectModuleVisible, before->visible, sizeof before->visible);
            g_projectLoaded = before->loaded; g_projectList = before->list; g_bp.ok = before->ok; g_projectProblem = before->problem;
            g_projectObjectCount = before->objectCount; g_bp.filled = before->filled; g_projectGeneration = before->generation + 1;
            g_bpPage = before->page; g_bpMirror = before->mirror; g_projectNeedN = before->needCount; g_projectLastProblem = before->lastProblem; g_bpBitsOk = 0;
        }
        HeapFree(GetProcessHeap(), 0, before);
    }
};
static bool component_frame(const extended::Component& c, int x, int y, int z) {
    if (c.kind == extended::Floor) return x < c.thickness || z < c.thickness || x >= c.size[0] - c.thickness || z >= c.size[2] - c.thickness;
    if (c.kind == extended::Wall) for (int i = 0; i < c.openingCount; ++i) {
        const auto& o = c.openings[i];
        if (x >= o.x - c.thickness && x < o.x + o.width + c.thickness && y >= o.y - c.thickness && y < o.y + o.height + c.thickness) return true;
    }
    return false;
}
static bool architecture_component(const extended::Component& component, const int* offset, int rotation, bool append, uint32_t selectedMaterial = 0) {
    ProjectStateGuard guard;
    uint8_t* block = g_bpItemById && g_bpItemReg ? g_bpItemById(selectedMaterial ? selectedMaterial : g_catalogMaterial, g_bpItemReg) : nullptr;
    uint8_t* frame = g_componentFrameMaterial && g_bpItemById ? g_bpItemById(g_componentFrameMaterial, g_bpItemReg) : nullptr;
    if (!extended::valid(component) || !bp_block_item(block) || (frame && !bp_block_item(frame)) || g_projectRunning) return false;
    ArchitectureEdit edit; if (!edit.before) return false;
    if (!append || !g_projectLoaded) { if (!project_generated_begin("Architecture")) return false; g_projectManifest.count = 0; }
    if (g_projectManifest.count >= project::MaxModules) return false;
    auto temporary = (project::CellEntry*)VirtualAlloc(nullptr, sizeof(project::CellEntry) * project::MaxCells, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!temporary) return false;
    project::Plan plan = g_projectPlan; plan.cells = temporary; memcpy(temporary, g_projectPlan.cells, (size_t)plan.count * sizeof(*temporary));
    auto material = [&](uint8_t* item) {
        int m = 0; while (m < plan.materialCount && plan.materials[m].item != *(uint32_t*)item) ++m;
        if (m == core::BP_MAX_MATS) return -1;
        if (m == plan.materialCount) plan.materials[plan.materialCount++] = core::BpMat{*(uint32_t*)item, item[0x454], 0, 0};
        return m;
    };
    int mainMaterial = material(block), borderMaterial = frame ? material(frame) : mainMaterial;
    if (mainMaterial < 0 || borderMaterial < 0) { VirtualFree(temporary, 0, MEM_RELEASE); return false; }
    const int module = g_projectManifest.count; bool valid = true;
    for (int y = 0; y < component.size[1] && valid; ++y) for (int z = 0; z < component.size[2] && valid; ++z) for (int x = 0; x < component.size[0]; ++x) {
        if (!extended::component_cell(component, x, y, z)) continue;
        if (plan.count == plan.capacity) { valid = false; break; }
        int rx, rz; core::bp_turn(rotation, x, z, component.size[0], component.size[2], &rx, &rz);
        auto& cell = plan.cells[plan.count++]; cell.p[0] = offset[0] + rx * 8; cell.p[1] = offset[1] + y * 8; cell.p[2] = offset[2] + rz * 8;
        cell.module = (uint16_t)module; cell.category = (uint8_t)component.kind;
        if (component.kind == extended::Tower) { auto walls = component; walls.floors = 0; if (!extended::component_cell(walls, x, y, z)) cell.category = extended::Floor; }
        cell.material = (uint8_t)(component_frame(component, x, y, z) ? borderMaterial : mainMaterial);
        for (int a = 0; a < 3; ++a) if (cell.p[a] < -project::MaxCoord || cell.p[a] > project::MaxCoord - 8) valid = false;
    }
    for (int m = 0; m < plan.materialCount; ++m) plan.materials[m].cells = 0;
    project::Error error;
    if (valid) valid = project::finish(&plan, &error);
    if (!valid) { VirtualFree(temporary, 0, MEM_RELEASE); return false; }
    VirtualFree(g_projectPlan.cells, 0, MEM_RELEASE); g_projectPlan = plan;
    auto& m = g_projectManifest.modules[module]; memset(&m, 0, sizeof m); m.category = component.kind; ++g_projectManifest.count;
    g_projectModuleVisible[module] = true;
    for (int a = 0; a < 3; ++a) {
        const int source = (rotation & 1) && a != 1 ? 2 - a : a;
        g_projectModuleBoxes[module].lo[a] = offset[a]; g_projectModuleBoxes[module].hi[a] = offset[a] + component.size[source] * 8;
    }
    g_projectNeedN = 0; g_projectProblem = false; g_projectLastProblem = nullptr;
    project_piece_bounds(); project_generated_finish(); edit.committed = true; return true;
}
static bool architecture_stairs(int kind, int flights, const int* offset, int baseRotation, int riseOverride, bool append, bool reserve) {
    ProjectStateGuard guard;
    uint8_t* item = g_bpItemById && g_bpItemReg ? g_bpItemById(g_catalogPiece, g_bpItemReg) : nullptr;
    uint8_t* mat = g_bpItemById && g_bpItemReg ? g_bpItemById(g_catalogMaterial, g_bpItemReg) : nullptr;
    project::NativePiece templatePiece{}; templatePiece.item = g_catalogPiece;
    if (!project_piece_item(templatePiece) || !bp_block_item(mat) || !g_bounds || g_projectRunning) return false;
    CursorXf zero{}; zero.rot[3] = 1; zero.scale = 1;
    const float identity[] = {0,0,0,1}; core::bp_turn_rot(g_stairModelTurn,identity,zero.rot);
    Aabb box{}; shape_bounds(item, &zero, &box);
    const int width = (int)((box.mx[0] - box.mn[0]) * 16 + 0.5f), height = riseOverride ? riseOverride : (int)((box.mx[1] - box.mn[1]) * 16 + 0.5f), run = (int)((box.mx[2] - box.mn[2]) * 16 + 0.5f);
    static extended::StairStep steps[160]; const int n = extended::stair_layout(kind, flights, width, height, run, steps, 160);
    if (!n) return false;
    for (int i = 0; i < n; ++i) steps[i] = extended::turn_stair(steps[i],baseRotation);
    ArchitectureEdit edit; if (!edit.before) return false;
    if (!append || !g_projectLoaded) { if (!project_generated_begin("Native stairs")) return false; g_projectManifest.count = 0; }
    int oldCount = g_projectManifest.pieceCount;
    if (oldCount + n > project::MaxPieces) return false;
    // Native flights first. Platforms are authoring voxels, never substitute steps.
    for (int i = 0; i < n; ++i) if (!steps[i].landing) {
        auto& piece = g_projectManifest.pieces[g_projectManifest.pieceCount++]; piece = project::NativePiece{}; piece.item = g_catalogPiece; piece.material = g_catalogMaterial; piece.rotation = (steps[i].rotation + g_stairModelTurn) & 3; piece.clearance = 32;
        CursorXf pose{}; pose.scale = 1; core::bp_turn_rot(piece.rotation, identity, pose.rot); Aabb bounds{}; shape_bounds(item, &pose, &bounds);
        for (int a = 0; a < 3; ++a) piece.offset[a] = offset[a] + steps[i].low[a] - extended::snap(bounds.mn[a] * 16, 1);
    }
    if (!project_piece_bounds()) { g_projectManifest.pieceCount = oldCount; return false; }
    if (reserve) {
        int count = 0;
        for (int i = 0; i < g_projectPlan.count; ++i) {
            const auto& cell = g_projectPlan.cells[i]; bool remove = false;
            const int category = cell.category ? cell.category : g_projectManifest.modules[cell.module].category;
            if (category == extended::Floor) for (int p = oldCount; p < g_projectManifest.pieceCount && !remove; ++p) {
                const auto& b = g_projectPieceBoxes[p]; bool overlap = true;
                for (int a = 0; a < 3; ++a) if (cell.p[a] + 8 <= b.lo[a] || cell.p[a] >= b.hi[a] + (a == 1 ? 32 : 0)) overlap = false;
                if (overlap) remove = true;
            }
            if (!remove) g_projectPlan.cells[count++] = cell;
        }
        g_projectPlan.count = count;
    }
    for (int i = 0; i < n; ++i) if (steps[i].landing) {
        extended::Component landing; landing.kind = extended::Floor; landing.thickness = 1; landing.size[0] = steps[i].width / 8; landing.size[1] = 1; landing.size[2] = steps[i].depth / 8;
        int position[3]; for (int a = 0; a < 3; ++a) position[a] = offset[a] + steps[i].low[a];
        if (!architecture_component(landing, position, 0, true)) return false;
    }
    if (!g_projectManifest.count) {
        g_projectManifest.count = 1; memset(&g_projectManifest.modules[0], 0, sizeof g_projectManifest.modules[0]); g_projectManifest.modules[0].category = extended::Stairs;
        for (int a = 0; a < 3; ++a) { g_projectModuleBoxes[0].lo[a] = g_projectPlan.lo[a]; g_projectModuleBoxes[0].hi[a] = g_projectPlan.hi[a]; }
    }
    project_piece_bounds(); project_generated_finish(); edit.committed = true; return true;
}
static bool architecture_native_example(int rotation) {
    ProjectStateGuard guard; if (g_projectRunning) return false;
    project::NativePiece selected{}; selected.item=g_catalogPiece; uint8_t* item=project_piece_item(selected); if (!item || !g_catalogMaterial) return false;
    CursorXf pose{}; pose.scale=1; const float identity[]={0,0,0,1}; core::bp_turn_rot(g_stairModelTurn,identity,pose.rot);
    Aabb box{}; shape_bounds(item,&pose,&box);
    for (int a=0;a<3;++a) if (!extended::finite(box.mn[a]) || !extended::finite(box.mx[a]) || box.mx[a]<=box.mn[a]) return false;
    const int width=(int)((box.mx[0]-box.mn[0])*16+0.5f), rise=(int)((box.mx[1]-box.mn[1])*16+0.5f), run=(int)((box.mx[2]-box.mn[2])*16+0.5f);
    if (width%8 || rise%8 || run%8 || width<16 || rise<8 || run<8) return false;
    ArchitectureEdit edit; if (!edit.before) return false;
    int flights=(64+rise-1)/rise; if (flights<2) flights=2;
    extended::Component floor; floor.kind=extended::Floor; floor.size[0]=(width+64)/8; floor.size[1]=1; floor.size[2]=(flights*run+64)/8; floor.thickness=1;
    int low[]={-32,-8,-32}; if (!architecture_component(floor,low,0,false,g_catalogMaterial)) return false;
    low[1]=flights*rise-8; if (!architecture_component(floor,low,0,true,g_catalogMaterial)) return false;
    const int origin[]={0,0,0}; if (!architecture_stairs(0,flights,origin,0,0,true,true)) return false;
    copy_text(g_projectManifest.name,"Native stairs - two levels",sizeof g_projectManifest.name); g_bpRot=rotation;
    project_generated_finish(); edit.committed=true; return true;
}
