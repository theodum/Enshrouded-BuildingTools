// Executes the same button routing, admission policy and close handshake as the
// DLL. The adapter records calls; no game, UI renderer or world is simulated.
#include "../src/project_actions.h"
#include <cstdio>
#include <limits>
#include <vector>
using namespace project;
static int checks=0,failures=0;
#define CHECK(c) do{++checks;if(!(c)){std::printf("FAIL actions line %d: %s\n",__LINE__,#c);++failures;}}while(0)
static ActionContext valid(){
    ActionContext c;c.window=c.loaded=c.local=c.enabled=c.mode=c.systems=c.bases=true;
    c.step[0]=c.step[1]=c.step[2]=0.5f;c.cells=312;return c;
}
struct Runtime {
    ActionContext state=valid();ActionIssue startFailure=ActionIssue::None,lastIssue=ActionIssue::None;
    std::vector<Action> receivedActions;int starts=0,closes=0;bool inspect=false;uint32_t now=100;
    Job job;Launch launch;
    ActionContext context(){return state;}
    void received(Action action){receivedActions.push_back(action);}
    ActionIssue start(Action action){++starts;if(startFailure!=ActionIssue::None)return startFailure;inspect=action==Action::Inspect;job.start(state.cells+state.pieces+state.objects);state.running=true;return ActionIssue::None;}
    void refused(Action,ActionIssue issue,const ActionContext&){lastIssue=issue;}
    void handoff(Action){++closes;state.armed=true;launch.begin(now);}
};
struct Buttons { bool check=false,construct=false;std::vector<bool> enabledCalls;bool button(Action action,bool enabled){enabledCalls.push_back(enabled);return enabled && (action==Action::Inspect ? check : construct);} };
static void click(Runtime& runtime,Action action){Buttons view{action==Action::Inspect,action==Action::Build};action_buttons(view,[&](Action a){dispatch_action(a,runtime);});CHECK(view.enabledCalls.size()==2 && view.enabledCalls[0] && view.enabledCalls[1]);}
int main(){
    for(Action action:{Action::Inspect,Action::Build}){
        Runtime rt;click(rt,action);CHECK(rt.receivedActions.size()==1 && rt.receivedActions[0]==action);
        CHECK(rt.starts==1 && rt.closes==1 && rt.state.armed && rt.state.running);
        CHECK(rt.inspect==(action==Action::Inspect));CHECK(rt.job.phase==Phase::Checking && rt.job.completed==0);
        CHECK(rt.launch.poll(115,true,false)==LaunchPoll::Wait);
        // The UI clears windowOpen before publishing the new menu/screen state.
        // A dev3 cancellation race must wait here, without releasing a batch.
        CHECK(rt.launch.window_closed(true));CHECK(rt.launch.poll(130,false,false)==LaunchPoll::Wait);
        CHECK(rt.job.next==0 && rt.job.completed==0);
        CHECK(rt.launch.poll(145,false,true)==LaunchPoll::Ready);
        CHECK(rt.launch.poll(150,false,true)==LaunchPoll::Ready);
        CHECK(rt.launch.poll(160,false,false)==LaunchPoll::Interrupted);
        click(rt,action);CHECK(rt.starts==1 && rt.closes==1 && rt.lastIssue==ActionIssue::Busy);
    }
    struct Case {ActionContext context;ActionIssue issue;};std::vector<Case> cases;
    auto add=[&](ActionContext c,ActionIssue issue){cases.push_back({c,issue});};
    auto c=valid();c.step[0]=c.step[1]=c.step[2]=1;add(c,ActionIssue::Shape); // observed dev3 symptom: button must still receive the click
    c=valid();c.step[1]=std::numeric_limits<float>::quiet_NaN();add(c,ActionIssue::Shape);
    c=valid();c.window=false;add(c,ActionIssue::Window);
    c=valid();c.armed=true;add(c,ActionIssue::Armed);
    c=valid();c.loaded=false;add(c,ActionIssue::NoProject);
    c=valid();c.local=false;add(c,ActionIssue::Remote);
    c=valid();c.enabled=false;add(c,ActionIssue::Disabled);
    c=valid();c.mode=false;add(c,ActionIssue::Mode);
    c=valid();c.systems=false;add(c,ActionIssue::Systems);
    c=valid();c.bases=false;add(c,ActionIssue::NoBases);
    c=valid();c.baseAge=10001;add(c,ActionIssue::StaleBases);
    c=valid();c.objects=1;add(c,ActionIssue::NativeDisabled);
    c=valid();c.pieces=1;add(c,ActionIssue::NativeDisabled);
    c=valid();c.cells=0;add(c,ActionIssue::Empty);
    c=valid();c.cells=MaxCells+1;add(c,ActionIssue::Limits);
    for(const auto& scenario:cases)for(Action action:{Action::Inspect,Action::Build}){
        Runtime rt;rt.state=scenario.context;click(rt,action);
        CHECK(rt.receivedActions.size()==1 && rt.lastIssue==scenario.issue);
        CHECK(rt.starts==0 && rt.closes==0 && !rt.state.running && !rt.launch.pending);
        CHECK(rt.state.window==scenario.context.window && rt.state.armed==scenario.context.armed);
        CHECK(rt.job.phase==Phase::Ready && rt.job.completed==0);
    }
    for(Action action:{Action::Inspect,Action::Build}){
        Runtime rt;rt.startFailure=ActionIssue::Journal;click(rt,action);
        CHECK(rt.lastIssue==ActionIssue::Journal && rt.closes==0 && rt.state.window && !rt.state.armed);
        rt.startFailure=ActionIssue::None;click(rt,action);CHECK(rt.starts==2 && rt.closes==1);
    }
    Runtime keyboard;CHECK(dispatch_action(Action::Build,keyboard));CHECK(!keyboard.inspect && keyboard.starts==1);
    Runtime doubleClick;Buttons both{true,true};action_buttons(both,[&](Action a){dispatch_action(a,doubleClick);});
    CHECK(doubleClick.starts==1 && doubleClick.closes==1 && doubleClick.inspect && doubleClick.lastIssue==ActionIssue::Busy);
    Runtime unchanged;Buttons none;action_buttons(none,[&](Action a){dispatch_action(a,unchanged);});CHECK(unchanged.receivedActions.empty());
    c=valid();c.baseAge=10000;CHECK(action_issue(c)==ActionIssue::None);c.native=true;c.pieces=1;CHECK(action_issue(c)==ActionIssue::None);
    Launch launch;CHECK(launch.poll(0,false,true)==LaunchPoll::Interrupted);launch.begin(100);
    CHECK(!launch.window_closed(false));CHECK(launch.poll(5100,true,false)==LaunchPoll::Wait);CHECK(launch.poll(5101,true,false)==LaunchPoll::Timeout);
    launch.begin(100);CHECK(launch.poll(110,false,true)==LaunchPoll::Wait); // no close acknowledgement, never send a click
    launch.begin(0xfffffff0u);CHECK(launch.window_closed(true));CHECK(launch.poll(10,false,false)==LaunchPoll::Wait);CHECK(launch.poll(20,false,true)==LaunchPoll::Ready);
    launch.reset();CHECK(launch.poll(21,false,true)==LaunchPoll::Interrupted);
    std::printf("actions: %d checks, %d failures\n",checks,failures);return failures!=0;
}
