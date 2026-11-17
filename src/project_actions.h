// Shared by the native buttons/Enter path and the regression tests. No engine
// calls here: a rejected command cannot arm, close the window or start a job.
#pragma once
#include "project.h"
namespace project {
enum class Action { Inspect, Build };
inline const char* action_name(Action a) { return a==Action::Inspect ? "inspect" : "build"; }
enum class ActionIssue { None, Busy, Window, Armed, NoProject, Remote, Disabled, Mode, Shape, Systems, NoBases, StaleBases, NativeDisabled, Empty, Limits, Journal };
inline const char* issue_code(ActionIssue issue) {
    const char* names[]={"OK","BUSY","WINDOW","ARMED","NO_PROJECT","REMOTE","DISABLED","MODE","SHAPE","SYSTEMS","BASES_UNOBSERVED","BASES_STALE","NATIVE_DISABLED","EMPTY","LIMITS","JOURNAL"};
    return names[(int)issue];
}
struct ActionContext {
    bool running=false, window=false, armed=false, loaded=false, local=false;
    bool enabled=false, mode=false, systems=false, bases=false, native=false;
    uint32_t baseAge=0;
    float step[3]={};
    int cells=0,pieces=0,objects=0;
};
inline ActionIssue action_issue(const ActionContext& c) {
    if(c.running)return ActionIssue::Busy;
    if(!c.window)return ActionIssue::Window;
    if(c.armed)return ActionIssue::Armed;
    if(!c.loaded)return ActionIssue::NoProject;
    if(!c.local)return ActionIssue::Remote;
    if(!c.enabled)return ActionIssue::Disabled;
    if(!c.mode)return ActionIssue::Mode;
    for(int a=0;a<3;++a)if(c.step[a]!=0.5f)return ActionIssue::Shape;
    if(!c.systems)return ActionIssue::Systems;
    if(!c.bases)return ActionIssue::NoBases;
    if(c.baseAge>10000)return ActionIssue::StaleBases;
    if((c.pieces || c.objects) && !c.native)return ActionIssue::NativeDisabled;
    if(c.cells<0 || c.cells>MaxCells || c.pieces<0 || c.pieces>MaxPieces || c.objects<0 || c.objects>MaxModules*core::BP_MAX_OBJS)return ActionIssue::Limits;
    if(!c.cells && !c.pieces && !c.objects)return ActionIssue::Empty;
    return ActionIssue::None;
}
template<class Runtime> bool dispatch_action(Action action, Runtime& runtime) {
    runtime.received(action);
    ActionContext context=runtime.context();
    ActionIssue issue=action_issue(context);
    if(issue==ActionIssue::None)issue=runtime.start(action);
    if(issue!=ActionIssue::None){runtime.refused(action,issue,context);return false;}
    runtime.handoff(action);return true;
}
// Always let the command policy explain a refusal, including a wrong cube size.
// Native UI disabled buttons cannot call back and were silent in dev3.
template<class View,class Request> void action_buttons(View& view,Request request,bool enterHeld=false) {
    // Enter has one owner (the existing keyboard Build path). The native UI can
    // also report Enter on its focused button; do not race Inspect against Build.
    if(view.button(Action::Inspect,true) && !enterHeld)request(Action::Inspect);
    if(view.button(Action::Build,true) && !enterHeld)request(Action::Build);
}
enum class LaunchPoll { Wait, Ready, Timeout, Interrupted };
inline bool live_commit(bool running,int pending,uint32_t age){return running && pending>0 && age<3000;}
struct Launch {
    bool pending=false,closed=false,ready=false;uint32_t at=0;
    void begin(uint32_t now){pending=true;closed=ready=false;at=now;}
    void reset(){pending=closed=ready=false;}
    bool window_closed(bool gameScreen){if(!pending || ready || !gameScreen)return false;closed=true;return true;}
    LaunchPoll poll(uint32_t now,bool windowOpen,bool screenReady){
        if(!pending)return LaunchPoll::Interrupted;
        if(ready)return !windowOpen && screenReady ? LaunchPoll::Ready : LaunchPoll::Interrupted;
        if((int32_t)(now-at)>5000)return LaunchPoll::Timeout;
        if(!closed || windowOpen || !screenReady)return LaunchPoll::Wait;
        ready=true;return LaunchPoll::Ready;
    }
};
}
