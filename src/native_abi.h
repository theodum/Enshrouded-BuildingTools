// Read-only reflection guards for the checked game build. A moved/changed
// descriptor disables only Extended native placement, never guesses a layout.
#pragma once
#include "project.h"
namespace extended {
inline bool entity_map_abi(const uint8_t* code){
    if(!code)return false;
    const uint8_t prefix[]={0x48,0x89,0x5c,0x24,0x08,0x48,0x89,0x7c,0x24,0x10,0x83,0x79,0x54,0,0x4c,0x8b,0xc9,0x4c,0x8b,0x59,0x10};
    const uint8_t bits[]={0x49,0x8b,0x79,0x08},keys[]={0x4d,0x8b,0x51,0x28};
    for(unsigned i=0;i<sizeof prefix;++i)if(code[i]!=prefix[i])return false;
    for(unsigned i=0;i<sizeof bits;++i)if(code[0x1b+i]!=bits[i]||code[0x5e + i]!=keys[i])return false;return true;
}
inline bool entity_table_abi(const uint8_t* code){
    if(!code)return false;const uint8_t prefix[]={0x40,0x53,0x48,0x83,0xec,0x20,0x48,0x8b,0x01,0x89,0x54,0x24,0x38,0x48,0x8d,0x54,0x24,0x38,0x48,0x8b,0x58,0x30};
    const uint8_t table[]={0x48,0x8d,0x8b,0x48,0x01,0,0},values[]={0x48,0x8b,0x8b,0x88,0x01,0,0};
    for(unsigned i=0;i<sizeof prefix;++i)if(code[i]!=prefix[i])return false;
    for(unsigned i=0;i<sizeof table;++i)if(code[0x16+i]!=table[i]||code[0x2b+i]!=values[i])return false;return code[0x1d]==0xe8;
}

template<class Read> bool transform_abi(uint64_t base,Read read){
    auto type=[&](uint32_t rva,const char* name,uint32_t size=56)->const uint8_t*{const uint8_t* t=read(base+rva,0x60);if(!t||*(const uint32_t*)(t+0x40)!=size)return nullptr;
        uint64_t length=*(const uint64_t*)(t+0x28);if(length>64)return nullptr;
        const char* text=(const char*)read(*(const uint64_t*)(t+0x20),length+1);
        return text&&project::equal(text,name)?t:nullptr;};
    const auto* current=type(0x194a4c0,"keen::ecs::CurrentTransform");const auto* world=type(0x196c7e0,"keen::WorldTransform");if(!current||!world)return false;
    auto member=[&](const uint8_t* t,const char* name,uint64_t off){uint32_t n=*(const uint32_t*)(t+0x48);if(n>64)return false;
        const uint8_t* entries=read(*(const uint64_t*)(t+0x58),(uint64_t)n*0x30);if(!entries)return false;
        for(uint32_t i=0;i<n;++i){const auto* e=entries+i*0x30;uint64_t length=*(const uint64_t*)(e+8);if(length>64)return false;
            const char* text=(const char*)read(*(const uint64_t*)e,length+1);
            if(text&&project::equal(text,name))return *(const uint64_t*)(e+24)==off;}return false;};
    const auto* used=type(0x1a3f1b0,"keen::ecs::UsedItem",4);
    return used&&member(used,"itemId",0)&&member(current,"transform",0)&&member(world,"position",0)&&member(world,"orientation",24)&&member(world,"scale",40);
}
template<class Read> bool item_abi(uint64_t base, Read read) {
    auto name = [&](uint64_t address,uint64_t length,const char* expected) {
        uint64_t n=0;while(expected[n])++n; if(length!=n)return false;
        const uint8_t* p=read(address,n+1);return p && project::equal((const char*)p,expected);
    };
    auto type = [&](uint32_t rva,const char* expected,uint32_t size)->const uint8_t* {
        const uint8_t* p=read(base+rva,0x68);
        return p && *(const uint32_t*)(p+0x40)==size && name(*(const uint64_t*)(p+0x20),*(const uint64_t*)(p+0x28),expected) ? p : nullptr;
    };
    auto member = [&](const uint8_t* t,const char* expected,uint64_t offset) {
        if(!t)return false;uint32_t count=*(const uint32_t*)(t+0x48);if(count>400)return false;
        const uint8_t* p=read(*(const uint64_t*)(t+0x58),(uint64_t)count*0x30);if(!p)return false;
        for(uint32_t i=0;i<count;++i,p+=0x30) if(name(*(const uint64_t*)p,*(const uint64_t*)(p+8),expected)) return *(const uint64_t*)(p+24)==offset;
        return false;
    };
    const uint8_t* item=type(0x1918790,"keen::ItemInfo",0x828);
    const uint8_t* equipment=type(0x19cfc30,"keen::EquipmentSetup",0x440);
    if(!member(item,"equipment",0x104) || !member(item,"flags",0x5d8) || !member(equipment,"placedEntity",0x2bc) || !member(equipment,"placementAABBmin",0x2cc) || !member(equipment,"placementAABBmax",0x2d8) || !member(equipment,"requiresAltarSlot",0x323) || !member(equipment,"voxelData",0x33c))return false;
    const uint8_t* flags=type(0x1989f80,"keen::ItemFlag",1);
    if(!flags || (*(const uint32_t*)(flags+0x4c)&255)!=12 || *(const uint32_t*)(flags+0x48)!=22)return false;
    const char* names[]={"HasKnowledgeUnlocks","HasPIDE","IsContainer","IsBlueprint","IsBuildingOvergrowthTool","IsBuildingDecayTool","IsBuildingScrewdriverTool","IsTerraformer","IsNpc","IsCookable","IsAnimal","IsAnimalBait","CanTriggerCooking","UsesFuel","TwitchDrop","CommerceItem","IsNonTintable","IsSalvageable","IsVanityItem","IsLogicEntity","IsFactoryEntity","HideDamageAttributeInItemViewer"};
    const uint8_t* entries=read(*(const uint64_t*)(flags+0x60),22*0x28);if(!entries)return false;
    for(int i=0;i<22;++i) { const uint8_t* p=entries+i*0x28;if(*(const uint64_t*)(p+16)!=(uint64_t)i || !name(*(const uint64_t*)p,*(const uint64_t*)(p+8),names[i]))return false; }
    return true;
}
}
