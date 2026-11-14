#pragma once
#include <stdint.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>

namespace Deck {
using Text=std::string;
inline int rounded(double v){return int(std::floor(v+0.5));}
inline int clamp(int n,int lo,int hi){return std::max(lo,std::min(hi,n));}
inline Text num(int n){return std::to_string(n);}
inline Text pad2(int n){return n<10?"0"+num(n):num(n);}
inline int quality(int rssi){return clamp(rounded((rssi+100)*100.0/60),0,100);}
inline Text uptime(uint64_t ms){uint64_t sec=ms/1000;return pad2(int(sec/3600))+":"+pad2(int(sec/60%60))+":"+pad2(int(sec%60));}
inline Text safeAscii(const Text& raw){Text out;for(uint8_t c:raw){if(c>=32&&c<127)out+=char(c);else if((c&0xC0)!=0x80)out+='?';}return out;}
constexpr uint32_t BOOT_MS=10000,POWER_OFF_MS=900,SCAN_MS=2600,BREACH_MS=30000;
constexpr int CAPACITY=4,MAX_NETWORKS=32;
inline const std::array<Text,5>& menu(){static const std::array<Text,5> v={"NET SCANNER","SIGNAL TRACKER","SECURITY PROFILE","SYSTEM MONITOR","BREACH PROTOCOL"};return v;}
inline const std::array<Text,4>& tokens(){static const std::array<Text,4> v={"BD","55","1C","E9"};return v;}
struct Network {
 Text ssid,bssid;int rssi=-100,channel=0;Text encryption="N/A",rawSsid;
 std::array<uint8_t,6> mac{};
};
inline Text nodeName(const Network& n){return n.ssid.empty()?"<hidden>":n.ssid;}
inline std::vector<Network> demoNetworks(){return {
 {"NEON-HOME","02:A7:11:9C:40:01",-42,1,"WPA2-PSK"},
 {"WORKSHOP","02:A7:11:9C:40:02",-51,6,"WPA2/WPA3-PSK"},
 {"GUEST_24G","02:A7:11:9C:40:03",-59,6,"OPEN"},
 {"GHOST-LINK","02:A7:11:9C:40:04",-67,11,"WPA3-SAE"},
 {"","02:A7:11:9C:40:05",-78,3,"WPA2-PSK"},
 {"NIGHT-CITY-MEGABUILDING-LOCAL-24G","02:A7:11:9C:40:06",-86,13,"WPA2-ENTERPRISE"}};}
struct Puzzle {std::array<Text,9> matrix;std::array<Text,3> target;int capacity=CAPACITY;uint32_t duration=BREACH_MS;};
inline Puzzle makePuzzle(uint32_t seed=2077,uint32_t round=0){
 Puzzle p;
 if(!round){p.matrix={"1C","BD","E9","BD","55","E9","55","1C","BD"};p.target={"BD","55","E9"};return p;}
 uint32_t state=seed+round*uint32_t(2654435761u);
 auto rand=[&](unsigned n){state=state*1664525u+1013904223u;return (state>>8)%n;};
 for(auto& cell:p.matrix)cell=tokens()[rand(4)];
 std::vector<int> path={int(rand(3))};bool column=true;
 for(int k=1;k<4;k++){std::vector<int> possible;int last=path.back();for(int i=0;i<9;i++)if(std::find(path.begin(),path.end(),i)==path.end()&&(column?i%3==last%3:i/3==last/3))possible.push_back(i);path.push_back(possible[rand(possible.size())]);column=!column;}
 std::vector<Text> pool(tokens().begin(),tokens().end());
 for(int i=0;i<3;i++){unsigned idx=rand(pool.size());p.target[i]=pool[idx];pool.erase(pool.begin()+idx);}
 for(int i=0;i<3;i++)p.matrix[path[i+round%2]]=p.target[i];
 return p;
}
struct BreachGame {
 std::array<Text,9> matrix;std::array<Text,3> target;
 int capacity=CAPACITY,cursor=0,axisIndex=0;uint32_t duration=BREACH_MS;
 std::vector<int> used;std::vector<Text> buffer;
 Text axis="row",result,reason;bool started=false,finished=false;
 uint64_t startedAt=0,finishedAt=0;
 explicit BreachGame(const Puzzle& p=makePuzzle()):matrix(p.matrix),target(p.target),capacity(p.capacity),duration(p.duration){}
 std::vector<int> legal()const{std::vector<int> v;for(int i=0;i<9;i++)if(std::find(used.begin(),used.end(),i)==used.end()&&(axis=="row"?i/3==axisIndex:i%3==axisIndex))v.push_back(i);return v;}
 uint32_t remaining(uint64_t now)const{if(!started)return duration;uint64_t elapsed=(finished?finishedAt:now)-startedAt;return elapsed>=duration?0:duration-uint32_t(elapsed);}
 void finish(const Text& r,const Text& why,uint64_t now){result=r;reason=why;finished=true;finishedAt=now;}
 void tick(uint64_t now){if(result.empty()&&started&&!remaining(now))finish("failure","TIME EXPIRED",now);}
 bool move(int direction,uint64_t now){tick(now);if(!result.empty())return false;auto v=legal();if(v.empty())return false;auto it=std::find(v.begin(),v.end(),cursor);int pos=it==v.end()?0:int(it-v.begin());cursor=v[(pos+direction+int(v.size()))%v.size()];return true;}
 bool matches()const{for(int i=0;i+3<=int(buffer.size());i++)if(std::equal(target.begin(),target.end(),buffer.begin()+i))return true;return false;}
 bool pick(uint64_t now,int index=-1){
  tick(now);if(index<0)index=cursor;auto v=legal();if(!result.empty()||std::find(v.begin(),v.end(),index)==v.end())return false;
  if(!started){started=true;startedAt=now;}used.push_back(index);buffer.push_back(matrix[index]);
  if(matches()){finish("success","SEQUENCE MATCH",now);return true;}
  if(int(buffer.size())>=capacity){finish("failure","BUFFER FULL",now);return true;}
  axis=axis=="row"?"column":"row";axisIndex=axis=="column"?index%3:index/3;v=legal();
  if(v.empty())finish("failure","NO ROUTE",now);else cursor=v[0];return true;
 }
 int progress()const{int best=0;for(int n=1;n<=std::min(3,int(buffer.size()));n++)if(std::equal(target.begin(),target.begin()+n,buffer.end()-n))best=n;return best;}
 Text sequence()const{return target[0]+" "+target[1]+" "+target[2];}
};
struct Button {
 bool raw=false,pressed=false;uint32_t changedAt=0,count=0;
 constexpr void begin(bool state,uint32_t now){raw=pressed=state;changedAt=now;count=0;}
 constexpr bool update(bool state,uint32_t now){if(state!=raw){raw=state;changedAt=now;}if(pressed!=raw&&uint32_t(now-changedAt)>=30){pressed=raw;if(pressed){count++;return true;}}return false;}
};
// BACK is committed on release; holding it emits one power action instead.
struct PowerKey {
 enum class Action {None,Back,Toggle};
 bool armed=false,fired=false,allowBack=false;uint64_t pressedAt=0;
 constexpr Action update(bool pressed,bool edge,uint64_t now,bool allowShort,uint32_t holdMs=2000){
  if(edge){armed=true;fired=false;allowBack=allowShort;pressedAt=now;}
  if(!armed)return Action::None;
  if(pressed&&!fired&&now-pressedAt>=holdMs){fired=true;return Action::Toggle;}
  if(!pressed){armed=false;return !fired&&allowBack?Action::Back:Action::None;}
  return Action::None;
 }
};
struct SystemInfo {int heap=0,total=1;Text ap="OFF",sta="IDLE";};
struct Cyberdeck {
 bool demo=false,scanRequested=false,scanError=false,scanWaiting=false,trackerMissing=false;
 uint32_t seed=2077,revision=0,round=0,scanWaitSeconds=0;
 uint64_t now=0,pageAt=0,lastInputAt=0,scanStartedAt=0,sampleAt=0,bootStartedAt=0,powerAt=0;
 Text page="boot",selectKind="netlist",profileReturn="netlist",scanReturn="netlist";
 int menuIndex=0,nodeIndex=0,dataTab=0,scanCount=0,liveRssi=-60,abortIndex=0;
 std::vector<Network> networks;std::vector<int> history;
 BreachGame game;SystemInfo system;
 explicit Cyberdeck(bool demoMode=false,uint32_t randomSeed=2077):demo(demoMode),seed(randomSeed){}
 void go(const Text& next){if(page!=next){page=next;pageAt=now;revision++;}}
 bool powerLocked()const{return page=="boot"||page=="off"||page=="shutdown";}
 void togglePower(uint64_t time){
  tick(time);if(page=="shutdown")return;
  if(page=="off"){
   const uint64_t timeNow=now;const uint32_t oldRevision=revision;const int choice=menuIndex;
   *this=Cyberdeck(demo,seed);now=pageAt=bootStartedAt=powerAt=lastInputAt=timeNow;menuIndex=choice;revision=oldRevision+1;
  }else{scanRequested=false;scanWaiting=false;powerAt=now;go("shutdown");}
 }
 const Network& node()const{if(!networks.empty())return networks[clamp(nodeIndex,0,int(networks.size())-1)];static const Network fallback={"NO TARGET","--:--:--:--:--:--",-100,0,"N/A"};static const Network first=demoNetworks()[0];return demo?first:fallback;}
 void beginScan(const Text& destination="netlist"){if(powerLocked())return;scanReturn=destination;scanStartedAt=now;scanCount=0;scanError=false;scanWaiting=false;scanRequested=true;go("scan");}
 void finishScan(std::vector<Network> values,bool error=false){
  if(powerLocked())return;
  scanError=error;scanRequested=false;scanWaiting=false;
  if(!error){std::stable_sort(values.begin(),values.end(),[](const Network& a,const Network& b){return a.rssi>b.rssi;});if(values.size()>MAX_NETWORKS)values.resize(MAX_NETWORKS);networks=std::move(values);}
  nodeIndex=clamp(nodeIndex,0,std::max(0,int(networks.size())-1));scanCount=int(networks.size());selectKind=scanReturn;
  if(page=="scan")go(scanReturn);
 }
 void ensureNodes(const Text& destination){if(networks.empty())beginScan(destination);else{nodeIndex=clamp(nodeIndex,0,int(networks.size())-1);go(destination);}}
 void sampleDemo(uint64_t t){double wave=std::sin(t/1700.0+nodeIndex)*3.8+std::sin(t/430.0+nodeIndex*2)*1.9;addSample(clamp(rounded(node().rssi+wave),-100,-30));}
 void addSample(int rssi){liveRssi=rssi;trackerMissing=false;history.push_back(rssi);if(history.size()>64)history.erase(history.begin());}
 void startTracker(){history.clear();sampleAt=now;liveRssi=node().rssi;trackerMissing=false;go("tracker");if(demo)sampleDemo(now);}
 void tick(uint64_t time){
  now=std::max(now,time);
  if(page=="off")return;
  if(page=="shutdown"){if(now-powerAt>=POWER_OFF_MS)go("off");return;}
  if(page=="boot"){if(now-bootStartedAt>=BOOT_MS){lastInputAt=now;go("menu");}return;}
  if(page=="scan"&&demo){uint64_t elapsed=now-scanStartedAt;scanCount=std::min(6,int(elapsed*6/SCAN_MS));if(elapsed>=SCAN_MS)finishScan(demoNetworks());}
  if(page=="tracker"&&demo){if(now>64*250)sampleAt=std::max(sampleAt,now-64*250);while(now-sampleAt>=250){sampleAt+=250;sampleDemo(sampleAt);}}
  if(page=="breach"||page=="abort"){game.tick(now);if(!game.result.empty())go(game.result=="success"?"success":"failure");}
  if(page=="menu"&&now-lastInputAt>45000)go("idle");
 }
 void enterMenu(){lastInputAt=now;switch(menuIndex){case 0:beginScan();break;case 1:selectKind="trackerSelect";ensureNodes(selectKind);break;case 2:selectKind="securitySelect";ensureNodes(selectKind);break;case 3:go("system");break;case 4:go("breachIntro");break;}}
 void newGame(){game=BreachGame(makePuzzle(seed,round));}
 void input(const Text& key,uint64_t time){
  if(key!="up"&&key!="down"&&key!="enter"&&key!="back")return;
  Text prior=page;tick(time);lastInputAt=now;
  if((prior=="breach"||prior=="abort")&&page=="failure")return;
  int direction=key=="down"?1:key=="up"?-1:0;
  if(powerLocked())return;
  if(page=="idle"){go("menu");return;}
  if(page=="menu"){if(direction)menuIndex=clamp(menuIndex+direction,0,4);if(key=="enter")enterMenu();if(key=="back")go("idle");return;}
  if(page=="scan"){if(key=="back"){scanRequested=false;go("menu");}return;}
  if(page=="netlist"||page=="trackerSelect"||page=="securitySelect"){
   if(direction)nodeIndex=clamp(nodeIndex+direction,0,std::max(0,int(networks.size())-1));
   if(key=="back")go("menu");
   if(key=="enter"){if(networks.empty()){beginScan(page);return;}if(page=="netlist"){profileReturn="netlist";go("details");}else if(page=="trackerSelect")startTracker();else{profileReturn="securitySelect";dataTab=0;go("security");}}return;
  }
  if(page=="details"){if(key=="back")go("netlist");if(key=="enter"){profileReturn="details";dataTab=0;go("security");}return;}
  if(page=="tracker"){if(key=="back")go("trackerSelect");if(direction&&!networks.empty()){nodeIndex=(nodeIndex+direction+int(networks.size()))%networks.size();history.clear();sampleAt=now;liveRssi=node().rssi;trackerMissing=false;if(demo)sampleDemo(now);}return;}
  if(page=="security"){if(key=="back")go(profileReturn);if(direction||key=="enter")dataTab=1-dataTab;return;}
  if(page=="system"){if(key=="back")go("menu");return;}
  if(page=="breachIntro"){if(key=="back")go("menu");if(key=="enter"){newGame();go("breach");}return;}
  if(page=="breach"){if(direction)game.move(direction,now);if(key=="enter")game.pick(now);if(key=="back"){abortIndex=0;go("abort");}if(!game.result.empty())go(game.result=="success"?"success":"failure");return;}
  if(page=="abort"){if(direction)abortIndex=1-abortIndex;if(key=="back"||(key=="enter"&&abortIndex==0))go("breach");else if(key=="enter"){newGame();go("breachIntro");}return;}
  if(page=="success"||page=="failure"){if(key=="back")go("menu");if(key=="enter"){round++;newGame();go("breachIntro");}}
 }
 SystemInfo metrics()const{if(!demo)return system;return {184+rounded(std::sin(now/9000.0)*4),320,"ON","IDLE"};}
};
}
