// Native Home controls; included after the existing object window's UI helpers.
static bool extended_number(const char* label, int* value, int step, int low, int high) {
    bool normalized = false;
    if (*value < low) { *value = low; normalized = true; } if (*value > high) { *value = high; normalized = true; }
    static char text[21][32]; const char* names[21]; int values[21], n = 0, selected = 0;
    for (int k = -10; k <= 10; ++k) {
        const int v = *value + k * step; if (v < low || v > high) continue;
        if (!k) selected = n; values[n] = v;
        Buf b; b.f(v / 16.0, step % 16 ? 1 : 0); copy_text(text[n], b.z(), sizeof text[n]); names[n] = text[n]; ++n;
    }
    LONG out; if (!n || !ui_option(label, names, n, selected, &out)) return normalized;
    *value = values[out]; return true;
}
static void draw_extended_tools() {
    ui_text(T(S_EXT_TITLE), STYLE_BODY);
    const char* yesNo[] = {T(S_OPT_OFF), T(S_OPT_ON)}; LONG value; bool changed = false;
    if (ui_option(T(S_GRID_ON), yesNo, 2, g_grid.enabled, &value)) { g_grid.enabled = value; changed = true; }
    if (g_grid.enabled) {
        const char* steps[] = {"0.5 m", "1 m"};
        if (ui_option(T(S_GRID_STEP), steps, 2, g_grid.spacing == 16, &value)) { g_grid.spacing = value ? 16 : 8; changed = true; }
        changed |= extended_number(T(S_GRID_HEIGHT), &g_grid.height, g_grid.spacing, -project::MaxCoord, project::MaxCoord);
        if (ui_button(T(S_GRID_CAMERA_Y))) {
            int64_t camera[3]; float rotation[4];
            if (frame_cam(camera, rotation)) { g_grid.height = extended::snap(camera[1] / core::kFix * 16, g_grid.spacing); changed = true; }
        }
        changed |= extended_number(T(S_GRID_X), &g_grid.x, g_grid.spacing, -project::MaxCoord, project::MaxCoord);
        changed |= extended_number(T(S_GRID_Z), &g_grid.z, g_grid.spacing, -project::MaxCoord, project::MaxCoord);
        if (ui_option(T(S_GRID_AXES), yesNo, 2, g_grid.axes, &value)) { g_grid.axes = value; changed = true; }
        static const char* levels[] = {"1", "2", "3", "4", "5", "6", "7", "8"};
        if (ui_option(T(S_GRID_LEVEL), levels, 8, g_gridLevel, &value)) g_gridLevel = value;
        if (ui_button(T(S_GRID_STORE))) { g_gridLevels[g_gridLevel] = g_grid.height; changed = true; }
        if (ui_button(T(S_GRID_RECALL))) { g_grid.height = g_gridLevels[g_gridLevel]; changed = true; }
        if (ui_button(T(S_GRID_AIM))) {
            I3 p; if (live_centre(&p)) {
                g_grid.x = extended::snap(p.x / core::kFix * 16, g_grid.spacing);
                g_grid.z = extended::snap(p.z / core::kFix * 16, g_grid.spacing); changed = true;
            }
        }
        bp_text(T(S_GRID_HELP));
    }
    if (ui_option(T(S_EXT_CREATIVE), yesNo, 2, g_creativeProps, &value)) { g_creativeProps = value; changed = true; }
    if (ui_option(T(S_EXT_OBJECTS), yesNo, 2, g_projectUseObjects, &value)) { g_projectUseObjects = value; changed = true; }
    if (ui_option(T(S_EXT_ZONES), yesNo, 2, g_showAltars, &value)) g_showAltars = value;
    if (ui_option(T(S_EXT_MODULES), yesNo, 2, g_showModules, &value)) g_showModules = value;
    if (ui_option(T(S_EXT_OBJECT_PREVIEW), yesNo, 2, g_showObjects, &value)) g_showObjects = value;
    if (changed) extended_save();
}
static void draw_catalog_window() {
    static int kind = 0, page = 0;
    static int append = 1, module = 1, offset[3] = {}, rotation = 0;
    BpFrame frame; bp_frame_begin(frame, T(S_EXT_CATALOG), T(S_EXT_CATALOG_HELP));
    const char* kinds[] = {T(S_EXT_MATERIALS), T(S_EXT_PROPS), T(S_EXT_PIECES), T(S_EXT_LINKED)}; LONG value;
    if (ui_option(T(S_EXT_CATEGORY), kinds, 4, kind, &value)) { kind = value; page = 0; }
    if (kind == 1) {
        const char* yesNo[] = {T(S_OPT_OFF),T(S_OPT_ON)};
        if (ui_option(T(S_EXT_APPEND), yesNo, 2, append, &value)) append = value;
        if (g_projectManifest.count) { int out; if (module > g_projectManifest.count) module = 1; if (ui_int(T(S_EXT_MODULE), module, 1, g_projectManifest.count, 1, &out)) module = out; }
        const Str axes[] = {S_GRID_X,S_GRID_HEIGHT,S_GRID_Z};
        for (int a = 0; a < 3; ++a) extended_number(T(axes[a]), &offset[a], 8, -project::MaxCoord, project::MaxCoord);
        const char* turns[] = {"0","90","180","270"}; if (ui_option(T(S_BP_ROTATION), turns, 4, rotation, &value)) rotation = value;
    }
    if (ui_button(T(S_EXT_REFRESH))) catalog_refresh();
    if (!g_catalogCount) bp_text(T(S_PROJECT_UNAVAILABLE));
    int total = 0; for (int i = 0; i < g_catalogCount; ++i) if (g_catalog[i].kind == kind) ++total;
    const int pages = (total + 19) / 20; if (page >= pages) page = 0;
    int newPage; if (pages > 1 && ui_int(T(S_EXT_PAGE), page + 1, 1, pages, 1, &newPage)) page = newPage - 1;
    int index = 0;
    for (int i = 0; i < g_catalogCount; ++i) {
        if (g_catalog[i].kind != kind) continue;
        if (index++ < page * 20 || index > (page + 1) * 20) continue;
        Buf label; bp_item_label(label, g_catalog[i].item); label.str(" (").x(g_catalog[i].item).c(')');
        if (ui_button(label.z(), kind != 3)) {
            if (kind == 0) { g_catalogMaterial = g_catalog[i].item; flash(T(S_EXT_SELECTED)); }
            if (kind == 2) { g_catalogPiece = g_catalog[i].item; flash(T(S_EXT_SELECTED)); }
            if (kind == 1 && catalog_object(g_catalog[i].item, offset, rotation, append != 0, module - 1)) break;
        }
    }
    bp_frame_rows_end(frame); BpRow row; bp_row_begin(row);
    if (ui_button(T(S_BP_BACK))) g_bpPage = 0;
    if (ui_button(T(S_EXACT_CANCEL))) g_menuReq = 2;
    bp_row_end(row); bp_keys(S_COUNT); bp_frame_end(frame);
}
static void draw_project_materials() {
    static int source = 0, target = 0, scope = 0, module = 1, category = 0;
    LONG value;
    if (!g_projectPlan.materialCount) return;
    if (g_projectNeedN) {
        bp_text(T(S_EXT_QUOTE));
        for (int i = 0; i < g_projectNeedN; ++i) {
            const auto& need = g_projectNeed[i]; Buf label, count, row;
            bp_item_label(label, need.item);
            const int64_t have = need.prop ? -1 : stock_of(need.item);
            if (creative_on() && (!need.prop || g_creativeProps)) count.str("-"); else if (have < 0) count.str("?"); else count.i(have);
            row.fmt(T(S_EXT_COST_ROW), label.z(), num(need.count).s, count.z()); bp_text(row.z());
        }
    }
    static char labels[64][96]; const char* names[64];
    for (int m = 0; m < g_projectPlan.materialCount; ++m) { Buf b; bp_item_label(b, g_projectPlan.materials[m].item); copy_text(labels[m], b.z(), sizeof labels[m]); names[m] = labels[m]; }
    if (source >= g_projectPlan.materialCount) source = 0; if (target >= g_projectPlan.materialCount) target = 0;
    if (ui_option(T(S_EXT_REPLACE_FROM), names, g_projectPlan.materialCount, source, &value)) source = value;
    if (ui_option(T(S_EXT_REPLACE_TO), names, g_projectPlan.materialCount, target, &value)) target = value;
    if (g_catalogMaterial && ui_button(T(S_EXT_USE_CATALOG))) {
        ProjectStateGuard guard;
        uint8_t* item = g_bpItemById ? g_bpItemById(g_catalogMaterial, g_bpItemReg) : nullptr;
        if (bp_block_item(item)) {
            int at = 0; while (at < g_projectPlan.materialCount && g_projectPlan.materials[at].item != g_catalogMaterial) ++at;
            if (at < core::BP_MAX_MATS) {
                if (at == g_projectPlan.materialCount) { g_projectPlan.materials[at] = core::BpMat{g_catalogMaterial, item[0x454], 0, 0}; ++g_projectPlan.materialCount; }
                target = at;
            }
        }
    }
    const char* scopes[] = {T(S_EXT_ALL), T(S_EXT_MODULE), T(S_EXT_CATEGORY)};
    if (ui_option(T(S_EXT_SCOPE), scopes, 3, scope, &value)) scope = value;
    if (scope == 1) { int v; if (ui_int(T(S_EXT_MODULE), module, 1, g_projectManifest.count, 1, &v)) module = v; }
    const char* categories[] = {T(S_EXT_GENERAL), T(S_EXT_WALL), T(S_EXT_FLOOR), T(S_EXT_ROOF), T(S_EXT_TOWER), T(S_EXT_STAIRS), T(S_EXT_PROPS)};
    if (scope == 2 && ui_option(T(S_EXT_CATEGORY), categories, 7, category, &value)) category = value;
    if (ui_button(T(S_EXT_REPLACE))) {
        ProjectStateGuard guard;
        int changed = project::replace_material(&g_projectPlan, g_projectManifest, source, target, scope == 1 ? module - 1 : -1, scope == 2 ? category : -1);
        g_projectNeedN = 0; g_projectProblem = false; g_projectLastProblem = nullptr;
        g_bp.h.nm = g_projectPlan.materialCount; for (int m = 0; m < g_bp.h.nm; ++m) g_bp.h.m[m] = g_projectPlan.materials[m];
        Buf b; b.fmt(T(S_EXT_REPLACED), num(changed < 0 ? 0 : changed).s); flash(b.z());
        ++g_projectGeneration; g_bpBitsOk = 0;
    }
}
static void draw_project_altars() {
    ui_text(T(S_EXT_ALTARS), STYLE_BODY);
    int selected;
    if (g_altarZoneCount && ui_int(T(S_EXT_ZONE_TEMPLATE), g_altarTemplate, 1, g_altarZoneCount, 1, &selected)) g_altarTemplate = selected;
    if (ui_button(T(S_EXT_PROPOSE))) { ProjectStateGuard guard; altar_assist_begin(); }
    if (g_altarScan >= 0) {
        Buf status; status.fmt(T(S_EXT_COVERAGE), num(g_altarUncovered).s, num(g_altarScan).s, num(g_projectPlan.count).s, num(g_altarProposalCount).s); bp_text(status.z());
        bp_text(T(S_EXT_ALTAR_HELP));
        if (g_altarUnassigned) { Buf b; b.fmt(T(S_EXT_UNASSIGNED), num(g_altarUnassigned).s); bp_text(b.z()); }
        if (g_altarProposalCount) {
            static int proposal = 1; if (proposal > g_altarProposalCount) proposal = 1;
            if (ui_int(T(S_EXT_PROPOSAL), proposal, 1, g_altarProposalCount, 1, &selected)) proposal = selected;
            ProjectStateGuard guard; auto& p = g_altarProposals[proposal - 1];
            bool changed = false; const Str labels[] = {S_GRID_X, S_GRID_HEIGHT, S_GRID_Z};
            for (int a = 0; a < 3; ++a) {
                int value = p.center[a];
                if (extended_number(T(labels[a]), &value, 8, -project::MaxCoord, project::MaxCoord)) {
                    int delta = value - p.center[a]; p.center[a] = value; p.box.lo[a] += delta; p.box.hi[a] += delta; changed = true;
                }
            }
            if (changed) altar_assist_begin(true);
        }
    }
}
static void draw_architecture_window() {
    static int type = 0, append = 1, rotation = 0, offset[3] = {}, size[3] = {96,64,64}, thickness = 8;
    static int opening = 0, openingData[4] = {16,0,16,32}, floorHole = 0, floorData[4] = {16,16,16,32};
    static int round = 0, floors = 3, floorHeight = 64, crenels = 1, roof = 0;
    static int stairKind = 0, flights = 2, rise = 0, reserve = 1;
    static uint32_t structureMaterial = 0;
    BpFrame f; bp_frame_begin(f, T(S_EXT_ARCHITECTURE), T(S_EXT_ARCH_HELP)); LONG value;
    const char* types[] = {T(S_EXT_WALL),T(S_EXT_FLOOR),T(S_EXT_ROOF),T(S_EXT_TOWER),T(S_EXT_STAIRS)};
    if (ui_option(T(S_EXT_CATEGORY), types, 5, type, &value)) type = value;
    const char* yesNo[] = {T(S_OPT_OFF),T(S_OPT_ON)};
    if (ui_option(T(S_EXT_APPEND), yesNo, 2, append, &value)) append = value;
    const char* turns[] = {"0", "90", "180", "270"};
    if (ui_option(T(S_BP_ROTATION), turns, 4, rotation, &value)) rotation = value;
    const Str axes[] = {S_GRID_X, S_GRID_HEIGHT, S_GRID_Z};
    for (int a = 0; a < 3; ++a) extended_number(T(axes[a]), &offset[a], 8, -project::MaxCoord, project::MaxCoord);
    if (g_catalogMaterial) {
        Buf b; bp_item_label(b, g_catalogMaterial); bp_text(b.z());
        if (ui_button(T(S_EXT_STRUCTURE_MAT))) structureMaterial = g_catalogMaterial;
        if (ui_button(T(S_EXT_FRAME_MAT))) g_componentFrameMaterial = g_catalogMaterial;
    }
    if (type != 4) {
        const Str dimensions[] = {S_EXT_WIDTH, S_EXT_HEIGHT, S_EXT_DEPTH};
        for (int a = 0; a < 3; ++a) extended_number(T(dimensions[a]), &size[a], 8, 8, 2048);
        extended_number(T(S_EXT_THICKNESS), &thickness, 8, 8, 64);
        if (type == 2) {
            const char* roofs[core::ROOF_COUNT]; for (int r = 0; r < core::ROOF_COUNT; ++r) roofs[r] = T(kRoofName[r]);
            if (ui_option(T(S_OPT_ROOF_SHAPE), roofs, core::ROOF_COUNT, roof, &value)) roof = value;
        }
        if (type == 0 || type == 1 || type == 3) {
            if (ui_option(T(S_EXT_OPENING), yesNo, 2, opening, &value)) opening = value;
            if (opening) {
                const Str fields[] = {S_EXT_OPENING_X,S_EXT_OPENING_Y,S_EXT_OPENING_W,S_EXT_OPENING_H};
                for (int i = 0; i < 4; ++i) extended_number(T(fields[i]), &openingData[i], 8, i < 2 ? 0 : 8, 2048);
            }
        }
        if (type == 3) {
            if (ui_option(T(S_EXT_ROUND), yesNo, 2, round, &value)) round = value;
            int out; if (ui_int(T(S_EXT_FLOORS), floors, 1, 32, 1, &out)) floors = out;
            extended_number(T(S_EXT_FLOOR_HEIGHT), &floorHeight, 8, thickness + 32, 512);
            if (ui_option(T(S_EXT_CRENELS), yesNo, 2, crenels, &value)) crenels = value;
            if (ui_option(T(S_EXT_FLOOR_HOLE), yesNo, 2, floorHole, &value)) floorHole = value;
            if (floorHole) {
                const Str fields[] = {S_EXT_OPENING_X,S_EXT_OPENING_Y,S_EXT_OPENING_W,S_EXT_OPENING_H};
                for (int i = 0; i < 4; ++i) extended_number(T(fields[i]), &floorData[i], 8, i < 2 ? 0 : 8, 2048);
            }
        }
    } else {
        const char* kinds[] = {T(S_EXT_STRAIGHT),T(S_EXT_TURNING),T(S_EXT_IMPERIAL),T(S_EXT_SPIRAL)};
        if (ui_option(T(S_EXT_LAYOUT), kinds, 4, stairKind, &value)) stairKind = value;
        int minimum = stairKind == 3 ? 4 : stairKind == 1 ? 2 : 1; if (flights < minimum) flights = minimum;
        int out; if (ui_int(T(S_EXT_FLIGHTS), flights, minimum, 32, 1, &out)) flights = out;
        extended_number(T(S_EXT_RISE), &rise, 8, 0, 512);
        if (ui_option(T(S_EXT_RESERVE), yesNo, 2, reserve, &value)) reserve = value;
        if (g_catalogPiece) {
            Buf b; bp_item_label(b, g_catalogPiece); bp_text(b.z());
            project::NativePiece piece{}; piece.item = g_catalogPiece; uint8_t* item = project_piece_item(piece);
            if (item) {
                CursorXf pose{}; pose.scale = 1; const float identity[] = {0,0,0,1}; core::bp_turn_rot(rotation, identity, pose.rot);
                Aabb bounds{}; shape_bounds(item, &pose, &bounds); Buf x,y,z,dimensions;
                x.f(bounds.mx[0]-bounds.mn[0],3); y.f(bounds.mx[1]-bounds.mn[1],3); z.f(bounds.mx[2]-bounds.mn[2],3);
                dimensions.fmt(T(S_EXT_NATIVE_DIMS),x.z(),y.z(),z.z()); bp_text(dimensions.z());
            }
        }
        bp_text(T(S_EXT_NATIVE_STAIR_HELP));
    }
    bp_frame_rows_end(f); BpRow row; bp_row_begin(row);
    if (ui_button(T(S_EXT_GENERATE))) {
        bool ok;
        if (type == 4) ok = architecture_stairs(stairKind, flights, offset, rotation, rise, append != 0, reserve != 0);
        else {
            extended::Component c; c.kind = type + extended::Wall;
            for (int a = 0; a < 3; ++a) c.size[a] = size[a] / 8;
            c.thickness = thickness / 8; c.roofShape = roof; c.round = round; c.floors = floors; c.floorHeight = floorHeight / 8; c.crenels = crenels;
            if (type == 3) c.size[1] = floors * c.floorHeight + (crenels ? 2 : 0);
            if (type == 0) c.size[2] = c.thickness; if (type == 1) c.size[1] = c.thickness;
            if (opening && (type == 0 || type == 1 || type == 3)) { c.openingCount = 1; c.openings[0] = {openingData[0]/8,openingData[1]/8,openingData[2]/8,openingData[3]/8}; }
            if (floorHole && type == 3) { c.floorHoleCount = 1; c.floorHoles[0] = {floorData[0]/8,floorData[1]/8,floorData[2]/8,floorData[3]/8}; }
            ok = architecture_component(c, offset, rotation, append != 0, structureMaterial);
        }
        if (!ok) flash(T(S_EXT_GENERATE_FAIL));
    }
    if (ui_button(T(S_EXT_CATALOG))) { catalog_refresh(); g_bpPage = 3; }
    if (ui_button(T(S_BP_BACK))) g_bpPage = 0;
    if (ui_button(T(S_EXACT_CANCEL))) g_menuReq = 2;
    bp_row_end(row); bp_keys(S_COUNT); bp_frame_end(f);
}
static void draw_project_preview_options() {
    const char* yesNo[] = {T(S_OPT_OFF), T(S_OPT_ON)}; LONG value; bool changed = false;
    if (ui_option(T(S_EXT_SLICE), yesNo, 2, g_projectSlice, &value)) { g_projectSlice = value; changed = true; }
    if (g_projectSlice) {
        changed |= extended_number(T(S_EXT_SLICE_Y), &g_projectSliceY, 8, -project::MaxCoord, project::MaxCoord);
        changed |= extended_number(T(S_EXT_SLICE_HEIGHT), &g_projectSliceHeight, 8, 8, 4096);
    }
    if (g_projectManifest.count) {
        static int selected = 1; if (selected > g_projectManifest.count) selected = 1;
        int out; if (ui_int(T(S_EXT_MODULE), selected, 1, g_projectManifest.count, 1, &out)) selected = out;
        if (ui_option(T(S_EXT_VISIBLE), yesNo, 2, g_projectModuleVisible[selected - 1], &value)) { g_projectModuleVisible[selected - 1] = value; changed = true; }
    }
    if (changed) { ++g_projectGeneration; g_bpBitsOk = 0; }
    bp_text(T(S_EXT_VISUAL_ONLY));
}
