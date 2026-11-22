// Export original source references, never flatten away captured support masks.
// Only an unchanged imported composition is eligible; no individual edit is lost.
static bool project_export_classic_refs(char* result,int capacity){
    if(g_projectPlan.count||g_projectManifest.pieceCount||g_projectManifest.spaceCount||composition::fingerprint(g_projectObjects,g_projectObjectCount)!=g_projectClassicObjects)return false;
    for(int i=0;i<g_projectManifest.count;++i){wchar_t leaf[192],path[MAX_PATH];
        if(!g_projectManifest.modules[i].classicObjects||!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,g_projectManifest.modules[i].file,-1,leaf,192)||!project_path(path,leaf))return false;
        DWORD size=0;char* data=read_file(path,&size);if(!data)return false;
        const auto hash=composition::bytes(14695981039346656037ull,data,size);HeapFree(GetProcessHeap(),0,data);if(hash!=g_projectClassicFiles[i])return false;
    }
    SYSTEMTIME time;GetLocalTime(&time);Buf name;
    name.str("Classic_Furniture_").u(time.wYear).u(time.wMonth,10,2).u(time.wDay,10,2).c('_').u(GetTickCount()).str(".btproject");
    wchar_t leaf[192],path[MAX_PATH],partial[MAX_PATH];if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,name.z(),-1,leaf,192)||!project_path(path,leaf)||lstrlenW(path)+5>=MAX_PATH)return false;
    lstrcpyW(partial,path);lstrcatW(partial,L".tmp");HANDLE file=CreateFileW(partial,GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);if(file==INVALID_HANDLE_VALUE)return false;
    bool ok=true;auto write=[&](Buf& b){DWORD written=0;if(b.n>=int(sizeof b.s)-1||!WriteFile(file,b.z(),b.n,&written,nullptr)||written!=(DWORD)b.n)ok=false;};
    auto quote=[](Buf& b,const char* text){b.c('"');for(;*text;++text){if(*text=='"'||*text=='\\')b.c('\\');if((uint8_t)*text>=32)b.c(*text);}b.c('"');};
    Buf first;first.str("{\"format\":\"BuildingToolsExtendedProject\",\"version\":1,\"name\":");quote(first,g_projectManifest.name);
    first.str(",\"gridSize\":0.5,\"anchor\":[");for(int a=0;a<3;++a){if(a)first.c(',');first.f(g_projectManifest.anchor[a]/16.,4);}first.str("],\"modules\":[\n");write(first);
    for(int i=0;i<g_projectManifest.count;++i){const auto& m=g_projectManifest.modules[i];Buf row;if(i)row.c(',');row.str("{\"file\":");quote(row,m.file);row.str(",\"offset\":[");
        for(int a=0;a<3;++a){if(a)row.c(',');row.f(m.offset[a]/16.,4);}row.str("],\"rotation\":").i(m.rotation*90).str(",\"category\":\"furniture\",\"content\":\"objects\",\"objectPlacement\":\"blueprint\"}\n");write(row);}
    Buf end;end.str("],\"pieces\":[]}\n");write(end);if(!FlushFileBuffers(file))ok=false;CloseHandle(file);
    if(!ok||!MoveFileExW(partial,path,MOVEFILE_WRITE_THROUGH))return false;copy_text(result,name.z(),capacity);
    LOG(.str("classic composition EXPORTED method=original_source_references objects=").i(g_projectObjectCount).str(" build_voxels=0 source_bytes=unchanged"));return true;
}
