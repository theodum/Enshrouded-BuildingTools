// Export observed native definitions into a new JSON file, atomic publication.
static bool native_catalog_export(char* result,int capacity){
    ProjectStateGuard guard;
    if(!g_bpItemReg||!g_bpItemById||!g_nativeObjectAbi||!g_catalogCount||g_catalogTruncated)return false;
    SYSTEMTIME time;GetLocalTime(&time);Buf name;
    name.str("NativeCatalog_").u(time.wYear).u(time.wMonth,10,2).u(time.wDay,10,2).c('_').u(GetTickCount()).str(".json");
    wchar_t leaf[192],path[MAX_PATH],partial[MAX_PATH],folder[MAX_PATH];
    if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,name.z(),-1,leaf,192)||!project_path(folder,L"")||!project_path(path,leaf))return false;
    if(lstrlenW(path)+9>=MAX_PATH)return false;lstrcpyW(partial,path);lstrcatW(partial,L".partial");
    CreateDirectoryW(folder,nullptr);HANDLE f=CreateFileW(partial,GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);if(f==INVALID_HANDLE_VALUE)return false;
    bool ok=true;auto write=[&](Buf& b){DWORD n=0;if(b.n>=int(sizeof b.s)-1||!WriteFile(f,b.z(),b.n,&n,nullptr)||n!=(DWORD)b.n)ok=false;};
    auto quoted=[](Buf& b,const char* s){b.c('"');if(s)for(;*s;++s){if(*s=='"'||*s=='\\')b.c('\\');if((uint8_t)*s>=32)b.c(*s);}b.c('"');};
    Buf first;first.str("{\"format\":\"BuildingToolsNativeCatalog\",\"version\":1,\"source\":\"live_registry\",\"mod\":\"").str(MOD_VERSION)
        .str("\",\"logicCopyOptIn\":").str(g_projectBlueprintLogic?"true":"false").str(",\"complete\":true,\"indexSHA256\":\"").str(itemindex::SHA256)
        .str("\",\"gameImage\":").str(native_catalog_image()).str(",\"callHistoryTruncated\":").str(g_nativeItemOutcomes.truncated?"true":"false").str(",\"items\":[\n");write(first);
    int entries=0;
    for(int i=0;i<g_catalogCount;++i){uint32_t id=g_catalog[i].item;uint8_t* it=g_bpItemById(id,g_bpItemReg);
        if(it&&(!readable(it,4)||*(uint32_t*)it!=id))it=nullptr;
        auto status=native_item_resolution(id);const bool isProp=it&&prop_item(it);
        float lo[3],hi[3];bool has=status==nativecatalog::Ready&&bp_item_bounds(it,lo,hi);props::Bounds b;const double zero[]={0,0,0};const float identity[]={0,0,0,1};has=has&&props::bounds(zero,identity,lo,hi,&b);
        const auto* source=itemindex::find(id);const char* text=it?bp_item_name(id):nullptr;if(!text&&source)text=source->name;
        uint32_t flags=it&&readable(it+0x5d8,4)?*(uint32_t*)(it+0x5d8):0xffffffffu;
        Buf row;if(entries++)row.str(",\n");row.str("{\"item\":").u(id).str(",\"name\":");if(text)quoted(row,text);else row.str("null");
        int nameLength=0;if(text)while(text[nameLength])++nameLength;
        // The name cache keeps whole UTF-8 codepoints in 63 bytes; a truncated
        // four-byte codepoint can leave a 60-byte prefix.
        row.str(",\"nameMayBeTruncated\":").str(nameLength>=60?"true":"false");
        row.str(",\"category\":\"").str(source?itemindex::Categories[source->category]:"other").str("\",\"resolution\":\"").str(nativecatalog::name(status)).c('"');
        row.str(",\"flags\":").u(flags).str(",\"compatible\":").str(status==nativecatalog::Ready?"true":"false").str(",\"logicLinkCopied\":false,\"bounds\":");
        if(has){row.str("{\"low\":[");for(int a=0;a<3;++a){if(a)row.c(',');row.f(lo[a],6);}row.str("],\"high\":[");for(int a=0;a<3;++a){if(a)row.c(',');row.f(hi[a],6);}row.str("]}");}else row.str("null");
        if(status==nativecatalog::InvalidBounds&&it){float rawLow[3],rawHigh[3];row.str(",\"boundsDiagnostic\":\"");
            if(bp_item_bounds_raw(it,rawLow,rawHigh)){row.str("native_placement low=(");for(int a=0;a<3;++a){if(a)row.c(',');row.f(rawLow[a],6);}row.str(") high=(");for(int a=0;a<3;++a){if(a)row.c(',');row.f(rawHigh[a],6);}row.c(')');}
            else row.str("native_bounds_function_unavailable");row.c('"');}
        row.str(",\"placedEntity\":");if(isProp){row.c('"');for(int k=0;k<16;++k)row.u(it[0x3c0+k],16,2);row.c('"');}else row.str("null");
        const auto* calls=g_nativeItemOutcomes.find(id);row.str(",\"placementCalls\":{\"accepted\":").u(calls?calls->accepted:0).str(",\"failed\":").u(calls?calls->failed:0).c('}');
        row.str(",\"interactionValidated\":false,\"requiredSupport\":\"not_exposed\",\"constraints\":\"contact, nonoverlap, altar coverage; native interaction untested\"}");write(row);
    }
    Buf last;last.str("\n],\"structureMaterial\":");uint8_t* mat=g_bpItemById(g_catalogMaterial,g_bpItemReg);
    if(bp_block_item(mat))last.str("{\"item\":").u(g_catalogMaterial).str(",\"byte\":").u(mat[0x454]).c('}');else last.str("null");
    last.str(",\"demoSelection\":{");for(int i=0;i<props::RoleCount;++i){if(i)last.c(',');quoted(last,g_furnitureRoles[i]);last.c(':');if(g_furnitureSelected[i])last.u(g_furnitureSelected[i]);else last.str("null");}last.str("}}\n");write(last);
    if(!FlushFileBuffers(f))ok=false;CloseHandle(f);if(ok)ok=MoveFileExW(partial,path,MOVEFILE_WRITE_THROUGH)!=0;
    if(ok)copy_text(result,name.z(),capacity);return ok;
}
