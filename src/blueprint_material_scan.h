// The building block item of a material byte (issue #61). Blueprints store the item the game's
// refund builder gives for each material; for some block materials that is not the block but a
// plain item (the #61 file: material 0xbf, item 0x57f0f395, kind 0, material byte 0). The build
// then needs a block item of that material: a read-only scan of the item data for a small
// building shape whose material byte is `mat`, and the item registry must return that same
// definition for its id (so only a real item counts, never stray data). Cached per material.
struct BpMatItem { uint8_t mat; uint8_t* p; };
static BpMatItem g_bpMatItem[64]; static int g_bpMatItemN = 0;
static uint8_t* bp_block_of_material(uint8_t* anchor, uint8_t mat) {
    if (mat < 0x80 || !anchor || !g_bpItemById || !g_bpItemReg) return nullptr;
    for (int q = 0; q < g_bpMatItemN; q++) if (g_bpMatItem[q].mat == mat) {
        uint8_t* p = g_bpMatItem[q].p;
        if (bp_block_item(p) && p[0x454] == mat && g_bpItemById(*(uint32_t*)p, g_bpItemReg) == p) return p;
        g_bpMatItem[q--] = g_bpMatItem[--g_bpMatItemN];
    }
    MEMORY_BASIC_INFORMATION mi;
    if (!VirtualQuery(anchor, &mi, sizeof mi)) return nullptr;
    uint8_t* base = (uint8_t*)mi.AllocationBase;
    uint8_t* found = nullptr; uint64_t scanned = 0; int tried = 0;
    const DWORD t0 = GetTickCount();
    for (uint8_t* r = base; !found && VirtualQuery(r, &mi, sizeof mi) && mi.AllocationBase == base; r = (uint8_t*)mi.BaseAddress + mi.RegionSize) {
        const DWORD okProt = PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE;
        if (mi.State != MEM_COMMIT || !(mi.Protect & okProt) || (mi.Protect & (PAGE_GUARD | PAGE_NOACCESS))) continue;
        uint8_t* lo = (uint8_t*)mi.BaseAddress; uint8_t* hi = lo + mi.RegionSize;
        if (hi - lo < 0x4b8) continue;
        scanned += mi.RegionSize;
        for (uint8_t* c = lo + (((uintptr_t)anchor - (uintptr_t)lo) & 7); c + 0x4b8 <= hi; c += 8) {
            if (c[0x454] != mat || !c[0x440] || !*(const uint32_t*)c) continue;      // as item_kind: small building shape
            if (!(*(const uint64_t*)(c + 0x458) | *(const uint64_t*)(c + 0x460))) continue;
            if (*(const uint64_t*)(c + 0x468) | *(const uint64_t*)(c + 0x470)) continue;
            if (++tried > 4096) break;
            if (g_bpItemById(*(const uint32_t*)c, g_bpItemReg) == c) { found = c; break; }
        }
        if (scanned > (512ull << 20) || tried > 4096) break;      // never more than 512 MB
    }
    LOG(.str("  blueprint: block item of material ").x(mat).str(found ? ": " : ": none").x(found ? *(uint32_t*)found : 0)
        .str(" (scanned ").u(scanned >> 20).str(" MB, ").i(tried).str(" candidates, ").u(GetTickCount() - t0).str(" ms)"));
    // A scan in an unrelated allocation must not poison later valid attempts.
    if (found && g_bpMatItemN < 64) g_bpMatItem[g_bpMatItemN++] = BpMatItem{ mat, found };
    return found;
}
