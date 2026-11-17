// Native Home controls; included after the existing object window's UI helpers.
static bool extended_number(const char* label, int* value, int step, int low, int high) {
    static char text[21][32]; const char* names[21]; int values[21], n = 0, selected = 0;
    for (int k = -10; k <= 10; ++k) {
        const int v = *value + k * step; if (v < low || v > high) continue;
        if (!k) selected = n; values[n] = v;
        Buf b; b.f(v / 16.0, step % 16 ? 1 : 0); copy_text(text[n], b.z(), sizeof text[n]); names[n] = text[n]; ++n;
    }
    LONG out; if (!n || !ui_option(label, names, n, selected, &out)) return false;
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
        ui_text(T(S_GRID_HELP), STYLE_BODY);
    }
    if (ui_option(T(S_EXT_CREATIVE), yesNo, 2, g_creativeProps, &value)) { g_creativeProps = value; changed = true; }
    if (ui_option(T(S_EXT_OBJECTS), yesNo, 2, g_projectUseObjects, &value)) { g_projectUseObjects = value; changed = true; }
    if (ui_option(T(S_EXT_ZONES), yesNo, 2, g_showAltars, &value)) g_showAltars = value;
    if (ui_option(T(S_EXT_MODULES), yesNo, 2, g_showModules, &value)) g_showModules = value;
    if (changed) extended_save();
}
static void draw_catalog_window() {
    static int kind = 0, page = 0;
    BpFrame frame; bp_frame_begin(frame, T(S_EXT_CATALOG), T(S_EXT_CATALOG_HELP));
    const char* kinds[] = {T(S_EXT_MATERIALS), T(S_EXT_PROPS), T(S_EXT_PIECES), T(S_EXT_LINKED)}; LONG value;
    if (ui_option(T(S_EXT_CATEGORY), kinds, 4, kind, &value)) { kind = value; page = 0; }
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
            if (kind == 1 && catalog_object(g_catalog[i].item)) break;
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
        g_bp.h.nm = g_projectPlan.materialCount; for (int m = 0; m < g_bp.h.nm; ++m) g_bp.h.m[m] = g_projectPlan.materials[m];
        Buf b; b.fmt(T(S_EXT_REPLACED), num(changed < 0 ? 0 : changed).s); flash(b.z());
        ++g_projectGeneration; g_bpBitsOk = 0;
    }
}
