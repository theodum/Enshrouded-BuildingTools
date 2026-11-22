// Shared original blueprint object placement, included by bpobjects.h and host tests.
static bool bp_place_object(void* ctx, uint32_t item, const double* posM, const float* rot) {
    if (!g_placePropOrig || !g_bpItemById || !g_bpItemReg) return false;
    uint8_t* it = g_bpItemById(item, g_bpItemReg);
    if (!it || !readable(it,4) || *(uint32_t*)it!=item || !prop_item(it) || !readable(it+0x5d8,4) || bp_logic_object(ctx, it) || !ctx || !readable((uint8_t*)ctx+0xe0,8)) return false;
    float mn[3],mx[3];props::Bounds checked;
    if(!bp_item_bounds(it,mn,mx)||!props::bounds(posM,rot,mn,mx,&checked)){
        LOG(.str("native object PLACE_REFUSED code=E_NATIVE_BOUNDS_OR_TRANSFORM item=").x(item));return false;
    }
    alignas(16) uint8_t info[0x50] = {};
    memcpy(info, it + 0x3c0, 16);                        // the placed entity template
    for (int a = 0; a < 3; a++) *(float*)(info + 0x10 + a * 4) = (float)posM[a];
    memcpy(info + 0x1c, rot, 16);
    // No default cube: missing/invalid native bounds refuse placement.
    for (int a = 0; a < 3; a++) { *(float*)(info + 0x2c + a * 4) = mn[a]; *(float*)(info + 0x38 + a * 4) = mx[a]; }
    void** evSlot = (void**)((uint8_t*)ctx + 0xe0);
    void* evSaved = *evSlot;
    const bool logic = bp_logic_item(it) && !evSaved;
    if (logic) {
        g_bpEvBuf.data = g_bpEvData; g_bpEvBuf.cap = sizeof g_bpEvData; g_bpEvBuf.used = 0;
        *(uint16_t*)(g_bpEvWriter + 0xb8) = 0;
        *(BpEventBuf**)(g_bpEvWriter + 0x1b8) = &g_bpEvBuf;
        *evSlot = g_bpEvWriter;
    }
    g_bpObjRecord = 1;
    uint8_t r = g_placePropOrig(ctx, info, item);
    g_nativeItemOutcomes.record(item,r!=0);
    g_bpObjRecord = 0;
    if (logic) *evSlot = evSaved;
    if (logic) LOG(.str("  logic object ").x(item).str(r ? " placed (its logic link event dropped)" : " not placed"));
    return r != 0;
}
