// Import writes NEW files only. The selected original BTBP is copied byte for
// byte; content=objects suppresses construction of its recorded voxels.
static bool project_import_classic_blueprint(){
    ProjectStateGuard guard;if(g_projectRunning||!g_bp.ok||!g_bp.h.objects)return false;
    DWORD size=0;char* data=read_file(g_bp.path,&size);if(!data)return false;
    static core::BpHeader header;static uint8_t cells[65536];
    if(!core::bp_decode((uint8_t*)data,(int)size,&header,cells,65536)||!header.objects){HeapFree(GetProcessHeap(),0,data);return false;}
    SYSTEMTIME time;GetLocalTime(&time);Buf prefix;
    prefix.str("Classic_").u(time.wYear).u(time.wMonth,10,2).u(time.wDay,10,2).c('_').u(GetTickCount());
    Buf source,manifest;source.str(prefix.z()).str("_Source.btbp");manifest.str(prefix.z()).str(".btproject");
    wchar_t folder[MAX_PATH];if(!project_path(folder,L"")){HeapFree(GetProcessHeap(),0,data);return false;}CreateDirectoryW(folder,nullptr);
    auto publish=[&](const char* name,const void* bytes,DWORD count){wchar_t leaf[192],path[MAX_PATH],partial[MAX_PATH];
        if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,name,-1,leaf,192)||!project_path(path,leaf)||lstrlenW(path)+5>=MAX_PATH)return false;
        lstrcpyW(partial,path);lstrcatW(partial,L".tmp");HANDLE file=CreateFileW(partial,GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);if(file==INVALID_HANDLE_VALUE)return false;
        DWORD written=0;bool ok=WriteFile(file,bytes,count,&written,nullptr)&&written==count&&FlushFileBuffers(file);CloseHandle(file);
        return ok&&MoveFileExW(partial,path,MOVEFILE_WRITE_THROUGH);};
    const bool copied=publish(source.z(),data,size);HeapFree(GetProcessHeap(),0,data);if(!copied)return false;
    int anchorY=0,solid=0,layer=-1;bool flat=true;
    const int total=header.n[0]*header.n[1]*header.n[2];
    for(int i=0;i<total;++i)if(cells[i]!=core::BP_EMPTY){const int y=i/(header.n[0]*header.n[2]);if(layer<0)layer=y;else if(layer!=y)flat=false;++solid;}
    // Recognize a full captured floor, rather than guessing an object's pivot.
    if(flat&&layer>=0&&solid==header.n[0]*header.n[2])anchorY=(int)((layer+1)*header.step[1]*16);
    Buf json;json.str("{\"format\":\"BuildingToolsExtendedProject\",\"version\":1,\"name\":\"Imported classic furniture\",\"gridSize\":0.5,\"anchor\":[0,").f(anchorY/16.,4)
        .str(",0],\"modules\":[{\"file\":\"").str(source.z()).str("\",\"offset\":[0,0,0],\"rotation\":0,\"category\":\"furniture\",\"content\":\"objects\",\"objectPlacement\":\"blueprint\"}],\"pieces\":[]}\n");
    if(!publish(manifest.z(),json.z(),json.n))return false;
    wchar_t leaf[192];if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,manifest.z(),-1,leaf,192))return false;
    g_projectList=true;const bool loaded=project_load(leaf);if(loaded){g_bpPage=1;g_bpPlaced=0;g_bpWin=BPW_PLACE;}
    LOG(.str("classic composition IMPORT source_bytes=unchanged objects=").i(header.objects).str(" ignored_voxels=").i(solid).str(" loaded=").i(loaded).str(" manifest=").str(manifest.z()));return loaded;
}
