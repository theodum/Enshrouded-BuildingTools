// Historical blueprint component access and item discovery, shared unchanged.
#pragma once
// inline component data of entity record `e` for component index k (null if absent)
static uint8_t* bp_comp(uint8_t* e, uint32_t k, uint32_t* size) {
    if (!e || !readable(e, 0x38) || k >= 640) return nullptr;
    uint8_t* arch = *(uint8_t**)(e + 0x18);
    if (!arch || !readable(arch, 0xa84 + 640 * 2)) return nullptr;
    if (!((*(uint64_t*)(arch + (k >> 6) * 8) >> (k & 63)) & 1)) return nullptr;
    uint32_t stride = *(uint16_t*)(arch + 0xa84 + k * 2), off = *(uint16_t*)(arch + 0x84 + k * 2);
    uint8_t* data = *(uint8_t**)(e + 0x20);
    uint32_t row = *(uint32_t*)(e + 0x30);
    if (size) *size = stride;
    if (!data) return nullptr;
    uint8_t* p = data + off + (size_t)row * stride;
    return readable(p, stride ? stride : 8) ? p : nullptr;
}
// The same without a memory check per call, for the scan of the whole entity table: checking
// every record with VirtualQuery took 36 s for 27671 entities (0.32.0-dev13). The records come
// from the game's own table; each archetype (a few hundred) is checked once.
static uint8_t* g_bpArchOk[512]; static int g_bpArchN = 0;
static bool bp_ptr(const void* p) { uintptr_t v = (uintptr_t)p; return v > 0x10000 && v < 0x7fffffff0000ull && !(v & 7); }
static uint8_t* bp_comp_fast(uint8_t* e, uint32_t k, uint32_t* size) {
    if (!bp_ptr(e) || k >= 640) return nullptr;
    uint8_t* arch = *(uint8_t**)(e + 0x18);
    if (!bp_ptr(arch)) return nullptr;
    bool known = false;
    for (int q = 0; q < g_bpArchN && !known; q++) known = g_bpArchOk[q] == arch;
    if (!known) {
        if (!readable(arch, 0xa84 + 640 * 2)) return nullptr;
        if (g_bpArchN < 512) g_bpArchOk[g_bpArchN++] = arch;
    }
    if (!((*(uint64_t*)(arch + (k >> 6) * 8) >> (k & 63)) & 1)) return nullptr;
    uint32_t stride = *(uint16_t*)(arch + 0xa84 + k * 2), off = *(uint16_t*)(arch + 0x84 + k * 2);
    uint8_t* data = *(uint8_t**)(e + 0x20);
    if (!bp_ptr(data)) return nullptr;
    if (size) *size = stride;
    return data + off + (size_t)*(uint32_t*)(e + 0x30) * stride;
}
// The item a placed prop came from: the 4-byte component holding an item id the lookup knows.
static uint32_t bp_entity_item(uint8_t* e) {
    if (!g_bpItemById || !g_bpItemReg) return 0;
    uint32_t sz = 0;
    auto test = [&](uint32_t k) -> uint32_t {
        uint8_t* c = bp_comp(e, k, &sz);
        if (!c || sz != 4) return 0;
        uint32_t v = *(uint32_t*)c;
        if (v < 0x10000) return 0;
        uint8_t* it = g_bpItemById(v, g_bpItemReg);
        return it && prop_item(it) ? v : 0;
    };
    if (g_bpItemComp < 640) { if (uint32_t v = test(g_bpItemComp)) return v; }
    for (uint32_t k = 0; k < 640; k++)
        if (uint32_t v = test(k)) {
            if (g_bpItemComp != k) LOG(.str("blueprint objects: the item is in component ").u(k));
            g_bpItemComp = k; return v;
        }
    return 0;
}
