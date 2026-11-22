// Original blueprint cost/commit queue entry, shared by every composite through
// bp_fill. No placement or second cost writer lives in the composite adapter.
static bool bp_queue_native_object(CostList* list,const uint8_t* item,uint32_t id,uint16_t index){
    if(g_pend.n>=kMaxPending)return false;const uint32_t before=(uint32_t)list->count;
    CostEntry* entry=list_push(list);if(!entry)return false;const uint32_t one=1;
    memcpy(entry->b,item+4,16);memcpy(entry->b+0x10,&id,4);memcpy(entry->b+0x14,&one,4);
    g_pendCost[g_pend.n]=before;g_pendMat[g_pend.n]=0xff;g_pendObj[g_pend.n]=index;
    memset(g_pendPos[g_pend.n],0,24);++g_pend.n;return true;
}
