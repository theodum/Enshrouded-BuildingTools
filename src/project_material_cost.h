static bool project_cost(uint32_t item, uint32_t count, const ProjectMaterialContext& c, bool accumulating, bool prop = false) {
    uint8_t* native = item && g_bpItemById && g_bpItemReg ? g_bpItemById(item, g_bpItemReg) : nullptr;
    if (!native || !readable(native, 4) || *(uint32_t*)native != item) {
        materials::CostResult r; r.issue = materials::CostIssue::UnknownItem; r.item = item; r.requested = r.attempted = count;
        return project_cost_result(r, c);
    }
    return project_cost_result(materials::add(accumulating ? g_projectNeed : g_projectBuildNeed,
        accumulating ? g_projectNeedN : g_projectBuildNeedN, item, count, prop), c);
}
static bool project_native_cost(const CostList& price, CostEntry* entries, const ProjectMaterialContext& c, bool accumulating) {
    if (!materials::valid_list(price.data, price.count, price.cap, entries, 256)) {
        LOG(.str("project cost LIST_INVALID count=").u(price.count).str(" cap=").u(price.cap).str(" data_matches=").i(price.data == entries));
        materials::CostResult r; r.issue = materials::CostIssue::InvalidList; r.attempted = price.cap > 256 ? price.cap : price.count; r.limit = 256;
        return project_cost_result(r, c);
    }
    for (uint64_t q = 0; q < price.count; ++q) {
        const uint32_t item = *(uint32_t*)(entries[q].b + 0x10), count = *(uint32_t*)(entries[q].b + 0x14);
        if (accumulating && (c.cell == 0 || c.piece == 0))
            LOG(.str("project cost NATIVE entry=").u(q).str(" item=").x(item).str(" count=").u(count).str(" creative=").i(creative_on()));
        if (!project_cost(item, count, c, accumulating)) return false;
    }
    return true;
}
