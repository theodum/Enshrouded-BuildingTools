// Bounded diagnostic geometry, published under its own lock. No game writes.
static bool frame_cam(int64_t*, float*);
struct SceneLine { float a[3], b[3], color[4]; };
static CRITICAL_SECTION g_sceneLock;
static SceneLine g_sceneLines[extended::MaxLines];
static int g_sceneCount = 0;
static DWORD g_sceneTick = 0;
static void scene_line(const double* a, const double* b, const float* color) {
    if (g_sceneCount == extended::MaxLines) return;
    SceneLine& line = g_sceneLines[g_sceneCount++];
    for (int i = 0; i < 3; ++i) { line.a[i] = (float)a[i]; line.b[i] = (float)b[i]; }
    memcpy(line.color, color, 16);
}
static void scene_box(const double* low, const double* high, const float* color) {
    for (int axis = 0; axis < 3; ++axis) for (int edge = 0; edge < 4; ++edge) {
        double a[3], b[3]; int bit = 0;
        for (int k = 0; k < 3; ++k) { a[k] = b[k] = k == axis ? low[k] : ((edge >> bit++) & 1) ? high[k] : low[k]; }
        b[axis] = high[axis]; scene_line(a, b, color);
    }
}
static void scene_update() {
    if (g_modOff || (!(g_grid.enabled && g_cfg.toolFree) && !(g_projectList && g_projectLoaded && g_bpPage == 1 && (g_exactOpen || g_projectRunning)))) {
        EnterCriticalSection(&g_sceneLock); g_sceneCount = 0; g_sceneTick = GetTickCount(); LeaveCriticalSection(&g_sceneLock); return;
    }
    EnterCriticalSection(&g_stateLock); EnterCriticalSection(&g_sceneLock);
    g_sceneCount = 0;
    altar_snapshot(); altar_assist_update();
    const float gridColor[] = {0.45f, 0.75f, 1.f, 0.30f}, xColor[] = {1.f, 0.25f, 0.2f, 0.85f}, zColor[] = {0.25f, 1.f, 0.4f, 0.85f};
    if (!g_modOff && g_grid.enabled && g_cfg.toolFree && (building_now(500) || g_exactOpen)) {
        int64_t cam[3]; float rotation[4];
        if (frame_cam(cam, rotation)) {
            int step = g_grid.spacing, radius = g_grid.extent * 16;
            int cx = extended::snap(cam[0] / core::kFix * 16 - g_grid.x, step) + g_grid.x;
            int cz = extended::snap(cam[2] / core::kFix * 16 - g_grid.z, step) + g_grid.z;
            for (int d = -radius; d <= radius; d += step) {
                double a[] = {(cx + d) / 16.0, g_grid.height / 16.0, (cz - radius) / 16.0}, b[] = {a[0], a[1], (cz + radius) / 16.0}; scene_line(a, b, gridColor);
                double c[] = {(cx - radius) / 16.0, a[1], (cz + d) / 16.0}, e[] = {(cx + radius) / 16.0, a[1], c[2]}; scene_line(c, e, gridColor);
            }
            if (g_grid.axes) {
                double a[] = {(g_grid.x - radius) / 16.0, g_grid.height / 16.0, g_grid.z / 16.0}, b[] = {(g_grid.x + radius) / 16.0, a[1], a[2]}; scene_line(a, b, xColor);
                double c[] = {g_grid.x / 16.0, a[1], (g_grid.z - radius) / 16.0}, e[] = {c[0], c[1], (g_grid.z + radius) / 16.0}; scene_line(c, e, zColor);
            }
        }
    }
    if (!g_modOff && g_projectList && g_projectLoaded && g_bpPage == 1 && (g_exactOpen || g_projectRunning)) {
        const I3 origin = g_projectRunning ? g_projectOrigin : project_origin_now();
        const int rotation = g_projectRunning ? g_projectRotation : (int)g_bpRot;
        const double world[] = {origin.x / core::kFix, origin.y / core::kFix, origin.z / core::kFix};
        if (g_showModules) for (int m = 0; m < g_projectManifest.count; ++m) {
            if (!g_projectModuleVisible[m]) continue;
            int low[3] = {}, high[3] = {};
            for (int c = 0; c < 8; ++c) {
                int point[3], turned[3];
                for (int a = 0; a < 3; ++a) point[a] = ((c >> a) & 1 ? g_projectModuleBoxes[m].hi[a] : g_projectModuleBoxes[m].lo[a]) - g_projectManifest.anchor[a];
                if (g_projectSlice) {
                    int slice = ((c >> 1) & 1 ? g_projectSliceY + g_projectSliceHeight : g_projectSliceY) - g_projectManifest.anchor[1];
                    if ((c >> 1) & 1) { if (point[1] > slice) point[1] = slice; } else if (point[1] < slice) point[1] = slice;
                }
                project::turn(rotation, point, turned);
                for (int a = 0; a < 3; ++a) { if (!c || turned[a] < low[a]) low[a] = turned[a]; if (!c || turned[a] > high[a]) high[a] = turned[a]; }
            }
            if (high[1] <= low[1]) continue;
            double l[3], h[3]; for (int a = 0; a < 3; ++a) { l[a] = world[a] + low[a] / 16.0; h[a] = world[a] + high[a] / 16.0; }
            float color[] = {0.2f + (m % 3) * 0.2f, 0.85f, 0.9f, 0.65f}; scene_box(l, h, color);
        }
        if (g_showObjects && g_projectUseObjects) {
            const float color[] = {0.85f, 0.4f, 1.f, 0.75f};
            for (int i = 0; i < g_projectObjectCount && g_sceneCount < extended::MaxLines - 12; ++i) {
                const auto& object = g_projectObjects[i];
                if (!g_projectModuleVisible[object.module] || (g_projectSlice && (object.p[1] * 16 < g_projectSliceY || object.p[1] * 16 >= g_projectSliceY + g_projectSliceHeight))) continue;
                double pos[3]; float rot[4]; extended::object_world(g_projectObjects[i], g_projectManifest.anchor, rotation, origin, pos, rot);
                double low[3], high[3]; for (int a = 0; a < 3; ++a) { low[a] = pos[a] - 0.2; high[a] = pos[a] + 0.2; }
                scene_box(low, high, color);
            }
        }
        for (int i = 0; i < g_projectManifest.pieceCount && g_sceneCount < extended::MaxLines - 12; ++i) {
            if (g_projectSlice && (g_projectPieceBoxes[i].hi[1] <= g_projectSliceY || g_projectPieceBoxes[i].lo[1] >= g_projectSliceY + g_projectSliceHeight)) continue;
            auto pose = project_piece_transform(g_projectManifest.pieces[i]); // build snapshots use g_projectRotation/origin
            if (!g_projectRunning) {
                int p[3]; for (int a = 0; a < 3; ++a) p[a] = g_projectManifest.pieces[i].offset[a] - g_projectManifest.anchor[a];
                int turned[3]; project::turn(rotation, p, turned); const int64_t anchor[] = {origin.x, origin.y, origin.z};
                for (int a = 0; a < 3; ++a) pose.pos[a] = anchor[a] + (int64_t)turned[a] * kProjectFixUnit;
                const float identity[] = {0,0,0,1}; core::bp_turn_rot(rotation + g_projectManifest.pieces[i].rotation, identity, pose.rot);
            }
            uint8_t* item = project_piece_item(g_projectManifest.pieces[i]); Aabb bounds{};
            if (item) {
                shape_bounds(item, &pose, &bounds); double low[3], high[3]; for (int a = 0; a < 3; ++a) { low[a] = bounds.mn[a]; high[a] = bounds.mx[a]; }
                const float color[] = {1.f, 0.6f, 0.3f, 0.8f}; scene_box(low, high, color);
            }
        }
        if (g_projectProblem) {
            double p[] = {g_projectProblemAt.x / core::kFix, g_projectProblemAt.y / core::kFix, g_projectProblemAt.z / core::kFix}, lo[3], hi[3];
            for (int a = 0; a < 3; ++a) { lo[a] = p[a] - 0.26; hi[a] = p[a] + 0.26; }
            const float red[] = {1, 0.1f, 0.1f, 1}; scene_box(lo, hi, red);
        }
    }
    if (!g_modOff && g_showAltars && ((g_grid.enabled && g_cfg.toolFree) || (g_projectList && g_exactOpen))) {
        const float color[] = {1.f, 0.7f, 0.15f, 0.45f};
        for (int b = 0; b < g_altarZoneCount; ++b) {
            double l[3], h[3]; for (int a = 0; a < 3; ++a) { l[a] = g_altarZones[b].lo[a] / 16.0; h[a] = g_altarZones[b].hi[a] / 16.0; }
            scene_box(l, h, color);
        }
        if (g_projectList && g_exactOpen && g_altarGeneration == g_projectGeneration) {
            const float proposal[] = {0.3f, 1.f, 0.5f, 0.6f};
            for (int p = 0; p < g_altarProposalCount; ++p) {
                double l[3], h[3]; for (int a = 0; a < 3; ++a) { l[a] = g_altarProposals[p].box.lo[a] / 16.0; h[a] = g_altarProposals[p].box.hi[a] / 16.0; }
                scene_box(l, h, proposal);
            }
        }
    }
    g_sceneTick = GetTickCount(); LeaveCriticalSection(&g_sceneLock); LeaveCriticalSection(&g_stateLock);
}
