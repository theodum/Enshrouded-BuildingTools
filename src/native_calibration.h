#pragma once
#include "native_props.h"
namespace calibration {
struct Definition {
    uint32_t item=0,entity=0,buildTime=0,imageSize=0;uint8_t templateId[16]{};
    char name[96]{};double position[3]{},local[3]{};float rotation[4]{0,0,0,1},scale[3]{1,1,1},low[3]{},high[3]{};
};
inline bool valid(const Definition& d){
    uint8_t nonzero=0;for(auto b:d.templateId)nonzero|=b;if(!d.item||!d.entity||!nonzero||!d.imageSize)return false;
    for(int a=0;a<3;++a)if(!(d.scale[a]>=.99999f&&d.scale[a]<=1.00001f)||!extended::finite(d.local[a]))return false;
    props::Bounds b;return props::bounds(d.position,d.rotation,d.low,d.high,&b);
}
inline bool number(project::Reader& r,double* out){
    r.space();bool negative=r.take('-');if(r.at==r.size||r.s[r.at]<'0'||r.s[r.at]>'9')return r.fail("Expected finite decimal");
    double v=0,den=1;int digits=0;bool zero=r.s[r.at]=='0';
    while(r.at<r.size&&r.s[r.at]>='0'&&r.s[r.at]<='9'){if(++digits>6||(zero&&digits>1))return r.fail("Decimal outside calibration range");v=v*10+r.s[r.at++]-'0';}
    if(r.at<r.size&&r.s[r.at]=='.'){++r.at;int n=0;while(r.at<r.size&&r.s[r.at]>='0'&&r.s[r.at]<='9'){if(++n>9)return r.fail("Too many fractional digits");v=v*10+r.s[r.at++]-'0';den*=10;}if(!n)return r.fail("Missing fraction");}
    *out=(negative?-v:v)/den;return extended::finite(*out)||r.fail("Nonfinite calibration value");
}
template<class T> bool vector(project::Reader& r,T* out,int n){if(!r.want('['))return false;for(int i=0;i<n;++i){double v;if(i&&!r.want(','))return false;if(!number(r,&v))return false;out[i]=(T)v;}return r.want(']');}
inline bool decode(const char* text,int size,Definition* out,project::Error* error){
    *error={};if(size<1||size>8192)return false;project::Reader r{text,size,0,error};Definition d;unsigned fields=0;
    if(!r.want('{'))return false;
    do{char key[32];if(!r.string(key,sizeof key)||!r.want(':'))return false;
        unsigned bit=0;
        if(project::equal(key,"format")){bit=1;char s[64];if(!r.string(s,sizeof s)||!project::equal(s,"BuildingToolsCalibration"))return r.fail("Unknown calibration format");}
        else if(project::equal(key,"version")){bit=2;uint32_t v;if(!r.item_id(&v)||v!=1)return r.fail("Unknown calibration version");}
        else if(project::equal(key,"item")){bit=4;if(!r.item_id(&d.item))return false;}
        else if(project::equal(key,"entity")){bit=8;if(!r.item_id(&d.entity))return false;}
        else if(project::equal(key,"template")){bit=16;char s[33];if(!r.string(s,sizeof s))return false;int len=0;while(s[len])++len;if(len!=32)return r.fail("Invalid template GUID");
            for(int i=0;i<32;++i){char c=s[i];int v=c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:-1;if(v<0)return r.fail("Invalid GUID hex");d.templateId[i/2]|=(uint8_t)(v<<(i%2?0:4));}}
        else if(project::equal(key,"buildTime")){bit=32;if(!r.item_id(&d.buildTime))return false;}
        else if(project::equal(key,"imageSize")){bit=64;if(!r.item_id(&d.imageSize))return false;}
        else if(project::equal(key,"name")){bit=128;if(!r.string(d.name,sizeof d.name))return false;}
        else if(project::equal(key,"position")){bit=256;if(!vector(r,d.position,3))return false;}
        else if(project::equal(key,"local")){bit=512;if(!vector(r,d.local,3))return false;}
        else if(project::equal(key,"rotation")){bit=1024;if(!vector(r,d.rotation,4))return false;}
        else if(project::equal(key,"scale")){bit=2048;if(!vector(r,d.scale,3))return false;}
        else if(project::equal(key,"low")){bit=4096;if(!vector(r,d.low,3))return false;}
        else if(project::equal(key,"high")){bit=8192;if(!vector(r,d.high,3))return false;}
        else return r.fail("Unknown calibration field");
        if(!r.field(&fields,bit))return false;
    }while(r.take(','));
    if(!r.want('}')||fields!=16383)return r.fail("Incomplete calibration");r.space();if(r.at!=size||!valid(d))return r.fail("Invalid calibration data");*out=d;return true;
}
// Reuse native measured placement volume; normalize its bottom-centre to the
// requested instance origin. Preserve captured rotation, never invent bounds.
inline bool instance(const Definition& d,extended::Object* o){if(!valid(d))return false;*o={};o->item=d.item;for(int a=0;a<4;++a)o->rotation[a]=d.rotation[a];
    const double zero[]={0,0,0};props::Bounds b;if(!props::bounds(zero,d.rotation,d.low,d.high,&b))return false;
    o->p[0]=-(b.lo[0]+b.hi[0])*.5;o->p[1]=-b.lo[1];o->p[2]=-(b.lo[2]+b.hi[2])*.5;return true;
}
}
