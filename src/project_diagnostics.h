#pragma once
#include "core.h"
namespace diagnostics {
enum class State { None, Checking, Passed, Failed };
struct Context {
    int generation=0,rotation=0;core::I3 origin{};bool creative=false,objects=false;
};
inline bool same(const Context& a,const Context& b) {
    return a.generation==b.generation && (a.rotation&3)==(b.rotation&3) && a.origin.x==b.origin.x && a.origin.y==b.origin.y && a.origin.z==b.origin.z
        && a.creative==b.creative && a.objects==b.objects;
}
struct Validation {
    Context context{};State state=State::None;bool fullQuote=false;int checked=0,total=0;
    void begin(const Context& c,int n) {context=c;state=State::Checking;fullQuote=false;checked=0;total=n;}
    bool progress(int n) {if(state!=State::Checking || n<checked || n>total)return false;checked=n;return true;}
    bool pass() {if(state!=State::Checking || checked!=total)return false;state=State::Passed;fullQuote=true;return true;}
    void fail() {state=State::Failed;}
    bool current(const Context& c) const {return state!=State::None && same(context,c);}
};
} // namespace diagnostics
