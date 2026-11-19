// Engine adapter, included after the read-only block scanner and refund builder.
// pdata is the held material ItemInfo, never the hammer shape definition.
struct BpMaterialNative {
    void* ctx;
    uint8_t* pdata;
    uint8_t* lookup(uint32_t id) { return g_bpItemById && g_bpItemReg ? g_bpItemById(id, g_bpItemReg) : nullptr; }
    bool block(uint8_t* p) { return bp_block_item(p); }
    uint32_t id(uint8_t* p) { return *(uint32_t*)p; }
    uint8_t byte(uint8_t* p) { return p[0x454]; }
    uint8_t* from_byte(uint8_t hint) { return bp_block_of_material(pdata, hint); }
    bool refund(uint8_t hint, uint32_t* id) { CostEntry entry{}; if (!bp_entry(ctx, hint, &entry)) return false; *id = *(uint32_t*)(entry.b + 0x10); return true; }
};
static materials::Resolution bp_resolve_material(void* ctx, uint8_t* pdata, const core::BpMat& m) {
    BpMaterialNative native{ctx, pdata}; return materials::resolve(native, m.item, m.mat);
}
