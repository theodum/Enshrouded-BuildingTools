// Shared classic/composite material policy. No native memory writes or world access.
#pragma once
#include <stdint.h>
namespace materials {
enum class Issue { None, Unknown, RefundMismatch };
enum class Route { None, Registry, LegacyRefund, LegacyNoRefund };
inline const char* name(Issue i) { return i == Issue::RefundMismatch ? "refund_id_mismatch" : i == Issue::Unknown ? "unknown_material" : "none"; }
inline const char* name(Route r) { return r == Route::Registry ? "registry" : r == Route::LegacyRefund ? "legacy_refund" : r == Route::LegacyNoRefund ? "legacy_no_refund" : "none"; }
struct Resolution {
    uint8_t* item = nullptr;
    uint32_t saved = 0, candidate = 0, refund = 0;
    uint8_t savedByte = 0, nativeByte = 0;
    bool refundKnown = false, lookupFound = false;
    Issue issue = Issue::Unknown;
    Route route = Route::None;
};
template<class Native> Resolution resolve(Native& n, uint32_t saved, uint8_t hint) {
    Resolution r; r.saved = saved; r.savedByte = hint;
    auto accept = [&](uint8_t* p, Route route) { r.item = p; r.nativeByte = n.byte(p); r.candidate = n.id(p); r.route = route; r.issue = Issue::None; };
    // An existing block ID is authoritative even when a game's material bytes moved.
    uint8_t* direct = saved ? n.lookup(saved) : nullptr;
    r.lookupFound = direct != nullptr;
    if (n.block(direct) && n.id(direct) == saved) { accept(direct, Route::Registry); return r; }
    if (hint < 0x80) return r;
    uint8_t* candidate = n.from_byte(hint);
    if (!n.block(candidate) || n.byte(candidate) != hint || n.lookup(n.id(candidate)) != candidate) return r;
    r.candidate = n.id(candidate); r.nativeByte = hint;
    r.refundKnown = n.refund(hint, &r.refund);
    // Retain the upstream #61 guard. A shared byte alone is never proof of identity.
    if (saved && r.refundKnown && r.refund != saved) { r.issue = Issue::RefundMismatch; return r; }
    accept(candidate, r.refundKnown ? Route::LegacyRefund : Route::LegacyNoRefund);
    return r;
}

constexpr uint64_t MaxCost = 0x7fffffff;
constexpr int MaxNeeds = 128;
struct Need { uint32_t item; uint64_t count; bool prop; };
enum class CostIssue { None, UnknownItem, Excessive, TooManyItems, InvalidList };
struct CostResult {
    CostIssue issue = CostIssue::None;
    uint32_t item = 0;
    uint64_t previous = 0, requested = 0, attempted = 0, limit = MaxCost;
};
inline const char* name(CostIssue i) {
    switch (i) { case CostIssue::UnknownItem: return "unknown_cost_item"; case CostIssue::Excessive: return "excessive_cost";
    case CostIssue::TooManyItems: return "too_many_cost_items"; case CostIssue::InvalidList: return "invalid_native_cost_list"; default: return "none"; }
}
inline bool valid_list(const void* data, uint64_t count, uint64_t cap, const void* expected, uint64_t capacity) {
    return data == expected && count <= cap && cap <= capacity;
}
// Reject before changing the ledger, including when the rejected entry is new.
inline CostResult add(Need* needs, int& size, uint32_t item, uint32_t count, bool prop = false) {
    CostResult r; r.item = item; r.requested = r.attempted = count;
    if (!item) { r.issue = CostIssue::UnknownItem; return r; }
    int at = 0; while (at < size && needs[at].item != item) ++at;
    if (at < size) { r.previous = needs[at].count; r.attempted += r.previous; }
    if (r.attempted > MaxCost) { r.issue = CostIssue::Excessive; return r; }
    if (!count) return r; // Native creative pricing may legitimately return zero.
    if (at == size) {
        if (size >= MaxNeeds) { r.issue = CostIssue::TooManyItems; r.limit = MaxNeeds; r.attempted = size + 1; return r; }
        needs[size++] = Need{item, count, prop};
    } else { needs[at].count = r.attempted; needs[at].prop = needs[at].prop && prop; }
    return r;
}
} // namespace materials
