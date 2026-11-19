# Changelog

Each `## x.y.z` section is used as the release notes on GitHub and as the file
changelog on Nexus Mods, so keep it short and written for players.

## 0.34.0-dev6

- Record the user's dev5 validation of the 312-cell composite, linked buildings
  and passages, material resolution and altar limits, with explicit scope.
- Add a parametric three-storey test project bound to the selected real stair
  and material: floors, landings, shafts with 2 m clearance, walls, door bays,
  windows and roof. Calibrate the actual rise first; export/reload as ordinary
  BTBP/project files. Interactive doors remain excluded; native placement is opt-in.
- Show rotated dimensions, validation/quote status tied to position and mode,
  unavailable native references, observed altar bounds and actionable spatial
  errors. Reports retain operation-specific palette, costs and zones.
- Canonicalize microscopic floating boundary error for native part bounds;
  preserve conservative checks for real protrusions. Add transaction, geometry,
  rotation, export/reload and diagnostic regression tests.
- Include a technical audit, prioritized architecture roadmap and focused game
  protocol. Dev6 stair walkability, interaction, payment and persistence remain
  subject to real in-game testing.

## 0.34.0-dev5

- **Composite materials**: share the guarded classic resolver and search from the
  held material, with revalidated positive caches. Legacy refund IDs still require
  native proof; conflicting IDs cannot silently select a different material.
- Correct the material ID in the ten shipped example BTBPs using the real block
  confirmed by the user's dev4 registry log. Geometry is unchanged. Install the
  updated examples alongside the DLL; personal blueprints are never rewritten.
- Show separate material, identity-conflict, unknown cost item, excessive cost,
  cost-item-count and native-list errors. Log module, file, material bytes, saved,
  native and refund IDs, plus exact rejected quantities and limits.
- Retain the cost limits in Creative Building and recheck native lists during
  execution. Add regression coverage of the actual resolver/scanner and pricing
  adapter. Real game behaviour still requires the user's dev5 test.

## 0.34.0-dev4

- **Composite actions**: Build and Check now always receive the click and explain
  refusals, rather than silently disabling the buttons for an incompatible held
  shape. The captured dimensions and required 0.5 m hammer cube are shown beside
  the actions. Source blueprints at 1/2/4 m remain supported.
- Buttons and Enter share one guarded command path, with persistent French error
  messages and explicit logs for receipt, refusal, startup, window handoff,
  native input, validation results and build batches. Duplicate commands cannot
  start a second operation.
- Wait for the requested placement window closure and a stable game screen before
  sending the first native click; timeout cleanly if the handoff fails. Do not
  mistake a one-frame menu transition for cancellation. Existing safety checks
  for shape, session, mode, altars, resources and collisions remain enforced.
- Add access to the native altar/stock selector from the project window. Errors
  before any published batch return to the preview with their reason; no automatic
  retry or replay of a potentially paid batch.
- Add regression tests for both button actions, refusal paths, journal failure,
  duplicate input and the window-close handshake. Actual dev4 placement remains
  subject to personal in-game testing; the package includes a focused retest guide.

## 0.34.0-dev3

- **Home integration**: sections for projects, grid, preview, materials, creative
  building, native objects, altars and diagnostics; native window shortcuts and
  a less crowded project view, with French controls.
- **Stair correction**: rotate the entire flight layout and landings together;
  separate native-model orientation from assembly rotation. An in-game example
  uses the selected real stair/material IDs with two floors and reserved openings.
- **Native safety**: automatic props/pieces are experimental and off by default.
  Validate reflected ItemInfo/EquipmentSetup layouts and all ItemFlag values,
  as well as native commit/pricing instructions. Special/commerce/reward/unknown
  prop flags are refused. Incompatible items stay read-only with an explanation.
- **Consolidation**: idempotent material counts, deterministic duplicate ownership,
  object bounds in the project envelope, object-only costs, and bounded voxel
  collision checks for props. Indexed planned-cell overlap queries replace full
  plan scans for each piece.
- **Large builds**: configurable 128-voxel default batches, reduced automatically
  after slow checks; one native piece or four props per batch. Large native bounds
  fail with a clear explanation instead of unbounded probing.
- **Altars/configuration**: estimates also sample native bounds in 3D; authoritative
  coverage checks still run before construction. Save visibility, slice and level
  preferences without replacing other INI keys.
- **Diagnostics**: operation ID, plan fingerprint, timing and a non-overwriting
  report of the acknowledged prefix. No automatic removal/replay or save editing.
- **Test pack**: connected buildings, three storeys with openings, a virtual-plane
  example and 36 buildings / 60 passages / 75,648 native cells. Additional tests
  cover 0.5/1/2/4 m grids, stairs, collisions, interruptions, ABI and configuration.
- **Development build**: no in-game validation. Logic-linked composite objects,
  entity collision queries, textured/model ghosts, round spiral stairs, automatic
  altars and full rollback remain unavailable. Manual acceptance guide included.

## 0.34.0-dev2

- **Virtual plane**: a drawn grid at a fixed altitude, 0.5 / 1 m snapping, movable
  origin, camera-altitude reference and eight persistent levels in the separate
  `BuildingTools-Extended.ini`. No invisible world blocks.
- **Projects**: compatible native objects and verified native building-piece IDs,
  quarter-turn transforms, item flags checked for special/logic-linked objects,
  native pricing and per-entry geometry/material at commit. Ordinary prop placement
  can be free with the opt-in Creative native objects option.
- **Native library and architecture**: real registry IDs/names; walls with openings
  and frames, floors with voids/borders, roofs, square/round multi-storey towers,
  native straight/turning/imperial/square-spiral stair assemblies and landings.
  Failed generation restores the previous in-memory draft.
- **Preview**: bounded instanced Vulkan lines for grid, modules, altar areas,
  native-piece bounds and object markers; module visibility and altitude slices.
  Check without building returns to the preview and marks an offending voxel.
- **Materials**: replacement in the preview for the whole project, a module or
  architectural category, with native costs shown after checking.
- **Altars**: observed 3D areas/sub-areas and editable theoretical proposals;
  no automatic altar placement or bypass of progression/slot limits.
- **Export**: new BTBP tiles plus a manifest published last; preserve native-piece
  IDs/rotations/materials/headroom. Never overwrite existing files. Failed writes
  do not publish a completed manifest.
- Extended portable and exporter tests, UTF-8 validation, Win32-sized overflow
  guards, native ABI checks and an opt-in read-only native-prop diagnostic trace.
- **Experimental**: no in-game validation yet. Logic/special composite objects,
  entity collision queries, exact textured/model ghosts and full rollback remain
  unavailable; the status document gives the reasons and acceptance tests.

## 0.34.0-dev1

- **Extended composite projects**: the Blueprint Place window adds Composite projects
  (`BuildingTools-Projects/*.btproject`). Load multiple unchanged BTBP v1 files,
  move one anchor, change height, turn the entire plan and confirm Build once.
- Compatible source grids from 0.5 to 4 m are expanded into native 0.5 m voxels.
  Up to 128 modules / 1,048,576 expanded occupied cells, with deterministic positions,
  duplicate elimination and conflicting-material rejection.
- Before writing, check all target cells against native altar coverage, existing
  building voxels and native costs. Build in batches of at most 256 cells, checking
  again each time. Stop on failure, cancellation, focus/tool/session change or timeout;
  keep a diagnostic journal. Existing partial undo limits still apply; no full rollback.
- Ship a three-blueprint, 312-cell example, French/English UI and an installation/test
  protocol. Composite v1 places blocks only; classic blueprint objects remain available.
  Large previews use the existing bounding-volume fallback.
- Guard BTBP decoding against 32-bit cell-count overflow, zero-length runs and a torn
  optional object count; old files without object sections still load.
- Development build: Windows DLL, unit tests and local executable signature checks
  pass. **In-game composite behavior awaits personal validation.**

## 0.33.2

- **Blueprints**: some blueprints built blocks that nothing could remove (hammer, Area
  delete, explosives, pickaxe). When the game had a blueprint material under another
  number, the mod could pick the wrong data and build a game-internal material instead.
  Blueprint materials are now looked up the way the game does, and only real building
  blocks are built; a material that cannot be found is left out (as before, with a
  message that now names the material). Blocks placed with the wrong material by
  0.33.0/0.33.1 blueprints can now be removed with the hammer or Area delete (no refund;
  ruins and other blocks the game protects stay protected). Thanks to Blaze1404 for the
  report and the blueprint (#61).
- **Blueprints**: a block material whose refund is a plain item (the #61 blueprint's stone
  wall blocks) was left out of the build. The mod now builds it with the building block of
  that material when the game has one, and new saves store the block itself.
- **Area delete** is lighter: shapes of the box that hold nothing are no longer committed
  one by one (fewer stutters when the box is mostly air, and undo now covers more of the
  box), and the box preview no longer re-reads the blocks in it every frame.

## 0.33.1

- Creative building and Blueprint mode can be switched in single player before you first
  build with the hammer; before, the game was taken for a server session and the switch
  was refused.
- Tools wheel and pop-up messages: the blurred background no longer shows as a black box on
  ultrawide screens.
- Magic chests of all bases now work on dedicated servers (update the server too).
  Thanks to holc-soptim for the report and analysis (#57).
- Docs: how Creative building works on servers.

## 0.33.0

- **Shroudforge**: a second download, "Building Tools for Shroudforge", for players who use
  the Shroudforge mod loader. It installs the mod in Shroudforge's `mods` folder, so
  Shroudforge keeps its own `dinput8.dll`, lists Building Tools in its Modloader and loads
  it. Note: Shroudforge's F9 and F10 are also Building Tools' Replace style and Free
  placement keys; change one of them in either menu.
- If Building Tools gets loaded twice (for example by Shroudforge and by an old copy in the
  game folder), the second copy stays off and `BuildingTools.log` says where it is.

## 0.32.2

- **Menu with other mods**: some players had no in-game menu ("menu: not available" in
  the log) because another mod changes the game's screen switching code that the menu
  opens through. The mod now finds that code even when it has been changed.

## 0.32.1

- Flying: clicking the side of a block now aims at that block (the preview box and Area
  delete / Replace sometimes took the block below or above it).
- Flying: Area delete no longer stops working after switching from a big shape to a small
  one (it kept treating the hammer as holding the big shape until it was put away).
- The hoe, seeds and other tools no longer mark Area corners (their clicks counted as
  building clicks, and single player could think it was on a remote server).

## 0.32.0

- **Blueprints** (issue #49): save part of a building to a file and build it again anywhere,
  also in another world. Blueprint is a new mode next to Prefab and Area (the mode on the
  tools wheel; Options tab: Blueprints on / off; keys in the Keys tab), with two actions on
  the tools wheel:
  - **Save**: click a block and a window like Exact mode's opens. Choose where the clicked
    block sits in the box (now also front middle and back middle), move that point by
    blocks, set width, depth and height, type a name, and Save. The box is drawn beside the
    window; hold the right mouse button to look around. The blocks in the box are saved with
    their own materials, and the objects in it (doors, windows, chests, beds, stations,
    furniture, lights, decorations, railings...) with their position and rotation, to a
    file in the new `BuildingTools-Blueprints` folder next to `enshrouded.exe`.
  - **Place**: click and pick a blueprint. The next page shows the materials and objects it
    needs, by name, and what you have; its rotation (90 degree steps); Mirror; Allow partial
    build; Objects (build / leave out); and which point of it sits at the block you look at.
    Its ghost follows your aim (hold the right mouse button) until you press Place here;
    arrow rows move it by blocks right, away and up. Build places everything in one action:
    blocks first, every one with its own material, then the objects, each costing one of its
    item (creative building: free). With partial build on, every block your materials cover
    is built, material by material. Undo works as for areas.
  - **Mirror** builds the blueprint with left and right swapped. The game has no mirrored
    models, so objects are turned to fit; with Mirror on, the window lists every kind of
    object with As is / Turned around for any that comes out facing the wrong way (kept for
    next time).
  Good to know: blocks are saved per cell of the shape in your hand (hold the small 0.5 m
  cube for full detail; a cell at least half full counts as a full block). Save inside a
  base: the objects are found through the game's comfort check, which runs there. Doors,
  windows and other wirable objects join the base's logic network after the world is loaded
  again. Blueprints do not work on a dedicated server yet.
- Exact mode: two more reference points, front middle and back middle.

## 0.31.2

- **Works with GreeneyeDS mods** (Find - Base Storage Search, The Worst Map Mod). They hook
  the game's HUD drawing function that Building Tools' menu, tools wheel, HUD rows and
  on-screen messages use, which hid it from the mod: the menu said "not available" and
  keys like F7 gave no message on screen. The mod now finds that function behind the other
  mod's hook and runs next to it.
- **Tools wheel with Find**: Find also hooks a drawing function the wheel uses; the wheel now
  draws through that hook.
- **Works with XHL Planting Assistant.** Its planting stayed off ("live body/ABI mismatch")
  because the object undo patch changed the game's object placement code before Planting
  Assistant checked it. That patch is now written only when you first hold an object with
  Object lines on, after Planting Assistant has started.
- Planting seedlings (or placing any object) with Area mode on no longer marks Area corners.
- With XHL Planting Assistant (planting on), seeds and seedlings are left to it: Object lines
  (line, curve, circle...) still work for plants when it is not there, and for all other objects.

## 0.31.1

- **Higher frame rate.** The mod's checks before it reads game memory were far slower than
  expected and cost about 2 ms every frame with the hammer or a material in hand, plus 3 to
  4.5 ms with the magic chests of all bases on (about 100 FPS dropped to about 60). The mod
  now remembers memory it has already checked, so its share is below 0.1 ms a frame.
- `Profile=1` under `[Debug]` writes the frame rate and the time each part of the mod takes
  to BuildingTools.log every 10 seconds (off by default), for reports about FPS.

## 0.31.0

- **Undo for objects** (#51). The game's undo now also takes back objects you placed
  (chairs, torches, plants...): one object, or a whole line, curve or circle in one press,
  and gives them back. Press Y with the object still in hand (`ObjectUndo` on the Keys
  tab), or the game's undo with the hammer. It uses the game's own undo and its own
  "pick up" removal. A line of more than 30 objects can undo its first 30 (the game's
  undo holds 32 entries). On while Object lines is on; worlds you host, not yet on a
  dedicated server.
- **Smoother object lines**: the preview of a long line no longer costs many frames
  (it is worked out again only when something moves, and reads the ground in fewer steps).

## 0.30.0

- **Object placement** (#18). With an object in hand (a chair, torch, plant...), the tools
  wheel picks how the next clicks place it: Single (as the game), Line (2 clicks), Curve
  (start, end, then a point it bends through), Circle (centre, then a point on it), Arch
  (its two ends, a half circle standing up) or Standing circle (centre, then a point on
  it). The line and a ring for each object are drawn over the world while you aim; the
  last click places all of them and charges them at once.
- **Object settings** (on the wheel): number of objects (2 to 200), random spacing, random
  rotation and random height, the reference plane (the ground under each object, level
  at the first click, or a slope from click to click; objects on walls keep the line's
  height) and Turn with the line, so each object faces along the line or, on arches and
  standing circles, round the centre. In Single mode the random rotation and height apply
  to every object you place.
- **Object lines** can be switched off in the menu's Options tab (`Objects=` in `[Tools]`);
  then an object in hand works exactly as in the game.
- Works in any world you host (also alone); not yet when you join someone else's server
  or on a dedicated server. The game has no undo for objects, so a placed line is taken
  back by picking the objects up.

## 0.29.1

- **Large areas build again.** A big area (about 850 shapes and more, like a large roof)
  was refused with "not enough material" even with plenty in the backpack. The game
  refuses a payment with that many cost lines, so the mod now pays each material in one
  line.

## 0.29.0

- **Exact mode** (#46). A third choice for Area corners (two corners, base + height,
  exact), on the tools wheel and its key. Click once (left builds or replaces, right
  removes) and a window opens next to the area: pick where the clicked block sits (the
  middle of the bottom or a bottom corner, front and left as you look) and set width, depth
  and height as shapes past the clicked one (0 = only it, negative = the other way) with
  the arrows or the -10 / +10 buttons. The fill and its options and the game's material
  picker are in the window, and the 3D preview follows every change. Hold the right mouse
  button to hide the window, look around and walk; let go and it is back. Build (Enter)
  or Cancel (Esc). Works while flying; the last sizes are kept.
- **Options tab: pick your tools.** It now switches whole tools on or off: Building Tools
  itself, Replace, Area mode, Area shapes (roof, round and diagonal fills), Free placement,
  Creative building and Magic chests of all bases (plus the language). A tool that is off
  does nothing: its keys are the game's again and it leaves the building box hints and the
  tools wheel. The tools' own settings stay on their keys and the wheel.
- The old info panel window at the bottom of the screen is gone (it still showed at game
  start and exit); the building box rows and the floating text show everything.
- The shed roof preview now always matches what is built.
- The Keys tab is wider, so long (translated) action names are no longer under the keys.

## 0.28.1

- **Area clicks only while building.** With Area mode (and Replace) on, attacking or
  mining with a weapon or tool no longer marks area corners, and the area box no longer
  shows while you play. Corners are taken only with the hammer or a building material in
  hand. This mostly hit games where the mod thought it was on a server (single player
  while not holding the hammer).
- The crash log names the module of addresses outside the game and the mod.

## 0.28.0

- **Conical and dome roofs.** Two new roof shapes after shed (Shift+R). They are round:
  they cover the circle (square area) or ellipse (rectangular area) that the round fill
  makes over the same area, so they sit on top of round walls and towers. Conical has
  straight sides up to a point (45 degrees over a flat circle); dome is half a sphere,
  steep at the edge and flat on top. Thickness 1 to 3; overhang carries a cone's slope on
  down and gives a dome a flat brim. 3D preview of the exact blocks, works for build,
  replace and remove, and on dedicated servers (update the server too).

## 0.27.0

- **Diagonal walls in Area mode.** A new area fill, **diagonal**. Standing: a wall from your
  first corner to your second, seen from above, as high as the area (mark a wall one shape
  thin and you get a diagonal brace in it). Lying: a ramp from your first corner up or down
  to the far side, along the area's longer side; over a flat area it rises at 45 degrees,
  over a 3D area it runs exactly corner to corner. The steps share faces, so there are no
  gaps.
- **R** turns it: standing to the other diagonal, lying to fall instead of rise or to run
  along the shorter side. Standing / lying shares the round fill's key (unassigned at
  first), and is in the Options tab and on the tools wheel. Thickness 1 to 3, a quick key
  for the fill, 3D preview of the exact blocks. Works for build, replace and remove, and
  on dedicated servers (update the server too).

## 0.26.0

- **Round walls in Area mode** (#17). A new area fill, **round**: a full circle, a half or a
  quarter, solid or hollow. Your two corners bound the shape, so a square area gives a
  circle (or half / quarter of one) and a rectangle gives an ellipse. **Shift+R** picks
  full, half or quarter, **R** turns a half or quarter four ways (a quarter starts as a
  curve from your first corner to your second). Both keys work only with the round or roof
  fill on.
- **Hollow** builds a round wall with no gaps, 1 to 3 shapes thick; a half or quarter
  stays open on its straight sides.
- **Standing or lying.** Standing rounds follow the ground plan (towers, round rooms); mark
  a wall one shape thin and the round stands in it (arches). Lying turns the area into a
  tube along its longer side: pipes, and tunnels from a half.
- In the menu's Options tab and on the tools wheel (now up to 12 options); quick keys for
  the round fill, solid / hollow and standing / lying, unassigned at first. The 3D preview
  shows exactly the blocks that will be built. Works for build, replace and remove, and on
  dedicated servers (update the server too).

## 0.25.0

- **Roofs in Area mode** (#19). A new area fill, **roof**, with five shapes: gable (two
  slopes), hip (four slopes), gambrel (barn roof), mansard and shed (one slope).
  **Shift+R** picks the shape, **R** turns the roof. Both keys work only while the roof
  fill is on, so R stays free for the game otherwise.
- Mark a flat area, for example the top of your walls, and the roof covers it, about one
  shape up for every shape in from the edge, like the game's roof blocks. Mark a 3D area
  instead and the box height becomes the roof height, for steeper or flatter roofs.
- **Roof overhang** (none to 3 shapes) carries the slope past the area's edge.
- **Thickness** 1 to 3 shapes for hollow rooms, walls and roofs.
- All of it is in the menu's Options tab (which now scrolls) and on the tools wheel.
  Thickness and overhang also have their own keys, unassigned at first (set them in the
  Keys tab). The 3D preview shows the result before you click. Gable ends stay open. Works for build, replace and remove, and on
  dedicated servers (update the server too).

## 0.24.1

- **Works with Enshrouded Patch #1** (1.0.1.0). The patch moved game code the mod hooks,
  so 0.24.0 turned itself off ("game version not supported"). Everything is back: area
  building, Replace, fill delete, the menu, the tools wheel, the 3D preview, the game's
  own sounds and the dedicated server support.
- Update the server too: the patched `enshrouded_server.exe` needs this version.
- This version needs the patched game; keep 0.24.0 for the original 1.0 client.

## 0.24.0

- **3D area preview.** While you set up an area, the mod draws its own see-through box
  over it, in the game's building-ghost style: a soft pulsing fill, a 0.5 m grid and bright
  edges. Parts hidden behind terrain or blocks stay visible, drawn dimmer with dashed edges,
  so you can tell front from back. Your own character never hides it, and the box turns
  red while you stand inside it.
- Inside the box you see **the blocks the action will touch**, as one shaded solid: every
  block for a solid fill, the shell for a hollow room, the walls for walls only. Blue
  builds, amber replaces, pink-red removes. Delete and Replace (overlap or connected
  style) show only the blocks that are already there.
- It replaces the edge highlights. If the box can't be drawn on your PC, the edge
  highlights come back by themselves. Turn it off with `[Area] DrawBox=0` in
  `BuildingTools.ini`; `DrawBoxBlocks=0` hides the blocks and keeps the box.

## 0.23.0

- Fix: **the area preview was one block off** when one corner was on the ground and the
  other on a wall. The area built was right; the preview and the size readout now match it.
- New option **base-area box preview**: `[Area] Preview=2` in `BuildingTools.ini` shows
  the area being set up with the box the game draws for a flame altar's base area
  (`Preview=3` shows it together with the edge highlights). Like the game's own, it
  lights up where the box meets blocks and terrain.

## 0.22.0

- **Tools wheel.** Hold Caps Lock with the hammer or a building material in hand: a
  blurred wheel in the middle of the screen, drawn with the game's own wheel. Move the mouse towards an option and let go
  of the key (or click it to change several things in one go; right click closes).
  It shows only what applies right now: mode, Replace, Free placement and the menu
  always, Replace style while Replace is on, Area fill and Clear corners in Area mode.
  The camera stays still and clicks don't build while it is open.
- **Quick keys** (a new section of the Keys tab, which now scrolls): Replace while a
  key is held (back as it was when you let go), and keys that set one option directly: Prefab or Area mode, Replace on or
  off, each replace style, each area fill, creative building on / off. They have no key
  until you set one.
- Creative building can be switched in the menu (Options tab) in single player.
- Middle mouse and the mouse side buttons (Mouse 4 / 5) can be bound to any action.

## 0.21.0

- **Dedicated servers.** Put `dinput8.dll` next to `enshrouded_server.exe` as well as
  in your game folder. Area mode, Replace, the fill bucket, connected delete and area
  remove then work on the server. Your game sends your Building Tools settings to the
  server with its normal input, so each player builds with their own settings, and
  players without the mod build as usual. The server's `BuildingTools.ini` decides
  creative building, chests of all bases and the shape limits.
- The area corners and guide lines show in your game while you play on a server, and
  "Area sent to the server: N shapes" confirms the final corner. This also works with
  the free camera flown far from your character.

## 0.20.3

- Fix: **Replace, the fill bucket and the area preview far from your character with the
  free camera** (Z + Space). Flying the camera away from the character, Replace failed
  with "could not read the aimed block" and the blue area preview disappeared: far from
  the character the game stops updating the cursor the mod reads. The mod now works out
  the aimed block from your crosshair itself, so all of these work however far the
  camera is.

## 0.20.2

- Fix: **works together with XHL Vein Mining again.** Building Tools rerouted every
  call of the game's item payment, including the one that gives you the ore you mine.
  Vein Mining checks that call before it starts, saw it changed and stayed off. Building
  Tools now only reroutes the building systems' own calls, so both mods run at the same
  time (load them through the XHL `dinput8.dll` and `dinput8-chain.ini`, see the README).

## 0.20.1

- Fix: **Area remove and connected delete on building pieces** (foundations, stairs and
  other placed pieces). The game removes a piece as one object, so a right click deleted
  the whole clicked piece, the area reset, and the refund could count pieces that stayed.
  Now Area remove and connected delete treat pieces like blocks: corners work, and the
  blocks of the area (or the touching blocks of the clicked material) are removed and
  refunded. In Prefab mode without connected delete, a right click still removes the
  whole piece as in the game.
- **A building piece in the hammer works as in the game:** Replace and the fill bucket
  leave it alone (Area mode still repeats it).
- **The fill bucket works with the small shapes** (the first row of the shape picker).
  With a bigger shape selected, connected replace repaints one whole shape and connected
  delete removes one whole shape, as the full-shape style does. Replace with a bigger
  shape repaints the part the new shape covers.
- Fix: the mod's rows in the building box no longer disappear while the hammer aims at
  the sky.
- A one-shape Replace you can't pay for now says "Not enough material" instead of doing
  nothing.

## 0.20.0

- New: **the mod speaks your language.** Menu, building box rows, floating text, info bar
  and messages follow the game's language, in all 15 languages of the game's language
  menu. English and Polish are checked; the other translations are drafts and may contain
  mistakes, sorry! Corrections are very welcome on Nexus or GitHub (see TRANSLATING.md).
- The translations are plain text files in the new `BuildingTools-Languages` folder next
  to the mod, so anyone can fix a word and see it in the game.
- New option **Language** (Options tab, or `Language=` in the ini): follow the game, or
  pick one by hand.
- Key names and distances use the language's own form (for example `Strg`, `4,0 m`).
- Polish translation checked and corrected by Aleksander.

## 0.19.0

- New: **connected delete.** With Replace on and the style set to connected (fill
  bucket), a right click removes every touching block of the clicked material, up to
  FillLimit. Other materials in the same spots stay. Works on blocks and roofs, and on
  terrain while a terrain material is selected (the hammer's own rule). The material is
  refunded with the normal pop-up; undo covers the first 30 or so shapes, as Area remove.
- New: **Replace works on terrain.** With a terrain material selected, Replace (all three
  styles) repaints ground too, for example dirt into stone, keeping its shape. The fill
  bucket spreads over terrain as well. The old terrain is refunded by how much of it was
  there.
- New: **the fill bucket follows roofs diagonally.** For roof materials, connected replace
  and connected delete also spread to diagonal neighbours (side to side, up and down and
  across), so a whole sloped roof is caught in one click.

## 0.18.0

- New: **Building Tools menu in the game's own UI.** Press Home (also while holding the
  hammer) for a full-screen menu that looks and works like the game's menus: blurred
  world, the game's tab bar (Q / E), mouse cursor, and Esc to close. The character and
  camera stay still while it is open.
- **Keys tab:** click a key, then press the new one. Shift, Ctrl and Alt combinations
  work (for example Shift+K). Esc cancels, Delete clears. Saved to BuildingTools.ini at
  once; a key taken from another action is removed there.
- **Options tab:** mode, Replace, replace style, area fill and free placement, as rows
  like the game's settings. The F-keys keep working.
- The menu key itself can be changed in the Keys tab or with `Menu=` in the ini.
- New: **the mod's keys in the game's building box** (bottom right, under "Undo Build
  Action"), with the game's own key icons: Area mode, Replace and Free placement with the
  game's on/off switch, plus fill, replace style, clear corners, wheel distance and the
  menu key while they apply. `HudRows=0` in the ini turns them off.
- New: **info as floating text** below the screen centre, drawn like the game's snapping
  hint (blurred, soft backdrop): area state, size and block count, and the mod's short
  messages. The old info bar at the bottom only shows up when the building box is not
  there (for example with `HudRows=0`).

## 0.17.4

- Fix: Replace often said "could not read the aimed block, try again", above all while
  flying (Airborne Architect) and on the first clicks after loading. The game skips its
  own placement preview while the ghost is blocked, and the mod only learned where you
  aim from that preview. It now reads your aim every frame.

## 0.17.3

- New: **partial area and connected actions.** When an area build, area replace or
  connected replace costs more than your backpack and chests hold, it now does as many
  shapes as you can pay for (bottom layer first; nearest blocks first for connected
  replace) instead of nothing. The banner says "Placed N of M shapes (not enough
  material)". Replace refunds and undo cover exactly the shapes that were done.
- Fix: such an action used to fail without a word (often right after the "FillLimit"
  message). If not even one shape can be paid, the banner now says "Not enough
  material for N shapes (needs X items)".
- Fix: very big area and connected actions (thousands of shapes) could take longer to
  price than the mod waited for the game to write them, and then only one shape or
  nothing was changed. The wait now starts when pricing is done.
- Fix: connected replace with a big shape said "that is already your material" when
  the aimed spot also held blocks of the new material (for example next to blocks
  already repainted). It now picks the clicked material from the other blocks there.
- The FillLimit message now reads "only the first N shapes (FillLimit)": the shapes
  found up to the limit are still replaced.

## 0.17.2

- Fix (magic chests of all bases): crafting at a bench in a base without any chests of
  its own (or outside every base) only used the backpack, although the costs showed
  the materials of the other bases. It now uses the chests of every base there too.

## 0.17.1

- Faster and lighter: the info panel no longer re-creates its font, looks up the game
  window and asks Windows for key names twenty times a second; hooks share one small
  block of memory instead of 64 KB each; a few unused game lookups at start-up are gone.
- Fix: connected replace (fill bucket) with a very large `FillLimit` could freeze the
  game for a long time; its search table is now big enough for the maximum limit.
- Fix: the mod no longer refuses to start when one unused piece of game code is not
  found (after a game update).
- The info panel shows the key you set for the free placement wheel (not always Q).
- Cleaner log: debug lines from earlier versions removed.

## 0.17.0

- New: **magic chests of all bases** (`GlobalMagicChests=1` in the new `[Chests]`
  section of `BuildingTools.ini`, off by default). Building and crafting take
  materials from the magic chests of every flame altar area, not only the one you
  stand in, also outside a base. The material picker, the red "missing materials"
  ghost and the crafting menus count them too. Order: backpack, the chests of the
  base you are in, then the other bases. The loot deposit stays local.
- Chests of a base the game has not loaded (far away) may not count.
- `BuildingTools.ini` gets the new `[Chests]` section on the first start; your
  settings are kept.

## 0.16.0

- New: **F5 turns the whole mod off and on** (`ToggleMod` in the ini). Off is the
  vanilla game: Prefab mode, Replace, free placement and creative building are off
  and the other mod keys do nothing until you press it again.
- New: **connected replace (fill bucket)**. F9 now cycles full shape / overlap only /
  connected. With Replace on in Prefab mode, click a block: every touching block of
  the same material is repainted with your current material, in steps of the
  selected shape (0.5 m cube = block-exact), up to `FillLimit` shapes (default 500).
  Other materials are never touched; costs, refunds and undo work as for Replace.
- Fix: Replace with a terrain material as the new material removed the blocks
  instead of repainting them; they now become that terrain.
- `BuildingTools.ini` gets the new options (`ToggleMod`, `FillLimit`) on the first
  start; your settings are kept. If F5 clashes with another key, change `ToggleMod`.

## 0.15.0

- New: **creative building** (`CreativeBuilding=1` in the `[Creative]` section of
  `BuildingTools.ini`). Blocks, roofs, terrain and prefabs cost nothing, removing
  and undoing give nothing back, so the inventory is never touched. Props, chests
  and crafting stay normal.
- In creative building the material picker lists every block, roof, terrain and
  overgrowth material in its "backpack and magic chests" order, also materials
  you have none of. Favourites, last used, amount and item sets stay as they are.
  A material type you have nothing of can be built with right away.
- Works next to Creative Mode and other mods that hook the same game functions:
  the payment function and the dismantle dependency copy are no longer patched
  at their entry.
- Installing next to other mods that ship a `dinput8.dll` (XHL Native Core
  `dinput8-chain.ini`) is described in the README.

## 0.14.1

- The mouse hook is installed only while free placement is on and the game has
  focus (fixes cursor delay and hover/click issues some players saw).

## 0.14.0

- Area preview: the edges of the area being set up light up with the hammer's
  block highlight (on block surfaces; `Preview=0` turns it off).

## 0.13.4

- Works when another mod has already hooked the same game functions
  (fixes "PayCost not found").

## 0.13.3

- Replace with larger shapes in flight mode no longer reports "no live aim".

## 0.13.2

- First public release: Replace, Area mode (solid, hollow room, walls), free
  placement, game sounds, info bar, refund pop-up and undo of Replace.
