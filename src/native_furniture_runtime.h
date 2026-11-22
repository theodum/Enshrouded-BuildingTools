// Live registry -> demo draft/export. No object/item ID is embedded here.
static uint32_t g_furnitureSelected[props::RoleCount]{};
static char g_furnitureError[512]{};
static const char* g_furnitureRoles[]={"bed","table","chair","wall_torch","painting","chest","small_decoration","floor_decoration"};
// Exact attributed CSV selections. Never substitute a missing/refused ID.
static void furniture_csv_roles(){
    const uint32_t ids[]={0x566c8dfe,0xd109e828,0xdd5f8bb9,0x66f057f6,0x500ddd97,0x02f09172,0xaf3025e4,0xd76bd513};
    for(int i=0;i<props::RoleCount;++i)g_furnitureSelected[i]=ids[i];
    g_furnitureError[0]=0;flash(T(S_EXT_INDEX_DEMO_READY));
}
static bool furniture_token(const char* text,const char* token){
    if(!text)return false;
    for(int i=0;text[i];++i){if(i && ((text[i-1]>='a'&&text[i-1]<='z')||(text[i-1]>='A'&&text[i-1]<='Z')))continue;
        int k=0;while(token[k]&&text[i+k]){char c=text[i+k];if(c>='A'&&c<='Z')c+=32;if(c!=token[k])break;++k;}
        const char end=text[i+k];if(!token[k]&&!(end>='a'&&end<='z')&&!(end>='A'&&end<='Z'))return true;
    }return false;
}
static void furniture_suggest(){
    const char* tokens[props::RoleCount][5]={{"lit","bed",nullptr},{"table",nullptr},{"chaise","chair",nullptr},
        {"flambeau","torche","torch","applique",nullptr},{"tableau","painting",nullptr},{"coffre","chest",nullptr},
        {"bougie","candle","gobelet","cup",nullptr},{"vase","pot",nullptr}};
    for(int role=0;role<props::RoleCount;++role){if(g_furnitureSelected[role])continue;
        double score=1e30;uint32_t choice=0;
        for(int i=0;i<g_catalogCount;++i){if(g_catalog[i].kind!=1)continue;uint32_t id=g_catalog[i].item;
            const char* name=bp_item_name(id);bool match=false;for(int k=0;k<5&&tokens[role][k];++k)if(furniture_token(name,tokens[role][k]))match=true;
            if(!match)continue;float lo[3],hi[3];uint8_t* it=g_bpItemById(id,g_bpItemReg);if(!bp_item_bounds(it,lo,hi))continue;
            double size[3];bool valid=true;for(int a=0;a<3;++a){size[a]=hi[a]-lo[a];if(!extended::finite(size[a])||size[a]<=0||size[a]>4)valid=false;}
            if(!valid)continue;const double value=size[0]*size[1]*size[2];
            if(value<score){score=value;choice=id;}
        }g_furnitureSelected[role]=choice;
    }
}
static bool furniture_prepare(){
    ProjectStateGuard guard;g_furnitureError[0]=0;
    auto fail=[&](int role,Str why){Buf b,id;id.x(role>=0?g_furnitureSelected[role]:0);
        b.str(T(why)).c(' ').str(role>=0?g_furnitureRoles[role]:"structure").c(' ').str(id.z());
        copy_text(g_furnitureError,b.z(),sizeof g_furnitureError);flash(b.z());return false;};
    if(g_projectRunning)return fail(-1,S_EXT_ACTION_BUSY);
    props::Definition definitions[props::RoleCount];
    for(int i=0;i<props::RoleCount;++i){auto& d=definitions[i];d.item=g_furnitureSelected[i];
        uint8_t* it=g_bpItemById&&g_bpItemReg?g_bpItemById(d.item,g_bpItemReg):nullptr;
        const auto resolution=native_item_resolution(d.item);
        if(resolution!=nativecatalog::Ready){Buf reason,id;id.x(d.item);reason.fmt(T(S_EXT_ROLE_REFUSED),g_furnitureRoles[i],id.z(),nativecatalog::name(resolution));
            copy_text(g_furnitureError,reason.z(),sizeof g_furnitureError);flash(reason.z());LOG(.str("native furniture REFUSED stage=resolve role=").str(g_furnitureRoles[i]).str(" item=").x(d.item).str(" code=").str(nativecatalog::name(resolution)));return false;}
        if(!bp_item_bounds(it,d.low,d.high))return fail(i,S_EXT_PROP_INVALID);
    }
    props::Demo demo;if(!props::furniture_demo(definitions,&demo))return fail(demo.badRole,S_EXT_PROP_DEMO_FIT);
    uint8_t* material=g_bpItemById?g_bpItemById(g_catalogMaterial,g_bpItemReg):nullptr;
    if(!bp_block_item(material))return fail(-1,S_EXT_VOXEL_MATERIAL);
    ArchitectureEdit edit;if(!edit.before)return fail(-1,S_EXT_DEMO_BUDGET);
    auto oldObjects=g_projectObjects;
    g_projectObjects=(extended::Object*)VirtualAlloc(nullptr,sizeof(extended::Object)*kProjectMaxObjects,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(!g_projectObjects)return fail(-1,S_EXT_DEMO_BUDGET);
    if(!project_generated_begin("Dev8 - piece temoin meublee (objets natifs)"))return fail(-1,S_EXT_DEMO_BUDGET);
    g_projectManifest.count=2;g_projectManifest.modules[0].category=extended::General;g_projectManifest.modules[1].category=extended::Furniture;
    g_projectPlan.materialCount=1;g_projectPlan.materials[0]={g_catalogMaterial,material[0x454],0,0};
    if(!props::demo_room([&](int x,int y,int z){if(g_projectPlan.count==g_projectPlan.capacity)return false;
        g_projectPlan.cells[g_projectPlan.count++]={{x,y,z},0,0,extended::Wall};return true;}))return fail(-1,S_EXT_DEMO_BUDGET);
    project::Error error;if(!project::finish(&g_projectPlan,&error))return fail(-1,S_EXT_DEMO_BUDGET);
    g_projectObjectCount=demo.count;memcpy(g_projectObjects,demo.objects,demo.count*sizeof(extended::Object));g_projectPlan.objects=demo.count;
    g_projectManifest.spaceCount=1;g_projectManifest.spaces[0]={{80,0,8},{112,48,136}};
    g_projectUseObjects=1;
    for(int m=0;m<2;++m){g_projectModuleVisible[m]=true;g_projectModuleBoxes[m]={{0,-8,0},{192,80,160}};}
    if(!project_validate_spaces())return fail(-1,S_EXT_PROP_DEMO_FIT);
    project_generated_finish();if(oldObjects)VirtualFree(oldObjects,0,MEM_RELEASE);edit.committed=true;
    flash(T(S_EXT_PROP_DEMO_READY));LOG(.str("native furniture PREPARED objects=").i(demo.count).str(" native registry IDs only; world unchanged"));return true;
}
static const char* native_catalog_image(){static Buf value;value.n=0;
    const auto* dos=(const IMAGE_DOS_HEADER*)g_base;const auto* pe=(const IMAGE_NT_HEADERS64*)(g_base+dos->e_lfanew);
    value.str("{\"timeDateStamp\":").u(pe->FileHeader.TimeDateStamp).str(",\"sizeOfImage\":").u(pe->OptionalHeader.SizeOfImage).c('}');return value.z();}
#include "native_catalog_export.h"
