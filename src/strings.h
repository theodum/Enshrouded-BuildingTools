// Every text the player sees, with its key in the translation files and the English
// text. {0}, {1}, ... are filled in by the mod (numbers, key names). The translation
// files (lang/<code>.txt, installed as BuildingTools-Languages\<code>.txt) use the same
// keys; lang/en.txt must match this list (the unit tests check it).
//
// X(id, key, English)
#define BT_STRINGS(X) \
    /* numbers */ \
    X(S_DECIMAL, "num.decimal", ".") \
    /* messages */ \
    X(S_READY, "msg.ready", "Building Tools {0} ready") \
    X(S_READY_CREATIVE, "msg.ready_creative", "Building Tools {0} ready (creative building)") \
    X(S_UNSUPPORTED, "msg.unsupported", "Building Tools: game version not supported") \
    X(S_MOD_OFF, "msg.mod_off", "Building Tools OFF (vanilla game)  |  {0} turns it on") \
    X(S_MOD_ON, "msg.mod_on", "Building Tools ON  |  {0} turns it off") \
    X(S_MODE_AREA, "msg.mode_area", "Area mode") \
    X(S_MODE_PREFAB, "msg.mode_prefab", "Prefab mode (vanilla)") \
    X(S_REPLACE_ON, "msg.replace_on", "Replace ON") \
    X(S_REPLACE_OFF, "msg.replace_off", "Replace OFF") \
    X(S_STYLE_FULL, "msg.style_full", "Replace style: full shape") \
    X(S_STYLE_OVERLAP, "msg.style_overlap", "Replace style: overlap only") \
    X(S_STYLE_CONNECTED, "msg.style_connected", "Replace style: connected (fill bucket, Prefab mode; right click deletes)") \
    X(S_FILL_SOLID, "msg.fill_solid", "Area fill: solid") \
    X(S_FILL_HOLLOW, "msg.fill_hollow", "Area fill: hollow room (walls, floor, ceiling)") \
    X(S_FILL_WALLS, "msg.fill_walls", "Area fill: walls only") \
    X(S_FILL_ROOF, "msg.fill_roof", "Area fill: roof ({0})") \
    X(S_ROOF_SHAPE, "msg.roof_shape", "Roof shape: {0}") \
    X(S_ROOF_TURNED, "msg.roof_turned", "Roof turned ({0} of {1})") \
    X(S_THICK_SET, "msg.thick", "Thickness: {0}") \
    X(S_OVERHANG_SET, "msg.overhang", "Roof overhang: {0}") \
    X(S_FILL_ROUND, "msg.fill_round", "Area fill: round ({0}, {1})") \
    X(S_ROUND_SHAPE, "msg.round_shape", "Round shape: {0}") \
    X(S_ROUND_TURNED, "msg.round_turned", "Round shape turned ({0} of {1})") \
    X(S_ROUND_SOLID, "msg.round_solid", "Round fill: solid") \
    X(S_ROUND_HOLLOW, "msg.round_hollow", "Round fill: hollow (a round wall)") \
    X(S_ROUND_STANDING, "msg.round_standing", "Round fill: standing") \
    X(S_ROUND_LYING, "msg.round_lying", "Round fill: lying (a tube along the area)") \
    X(S_FILL_DIAG, "msg.fill_diag", "Area fill: diagonal ({0})") \
    X(S_DIAG_TURNED, "msg.diag_turned", "Diagonal turned ({0} of {1})") \
    X(S_DIAG_STANDING, "msg.diag_standing", "Diagonal fill: standing (a wall from corner to corner)") \
    X(S_EXACT_ON, "msg.exact_on", "Exact mode ON: click once, then enter the sizes") \
    X(S_EXACT_CANCELLED, "msg.exact_cancelled", "Exact area cancelled") \
    X(S_EXACT_CLICK, "msg.exact_click", "Click to build the exact area") \
    X(S_EXACT_TOO_BIG, "msg.exact_too_big", "Too big: more than {0} shapes") \
    X(S_DIAG_LYING, "msg.diag_lying", "Diagonal fill: lying (a ramp)") \
    X(S_FREE_ON, "msg.free_on", "Free placement ON (no surface snapping)") \
    X(S_FREE_OFF, "msg.free_off", "Free placement OFF") \
    X(S_FREE_DISTANCE, "msg.free_distance", "Free placement distance {0} m") \
    X(S_CREATIVE_ON, "msg.creative_on", "Creative building ON (building costs nothing)") \
    X(S_CREATIVE_OFF, "msg.creative_off", "Creative building OFF") \
    X(S_CREATIVE_SERVER, "msg.creative_server", "Creative building is set on the server (its BuildingTools.ini)") \
    X(S_CORNERS_CLEARED, "msg.corners_cleared", "Corners cleared") \
    X(S_BUILD_CORNER, "msg.build_corner", "Build corner {0} of {1} set") \
    X(S_REPLACE_CORNER, "msg.replace_corner", "Replace corner {0} of {1} set") \
    X(S_REMOVE_CORNER, "msg.remove_corner", "Remove corner {0} of {1} set") \
    X(S_REMOVE_PIECE, "msg.remove_piece", "Building pieces are removed one at a time (Area remove works on blocks)") \
    X(S_TOO_BIG, "msg.too_big", "Too big: {0} shapes (max {1})") \
    X(S_PLACED, "msg.placed", "Placed {0} shapes") \
    X(S_PLACED_PARTIAL, "msg.placed_partial", "Placed {0} of {1} shapes (not enough material)") \
    X(S_REMOVED, "msg.removed", "Removed {0} shapes") \
    X(S_AREA_SENT, "msg.area_sent", "Area sent to the server: {0} shapes") \
    X(S_UNDO_COVERS, "msg.undo_covers", "undo covers {0}") \
    X(S_NOT_ENOUGH, "msg.not_enough", "Not enough material for {0} shapes (needs {1} items)") \
    X(S_UNDO_NEEDS, "msg.undo_needs", "Undo needs the replaced material back in your inventory") \
    X(S_NOTHING_TO_REPLACE, "msg.nothing_to_replace", "Nothing to replace there") \
    X(S_REPLACE_READ_FAIL, "msg.replace_read_fail", "Replace: could not read the aimed block, try again") \
    X(S_CREPLACE_OWN, "msg.creplace_own", "Connected replace: that is already your material") \
    X(S_CREPLACE_NOTHING, "msg.creplace_nothing", "Connected replace: nothing to replace there") \
    X(S_CREPLACE_CAPPED, "msg.creplace_capped", "Connected replace: only the first {0} shapes (FillLimit)") \
    X(S_CDELETE_UNAVAILABLE, "msg.cdelete_unavailable", "Connected delete: not available in this game version") \
    X(S_CDELETE_NOTHING, "msg.cdelete_nothing", "Connected delete: nothing to remove there") \
    X(S_CDELETE_CAPPED, "msg.cdelete_capped", "Connected delete: only the first {0} shapes (FillLimit)") \
    X(S_LANGUAGE_SET, "msg.language_set", "Building Tools language: English") \
    /* info bar (bottom centre) */ \
    X(S_BAR_AREA_BUILD, "bar.area_build", "Area build {0}/{1}") \
    X(S_BAR_AREA_REPLACE, "bar.area_replace", "Area replace {0}/{1}") \
    X(S_BAR_AREA_REMOVE, "bar.area_remove", "Area remove {0}/{1}") \
    /* rows in the game's building box (bottom right; the box is narrow) */ \
    X(S_HUD_AREA, "hud.area", "Area mode") \
    X(S_HUD_AREA_ON, "hud.area_on", "Area mode: on") \
    X(S_HUD_AREA_OFF, "hud.area_off", "Area mode: off") \
    X(S_HUD_FILL_SOLID, "hud.fill_solid", "Fill: solid") \
    X(S_HUD_FILL_HOLLOW, "hud.fill_hollow", "Fill: hollow") \
    X(S_HUD_FILL_WALLS, "hud.fill_walls", "Fill: walls") \
    X(S_HUD_FILL_ROOF, "hud.fill_roof", "Fill: roof") \
    X(S_HUD_ROOF_SHAPE, "hud.roof_shape", "Roof: {0}") \
    X(S_HUD_ROOF_TURN, "hud.roof_turn", "Turn roof") \
    X(S_HUD_FILL_ROUND, "hud.fill_round", "Fill: round") \
    X(S_HUD_ROUND_SHAPE, "hud.round_shape", "Round: {0}") \
    X(S_HUD_ROUND_TURN, "hud.round_turn", "Turn round shape") \
    X(S_HUD_ROUND_SOLID, "hud.round_solid", "Round: solid") \
    X(S_HUD_ROUND_HOLLOW, "hud.round_hollow", "Round: hollow") \
    X(S_HUD_ROUND_STANDING, "hud.round_standing", "Round: standing") \
    X(S_HUD_ROUND_LYING, "hud.round_lying", "Round: lying") \
    X(S_HUD_FILL_DIAG, "hud.fill_diag", "Fill: diagonal") \
    X(S_HUD_DIAG_TURN, "hud.diag_turn", "Turn diagonal") \
    X(S_HUD_DIAG_STANDING, "hud.diag_standing", "Diagonal: standing") \
    X(S_HUD_DIAG_LYING, "hud.diag_lying", "Diagonal: lying") \
    X(S_HUD_THICK, "hud.thick", "Thickness") \
    X(S_HUD_CLEAR, "hud.clear", "Clear corners") \
    X(S_HUD_REPLACE, "hud.replace", "Replace") \
    X(S_HUD_REPLACE_ON, "hud.replace_on", "Replace: on") \
    X(S_HUD_REPLACE_OFF, "hud.replace_off", "Replace: off") \
    X(S_HUD_STYLE_FULL, "hud.style_full", "Style: full") \
    X(S_HUD_STYLE_OVERLAP, "hud.style_overlap", "Style: overlap") \
    X(S_HUD_STYLE_CONNECTED, "hud.style_connected", "Style: connected") \
    X(S_HUD_FREE, "hud.free", "Free placement") \
    X(S_HUD_FREE_ON, "hud.free_on", "Free placement: on") \
    X(S_HUD_FREE_OFF, "hud.free_off", "Free placement: off") \
    X(S_HUD_WHEEL, "hud.wheel", "+ wheel: {0} m") \
    X(S_HUD_MENU, "hud.menu", "Tools menu") \
    X(S_HUD_RADIAL, "hud.radial", "Tools wheel") \
    /* floating text under the crosshair */ \
    X(S_FLOAT_TOO_BIG, "float.too_big", "too big") \
    X(S_FLOAT_SIZE, "float.size", "{0} x {1}, h {2} = {3}") \
    /* key names */ \
    X(S_KEY_CTRL, "key.ctrl", "Ctrl") \
    X(S_KEY_SHIFT, "key.shift", "Shift") \
    X(S_KEY_ALT, "key.alt", "Alt") \
    X(S_KEY_NUMBER, "key.number", "key {0}") \
    X(S_KEY_MOUSE_MIDDLE, "key.mouse_middle", "Middle mouse") \
    X(S_KEY_MOUSE4, "key.mouse4", "Mouse 4") \
    X(S_KEY_MOUSE5, "key.mouse5", "Mouse 5") \
    /* menu */ \
    X(S_MENU_TITLE, "menu.title", "Building Tools") \
    X(S_MENU_KEYS, "menu.keys", "Keys") \
    X(S_MENU_OPTIONS, "menu.options", "Options") \
    X(S_MENU_QUICK_KEYS, "menu.quick_keys", "Quick keys") \
    X(S_MENU_FOOTER, "menu.footer", "Version {0}. Esc or the menu key closes the menu.") \
    X(S_MENU_CLOSE, "menu.close", "Close") \
    X(S_MENU_CANCEL, "menu.cancel", "Cancel") \
    X(S_MENU_PRESS_KEY, "menu.press_key", "Press a key...") \
    X(S_MENU_NO_KEY, "menu.no_key", "None") \
    X(S_MENU_KEYS_HELP, "menu.keys_help", "Click a key to change it. Saved to BuildingTools.ini.") \
    X(S_MENU_KEYS_WAITING, "menu.keys_waiting", "Press the new key, also with Shift, Ctrl or Alt. Esc cancels, Delete clears it.") \
    X(S_MENU_TRANSLATION_NOTE, "menu.translation_note", "") \
    X(S_ACT_MENU, "menu.key_menu", "Building Tools menu") \
    X(S_ACT_RADIAL, "menu.key_radial", "Tools wheel (hold)") \
    X(S_ACT_AREA, "menu.key_area", "Area mode on / off") \
    X(S_ACT_REPLACE, "menu.key_replace", "Replace on / off") \
    X(S_ACT_STYLE, "menu.key_style", "Replace style") \
    X(S_ACT_FILL, "menu.key_fill", "Area fill") \
    X(S_ACT_ROOF_SHAPE, "menu.key_roof_shape", "Next roof / round shape (Area mode, roof or round fill)") \
    X(S_ACT_ROOF_TURN, "menu.key_roof_turn", "Turn roof / round / diagonal (Area mode, those fills)") \
    X(S_ACT_THICK, "menu.key_thick", "Thickness 1 / 2 / 3 (rooms, walls, roofs)") \
    X(S_ACT_OVERHANG, "menu.key_overhang", "Roof overhang none / 1 / 2 / 3") \
    X(S_ACT_ROUND_HOLLOW, "menu.key_round_hollow", "Round fill: solid / hollow") \
    X(S_ACT_ROUND_LYING, "menu.key_round_lying", "Round and diagonal fills: standing / lying") \
    X(S_ACT_CLEAR, "menu.key_clear", "Clear area corners") \
    X(S_ACT_EXACT, "menu.key_exact", "Exact mode on / off (one click, enter the sizes)") \
    X(S_ACT_FREE, "menu.key_free", "Free placement on / off") \
    X(S_ACT_FARTHER, "menu.key_farther", "Free placement farther") \
    X(S_ACT_CLOSER, "menu.key_closer", "Free placement closer") \
    X(S_ACT_WHEEL, "menu.key_wheel", "Distance (hold + mouse wheel)") \
    X(S_ACT_OFF, "menu.key_off", "Whole mod off / on") \
    X(S_ACT_STYLE_FULL, "menu.key_style_full", "Replace style: full shape") \
    X(S_ACT_STYLE_OVERLAP, "menu.key_style_overlap", "Replace style: overlap only") \
    X(S_ACT_STYLE_CONNECTED, "menu.key_style_connected", "Replace style: connected") \
    X(S_ACT_FILL_SOLID, "menu.key_fill_solid", "Area fill: solid") \
    X(S_ACT_FILL_HOLLOW, "menu.key_fill_hollow", "Area fill: hollow room") \
    X(S_ACT_FILL_WALLS, "menu.key_fill_walls", "Area fill: walls only") \
    X(S_ACT_FILL_ROOF, "menu.key_fill_roof", "Area fill: roof") \
    X(S_ACT_FILL_ROUND, "menu.key_fill_round", "Area fill: round") \
    X(S_ACT_FILL_DIAG, "menu.key_fill_diag", "Area fill: diagonal") \
    X(S_ACT_CREATIVE, "menu.key_creative", "Creative building on / off") \
    X(S_ACT_REPLACE_HOLD, "menu.key_replace_hold", "Replace while held") \
    X(S_ACT_PREFAB, "menu.key_prefab", "Prefab mode (vanilla)") \
    X(S_ACT_AREA_ON, "menu.key_area_on", "Area mode") \
    X(S_ACT_REPLACE_ON, "menu.key_replace_on", "Replace on") \
    X(S_ACT_REPLACE_OFF, "menu.key_replace_off", "Replace off") \
    X(S_MENU_QUICK_HELP, "menu.quick_help", "Keys that set an option directly. Click a key to set it.") \
    X(S_OPT_OFF, "option.off", "Off") \
    X(S_OPT_ON, "option.on", "On") \
    X(S_OPT_MODE, "option.mode", "Mode") \
    X(S_OPT_MODE_PREFAB, "option.mode_prefab", "Prefab (vanilla)") \
    X(S_OPT_MODE_AREA, "option.mode_area", "Area") \
    X(S_OPT_REPLACE, "option.replace", "Replace") \
    X(S_OPT_STYLE, "option.style", "Replace style") \
    X(S_OPT_STYLE_FULL, "option.style_full", "Full shape") \
    X(S_OPT_STYLE_OVERLAP, "option.style_overlap", "Overlap only") \
    X(S_OPT_STYLE_CONNECTED, "option.style_connected", "Connected (fill bucket)") \
    X(S_OPT_CORNERS, "option.corners", "Area corners") \
    X(S_OPT_CORNERS_2, "option.corners_2", "Two corners") \
    X(S_OPT_CORNERS_3, "option.corners_3", "Base corners + height") \
    X(S_OPT_CORNERS_EXACT, "option.corners_exact", "Exact (enter the sizes)") \
    X(S_OPT_FILL, "option.fill", "Area fill") \
    X(S_OPT_FILL_SOLID, "option.fill_solid", "Solid") \
    X(S_OPT_FILL_HOLLOW, "option.fill_hollow", "Hollow room") \
    X(S_OPT_FILL_WALLS, "option.fill_walls", "Walls only") \
    X(S_OPT_FILL_ROOF, "option.fill_roof", "Roof") \
    X(S_OPT_FILL_ROUND, "option.fill_round", "Round") \
    X(S_OPT_FILL_DIAG, "option.fill_diag", "Diagonal") \
    X(S_OPT_ROOF_SHAPE, "option.roof_shape", "Roof shape") \
    X(S_OPT_ROOF_GABLE, "option.roof_gable", "Gable (2 slopes)") \
    X(S_OPT_ROOF_HIP, "option.roof_hip", "Hip (4 slopes)") \
    X(S_OPT_ROOF_GAMBREL, "option.roof_gambrel", "Gambrel (barn)") \
    X(S_OPT_ROOF_MANSARD, "option.roof_mansard", "Mansard") \
    X(S_OPT_ROOF_SHED, "option.roof_shed", "Shed (1 slope)") \
    X(S_OPT_ROOF_CONE, "option.roof_cone", "Conical (round)") \
    X(S_OPT_ROOF_DOME, "option.roof_dome", "Dome (round)") \
    X(S_OPT_ROOF_TURN, "option.roof_turn", "Roof turn") \
    X(S_OPT_OVERHANG, "option.overhang", "Roof overhang") \
    X(S_OPT_ROUND_SHAPE, "option.round_shape", "Round shape") \
    X(S_OPT_ROUND_FULL, "option.round_full", "Full") \
    X(S_OPT_ROUND_HALF, "option.round_half", "Half") \
    X(S_OPT_ROUND_QUARTER, "option.round_quarter", "Quarter") \
    X(S_OPT_ROUND_TURN, "option.round_turn", "Round turn") \
    X(S_OPT_ROUND_FILL, "option.round_fill", "Round inside") \
    X(S_OPT_ROUND_HOLLOW, "option.round_hollow", "Hollow (wall)") \
    X(S_OPT_ROUND_DIR, "option.round_dir", "Round direction") \
    X(S_OPT_ROUND_STANDING, "option.round_standing", "Standing") \
    X(S_OPT_ROUND_LYING, "option.round_lying", "Lying (tube)") \
    X(S_OPT_DIAG_DIR, "option.diag_dir", "Diagonal direction") \
    X(S_OPT_DIAG_LYING, "option.diag_lying", "Lying (ramp)") \
    X(S_OPT_DIAG_TURN, "option.diag_turn", "Diagonal turn") \
    X(S_OPT_THICK, "option.thick", "Thickness (rooms, walls, roofs)") \
    X(S_OPT_NONE, "option.none", "None") \
    X(S_OPT_FREE, "option.free", "Free placement") \
    X(S_OPT_CREATIVE, "option.creative", "Creative building") \
    X(S_OPT_TOOLS_HELP, "option.tools_help", "Tools in use. A tool that is off leaves its keys to the game and leaves the hints and the wheel.") \
    X(S_OPT_MOD, "option.mod", "Building Tools (whole mod)") \
    X(S_OPT_TOOL_AREA, "option.tool_area", "Area mode") \
    X(S_OPT_TOOL_SHAPES, "option.tool_shapes", "Area shapes (roofs, round, diagonal)") \
    X(S_OPT_GLOBAL_CHESTS, "option.global_chests", "Magic chests of all bases") \
    X(S_OPT_LANGUAGE, "option.language", "Language") \
    X(S_OPT_LANGUAGE_GAME, "option.language_game", "Same as the game") \
    /* exact mode window */ \
    X(S_EXACT_TITLE, "exact.title", "Exact size") \
    X(S_EXACT_HELP, "exact.help", "Shapes past the clicked one: 0 = only it, negative = the other way.") \
    X(S_EXACT_KEY_LOOK, "exact.key_look", "Hold: hide the window, look around, walk") \
    X(S_EXACT_MATERIAL, "exact.material", "Material") \
    X(S_EXACT_REF, "exact.reference", "Clicked block") \
    X(S_EXACT_REF_CENTRE, "exact.ref_centre", "Middle (bottom)") \
    X(S_EXACT_REF_FL, "exact.ref_front_left", "Front left corner") \
    X(S_EXACT_REF_FR, "exact.ref_front_right", "Front right corner") \
    X(S_EXACT_REF_BL, "exact.ref_back_left", "Back left corner") \
    X(S_EXACT_REF_BR, "exact.ref_back_right", "Back right corner") \
    X(S_EXACT_REF_FC, "exact.ref_front_centre", "Front middle (bottom)") \
    X(S_EXACT_REF_BC, "exact.ref_back_centre", "Back middle (bottom)") \
    X(S_EXACT_WIDTH, "exact.width", "Width") \
    X(S_EXACT_DEPTH, "exact.depth", "Depth") \
    X(S_EXACT_HEIGHT, "exact.height", "Height") \
    X(S_EXACT_SIZE, "exact.size", "{0} x {1}, h {2} = {3} shapes") \
    X(S_EXACT_BUILD, "exact.build", "Build") \
    X(S_EXACT_REMOVE, "exact.remove", "Remove") \
    X(S_EXACT_REPLACE, "exact.replace", "Replace") \
    X(S_EXACT_CANCEL, "exact.cancel", "Cancel") \
    /* object placement */ \
    X(S_OBJ_SINGLE, "obj.single", "Single") \
    X(S_OBJ_LINE, "obj.line", "Line") \
    X(S_OBJ_CURVE, "obj.curve", "Curve") \
    X(S_OBJ_CIRCLE, "obj.circle", "Circle") \
    X(S_OBJ_ARCH, "obj.arch", "Arch") \
    X(S_OBJ_VCIRCLE, "obj.vcircle", "Standing circle") \
    X(S_OBJ_MODE, "obj.mode", "Objects: {0}") \
    X(S_OBJ_POINT, "obj.point", "{0}: point {1} of {2} set") \
    X(S_OBJ_PLACED, "obj.placed", "Objects placed: {0}") \
    X(S_OBJ_NOT_ENOUGH, "obj.not_enough", "Not enough for {0} objects") \
    X(S_OBJ_CLEARED, "obj.cleared", "Object line points cleared") \
    X(S_OBJ_REMOTE, "obj.remote", "Object lines work only in a world you host (not yet when you join a server)") \
    X(S_FLOAT_OBJ, "obj.float", "{0}: click {1} of {2}, {3} objects") \
    X(S_OBJ_TITLE, "obj.title", "Object placement") \
    X(S_OBJ_HELP, "obj.help", "The clicks before the last only mark points.") \
    X(S_OBJ_HOW_SINGLE, "obj.how_single", "Single: each click places one object, as in the game.") \
    X(S_OBJ_HOW_LINE, "obj.how_line", "Line: click the start, then the end.") \
    X(S_OBJ_HOW_CURVE, "obj.how_curve", "Curve: start, end, then a point it bends through.") \
    X(S_OBJ_HOW_CIRCLE, "obj.how_circle", "Circle: the centre, then a point on the circle.") \
    X(S_OBJ_HOW_ARCH, "obj.how_arch", "Arch: its two ends (a half circle standing up).") \
    X(S_OBJ_HOW_VCIRCLE, "obj.how_vcircle", "Standing circle: the centre, then a point on it.") \
    X(S_OBJ_MODE_LABEL, "obj.mode_label", "Mode") \
    X(S_OBJ_COUNT, "obj.count", "Number of objects") \
    X(S_OBJ_SPACING, "obj.spacing", "Random spacing (%)") \
    X(S_OBJ_ROT, "obj.rotation", "Random rotation (degrees)") \
    X(S_OBJ_ELEV, "obj.elevation", "Random elevation (cm)") \
    X(S_OBJ_REF, "obj.reference", "Reference plane") \
    X(S_OBJ_REF_GROUND, "obj.ref_ground", "Ground under each object (objects on walls: as Slope)") \
    X(S_OBJ_REF_LEVEL, "obj.ref_level", "Level (first click height)") \
    X(S_OBJ_REF_SLOPE, "obj.ref_slope", "Slope (straight from click to click)") \
    X(S_OBJ_TURN, "obj.turn", "Turn with the line (each object faces along it)") \
    X(S_OBJ_REROLL, "obj.reroll", "New random values") \
    X(S_OBJ_CLOSE, "obj.close", "Close") \
    X(S_WHEEL_OBJ_MODE, "obj.wheel_mode", "Objects") \
    X(S_WHEEL_OBJ_MENU, "obj.wheel_menu", "Object settings") \
    X(S_WHEEL_OBJ_CLEAR, "obj.wheel_clear", "Clear points") \
    X(S_ACT_OBJ_MODE, "obj.key_mode", "Object placement: next mode (single, line, curve, circle, arch, standing circle)") \
    X(S_ACT_OBJ_MENU, "obj.key_menu", "Object placement: settings window") \
    X(S_ACT_OBJ_UNDO, "obj.key_undo", "Object placement: undo (object in hand)") \
    X(S_OPT_TOOL_OBJECTS, "obj.tool", "Object lines") \
    /* blueprints */ \
    X(S_OPT_TOOL_BP, "bp.tool", "Blueprints") \
    X(S_ACT_BP, "bp.key", "Blueprint mode on / off") \
    X(S_ACT_BP_SUB, "bp.key_sub", "Blueprint mode: save <-> place") \
    X(S_OPT_MODE_BP, "bp.mode", "Blueprint") \
    X(S_BP_SUB_NAME, "bp.sub_name", "Blueprint") \
    X(S_BP_SUB_SAVE, "bp.sub_save", "Save") \
    X(S_BP_SUB_PLACE, "bp.sub_place", "Place") \
    X(S_MODE_BP, "bp.mode_on", "Blueprint mode ({0}): click a block") \
    X(S_HUD_BP, "bp.hud", "Blueprint mode") \
    X(S_HUD_BP_SAVE, "bp.hud_save", "Click: area to save") \
    X(S_HUD_BP_PLACE, "bp.hud_place", "Click: pick a blueprint") \
    X(S_BP_SAVED, "bp.saved", "Blueprint saved: {0} ({1} blocks, {2} materials)") \
    X(S_BP_EMPTY, "bp.empty", "Nothing to save there: only building blocks are saved") \
    X(S_BP_WRITE_FAIL, "bp.write_fail", "Could not write the blueprint file") \
    X(S_BP_REMOTE, "bp.remote", "Blueprints work only in a world you host (not yet on a dedicated server)") \
    X(S_BP_CANCELLED, "bp.cancelled", "Blueprint cancelled") \
    X(S_BP_SHAPE, "bp.shape", "Hold a block of the blueprint's size ({0} x {1} x {2} m) to build it") \
    X(S_BP_NOT_ENOUGH, "bp.not_enough", "Not enough materials for the whole blueprint (Allow partial build is off)") \
    X(S_BP_NO_MATERIAL, "bp.no_material", "Blueprint materials left out (no building block of them in this game): {0}") \
    X(S_BP_NOTHING, "bp.nothing", "Nothing to build: no material of this blueprint could be used") \
    X(S_BP_SAVE_TITLE, "bp.save_title", "Save blueprint") \
    X(S_BP_SAVE_HELP, "bp.save_help", "The clicked block is the reference. Blocks in the drawn box are saved (only building blocks).") \
    X(S_BP_NAME, "bp.name", "Name: {0}") \
    X(S_BP_NAME_HINT, "bp.name_hint", "Click the name to type it; Enter or another click ends typing (empty = date and time).") \
    X(S_BP_CLEAR_NAME, "bp.clear_name", "Clear name") \
    X(S_BP_SAVE_BTN, "bp.save_button", "Save") \
    X(S_BP_CELLS, "bp.cells", "Cells of {0} m (the shape in your hand): {1} x {2} x {3}") \
    X(S_BP_LIST_TITLE, "bp.list_title", "Blueprints") \
    X(S_BP_NONE, "bp.none", "No blueprints yet. Switch Blueprint to Save on the tools wheel, click a block and save one.") \
    X(S_BP_BAD_FILE, "bp.bad_file", "This file is not a blueprint this version can read") \
    X(S_BP_SIZE, "bp.size", "Size {0} x {1} x {2} cells, {3} blocks") \
    X(S_BP_ROTATION, "bp.rotation", "Rotation") \
    X(S_BP_PARTIAL, "bp.partial", "Allow partial build") \
    X(S_BP_REF, "bp.reference", "Point you look at") \
    X(S_BP_FOLLOWING, "bp.following", "The ghost follows where you look: hold the right mouse button and aim, then Place here.") \
    X(S_BP_PLACED, "bp.placed", "Placed: move it with the rows below, then Build.") \
    X(S_BP_PLACE_BTN, "bp.place_here", "Place here") \
    X(S_BP_FOLLOW_BTN, "bp.follow", "Follow the aim") \
    X(S_BP_MOVE, "bp.move", "Move the ghost (blocks; negative = left, closer, down):") \
    X(S_BP_RIGHT, "bp.right", "Right") \
    X(S_BP_AWAY, "bp.away", "Away") \
    X(S_BP_UP, "bp.up", "Up") \
    X(S_BP_BACK, "bp.back", "Back to the list") \
    X(S_BP_MOVE_POINT, "bp.move_point", "Move the clicked point (blocks; negative = left, closer, down):") \
    X(S_BP_SIZE_HDR, "bp.size_header", "Size (blocks past the reference point):") \
    X(S_BP_NOT_ENOUGH_ANY, "bp.not_enough_any", "Not enough materials for any block of this blueprint") \
    X(S_BP_SAVED_OBJ, "bp.saved_obj", "Blueprint saved: {0} ({1} blocks, {2} materials, {3} objects)") \
    X(S_BP_SAVING, "bp.saving", "Saving the blueprint: looking for objects...") \
    X(S_BP_OBJ_NONE_HERE, "bp.obj_none_here", "Objects not saved: save inside a base (the game finds them through its comfort check)") \
    X(S_BP_OBJ_COUNT, "bp.obj_count", "Objects ({0})") \
    X(S_BP_OBJ_BUILD, "bp.obj_build", "Build") \
    X(S_BP_OBJ_LEAVE, "bp.obj_leave", "Leave out") \
    X(S_BP_OBJ_ROW, "bp.object_row", "Object {0}: {1} needed, you have {2}") \
    X(S_BP_OBJ_MISSING, "bp.obj_missing", "Objects the game does not know: {0} (left out)") \
    X(S_BP_OBJ_LOGIC, "bp.obj_logic", "Logic objects (wirable: switches, lamps...) are left out for now: {0}") \
    X(S_BP_MIRROR, "bp.mirror", "Mirror") \
    X(S_BP_MIRROR_OBJS, "bp.mirror_objs", "Mirrored objects: turn around any kind that comes out facing the wrong way.") \
    X(S_BP_MIRROR_ASIS, "bp.mirror_asis", "As is") \
    X(S_BP_MIRROR_TURN, "bp.mirror_turn", "Turned around") \
    X(S_BP_COSTS, "bp.costs", "Materials needed:") \
    X(S_BP_MAT_ROW, "bp.material_row", "Material {0}: {1} needed, you have {2}") \
    X(S_BP_MAT_UNKNOWN, "bp.have_unknown", "?") \
    X(S_BP_STOCK_HINT, "bp.stock_hint", "? = not known yet: open the material picker once") \
    X(S_BP_SHORT, "bp.short", "Not enough of {0} materials: only part can be built") \
    X(S_BP_OBJECTS_LATER, "bp.objects_later", "Objects are saved too: what you can use (doors, chests, beds, stations), what gives comfort (furniture, lights) and ladders.") \
    /* composite projects */ \
    X(S_PROJECT_LIST, "project.list", "Composite projects") \
    X(S_PROJECT_CLASSIC, "project.classic", "Individual blueprints") \
    X(S_PROJECT_NONE, "project.none", "No .btproject files in BuildingTools-Projects beside enshrouded.exe.") \
    X(S_PROJECT_HELP, "project.help", "One anchor, one Build confirmation. All cells are checked before building, then checked again per batch. Hold the 0.5 m cube. Escape stops future batches; existing blocks remain. Native undo covers only part of large builds.") \
    X(S_PROJECT_SUMMARY, "project.summary", "{0} modules, {1} native cells, {2} duplicate cells removed") \
    X(S_PROJECT_OBJECTS, "project.objects", "Objects in these blueprints: {0}. The Include objects option controls their placement.") \
    X(S_PROJECT_ANCHOR, "project.anchor", "Anchor at the low corner of the aimed cell: X={0}, Y={1}, Z={2} m") \
    X(S_PROJECT_MATERIAL_ROW, "project.material_row", "{0}: {1} native voxels (exact costs checked before building)") \
    X(S_PROJECT_ERROR, "project.error", "Cannot load project: {0}") \
    X(S_PROJECT_BEGIN, "project.begin", "Project confirmed: checking the entire project before building") \
    X(S_PROJECT_CHECKING, "project.checking", "Checking project: {0}/{1} cells. Escape cancels.") \
    X(S_PROJECT_BUILDING, "project.building", "Building project: {0}/{1} cells. Escape stops future batches.") \
    X(S_PROJECT_COMPLETE, "project.complete", "Project finished: {0} cells written") \
    X(S_PROJECT_STOP, "project.stop", "Project stopped after {0} confirmed cells: {1}. See BuildingTools-Projects.log.") \
    X(S_PROJECT_SHAPE, "project.shape", "Select the native 0.5 m cube before opening the project") \
    X(S_PROJECT_UNAVAILABLE, "project.unavailable", "Native validation unavailable. Open the material picker in a base, then retry.") \
    X(S_PROJECT_COVERAGE, "project.coverage", "A cell is outside the existing altar areas") \
    X(S_PROJECT_COLLISION, "project.collision", "A target cell already contains building blocks") \
    X(S_PROJECT_MATERIAL, "project.material", "An unknown material or excessive cost was refused") \
    X(S_PROJECT_STOCK, "project.stock", "Not enough materials for the whole project, or stock unknown. Open the material picker.") \
    X(S_PROJECT_RANGE, "project.range", "A coordinate exceeds this version's placement range") \
    X(S_PROJECT_NATIVE, "project.native", "The game refused a cell or the batch could not be priced") \
    X(S_PROJECT_CHANGED, "project.changed", "The tool, screen, focus, session or creative setting changed") \
    X(S_PROJECT_CANCEL, "project.cancel", "Cancelled by the player") \
    X(S_PROJECT_TIMEOUT, "project.timeout", "No acknowledgement in 10 seconds; automatic retry disabled") \
    X(S_PROJECT_PARTIAL, "project.partial", "A batch was partially written. Inspect the world and log before retrying; no automatic rollback.") \
    X(S_PROJECT_JOURNAL, "project.journal", "Cannot write the project journal; construction stopped") \
    X(S_EXT_TITLE, "extended.title", "Building Tools Extended") \
    X(S_GRID_ON, "grid.on", "Virtual construction plane") \
    X(S_GRID_STEP, "grid.step", "Grid increment") \
    X(S_GRID_HEIGHT, "grid.height", "Plane altitude Y (m)") \
    X(S_GRID_CAMERA_Y, "grid.camera_y", "Use the camera altitude as a reference") \
    X(S_GRID_X, "grid.x", "Grid origin X (m)") \
    X(S_GRID_Z, "grid.z", "Grid origin Z (m)") \
    X(S_GRID_AXES, "grid.axes", "Show grid axes") \
    X(S_GRID_LEVEL, "grid.level", "Reference level") \
    X(S_GRID_STORE, "grid.store", "Save this level") \
    X(S_GRID_RECALL, "grid.recall", "Use this level") \
    X(S_GRID_AIM, "grid.aim", "Set grid origin at the aimed point") \
    X(S_GRID_HELP, "grid.help", "Aim down or up at the plane. The origin and eight saved altitudes persist between placements and sessions. No blocks are created by the grid.") \
    X(S_EXT_CREATIVE, "extended.creative", "Creative native objects (with creative building)") \
    X(S_EXT_OBJECTS, "extended.objects", "Include compatible objects in projects") \
    X(S_EXT_ZONES, "extended.zones", "Show altar coverage") \
    X(S_EXT_MODULES, "extended.modules", "Show module volumes") \
    X(S_EXT_OBJECT_CREATIVE, "extended.object_creative", "Enable Creative native objects, or turn creative building off to pay for objects") \
    X(S_EXT_OBJECT_UNSAFE, "extended.object_unsafe", "Unknown, special or disabled logic object refused; no ID was replaced") \
    X(S_EXT_OBJECT_HELP, "extended.object_help", "Props reuse original blueprint placement. Logic objects require explicit experimental opt-in; links and contents are not copied. Interactions need in-game testing. Native stock is checked per batch; failure can stop after blocks were built.") \
    X(S_EXT_CATALOG, "extended.catalog", "Native materials and objects library") \
    X(S_EXT_CATALOG_HELP, "extended.catalog_help", "Real IDs, names and model bounds are read from the game. Select a material, assign eight furniture roles and prepare/export the furnished demo. Suggestions use names only; confirm the selected models. Logic items require explicit opt-in.") \
    X(S_EXT_MATERIALS, "extended.materials", "Building materials") \
    X(S_EXT_PROPS, "extended.props", "Compatible native objects") \
    X(S_EXT_PIECES, "extended.pieces", "Native building pieces") \
    X(S_EXT_LINKED, "extended.linked", "Special or logic-linked objects (read only)") \
    X(S_EXT_CATEGORY, "extended.category", "Category") \
    X(S_EXT_REFRESH, "extended.refresh", "Refresh library") \
    X(S_EXT_PAGE, "extended.page", "Page") \
    X(S_EXT_SELECTED, "extended.selected", "Native item selected") \
    X(S_EXT_REPLACE_FROM, "extended.replace_from", "Source material") \
    X(S_EXT_REPLACE_TO, "extended.replace_to", "Replacement material") \
    X(S_EXT_USE_CATALOG, "extended.use_catalog", "Use the material selected in the library") \
    X(S_EXT_ALL, "extended.all", "Whole project") \
    X(S_EXT_MODULE, "extended.module", "Module") \
    X(S_EXT_SCOPE, "extended.scope", "Replacement scope") \
    X(S_EXT_GENERAL, "extended.general", "General") \
    X(S_EXT_WALL, "extended.wall", "Wall") \
    X(S_EXT_FLOOR, "extended.floor", "Floor") \
    X(S_EXT_ROOF, "extended.roof", "Roof") \
    X(S_EXT_TOWER, "extended.tower", "Tower") \
    X(S_EXT_STAIRS, "extended.stairs", "Native stairs") \
    X(S_EXT_REPLACE, "extended.replace", "Apply material replacement to the preview") \
    X(S_EXT_REPLACED, "extended.replaced", "Material replaced in {0} cells; world unchanged") \
    X(S_EXT_CHECK, "extended.check", "Check without building") \
    X(S_EXT_CHECK_OK, "extended.check_ok", "Project checked without world changes. Build will check again.") \
    X(S_EXT_ALTARS, "extended.altars", "Altar coverage assistance") \
    X(S_EXT_ZONE_TEMPLATE, "extended.zone_template", "Observed coverage zone used as template") \
    X(S_EXT_PROPOSE, "extended.propose", "Analyze coverage and suggest altar positions") \
    X(S_EXT_COVERAGE, "extended.coverage", "Uncovered samples: {0}. Checked {1}/{2}. Suggested areas: {3}.") \
    X(S_EXT_ALTAR_HELP, "extended.altar_help", "Gold: existing areas. Green: theoretical proposals using the observed area's dimensions. Positions are editable. Altar limits, upgrades, terrain suitability and permission still need checking in the game; no altar is placed automatically.") \
    X(S_EXT_PROPOSAL, "extended.proposal", "Suggested altar") \
    X(S_EXT_UNASSIGNED, "extended.unassigned", "Cells still outside the proposed areas: {0} (maximum 64 proposals)") \
    X(S_EXT_ARCHITECTURE, "extended.architecture", "Architectural components") \
    X(S_EXT_ARCH_HELP, "extended.arch_help", "Choose real materials in the library first. Components change only the preview; use Check and Build afterwards. Dimensions and offsets are metres. Floors use opening Y as depth Z. Rotation follows the project's native quarter turns.") \
    X(S_EXT_APPEND, "extended.append", "Add to the current project") \
    X(S_EXT_STRUCTURE_MAT, "extended.structure_mat", "Use library selection for the structure") \
    X(S_EXT_FRAME_MAT, "extended.frame_mat", "Use library selection for frames and borders") \
    X(S_EXT_WIDTH, "extended.width", "Width X (m)") \
    X(S_EXT_HEIGHT, "extended.height", "Height Y (m)") \
    X(S_EXT_DEPTH, "extended.depth", "Depth Z (m)") \
    X(S_EXT_THICKNESS, "extended.thickness", "Thickness (m)") \
    X(S_EXT_OPENING, "extended.opening", "Opening / floor void") \
    X(S_EXT_OPENING_X, "extended.opening_x", "Opening offset X (m)") \
    X(S_EXT_OPENING_Y, "extended.opening_y", "Opening offset Y / Z for floors (m)") \
    X(S_EXT_OPENING_W, "extended.opening_w", "Opening width (m)") \
    X(S_EXT_OPENING_H, "extended.opening_h", "Opening height / depth for floors (m)") \
    X(S_EXT_ROUND, "extended.round", "Round tower") \
    X(S_EXT_FLOORS, "extended.floors", "Storeys") \
    X(S_EXT_FLOOR_HEIGHT, "extended.floor_height", "Storey height (m)") \
    X(S_EXT_CRENELS, "extended.crenels", "Battlements") \
    X(S_EXT_FLOOR_HOLE, "extended.floor_hole", "Reserve openings through tower floors") \
    X(S_EXT_STRAIGHT, "extended.straight", "Straight") \
    X(S_EXT_TURNING, "extended.turning", "Quarter turn with landing") \
    X(S_EXT_IMPERIAL, "extended.imperial", "Imperial with two branches") \
    X(S_EXT_SPIRAL, "extended.spiral", "Square spiral with landings") \
    X(S_EXT_LAYOUT, "extended.layout", "Stair arrangement") \
    X(S_EXT_FLIGHTS, "extended.flights", "Native sections per flight") \
    X(S_EXT_RISE, "extended.rise", "Rise per native section (m; 0 = measured bounds)") \
    X(S_EXT_RESERVE, "extended.reserve", "Cut preview floor openings and 2 m headroom") \
    X(S_EXT_NATIVE_STAIR_HELP, "extended.native_stair_help", "Select a real stair piece in the library. Model orientation must make its climb face +Z; project rotation then turns the entire layout and landings. Measured bounds drive the layout. Spiral is square. Set the actual rise and test walkability in-game. No cube steps are generated.") \
    X(S_EXT_GENERATE, "extended.generate", "Generate into the project preview") \
    X(S_EXT_GENERATE_FAIL, "extended.generate_fail", "Generation refused: check dimensions, openings, material, native piece, overlaps and size limits") \
    X(S_EXT_PIECE_OVERLAP, "extended.piece_overlap", "A native piece overlaps planned blocks; reserve the floor opening or adjust its offset") \
    X(S_EXT_EXPORT, "extended.export", "Save the current project as new files") \
    X(S_EXT_EXPORTED, "extended.exported", "Project exported: {0}") \
    X(S_EXT_EXPORT_FAIL, "extended.export_fail", "Export refused: check write access and the 128 output-module limit. New incomplete files may remain; existing files were not overwritten.") \
    X(S_EXT_SLICE, "extended.slice", "Preview one altitude range") \
    X(S_EXT_SLICE_Y, "extended.slice_y", "Preview lower project Y (m)") \
    X(S_EXT_SLICE_HEIGHT, "extended.slice_height", "Preview level height (m)") \
    X(S_EXT_VISIBLE, "extended.visible", "Module visible in the preview") \
    X(S_EXT_VISUAL_ONLY, "extended.visual_only", "Visibility and altitude range affect the preview only. All project modules are built. Large plans use module volumes and native-piece bounds.") \
    X(S_EXT_OBJECT_PREVIEW, "extended.object_preview", "Show object position markers") \
    X(S_EXT_QUOTE, "extended.quote", "Costs calculated in the last check (partial if the check stopped early):") \
    X(S_EXT_COST_ROW, "extended.cost_row", "{0}: {1} required; known stock {2}") \
    X(S_EXT_RESUME, "extended.resume", "Resume the project kept in memory") \
    X(S_EXT_NATIVE_COUNT, "extended.native_count", "Native building pieces: {0}") \
    X(S_EXT_JOB_CHECK, "extended.job_check", "Checking placements: {0}/{1}. Escape cancels.") \
    X(S_EXT_JOB_BUILD, "extended.job_build", "Building placements: {0}/{1}. Escape stops future batches.") \
    X(S_EXT_JOB_DONE, "extended.job_done", "Project finished: {0} placements completed") \
    X(S_EXT_JOB_STOP, "extended.job_stop", "Project stopped after {0} completed placements: {1}. See BuildingTools-Projects.log.") \
    X(S_EXT_NATIVE_DIMS, "extended.native_dims", "Measured native bounds X/Y/Z: {0} / {1} / {2} m") \
    X(S_EXT_SECTION, "extended.section", "Extended section") \
    X(S_EXT_PREVIEW, "extended.preview", "Preview") \
    X(S_EXT_DIAGNOSTICS, "extended.diagnostics", "Diagnostics") \
    X(S_EXT_OVERVIEW, "extended.overview", "Overview") \
    X(S_EXT_OPEN_PROJECTS, "extended.open_projects", "Open composite projects") \
    X(S_EXT_OPEN_PREVIEW, "extended.open_preview", "Open the draft preview") \
    X(S_EXT_LAUNCH_HELP, "extended.launch_help", "Hold the construction hammer. This opens the native placement window without building; select the 0.5 m cube for composite construction.") \
    X(S_EXT_EXPERIMENTAL, "extended.experimental", "Experimental native placement (test world)") \
    X(S_EXT_NATIVE_DISABLED, "extended.native_disabled", "Automatic native objects/pieces are disabled. Enable Experimental native placement in Home > Options > Construction creative/native objects after reading the limits. Block-only projects remain available.") \
    X(S_EXT_EXPERIMENT_HELP, "extended.experiment_help", "Native objects and stairs are unvalidated in-game. Special/progression/logic-linked composite objects are refused; entity collisions and full rollback are unavailable. Use a test world.") \
    X(S_EXT_LINKED_HELP, "extended.linked_help", "Read-only category: special, progression or currently disabled logic objects. Experimental original-blueprint logic copy is available through the explicit option; special/progression items remain refused.") \
    X(S_EXT_BATCH, "extended.batch", "Maximum voxels per batch") \
    X(S_EXT_PIECE_BUDGET, "extended.piece_budget", "Native bounds exceed the safe probe budget (4096 voxels including headroom). Use a smaller native piece or place it manually.") \
    X(S_EXT_TRACE, "extended.trace", "Trace native manual object events (read only)") \
    X(S_EXT_ABI, "extended.abi", "Native piece ABI") \
    X(S_EXT_REGISTRY, "extended.registry", "Item registry") \
    X(S_EXT_BASES, "extended.bases", "Observed altar zones") \
    X(S_EXT_AVAILABLE, "extended.available", "Available") \
    X(S_EXT_UNAVAILABLE, "extended.unavailable", "Unavailable or not observed") \
    X(S_EXT_DIAG_ROW, "extended.diag_row", "{0}: {1}") \
    X(S_EXT_DIAG_HELP, "extended.diag_help", "Binary checks confirm instruction patterns, not in-game behavior. Open the hammer material selector once to observe the current registry and altar zones. Logs: BuildingTools.log and BuildingTools-Projects.log.") \
    X(S_EXT_OPERATION, "extended.operation", "Operation {0}; acknowledged placements {1}/{2}") \
    X(S_EXT_REPORT, "extended.report", "Save operation diagnostic report") \
    X(S_EXT_REPORTED, "extended.reported", "Diagnostic report saved in BuildingTools-Projects: {0}") \
    X(S_EXT_REPORT_FAIL, "extended.report_fail", "No finished/stopped operation to report, or file creation failed. See BuildingTools.log.") \
    X(S_EXT_RECOVERY_HELP, "extended.recovery_help", "Stopping preserves accepted batches. The report identifies the operation and acknowledged prefix; it does not authorize automatic removal or replay. Native undo is partial.") \
    X(S_EXT_ALTAR_SAMPLES, "extended.altar_samples", "Blocks are checked individually. Native parts use bounds corners and centre as estimates; construction repeats the authoritative native coverage checks.") \
    X(S_EXT_STAIR_MODEL, "extended.stair_model", "Native model orientation (climb toward +Z)") \
    X(S_EXT_NATIVE_EXAMPLE, "extended.native_example", "Prepare a native stair example between two levels") \
    X(S_EXT_OBJECT_BUDGET, "extended.object_budget", "Object bounds exceed 512 probe voxels. Use a smaller compatible object or the original manual placement tool.") \
    X(S_EXT_CHECK_MS, "extended.check_ms", "Slowest check (ms); current batch ceiling: {0}; {1}") \
    X(S_EXT_OBJECT_BOUNDS_FAIL, "extended.object_bounds_fail", "Native object could not be prepared: incompatible item, unavailable registry/bounds or project limit reached. No object was placed. See Diagnostics and BuildingTools.log.") \
    X(S_EXT_CONFIG_FAIL, "extended.config_fail", "Extended settings could not be saved. Current values work for this session; check write access to BuildingTools-Extended.ini. See BuildingTools.log.") \
    X(S_EXT_ACTION_BUSY, "extended.action_busy", "A composite operation is already running. Stop it before starting another.") \
    X(S_EXT_ACTION_WINDOW, "extended.action_window", "Open the placement window and select a composite project before using this action.") \
    X(S_EXT_ACTION_ARMED, "extended.action_armed", "A placement is already armed. Cancel it before requesting another action.") \
    X(S_EXT_ACTION_NO_PROJECT, "extended.action_no_project", "No valid composite project is loaded. Select or reload a project.") \
    X(S_EXT_ACTION_DISABLED, "extended.action_disabled", "Building Tools is disabled. Enable the mod before checking or building.") \
    X(S_EXT_ACTION_MODE, "extended.action_mode", "Composite execution requires Blueprint mode and the Blueprint tool enabled. The operation was not continued.") \
    X(S_EXT_ACTION_SHAPE, "extended.action_shape", "Action refused: captured shape is {0} x {1} x {2} m. A 0.5 x 0.5 x 0.5 m cube is required. Close this window, select that cube in the hammer and reopen the project.") \
    X(S_EXT_ACTION_SYSTEMS, "extended.action_systems", "Required native validation/placement hooks are unavailable. Check Diagnostics and BuildingTools.log; no construction was started.") \
    X(S_EXT_ACTION_NO_BASES, "extended.action_no_bases", "Altar zones have not been observed. Open Refresh altar zones and stock, then return to this project.") \
    X(S_EXT_ACTION_STALE_BASES, "extended.action_stale_bases", "Altar data is too old to start safely. Open Refresh altar zones and stock, then try again. Coverage checks remain required.") \
    X(S_EXT_ACTION_CURRENT_SHAPE, "extended.action_current_shape", "Captured shape: {0} x {1} x {2} m. Required cube: 0.5 m.") \
    X(S_EXT_ACTION_REFRESH, "extended.action_refresh", "Refresh altar zones and stock (native selector)") \
    X(S_EXT_ACTION_CLOSE_TIMEOUT, "extended.action_close_timeout", "The native placement window did not return to the game within 5 seconds. Operation stopped before sending another click. See BuildingTools.log.") \
    X(S_EXT_ACTION_WINDOW_CHANGED, "extended.action_window_changed", "The active game screen changed unexpectedly. Composite execution was stopped; no further batch was sent.") \
    X(S_EXT_ACTION_FOCUS, "extended.action_focus", "The game lost focus. Composite execution was stopped; no further batch was sent.") \
    X(S_EXT_ACTION_CREATIVE, "extended.action_creative", "Creative building changed during the operation. Execution stopped to keep pricing consistent.") \
    X(S_EXT_MATERIAL_UNKNOWN, "extended.material_unknown", "Unknown building material: no verified native block. Select a real replacement in Materials.") \
    X(S_EXT_MATERIAL_MISMATCH, "extended.material_mismatch", "Material IDs conflict: the saved byte now refunds a different item. Select an explicit replacement in Materials.") \
    X(S_EXT_MATERIAL_CONTEXT, "extended.material_context", "Module {0} ({1}), saved item {2}.") \
    X(S_EXT_MATERIAL_IDS, "extended.material_ids", "Native block {0}, refund item {1}, cost {2}. See BuildingTools.log.") \
    X(S_EXT_COST_ITEM, "extended.cost_item", "Unknown native cost item; execution refused.") \
    X(S_EXT_COST_EXCESSIVE, "extended.cost_excessive", "Excessive native cost; execution refused.") \
    X(S_EXT_COST_ITEMS, "extended.cost_items", "Too many different cost items; execution refused.") \
    X(S_EXT_COST_LIST, "extended.cost_list", "Invalid native cost list; execution refused.") \
    X(S_EXT_COST_VALUES, "extended.cost_values", "Cost item {0}, rejected value {1}, limit {2}. See BuildingTools.log.") \
    X(S_EXT_DEMO_STOREYS, "extended.demo_storeys", "Prepare the three-storey building with native stairs") \
    X(S_EXT_DEMO_SELECT, "extended.demo_select", "Select a real stair piece and a building material in the native library first.") \
    X(S_EXT_DEMO_RISE, "extended.demo_rise", "Set the actual rise of one tested stair flight (at least 0.5 m, in 0.5 m steps). Zero is an estimate and cannot prepare this test.") \
    X(S_EXT_DEMO_ABI, "extended.demo_abi", "Native stair bounds or ABI unavailable; the previous draft was kept.") \
    X(S_EXT_DEMO_BOUNDS, "extended.demo_bounds", "This model's bounds do not fit the demo grid. Use a stair with dimensions in 0.5 m steps; the previous draft was kept.") \
    X(S_EXT_DEMO_CLEARANCE, "extended.demo_clearance", "The measured stair volumes or their 2 m clearance intersect the building. The previous draft was kept.") \
    X(S_EXT_DEMO_BUDGET, "extended.demo_budget", "The selected stair would exceed the demo size, cell or native-part budgets. The previous draft was kept.") \
    X(S_EXT_DEMO_READY, "extended.demo_ready", "Three-storey draft ready: floors, landings, reserved openings, walls and roof. Door bays contain no interactive doors. Check, export and test stair walkability in-game.") \
    X(S_EXT_PROJECT_DIMS, "extended.project_dims", "Total dimensions after rotation: X={0}, Y={1}, Z={2} m (occupied/native bounds).") \
    X(S_EXT_VALIDATION_NONE, "extended.validation_none", "No validation recorded for this draft, position and mode. Use Check without building.") \
    X(S_EXT_VALIDATION_PROGRESS, "extended.validation_progress", "Preflight in progress: {0}/{1} placements; the quote is partial.") \
    X(S_EXT_VALIDATION_PASSED, "extended.validation_passed", "Last full preflight passed at this position. The world may have changed; Build checks again.") \
    X(S_EXT_VALIDATION_FAILED, "extended.validation_failed", "Last operation refused or interrupted. Read its reason and check again before building.") \
    X(S_EXT_NATIVE_UNOBSERVED, "extended.native_unobserved", "Native item registry not observed; availability is unknown.") \
    X(S_EXT_NATIVE_ISSUES, "extended.native_issues", "Unavailable or refused native references: {0}. First ID: {1}. Availability does not prove placement or interaction.") \
    X(S_EXT_QUOTE_PARTIAL, "extended.quote_partial", "Partial native costs; the entire project has not been validated. These are not a complete quote.") \
    X(S_EXT_QUOTE_STALE, "extended.quote_stale", "The project, position or mode changed after pricing. Recheck to obtain a current quote.") \
    X(S_EXT_ALTAR_BOUNDS, "extended.altar_bounds", "Observed altar zone {0}: low ({1}), high ({2}) m. Native coverage checks remain authoritative.") \
    X(S_EXT_SPATIAL_DETAIL, "extended.spatial_detail", "Position ({0}) m; {1}. See BuildingTools.log.") \
    X(S_EXT_NATIVE_MISSING, "extended.native_missing", "Native piece {0}, ID {1}, material {2}: {3}.") \
    X(S_EXT_PROP_ASSIGN, "extended.prop_assign", "Use catalogue choice for") \
    X(S_EXT_CLASSIC_BATCH, "extended.classic_batch", "The captured composition exceeds this native batch budget. It was not split or placed partially.") \
    X(S_EXT_CLASSIC_EDITED, "extended.classic_edited", "This captured composition has individual edits. Reload its source or import a new classic blueprint to preserve its recorded contacts.") \
    X(S_EXT_CLASSIC_IMPORT, "extended.classic_import", "Import this blueprint as a furniture-only composite (original native placement)") \
    X(S_EXT_BP_OBJECTS_OFF, "extended.bp_objects_off", "Enable Objects: Build in the classic blueprint options; the shared blueprint queue currently leaves objects out.") \
    X(S_EXT_CLASSIC_HELP, "extended.classic_help", "Captured composition: source object order and contacts retained; outside props, voxels and altar coverage checked. No source voxel will be built. Original logic links and chest contents are not copied.") \
    X(S_EXT_SCAN_EXACT, "extended.scan_exact", "World reading refused: {0}. This is not a confirmed collision. See native scan DATA in BuildingTools.log; index -1 denotes world reading.") \
    X(S_EXT_CALIBRATE, "extended.calibrate", "Calibrate one manually placed object during this blueprint capture") \
    X(S_EXT_CALIBRATION_HELP, "extended.calibration_help", "Capture exactly one complete furniture object. Native item, template, bounds and unit scale are verified; no guessed dimensions. World unchanged.") \
    X(S_EXT_CALIBRATION_LOAD, "extended.calibration_load", "Load a saved calibration (prepares one furniture object)") \
    X(S_EXT_SINGLE_TABLE, "extended.single_table", "Prepare one verified crude wooden table (zero structure)") \
    X(S_EXT_CALIBRATION_READY, "extended.calibration_ready", "Calibration saved: {0}. Original blueprint capture preserved. No object placed.") \
    X(S_EXT_CALIBRATION_ERROR, "extended.calibration_error", "Calibration refused: {0}. See BuildingTools.log. No native data substituted.") \
    X(S_EXT_ALREADY_PRESENT, "extended.already_present", "An existing object occupies this placement. Stopped to avoid duplication; no automatic replay.") \
    X(S_EXT_ROLE_REFUSED, "extended.role_refused", "Furniture role {0}, item {1}: native resolution refused ({2}). No replacement made.") \
    X(S_EXT_INDEX_SEARCH, "extended.index_search", "Search name or ID (click to type, Enter ends typing)") \
    X(S_EXT_INDEX_CATEGORY, "extended.index_category", "Imported category (name hint only)") \
    X(S_EXT_INDEX_DEMO, "extended.index_demo", "Select the eight exact CSV demo IDs") \
    X(S_EXT_INDEX_DEMO_READY, "extended.index_demo_ready", "CSV demo IDs selected, not validated. Enable original logic copy if needed, then prepare the measured room. No world change.") \
    X(S_EXT_FURNITURE_EXPORT, "extended.furniture_export", "Export furniture only (zero voxels and native pieces)") \
    X(S_EXT_PROP_ADD, "extended.prop_add", "Add one object to the current draft") \
    X(S_EXT_PROP_BED, "extended.prop_bed", "Demo: bed") \
    X(S_EXT_PROP_TABLE, "extended.prop_table", "Demo: table") \
    X(S_EXT_PROP_CHAIR, "extended.prop_chair", "Demo: two chairs") \
    X(S_EXT_PROP_TORCH, "extended.prop_torch", "Demo: two wall torches") \
    X(S_EXT_PROP_PAINTING, "extended.prop_painting", "Demo: painting") \
    X(S_EXT_PROP_CHEST, "extended.prop_chest", "Demo: chest") \
    X(S_EXT_PROP_SMALL, "extended.prop_small", "Demo: two table decorations") \
    X(S_EXT_PROP_FLOOR, "extended.prop_floor", "Demo: floor decoration") \
    X(S_EXT_PROP_SUGGEST, "extended.prop_suggest", "Suggest available demo objects (French/English names)") \
    X(S_EXT_PROP_ROOM, "extended.prop_room", "Prepare the furnished test room (replaces the draft)") \
    X(S_EXT_PROP_DEMO_READY, "extended.prop_demo_ready", "Draft ready: room and 11 native objects. Export, reload and Check before building. No world change.") \
    X(S_EXT_PROP_DEMO_FIT, "extended.prop_demo_fit", "Selected model does not fit the room, a support surface or a clear aisle. Select a smaller suitable object; previous draft kept.") \
    X(S_EXT_PROP_CATALOG_EXPORT, "extended.prop_catalog_export", "Export the identified native object catalogue") \
    X(S_EXT_PROP_CATALOG_DONE, "extended.prop_catalog_done", "Catalogue saved in BuildingTools-Projects: {0}") \
    X(S_EXT_PROP_CATALOG_FAIL, "extended.prop_catalog_fail", "Complete verified catalogue unavailable or export failed. Open the material selector and refresh the library.") \
    X(S_EXT_PROP_INVALID, "extended.prop_invalid", "Native object ID or measured bounds unavailable. No replacement was made.") \
    X(S_EXT_PROP_OVERLAP, "extended.prop_overlap", "Native object bounds intersect another planned or existing object.") \
    X(S_EXT_PROP_SUPPORT, "extended.prop_support", "No verified floor, wall or supporting furniture surface for this object.") \
    X(S_EXT_PROP_LIMIT, "extended.prop_limit", "This object operation exceeds the 2048-object validation budget.") \
    X(S_EXT_PROP_SCAN_LIMIT, "extended.prop_scan_limit", "More than 512 objects in the capture area; use a smaller area. No incomplete blueprint was saved.") \
    X(S_EXT_PROP_SCAN_FAILED, "extended.prop_scan_failed", "Existing-object scan unavailable, incomplete, changed or over budget. Operation refused; see the item ID and log.") \
    X(S_EXT_PROP_SCANNING, "extended.prop_scanning", "Reading existing objects for collision and support checks; world unchanged...") \
    X(S_EXT_PROP_DETAIL, "extended.prop_detail", "Object index {0}, native item {1}.") \
    X(S_EXT_PROP_LOGIC, "extended.prop_logic", "Allow the original blueprint logic-object copy (experimental)") \
    X(S_EXT_PROP_LOGIC_HELP, "extended.prop_logic_help", "Reuses the original blueprint event writer. Logic links, wiring and container contents are not copied. Door and light interactions require an in-game test. Special items remain refused.") \
    X(S_EXT_PROP_COUNTS, "extended.prop_counts", "Native objects placed: {0}/{1}; failed placement calls: {2}.") \
    X(S_EXT_VOXEL_STAIRS, "extended.voxel_stairs", "Voxel stairs") \
    X(S_EXT_VOXEL_L, "extended.voxel_l", "L-shaped") \
    X(S_EXT_VOXEL_U, "extended.voxel_u", "U-shaped") \
    X(S_EXT_VOXEL_MONUMENTAL, "extended.voxel_monumental", "Monumental (central flight and two branches)") \
    X(S_EXT_VOXEL_TREAD, "extended.voxel_tread", "Tread depth") \
    X(S_EXT_VOXEL_STEPS, "extended.voxel_steps", "Risers per route") \
    X(S_EXT_VOXEL_LANDING, "extended.voxel_landing", "Landing depth") \
    X(S_EXT_VOXEL_HEADROOM, "extended.voxel_headroom", "Reserved headroom") \
    X(S_EXT_VOXEL_SUPPORT, "extended.voxel_support", "Support structure") \
    X(S_EXT_VOXEL_SOLID, "extended.voxel_solid", "Solid infill") \
    X(S_EXT_VOXEL_WALLS, "extended.voxel_walls", "Side walls and tread decks") \
    X(S_EXT_VOXEL_STEP_RISE, "extended.voxel_step_rise", "Calculated rise per step: {0} m. This does not prove walkability.") \
    X(S_EXT_VOXEL_HELP, "extended.voxel_help", "The current composite backend uses full 0.5 m cubes. Height / risers must be an exact multiple of 0.5 m; smaller rises are refused. Steps may require jumping. Monumental width applies to each branch; the central flight is twice as wide. Check before building. The three-level demo uses straight flights, your dimensions and at least 4 m per level; it replaces the draft.") \
    X(S_EXT_VOXEL_DEMO, "extended.voxel_demo", "Prepare three levels with voxel stairs (no native pieces)") \
    X(S_EXT_VOXEL_DIM, "extended.voxel_dim", "Invalid voxel stair dimensions, rotation or grid-aligned position. Previous draft kept.") \
    X(S_EXT_VOXEL_RISE, "extended.voxel_rise", "Height / risers cannot be represented exactly by the current 0.5 m cubes. Change height or riser count; no smaller shape is certified.") \
    X(S_EXT_VOXEL_BAD_LANDING, "extended.voxel_bad_landing", "Landing must be at least as deep as the flight is wide, on the 0.5 m grid; turning layouts need at least two risers.") \
    X(S_EXT_VOXEL_OBSTRUCTED, "extended.voxel_obstructed", "Stairs, supports or reserved headroom intersect the project, or this room is too small.") \
    X(S_EXT_VOXEL_MATERIAL, "extended.voxel_material", "Select a registered building material in the library first.") \
    X(S_EXT_VOXEL_READY, "extended.voxel_ready", "Voxel stair draft ready. Preview, Check and export before building. Walkability remains to be tested in-game.") \
    X(S_EXT_VOXEL_AIR, "extended.voxel_air", "A building block obstructs the reserved stair headroom.") \
    X(S_EXT_VOXEL_SURFACE, "extended.voxel_surface", "Reserved stair air has no complete walking surface below it.") \
    X(S_EXT_VOXEL_SPACE_DETAIL, "extended.voxel_space_detail", "Reserved space index: {0}. See BuildingTools.log.") \
    X(S_EXT_VOXEL_SPACE_COUNT, "extended.voxel_space_count", "Reserved voxel surfaces: {0}. Headroom checks are geometric; the character may still need to jump. Test walkability in-game.")

enum Str {
#define BT_STR_ID(id, key, en) id,
    BT_STRINGS(BT_STR_ID)
#undef BT_STR_ID
    S_COUNT
};

// The languages of the game's language menu: file code (BuildingTools-Languages\<code>.txt),
// name in the language itself, the game's LanguageId names (user_settings.json) and the
// Steam language names that select it.
struct LangInfo { const char* code; const char* name; const char* game[3]; const char* steam[3]; };
static const LangInfo kLanguages[] = {
    { "en",    "English",            { "En_Us", "En_Gb" },           { "english" } },
    { "de",    "Deutsch",            { "De_De" },                    { "german" } },
    { "fr",    "Français",           { "Fr_Fr", "Fr_Ca" },           { "french" } },
    { "it",    "Italiano",           { "It_It" },                    { "italian" } },
    { "ja",    "日本語 (ja)",        { "Ja_Jp" },                    { "japanese" } },
    { "ko",    "한국어 (ko)",        { "Ko_Kr" },                    { "koreana" } },
    { "pl",    "Polski",             { "Pl_Pl" },                    { "polish" } },
    { "pt-BR", "Português (Brasil)", { "Pt_Br", "Pt_Pt" },           { "brazilian", "portuguese" } },
    { "ru",    "Русский",            { "Ru_Ru" },                    { "russian" } },
    { "es",    "Español",            { "Es_Es", "Es_Mx", "Es_419" }, { "spanish", "latam" } },
    { "th",    "ไทย (th)",           { "Th_Th" },                    { "thai" } },
    { "tr",    "Türkçe",             { "Tr_Tr" },                    { "turkish" } },
    { "uk",    "Українська",         { "Uk_Ua" },                    { "ukrainian" } },
    { "zh-CN", "简体中文 (zh-CN)",   { "Zh_Cn" },                    { "schinese" } },
    { "zh-TW", "繁體中文 (zh-TW)",   { "Zh_Tw" },                    { "tchinese" } },
};
static const int kLanguageCount = sizeof kLanguages / sizeof kLanguages[0];
