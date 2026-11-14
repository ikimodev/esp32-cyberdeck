#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_wifi.h>
#include <esp_timer.h>

// ---- config.h ----

#include <stdint.h>
namespace Config {
constexpr uint8_t UP_PIN=14,DOWN_PIN=13,ENTER_PIN=12,BACK_PIN=27;
constexpr uint8_t SDA_PIN=21,SCL_PIN=22,OLED_ADDRESS=0x3C;
constexpr uint32_t I2C_HZ=400000,FRAME_MS=83,DISPLAY_MS=20000;
constexpr uint32_t SCAN_TIMEOUT_MS=15000,SCAN_COOLDOWN_MS=10000,TRACK_PERIOD_MS=1000;
constexpr uint32_t POWER_HOLD_MS=2000;
constexpr char VERSION[]="4.2.4-HW2";
constexpr bool REDUCED_MOTION=false;
constexpr char AP_SSID[]="ESP32-LAB";
constexpr char AP_PASSWORD[]="Lab32-2026";
}
// Production always uses real radio/system readings. The separate Wokwi
// validation build explicitly opts into demo data and is labelled DEMO.
#ifndef DECK_DEMO
#define DECK_DEMO 0
#endif
#ifndef DECK_SELF_TEST
#define DECK_SELF_TEST 0
#endif


// ---- deck_assets.h ----

#include <stdint.h>
namespace Deck { namespace Assets {
struct Bitmap {int w,h,stride;const uint8_t* data;};
static const uint8_t font[]={0,0,0,0,0,0,0,95,0,0,0,7,0,7,0,20,127,20,127,20,36,42,127,42,18,35,19,8,100,98,54,73,86,32,80,0,8,7,3,0,0,28,34,65,0,0,65,34,28,0,42,28,127,28,42,8,8,62,8,8,0,128,112,48,0,8,8,8,8,8,0,0,96,96,0,32,16,8,4,2,62,81,73,69,62,0,66,127,64,0,114,73,73,73,70,33,65,73,77,51,24,20,18,127,16,39,69,69,69,57,60,74,73,73,49,65,33,17,9,7,54,73,73,73,54,70,73,73,41,30,0,0,20,0,0,0,64,52,0,0,0,8,20,34,65,20,20,20,20,20,0,65,34,20,8,2,1,89,9,6,62,65,93,89,78,124,18,17,18,124,127,73,73,73,54,62,65,65,65,34,127,65,65,65,62,127,73,73,73,65,127,9,9,9,1,62,65,65,81,115,127,8,8,8,127,0,65,127,65,0,32,64,65,63,1,127,8,20,34,65,127,64,64,64,64,127,2,28,2,127,127,4,8,16,127,62,65,65,65,62,127,9,9,9,6,62,65,81,33,94,127,9,25,41,70,38,73,73,73,50,3,1,127,1,3,63,64,64,64,63,31,32,64,32,31,63,64,56,64,63,99,20,8,20,99,3,4,120,4,3,97,89,73,77,67,0,127,65,65,65,2,4,8,16,32,0,65,65,65,127,4,2,1,2,4,64,64,64,64,64,0,3,7,8,0,32,84,84,120,64,127,40,68,68,56,56,68,68,68,40,56,68,68,40,127,56,84,84,84,24,0,8,126,9,2,24,164,164,156,120,127,8,4,4,120,0,68,125,64,0,32,64,64,61,0,127,16,40,68,0,0,65,127,64,0,124,4,120,4,120,124,8,4,4,120,56,68,68,68,56,252,24,36,36,24,24,36,36,24,252,124,8,4,4,8,72,84,84,84,36,4,4,63,68,36,60,64,64,32,124,28,32,64,32,28,60,64,48,64,60,68,40,16,40,68,76,144,144,144,124,68,100,84,76,68,0,8,54,65,0,0,0,119,0,0,0,65,54,8,0,2,1,2,4,2};
static const uint8_t logoData[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,240,0,7,0,0,0,0,0,0,0,0,0,0,0,0,7,240,0,14,0,0,0,0,0,0,0,0,0,0,0,0,31,0,0,28,0,0,0,0,0,0,0,0,1,0,0,0,252,0,0,48,0,0,0,0,0,0,0,0,15,0,0,3,240,0,0,224,0,56,0,0,0,136,0,0,60,0,0,15,192,12,121,192,127,127,192,31,255,205,0,224,224,0,0,127,0,12,243,255,240,227,254,63,255,155,140,231,128,0,3,252,0,77,230,63,156,195,248,112,252,51,157,252,0,0,15,255,255,255,220,231,253,255,192,231,252,55,153,248,0,0,255,255,255,243,187,206,33,254,0,255,12,102,243,254,0,0,127,255,128,7,127,31,251,191,135,248,24,206,247,31,128,0,0,0,0,12,252,30,7,7,243,192,31,204,103,3,192,0,0,0,0,56,112,0,7,1,251,128,63,152,78,0,240,0,0,0,0,112,32,0,6,0,103,0,17,0,0,0,60,0,0,0,0,224,0,0,0,0,6,0,0,0,0,0,7,0,0,0,1,128,0,0,0,0,14,0,0,0,0,0,0,192,0,0,6,0,0,0,0,0,28,0,0,0,0,0,0,48,0,0,8,0,0,0,0,0,28,0,0,59,187,128,0,0,0,0,0,0,0,0,0,0,48,0,0,10,136,128,0,0,0,0,0,0,0,0,0,0,32,0,0,58,145,0,0,0,0,0,0,0,0,0,0,0,64,0,0,34,145,0,0,0,0,0,0,0,0,0,0,0,0,0,0,59,145,0,0,0};
static const Bitmap logo={126,25,16,logoData};
static const uint8_t smallLogoData[]={0,0,0,0,0,0,0,0,0,16,0,0,0,0,0,224,32,0,0,0,0,3,128,64,0,0,0,128,15,2,205,241,255,19,0,62,63,253,251,247,252,0,255,255,255,231,247,252,0,0,31,55,126,62,230,0,0,48,2,60,48,1,128,0,32,0,8,0,0,64};
static const Bitmap smallLogo={52,10,7,smallLogoData};
static const uint8_t cityData[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,248,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23,250,0,0,0,0,15,255,255,0,0,0,0,0,0,0,32,2,0,48,0,0,48,0,1,255,255,224,127,0,0,0,32,1,0,44,0,0,196,0,1,128,0,56,65,3,128,0,33,16,128,35,0,3,4,255,253,128,0,38,65,28,96,31,33,16,128,33,0,4,4,128,5,191,255,249,65,96,24,17,225,16,128,33,7,4,4,136,5,160,0,9,193,64,8,17,1,19,223,225,56,196,4,136,133,165,112,9,1,64,8,19,255,255,250,73,192,52,4,148,197,167,64,9,1,64,11,242,0,0,10,73,128,20,4,149,37,167,64,9,1,64,10,75,255,255,248,1,146,127,132,149,21,167,64,9,127,73,10,73,1,19,210,73,146,64,132,162,5,165,112,9,200,73,10,1,37,18,82,73,128,68,132,162,5,160,0,9,0,64,10,73,1,16,16,1,146,94,132,128,5,160,0,9,72,73,10,73,1,16,18,65,146,68,132,128,5,191,255,249,72,73,10,1,33,3,210,65,128,68,132,255,255,159,255,249,0,64,10,73,1,0,16,1,128,64,132,31,224,191,192,33,72,73,10,73,1,0,16,1,128,68,159,224,0,128,0,33,72,73,10,1,5,0,80,1,128,95,224,0,0,128,7,33,0,64,11,193,0,0,16,63,255,224,0,0,31,128,0,33,0,126,8,121,192,0,3,255,192,0,0,63,225,191,192,41,0,1,248,7,0,0,63,192,0,0,127,192,1,128,0,33,0,0,0,12,0,63,192,0,0,255,132,128,1,128,0,33,1,240,0,48,127,192,0,1,255,94,132,252,17,128,0,33,1,8,0,255,128,0,3,254,0,68,132,128,1,191,199,41,1,0,127,128,0,7,252,19,0,68,132,128,1,128,0,33,1,127,128,0,15,248,0,18,207,192,132,128,1,128,0,33,15,128,0,31,240,0,0,18,40,127,132,128,1,128,0,33,1,0,63,224,0,0,0,18,24,4,4,128,1,191,192,41,1,127,192,15,240,0,0,18,14,4,4,252,17,128,0,33,15,140,0,0,0,0,0,18,9,132,4,128,1,128,7,33,1,48,0,0,0,3,255,18,8,68,4,128,1,128,0,33,1,192,0,0,0,0,0,18,8,52,4,128,1,191,192,41,3,0,0,0,0,0,0,18,8,12,4,252,17,128,0,33,12,0,31,248,0,0,0,127,8,4,4,128,1,255,255,255,48,0,0,0,0,0,0,0,0,7,255,255,255};
static const Bitmap city={128,64,16,cityData};
}}


// ---- cyberdeck_core.h ----

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


// ---- oled_renderer.h ----



#include <cstring>

namespace Deck {
struct Framebuffer {
 std::array<uint8_t,1024> bits{};
 void clear(){bits.fill(0);}
 int get(int x,int y)const{if(x<0||x>=128||y<0||y>=64)return 0;return (bits[y*16+(x>>3)]>>(7-(x&7)))&1;}
 void set(int x,int y,int value=1){if(x<0||x>=128||y<0||y>=64)return;uint8_t mask=128>>(x&7);if(value)bits[y*16+(x>>3)]|=mask;else bits[y*16+(x>>3)]&=uint8_t(~mask);}
 void line(int x0,int y0,int x1,int y1,int value=1){int dx=std::abs(x1-x0),dy=-std::abs(y1-y0),sx=x0<x1?1:-1,sy=y0<y1?1:-1,err=dx+dy;for(;;){set(x0,y0,value);if(x0==x1&&y0==y1)break;int e=err*2;if(e>=dy){err+=dy;x0+=sx;}if(e<=dx){err+=dx;y0+=sy;}}}
 void rect(int x,int y,int w,int h,bool fill=false,int value=1){if(fill){for(int yy=y;yy<y+h;yy++)for(int xx=x;xx<x+w;xx++)set(xx,yy,value);}else{line(x,y,x+w-1,y,value);line(x,y+h-1,x+w-1,y+h-1,value);line(x,y,x,y+h-1,value);line(x+w-1,y,x+w-1,y+h-1,value);}}
 void text(int x,int y,const Text& label,int value=1){for(uint8_t c:label){int offset=((c>=32&&c<127?c:63)-32)*5;for(int xx=0;xx<5;xx++){uint8_t b=Assets::font[offset+xx];for(int yy=0;yy<8;yy++)if(b&(1<<yy))set(x+xx,y+yy,value);}x+=6;}}
 void label(int x,int y,const Text& s,int columns=21,int value=1){text(x,y,int(s.size())>columns?s.substr(0,columns-3)+"...":s,value);}
 void bitmap(const Assets::Bitmap& a,int x=0,int y=0,double beam=-100,int glitch=0,double reveal=1){for(int yy=0;yy<a.h;yy++)for(int xx=0;xx<a.w;xx++){
  if(xx>a.w*reveal+(yy%4)*2)continue;
  int lit=(a.data[yy*a.stride+(xx>>3)]>>(7-(xx&7)))&1;
  if(!lit&&std::abs(xx-beam)<3&&(xx+yy)%2==0&&xx>0&&xx<a.w-1){int l=(a.data[yy*a.stride+((xx-1)>>3)]>>(7-((xx-1)&7)))&1,r=(a.data[yy*a.stride+((xx+1)>>3)]>>(7-((xx+1)&7)))&1;lit=l||r;}
  if(lit)set(x+xx+(yy>=10&&yy<12?glitch:0),y+yy);
 }}
 void reticle(int x,int y,int w,int h){for(int sideX=0;sideX<2;sideX++)for(int sideY=0;sideY<2;sideY++){int xx=sideX?x+w-1:x,yy=sideY?y+h-1:y,sx=sideX?-1:1,sy=sideY?-1:1;line(xx,yy,xx+sx*3,yy);line(xx,yy,xx,yy+sy*2);}}
 uint32_t hash()const{uint32_t h=2166136261u;for(uint8_t b:bits){h^=b;h*=16777619u;}return h;}
};
struct OledRenderer {
 static constexpr uint32_t GLITCH_PERIOD_MS=2000,GLITCH_BURST_MS=320;
 Framebuffer f;std::array<uint8_t,1024> previous{};bool hasPrevious=false,reducedMotion=false;
 // Static renderer workspaces keep animation snapshots off the ESP32 loop stack.
 std::array<uint8_t,1024> workA{},workB{};
 Text lastPage,resolveKey;uint64_t transitionAt=0,lastTime=0,resolveAt=0,logoTime=0;double cursorY=20;
 explicit OledRenderer(bool reduced=false):reducedMotion(reduced){}
 static int pixel(const std::array<uint8_t,1024>& data,int x,int y){return x>=0&&x<128&&y>=0&&y<64?(data[y*16+(x>>3)]>>(7-(x&7)))&1:0;}
 static double unit(double value){return std::max(0.0,std::min(1.0,value));}
 static double ease(double value){double p=unit(value);return p*p*(3-2*p);}
 void glitchLogo(const Assets::Bitmap& asset,int x,int y,uint64_t time,double reveal=1){
  uint32_t phase=time%GLITCH_PERIOD_MS;f.bitmap(asset,x,y,reducedMotion?-100:double(phase)/GLITCH_PERIOD_MS*(asset.w+18)-9,0,reveal);
  if(reducedMotion||phase>=GLITCH_BURST_MS)return;
  workA=f.bits;const auto& source=workA;int magnitude=asset.w>100?6:3,direction=phase<100?1:phase<220?-1:1;
  const int starts[]={int(std::floor(asset.h*.28)),int(std::floor(asset.h*.56)),int(std::floor(asset.h*.8))};
  const int heights[]={2,asset.h>15?3:2,1},shifts[]={direction*magnitude,-direction*(magnitude-1),direction*(magnitude+1)};
  for(int strip=0;strip<3;strip++)for(int yy=y+starts[strip];yy<std::min(y+asset.h,y+starts[strip]+heights[strip]);yy++)for(int xx=x;xx<x+asset.w;xx++){
   int from=xx-shifts[strip],ghost=xx+shifts[strip];bool lit=from>=x&&from<x+asset.w&&pixel(source,from,yy);
   bool echo=phase<160&&(xx+yy)%3==0&&ghost>=x&&ghost<x+asset.w&&pixel(source,ghost,yy);f.set(xx,yy,lit||echo);
  }
 }
 void header(const Text& title){f.text(2,4,title);glitchLogo(Assets::smallLogo,75,2,logoTime);f.line(0,15,85,15);f.line(90,15,127,15);}
 void footer(const Text& left,const Text& right=""){f.line(0,54,127,54);f.label(2,56,left,20);if(!right.empty())f.text(127-int(right.size())*6,56,right);}
 void fullLogo(uint64_t t,double reveal=1){glitchLogo(Assets::logo,1,0,t,reveal);}
 void scaleFrame(const std::array<uint8_t,1024>& data,double sx,double sy){
  f.bits.fill(0);if(sx<=0||sy<=0)return;
  for(int y=0;y<64;y++)for(int x=0;x<128;x++){
   if(std::abs(x-63.5)>64*sx||std::abs(y-31.5)>32*sy)continue;
   int xx=rounded((x-63.5)/sx+63.5),yy=rounded((y-31.5)/sy+31.5);if(pixel(data,xx,yy))f.set(x,y);
  }
 }
 void startup(const Cyberdeck& s){
  uint64_t t=s.now-s.bootStartedAt;if(t<650)return;
  if(reducedMotion){if(t<8800){city(0);fullLogo(2000);}else renderMenu(s);return;}
  if(t<1600){int half=1+rounded(ease((t-650)/950.0)*39);f.line(64-half,31,63+half,31);if(t>1200){f.set(63,30);f.set(64,32);}return;}
  city(t-1600);fullLogo(t-1600);
  if(t<3600){workA=f.bits;double p=ease((t-1600)/2000.0);scaleFrame(workA,.62+.38*p,.025+.975*p);return;}
  if(t<8800)return;
  workB=f.bits;f.clear();renderMenu(s);workA=f.bits;double p=ease((t-8800)/1200.0);int hy=int(std::ceil(32*p)),hx=int(std::ceil(64*std::min(1.0,p*1.4)));
  f.bits=workB;for(int y=0;y<64;y++)for(int x=0;x<128;x++)if(std::abs(y-31.5)<=hy&&std::abs(x-63.5)<=hx)f.set(x,y,pixel(workA,x,y));
 }
 void city(uint64_t t){f.bitmap(Assets::city);int car=31+int(t/70%89),y=55-(car-28)*12/99;f.line(car,y-1,car+2,y-1);for(int i=0;i<4;i++){int yy=56+(i*3+t/500)%8,xx=39+(i*13+t/330)%48;if(std::abs(xx-65)<(yy-48)*2)f.line(xx,yy,xx+2,yy);}}
 int animateCursor(int target,uint64_t now){double dt=double(std::min<uint64_t>(100,now>=lastTime?now-lastTime:0));cursorY=reducedMotion?target:cursorY+(target-cursorY)*(1-std::exp(-dt/45));if(std::abs(cursorY-target)<.15)cursorY=target;return rounded(cursorY);}
 void selectionBar(int y){for(int yy=y-1;yy<y+10;yy++)for(int x=0;x<128;x++)f.set(x,yy,1-f.get(x,yy));}
 Text decodeSelected(const Text& label,const Text& key,uint64_t now){
  if(resolveKey!=key){resolveKey=key;resolveAt=now;}uint64_t elapsed=now-resolveAt;
  if(reducedMotion||elapsed>=420)return label;
  int resolved=int(label.size()*elapsed/420),tick=int(elapsed*12/1000);const char* alphabet="01ABCDEF";Text out=label;
  for(int i=0;i<int(label.size());i++){char c=label[i];bool alpha=(c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9');if(i>=resolved&&alpha)out[i]=alphabet[(i*5+tick*3+key.size())%8];}return out;
 }
 void renderMenu(const Cyberdeck& s){
  // Preview menu uses page="menu" even while the startup shutter is opening.
  header("CYBERDECK");int first=std::max(0,s.menuIndex-2),y=animateCursor(20+(s.menuIndex-first)*11,s.now);
  for(int row=0;row<3;row++){int i=first+row;if(i>=5)break;int yy=20+row*11;if(i==s.menuIndex)f.text(0,yy,">");Text label=menu()[i];if(label.size()>18)label=label.substr(0,15)+"...";
   if(i==s.menuIndex)label=decodeSelected(label,num(s.revision)+":menu:"+num(i)+":"+menu()[i],s.now);f.text(11,yy,label);
  }selectionBar(y);footer("OK:OPEN    "+num(s.menuIndex+1)+"/5");
 }
 void list(const std::vector<Text>& items,int index,const Cyberdeck& s,bool network,const Text& title,const Text& hint){
  header(title);int first=std::max(0,index-2);
  if(items.empty()){f.text(10,26,"NO NODES FOUND");f.text(10,39,"OK:RESCAN");footer(s.scanError?"SCAN ERR BK:MENU":"BK:MENU");return;}
  int y=animateCursor(20+(index-first)*11,s.now);
  for(int row=0;row<3;row++){int i=first+row;if(i>=int(items.size()))break;int yy=20+row*11;if(i==index)f.text(0,yy,">");Text raw=items[i];int columns=network?15:18;Text label=int(raw.size())>columns?raw.substr(0,columns-3)+"...":raw;
   if(i==index)label=decodeSelected(label,num(s.revision)+":"+s.page+":"+num(index)+":"+raw,s.now);
   f.text(11,yy,label);if(network){Text rssi=num(s.networks[i].rssi);f.text(rssi.size()>3?102:108,yy,rssi);}
  }selectionBar(y);footer(!s.demo&&s.scanError?"SCAN ERR BK:MENU":hint);
 }
 void breach(const Cyberdeck& s){
  const auto& g=s.game;int seconds=(g.remaining(s.now)+999)/1000;
  f.text(2,0,"BREACH");f.text(103,0,pad2(seconds)+"s");f.text(2,8,"SEQ");f.text(49,8,g.sequence());f.line(0,16,127,16);auto legal=g.legal();
  for(int i=0;i<9;i++){int x=2+i%3*20,y=20+i/3*12;bool used=std::find(g.used.begin(),g.used.end(),i)!=g.used.end();if(g.cursor==i){f.rect(x,y,18,11,true);f.text(x+3,y+2,g.matrix[i],0);}else{if(std::find(legal.begin(),legal.end(),i)!=legal.end())f.reticle(x,y,18,11);f.text(x+3,y+2,g.matrix[i]);if(used){f.line(x,y+10,x+17,y);f.set(x+17,y+10);}}}
  f.line(64,19,64,55);f.text(70,19,"BUFFER");for(int i=0;i<4;i++){int x=70+i%2*28,y=29+i/2*10;f.text(x,y,i<int(g.buffer.size())?g.buffer[i]:"--");if(i==int(g.buffer.size()))f.line(x,y+8,x+10,y+8);}
  int progress=g.progress();for(int i=0;i<3;i++)f.rect(70+i*9,50,6,4,i<progress);f.text(105,48,num(g.buffer.size())+"/4");f.text(2,56,(g.axis=="row"?"ROW ":"COL ")+num(g.axisIndex+1));f.text(86,56,"OK:ADD");
 }
 void renderPage(const Cyberdeck& s){
  uint64_t t=s.now;const auto& n=s.node();
  if(s.page=="boot"){startup(s);return;}
  if(s.page=="idle"){city(reducedMotion?0:t);fullLogo(reducedMotion?2000:t);return;}
  if(s.page=="menu"){renderMenu(s);return;}
  if(s.page=="scan"){
   header("NET SCANNER");f.text(3,21,"FOUND: "+pad2(s.scanCount)+" NODES");
   for(int x=8;x<125;x+=19){f.line(x,39,std::min(123,x+19),39);f.line(x,39,x,48);f.rect(x-1,38,3,3);f.set(x,48);}int x=8+int((t-s.scanStartedAt)/180%7)*19;f.rect(x-2,37,5,5,true);
   footer(s.demo?"DEMO SURVEY BK:MENU":s.scanWaiting?"WAIT "+num(s.scanWaitSeconds)+"s BK:MENU":"2.4GHz SCAN BK:MENU");return;
  }
  if(s.page=="netlist"||s.page=="trackerSelect"||s.page=="securitySelect"){
   Text hint=s.page=="netlist"?"OK:DETAIL BK:MENU":s.page=="trackerSelect"?"OK:TRACK BK:MENU":"OK:DATA BK:MENU";
   Text title=s.page=="netlist"?"NET "+pad2(s.nodeIndex+1)+"/"+pad2(s.networks.size()):"SELECT NODE";std::vector<Text> names;for(const auto& net:s.networks)names.push_back(nodeName(net));list(names,s.nodeIndex,s,true,title,hint);return;
  }
  if(s.page=="details"){header("NODE / DATA");Text name=nodeName(n);f.text(2,19,name.substr(0,21));f.text(2,28,name.size()>21?name.substr(21,21):"");f.text(2,38,"RSSI: "+num(n.rssi)+" dBm");f.text(2,46,"CH:"+num(n.channel)+"  2.4GHz");footer("OK:DATA BK:LIST");return;}
  if(s.page=="security"){
   header("DATA "+num(s.dataTab+1)+"/2");if(s.dataTab==0){Text name=nodeName(n);f.text(2,19,name.substr(0,21));f.text(2,28,name.size()>21?name.substr(21,21):"");f.text(2,37,"BSSID");f.text(2,46,n.bssid);}else{f.label(2,19,"ENC: "+n.encryption,21);f.text(2,30,"CH: "+num(n.channel)+" / 2.4GHz");f.text(2,41,"RSSI: "+num(n.rssi)+" dBm");}footer("OK:NEXT BK:BACK");return;
  }
  if(s.page=="tracker"){
   header(s.trackerMissing?"SIG / LAST":"SIG TRACKER");f.label(2,19,nodeName(n),21);constexpr int left=2,right=125,top=29,bottom=45;
   for(int x=left;x<=right;x+=8){f.set(x,top);f.set(x,bottom);}for(int y=top;y<=bottom;y+=4)f.set(left,y);
   auto yy=[](int v){return rounded(bottom-(clamp(v,-100,-30)+100)/70.0*(bottom-top));};int start=std::max(0,int(s.history.size())-1);
   for(int i=1;i<int(s.history.size());i++){int x0=right-(start-i+1)*2,x1=right-(start-i)*2;if(x0>=left)f.line(x0,yy(s.history[i-1]),x1,yy(s.history[i]));}
   if(!s.history.empty())f.rect(right-1,yy(s.history.back())-1,3,3,true);
   if(!s.demo&&s.history.empty())f.text(16,35,"WAIT FOR SAMPLE");
   f.text(2,46,num(s.liveRssi)+"dBm CH"+num(n.channel)+" Q"+num(quality(s.liveRssi))+"%");footer(s.trackerMissing?"LOST / BK:LIST":"UP/DN:NODE BK:LIST");return;
  }
  if(s.page=="system"){
   header("SYS MONITOR");auto sys=s.metrics();f.text(2,19,"HEAP "+num(sys.heap)+"/"+num(sys.total)+" KiB");f.rect(2,30,123,4);f.rect(4,31,clamp(rounded(double(sys.heap)/std::max(1,sys.total)*119),0,119),2,true);f.label(2,37,"UP "+uptime(t),21);f.text(2,46,"AP "+sys.ap+" / STA "+sys.sta);footer(s.demo?"DEMO METRICS BK:MENU":"LIVE METRICS BK:MENU");return;
  }
  if(s.page=="breachIntro"){header("BREACH");f.text(2,19,"3x3 / 4 BUFFER SLOTS");f.text(2,29,"SEQ: "+s.game.sequence());f.text(2,38,"TOP ROW FIRST");f.text(2,46,"30s ON FIRST PICK");footer("OK:START BK:MENU");return;}
  if(s.page=="abort"){
   header("ABORT BREACH");f.text(2,20,"TIMER KEEPS RUNNING");for(int i=0;i<2;i++){Text label=i==0?"RESUME":"ABORT";f.text(16,33+i*11,i==s.abortIndex?decodeSelected(label,num(s.revision)+":abort:"+num(i),s.now):label);}selectionBar(33+s.abortIndex*11);footer("OK:CHOOSE BK:RESUME");return;
  }
  if(s.page=="success"||s.page=="failure"){
   bool ok=s.page=="success";header(ok?"BREACH OK":"BREACH FAIL");f.reticle(3,20,122,30);f.text(10,25,ok?"DAEMON UPLOADED":"BREACH FAILED");f.text(10,38,ok?"SEQUENCE MATCH":s.game.reason);if(!reducedMotion&&t-s.pageAt<500){int edge=20+int((t-s.pageAt)*30/500);f.line(4,edge,123,edge);}footer("OK:RETRY BK:MENU");return;
  }
  if(s.page=="breach")breach(s);
 }
 const Framebuffer& render(const Cyberdeck& s){
  logoTime=s.now>=s.bootStartedAt+1600?s.now-s.bootStartedAt-1600:0;
  bool wasPower=lastPage=="boot"||lastPage=="off"||lastPage=="shutdown";
  if(s.page=="off"||s.page=="boot"||s.page=="shutdown"){
   if(lastPage!=s.page&&s.page=="shutdown")previous=f.bits;
   lastPage=s.page;hasPrevious=false;f.clear();
   if(s.page=="boot")startup(s);
   if(s.page=="shutdown"&&!reducedMotion){uint64_t t=s.now-s.powerAt;
    if(t<510){double p=ease(t/510.0);scaleFrame(previous,1-p*.15,std::max(.015,1-p));}
    else if(t<760){int half=rounded((1-ease((t-510)/250.0))*53);if(half)f.line(64-half,31,63+half,31);}
    else if(t<POWER_OFF_MS-40)f.set(64,31);
   }lastTime=s.now;return f;
  }
  if(wasPower){lastPage=s.page;hasPrevious=false;}
  if(lastPage!=s.page){hasPrevious=!lastPage.empty();if(hasPrevious)previous=f.bits;transitionAt=s.now;cursorY=20;lastPage=s.page;}
  f.clear();renderPage(s);uint64_t dt=s.now-transitionAt;
  bool exempt=s.page=="boot"||s.page=="idle"||s.page=="breach"||s.page=="success"||s.page=="failure";
  if(hasPrevious&&!reducedMotion&&!exempt&&dt<180){workA=f.bits;int offset=std::max(1,rounded((1-dt/180.0)*128));for(int y=16;y<64;y++)for(int x=0;x<128;x++)f.set(x,y,x<offset?pixel(previous,x+128-offset,y):pixel(workA,x-offset,y));}
  lastTime=s.now;return f;
 }
};
}


// ---- lab_core.h ----

#include <stdint.h>
#include <stddef.h>

namespace Lab {
constexpr uint32_t DEBOUNCE_MS = 30;
constexpr int VISIBLE_ROWS = 3;
namespace Oled {
constexpr int HEADER_HEIGHT = 16;
constexpr int HEADER_TEXT_Y = 4;
constexpr int CONTENT_Y = 19;
constexpr int TEXT_ROW_HEIGHT = 9;
constexpr int LIST_TEXT_Y = 20;
constexpr int LIST_ROW_HEIGHT = 11;
constexpr int FOOTER_Y = 54;
}
enum class Page { List, Details, Message, Drawing };
constexpr int DRAWING_WIDTH = 128, DRAWING_HEIGHT = 64;
constexpr size_t DRAWING_BYTES = DRAWING_WIDTH * DRAWING_HEIGHT / 8;

constexpr int hexDigit(char c) {
  return c >= '0' && c <= '9' ? c - '0' :
         c >= 'a' && c <= 'f' ? c - 'a' + 10 :
         c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
}

// Row-major, leftmost pixel in bit 7: Adafruit_GFX drawBitmap format.
// Validate everything first so rejected input cannot corrupt the current picture.
constexpr bool decodeDrawing(const char* hex, size_t length, uint8_t* out) {
  if (length != DRAWING_BYTES * 2) return false;
  for (size_t i = 0; i < length; ++i) if (hexDigit(hex[i]) < 0) return false;
  for (size_t i = 0; i < DRAWING_BYTES; ++i)
    out[i] = uint8_t((hexDigit(hex[i * 2]) << 4) | hexDigit(hex[i * 2 + 1]));
  return true;
}

// GPIO-independent logic: the same code is exercised by firmware self-tests.
struct Button {
  bool raw = false, pressed = false;
  uint32_t changedAt = 0, count = 0;
  constexpr void begin(bool initial, uint32_t now) {
    raw = pressed = initial;
    changedAt = now;
    count = 0;
  }
  constexpr bool update(bool sample, uint32_t now) {
    if (sample != raw) { raw = sample; changedAt = now; }
    if (pressed != raw && uint32_t(now - changedAt) >= DEBOUNCE_MS) {
      pressed = raw;
      if (pressed) { ++count; return true; }
    }
    return false;
  }
};

struct UI {
  Page page = Page::List;
  int count = 0, selected = 0, first = 0;
  uint32_t messageAt = 0;
  constexpr void results(int n) {
    count = n > 0 ? n : 0;
    selected = first = 0;
    if (page == Page::Details) page = Page::List;
  }
  constexpr void back() { page = Page::List; }
  constexpr void showMessage(uint32_t now) { page = Page::Message; messageAt = now; }
  constexpr void showDrawing(uint32_t now) { page = Page::Drawing; messageAt = now; }
  constexpr bool temporary() const { return page == Page::Message || page == Page::Drawing; }
  constexpr bool expire(uint32_t now, uint32_t duration) {
    if (!temporary() || uint32_t(now - messageAt) < duration) return false;
    back(); return true;
  }
  constexpr bool input(bool up, bool down, bool ok) {
    if (ok) {
      if (page != Page::List) back();
      else if (count) page = Page::Details;
      return true;
    }
    if (page != Page::List || up == down || !count) return false;
    int next = selected + (down ? 1 : -1);
    if (next < 0 || next >= count) return false;
    selected = next;
    if (selected < first) first = selected;
    if (selected >= first + VISIBLE_ROWS) first = selected - VISIBLE_ROWS + 1;
    return true;
  }
};

// At size 1, 21 columns x 4 rows fit above the OLED footer.
constexpr bool validMessage(const char* text, size_t length) {
  if (!length || length > 84) return false;
  int row = 0, column = 0;
  bool visible = false;
  for (size_t i = 0; i < length; ++i) {
    const uint8_t c = uint8_t(text[i]);
    if (c == '\n') { ++row; column = 0; }
    else {
      if (c < 32 || c > 126) return false;
      if (column == 21) { ++row; column = 0; }
      ++column;
      if (c != ' ') visible = true;
    }
    if (row >= 4) return false;
  }
  return visible;
}
}


// ---- web_page.h ----

#include <Arduino.h>
static const char WEB_PAGE[] PROGMEM=R"LABHTML(<!doctype html>
<html lang="ar" dir="rtl">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta name="theme-color" content="#101b2c">
<title>ESP32 LAB · لوحة التحكم</title>
<style>
:root{color-scheme:dark;--bg:#0b1320;--panel:#131f30;--line:#2a394e;--text:#eef4fc;--muted:#a9b9cf;--accent:#68e0ba;--warn:#ffd28a;--bad:#ffaca7}
*{box-sizing:border-box}body{margin:0;background:radial-gradient(ellipse at top left,#19364a 0,transparent 55%),var(--bg);color:var(--text);font:16px/1.65 system-ui,-apple-system,"Segoe UI",sans-serif;min-height:100vh}main{max-width:1040px;margin:auto;padding:28px 18px 46px}header{display:flex;align-items:center;justify-content:space-between;gap:18px;margin-bottom:24px}.brand{font-size:13px;color:var(--accent);letter-spacing:.17em;margin:0}h1{font-size:clamp(24px,4vw,34px);line-height:1.4;margin:4px 0}h2{font-size:19px;margin:0 0 14px}.sub,small,.muted{color:var(--muted)}.sub{margin:0}.badge{border:1px solid var(--line);border-radius:30px;padding:6px 13px;white-space:nowrap;font-size:14px}.badge.online{color:var(--accent)}.badge.offline{color:var(--bad)}.grid{display:grid;grid-template-columns:1fr 1fr;gap:16px}.card{background:var(--panel);border:1px solid var(--line);border-radius:18px;padding:22px;min-width:0}.wide{grid-column:1/-1}.note{padding:11px 14px;border-radius:10px;background:#213347;color:var(--muted);font-size:14px;margin:14px 0 0}.warning{color:var(--warn)}.demo{background:#4d351c;color:var(--warn);border:1px solid #a77936;margin:0 0 18px;padding:12px;border-radius:12px}[hidden]{display:none!important}.buttons{display:flex;gap:10px;direction:ltr}.physical{flex:1;text-align:center;border:1px solid var(--line);border-radius:12px;padding:14px 4px;transition:background .12s}.physical strong{display:block;letter-spacing:.08em}.physical span{display:block;font-size:13px;color:var(--muted)}.physical.pressed{background:#245549;border-color:var(--accent)}.physical.pulse{box-shadow:inset 0 0 0 2px var(--accent)}.physical small{font-variant-numeric:tabular-nums}label{display:block;margin-bottom:7px}textarea{width:100%;min-height:112px;resize:vertical;background:var(--bg);color:var(--text);border:1px solid #53657d;border-radius:10px;padding:12px;font:16px/1.5 ui-monospace,monospace}textarea:focus{outline:2px solid var(--accent);outline-offset:2px}.actions{display:flex;gap:9px;flex-wrap:wrap;align-items:center;margin-top:12px}button{cursor:pointer;border:1px solid transparent;border-radius:10px;background:var(--accent);color:#102720;font:600 15px/1.5 system-ui;padding:10px 16px;min-height:44px}button.secondary{background:#243449;border-color:#52657e;color:var(--text)}button:disabled{opacity:.45;cursor:wait}button:focus-visible{outline:2px solid white;outline-offset:3px}#feedback{min-height:26px;font-size:14px;margin:10px 0 0}#feedback.error{color:var(--bad)}.scan-head{display:flex;justify-content:space-between;gap:16px;align-items:flex-start}.scan-head h2{margin-bottom:4px}.table-wrap{overflow-x:auto}table{border-collapse:collapse;width:100%;margin-top:16px;font-size:15px}th,td{text-align:right;padding:11px 10px;border-bottom:1px solid var(--line)}th{color:var(--muted);font-size:13px;font-weight:500}td:first-child{max-width:400px;overflow-wrap:anywhere}tr.selected{background:#1a3a39}td.num,th.num{text-align:center;white-space:nowrap;font-variant-numeric:tabular-nums}#empty{text-align:center;color:var(--muted);padding:18px}.stats{display:grid;grid-template-columns:1fr 1fr;gap:12px}.stat{padding-bottom:9px;border-bottom:1px solid var(--line)}.stat span{display:block;color:var(--muted);font-size:13px}.stat b{font-size:15px;font-weight:500;overflow-wrap:anywhere}.ltr{direction:ltr;unicode-bidi:isolate}footer{color:var(--muted);font-size:13px;text-align:center;margin-top:22px}#scan-error{color:var(--bad);margin:8px 0}#screen-state{color:var(--accent);font-size:14px}
@media(max-width:680px){main{padding:20px 12px 30px}.grid{grid-template-columns:1fr}.card{padding:18px}header{align-items:flex-start;gap:8px}.badge{font-size:12px;padding:5px 9px}.scan-head{gap:8px}th,td{padding:10px 5px}.scan-head button{font-size:13px;padding:9px 12px}.stats{gap:10px}}
.canvas-wrap{border:2px solid #53657d;border-radius:10px;overflow:hidden;background:#000;margin-top:12px;direction:ltr}#drawing{display:block;width:100%;height:auto;aspect-ratio:2;image-rendering:pixelated;touch-action:none;cursor:crosshair;user-select:none}.draw-tools{display:flex;flex-wrap:wrap;gap:8px;align-items:center;margin-top:12px}.draw-tools label{margin:0}.draw-tools select{font:inherit;background:#243449;color:var(--text);padding:8px;border:1px solid #52657e;border-radius:8px;min-height:44px}.draw-tools button[aria-pressed="true"]{background:var(--accent);color:#102720}.text-option{margin-top:18px;border-top:1px solid var(--line);padding-top:12px}.text-option summary{cursor:pointer;color:var(--muted);padding:5px 0}.text-option form{margin-top:12px}
</style>
</head>
<body><main>
<header><div><p class="brand" dir="ltr">ESP32 / LAB</p><h1>مختبرك، متصل.</h1><p class="sub">شبكات Wi-Fi، شاشة OLED، وأزرارك الثلاثة.</p></div><span id="connection" class="badge" role="status">جارٍ الاتصال…</span></header>
<p id="demo" class="demo" hidden>وضع DEMO: نتائج الشبكات مختلقة لاختبار التنقل، وليست قياسات لاسلكية.</p>
<div class="grid">
<section class="card"><h2>ارسم على OLED</h2>
<small id="drawing-help">ارسم بإصبعك داخل المساحة السوداء، ثم اضغط «إرسال الرسم». الأبيض يضيء على الشاشة.</small>
<div class="canvas-wrap"><canvas id="drawing" width="128" height="64" role="img" aria-label="مساحة رسم باللمس بدقة 128 في 64" aria-describedby="drawing-help"></canvas></div>
<div class="draw-tools"><label for="brush">حجم القلم</label><select id="brush"><option value="1">رفيع</option><option value="3" selected>متوسط</option><option value="5">عريض</option></select><button id="eraser" class="secondary" type="button" aria-pressed="false">ممحاة</button><button id="undo-drawing" class="secondary" type="button" disabled>تراجع</button><button id="clear-drawing" class="secondary" type="button">مسح الكل</button></div>
<div class="actions"><button id="send-drawing" type="button" disabled>إرسال الرسم</button><button id="reset" class="secondary" type="button" disabled>العودة إلى الماسح</button></div>
<details class="text-option"><summary>إرسال نص بدل الرسم (اختياري)</summary>
<form id="text-form"><label for="text">النص الإنجليزي</label><textarea id="text" name="text" maxlength="84" dir="ltr" spellcheck="false" placeholder="Hello from ESP32 LAB!" aria-describedby="text-help"></textarea>
<small id="text-help">ASCII إنجليزي فقط · 4 أسطر × 21 حرفًا · 84 حرفًا كحد أقصى. العربية غير مدعومة على OLED.</small>
<div class="actions"><button id="send" type="submit" disabled>إرسال النص</button></div></form></details>
<p id="feedback" role="status" aria-live="polite"></p><div id="screen-state">حالة الشاشة: —</div><p class="note">يظهر الرسم بكامل OLED لمدة 20 ثانية من آخر إرسال. بعدها تعود واجهة الجهاز تلقائيًا. للرجوع فورًا اضغط ENTER أو BACK أو زر العودة هنا.</p></section>
<section class="card"><h2>الأزرار · مباشر</h2><div class="buttons">
<div class="physical" id="button-UP"><strong>UP</strong><span class="state">—</span><small class="count">0 ضغطات</small></div>
<div class="physical" id="button-DOWN"><strong>DOWN</strong><span class="state">—</span><small class="count">0 ضغطات</small></div>
<div class="physical" id="button-OK"><strong>ENTER</strong><span class="state">—</span><small class="count">0 ضغطات</small></div>
<div class="physical" id="button-BACK"><strong>BACK</strong><span class="state">—</span><small class="count">0 ضغطات</small></div></div><p class="note">تحديث كل 400ms تقريبًا. عداد الضغطات يحفظ الضغطات القصيرة بين التحديثات؛ تثبيت الحالة لمدة 30ms لمعالجة اهتزاز التلامس.</p><p class="muted">UP / DOWN للتنقل. ENTER للفتح وBACK للرجوع. أعد الفحص من NET SCANNER في القائمة الرئيسية.</p></section>
<section class="card wide"><div class="scan-head"><div><h2>الشبكات المحيطة <span class="muted ltr">2.4 GHz</span></h2><small id="scan-state">بانتظار الجهاز…</small></div><button id="scan" class="secondary" type="button" disabled>إعادة الفحص</button></div>
<p class="note warning">أثناء الفحص قد تتأخر الطلبات أو ينقطع اتصال الهاتف مؤقتًا بسبب مشاركة الراديو. تحاول الصفحة الاتصال مجددًا تلقائيًا. النتائج المعروضة هي آخر نتائج مكتملة.</p>
<p id="scan-error" role="status"></p><div class="table-wrap"><table><thead><tr><th>SSID / اسم الشبكة</th><th class="num">RSSI · dBm</th><th class="num">القناة</th></tr></thead><tbody id="networks"></tbody></table></div><div id="empty">لم تصل نتائج بعد.</div><small>الصف المظلل هو اختيار OLED. تُعرض حتى 32 نتيجة؛ كل BSSID نتيجة مستقلة. الأسماء غير ASCII تظهر كقيم بايتات.</small></section>
<section class="card wide"><h2>الجهاز والاتصال</h2><div id="device" class="stats"></div><p class="note">هذه شبكة محلية للتحكم فقط، دون اتصال بالإنترنت. يمكن للأجهزة المتصلة بـ ESP32-LAB استخدام لوحة التحكم.</p></section>
</div><footer><span class="ltr">ESP32-LAB · http://192.168.4.1</span> · واجهة مستقلة دون مكتبات أو خطوط خارجية</footer>
</main>
<script>
'use strict';
const $ = id => document.getElementById(id);
let online = false, busy = false, lastRevision = -1, rows = [], lastStatus = null, lastBoot = null;
const counts = {}, pulses = {};
async function request(path, options = {}) {
  const controller = new AbortController();
  const timeout = setTimeout(() => controller.abort(), 4500);
  try {
    const response = await fetch(path, {...options, cache:'no-store', signal:controller.signal});
    const data = await response.json();
    if (!response.ok) throw new Error(data.error || `HTTP ${response.status}`);
    return data;
  } finally { clearTimeout(timeout); }
}
function feedback(text, error = false) { $('feedback').textContent = text; $('feedback').classList.toggle('error', error); }
function controls() {
  $('send').disabled = $('send-drawing').disabled = $('reset').disabled = !online || busy;
  const scan = lastStatus && lastStatus.scan;
  $('scan').disabled = !online || busy || !scan || scan.running || scan.queued || scan.retryAfterMs > 0;
}
function validText(text) {
  if (!text.length || text.length > 84 || !/^[\x20-\x7e\n]+$/.test(text) || !/[^ \n]/.test(text)) return false;
  let row = 0, col = 0;
  for (const c of text) {
    if (c === '\n') { row++; col = 0; }
    else { if (col === 21) { row++; col = 0; } col++; }
    if (row >= 4) return false;
  }
  return true;
}
function renderNetworks(data) {
  const fragment = document.createDocumentFragment();
  rows = data.networks.map((network, index) => {
    const tr = document.createElement('tr');
    tr.dataset.index = index;
    const name = document.createElement('td'); name.dir = 'ltr';
    name.textContent = network.ssid || '<hidden>'; name.title = network.bssid;
    tr.appendChild(name);
    for (const value of [network.rssi, network.channel]) {
      const td = document.createElement('td'); td.className = 'num'; td.dir = 'ltr'; td.textContent = String(value); tr.appendChild(td);
    }
    fragment.appendChild(tr); return tr;
  });
  $('networks').replaceChildren(fragment);
  $('empty').hidden = rows.length > 0;
  $('empty').textContent = 'لا توجد شبكات في آخر فحص مكتمل.';
  lastRevision = data.revision;
}
function renderStatus(s) {
  lastStatus = s; online = true;
  $('connection').textContent = 'متصل بالجهاز'; $('connection').className = 'badge online';
  $('demo').hidden = !s.demo;
  for (const name of ['UP','DOWN','OK','BACK']) {
    const b = s.buttons[name], el = $('button-' + name);
    el.classList.toggle('pressed', b.pressed);
    el.querySelector('.state').textContent = b.pressed ? 'مضغوط' : 'محرر';
    el.querySelector('.count').textContent = b.count + ' ضغطات';
    if (counts[name] !== undefined && b.count > counts[name]) {
      el.classList.add('pulse'); clearTimeout(pulses[name]);
      pulses[name] = setTimeout(() => el.classList.remove('pulse'), 700);
    }
    counts[name] = b.count;
  }
  const scan = s.scan;
  $('scan-state').textContent = (scan.running || scan.queued ? 'جارٍ الفحص… ' : '') + (scan.hasResults ? `${scan.count} / ${scan.totalFound} نتيجة · عمر النتائج ${Math.floor(scan.ageMs/1000)}ث` : 'لم يكتمل فحص بعد') + (scan.retryAfterMs ? ` · إعادة الفحص بعد ${Math.ceil(scan.retryAfterMs/1000)}ث` : '');
  $('scan-error').textContent = scan.error;
  const page = {off:'مطفأ',shutdown:'إطفاء الجهاز',boot:'الإقلاع',menu:'القائمة الرئيسية',idle:'الانتظار',scan:'فحص الشبكات',netlist:'قائمة الشبكات',trackerSelect:'اختيار شبكة للإشارة',securitySelect:'اختيار شبكة للبيانات',tracker:'مراقبة الإشارة',security:'ملف الشبكة',system:'مراقبة النظام',breachIntro:'استعداد Breach',breach:'Breach Protocol',abort:'تأكيد الخروج',success:'نجاح Breach',failure:'فشل Breach',list:'قائمة الشبكات',details:'تفاصيل الشبكة',message:'رسالة مؤقتة',drawing:'الرسم'}[s.page] || s.page;
  $('screen-state').textContent = s.oledReady ? `حالة الشاشة: ${page}` + (s.page === 'message' || s.page === 'drawing' ? ` · ${Math.ceil((s.displayRemainingMs ?? s.messageRemainingMs)/1000)}ث` : '') : 'OLED غير متاحة: افحص التوصيلات والعنوان 0x3C';
  rows.forEach((tr, i) => tr.classList.toggle('selected', i === s.selected));
  const stats = [ ['شبكة الجهاز',s.apSsid],['عنوان AP',s.apIp],['حالة AP',s.apReady?'جاهزة':'فشل الإعداد'],['عملاء AP',s.clients],['المعالج',`${s.chip} · Rev ${s.revision}`],['التردد والأنوية',`${s.cpuMHz} MHz · ${s.cores} cores`],['الذاكرة الحرة',`${Math.round(s.heapBytes/1024)} KiB`],['حجم Flash',`${s.flashBytes/1048576} MiB`],['مدة التشغيل',`${Math.floor(s.uptimeMs/1000)} s`],['STA',s.staConnected?s.staIp:'للفحص فقط'] ];
  const fragment = document.createDocumentFragment();
  for (const [label,value] of stats) {
    const box = document.createElement('div'); box.className='stat';
    const key=document.createElement('span'); key.textContent=label;
    const val=document.createElement('b'); val.textContent=String(value); val.className='ltr';
    box.append(key,val); fragment.appendChild(box);
  }
  $('device').replaceChildren(fragment); controls();
}
async function poll() {
  let delay = 400;
  try {
    const s = await request('/api/status');
    if (s.bootId !== lastBoot) {
      lastRevision=-1; lastBoot=s.bootId;
      for (const name of ['UP','DOWN','OK','BACK']) { delete counts[name]; clearTimeout(pulses[name]); $('button-'+name).classList.remove('pulse'); }
    }
    if (lastRevision !== s.scan.revision) renderNetworks(await request('/api/networks'));
    renderStatus(s);
  } catch (error) {
    online = false;
    $('connection').textContent = 'غير متصل · إعادة محاولة'; $('connection').className = 'badge offline';
    for (const name of ['UP','DOWN','OK','BACK']) {
      const el=$('button-'+name); el.classList.remove('pressed'); el.querySelector('.state').textContent='غير محدث';
    }
    $('scan-state').textContent='الاتصال متوقف؛ النتائج السابقة محفوظة.';
    controls(); delay=1500;
  } finally { setTimeout(poll,delay); }
}
async function action(path, options, success) {
  if (busy || !online) return;
  busy=true; controls();
  try { await request(path,options); feedback(success); }
  catch(error) { feedback('تعذّر تأكيد الطلب: '+error.message+' — تحقق من حالة الجهاز قبل إعادة الإرسال.',true); }
  finally { busy=false; controls(); }
}
$('text-form').addEventListener('submit',event=>{
  event.preventDefault();
  const text=$('text').value.replace(/\r\n?/g,'\n');
  if(!validText(text)) { feedback('أدخل نص ASCII إنجليزيًا يناسب 4 أسطر × 21 حرفًا، بحد أقصى 84 حرفًا.',true); return; }
  action('/api/text',{method:'POST',body:new URLSearchParams({text})},'استلم الجهاز الرسالة؛ تحقق من حالة الشاشة.');
});
$('reset').addEventListener('click',()=>action('/api/display/reset',{method:'POST'},'عاد العرض إلى ماسح الشبكات.'));
$('scan').addEventListener('click',()=>action('/api/scan',{method:'POST'},'تم طلب الفحص. ستظهر النتائج عند اكتماله.'));

// One monochrome pixel model drives both preview and transmitted bitmap.
const WIDTH=128, HEIGHT=64, canvas=$('drawing'), ctx=canvas.getContext('2d');
let pixels=new Uint8Array(WIDTH*HEIGHT), drawingHistory=[], pointer=null, previous=null, erasing=false;
function paintPreview() {
  ctx.fillStyle='#000'; ctx.fillRect(0,0,WIDTH,HEIGHT);
  ctx.fillStyle='#fff';
  for(let y=0;y<HEIGHT;y++) for(let x=0;x<WIDTH;x++) if(pixels[y*WIDTH+x]) ctx.fillRect(x,y,1,1);
  $('undo-drawing').disabled=drawingHistory.length===0;
}
function rememberDrawing() {
  drawingHistory.push(pixels.slice()); if(drawingHistory.length>20)drawingHistory.shift();
}
function drawingPoint(event) {
  const rect=canvas.getBoundingClientRect();
  return {x:Math.max(0,Math.min(WIDTH-1,Math.floor((event.clientX-rect.left)*WIDTH/rect.width))), y:Math.max(0,Math.min(HEIGHT-1,Math.floor((event.clientY-rect.top)*HEIGHT/rect.height)))};
}
function dab(x,y) {
  const radius=(Number($('brush').value)-1)/2;
  for(let yy=y-radius;yy<=y+radius;yy++) for(let xx=x-radius;xx<=x+radius;xx++)
    if(xx>=0&&xx<WIDTH&&yy>=0&&yy<HEIGHT)pixels[yy*WIDTH+xx]=erasing?0:1;
}
function drawLine(a,b) {
  let x=a.x,y=a.y,dx=Math.abs(b.x-x),dy=-Math.abs(b.y-y),sx=x<b.x?1:-1,sy=y<b.y?1:-1,err=dx+dy;
  for(;;) {
    dab(x,y); if(x===b.x&&y===b.y)break;
    const e2=2*err; if(e2>=dy){err+=dy;x+=sx;} if(e2<=dx){err+=dx;y+=sy;}
  }
}
function endDrawing() {
  const captured=pointer; pointer=null; previous=null;
  if(captured!==null&&canvas.hasPointerCapture(captured))canvas.releasePointerCapture(captured);
}
canvas.addEventListener('pointerdown',event=>{
  if(pointer!==null||event.button!==0)return;
  event.preventDefault(); rememberDrawing(); pointer=event.pointerId;
  canvas.setPointerCapture(pointer); previous=drawingPoint(event); dab(previous.x,previous.y); paintPreview();
});
canvas.addEventListener('pointermove',event=>{
  if(event.pointerId!==pointer)return;
  event.preventDefault(); const point=drawingPoint(event); drawLine(previous,point); previous=point; paintPreview();
});
for(const name of ['pointerup','pointercancel','lostpointercapture'])canvas.addEventListener(name,event=>{
  if(event.pointerId===pointer)endDrawing();
});
$('eraser').addEventListener('click',()=>{erasing=!erasing;$('eraser').setAttribute('aria-pressed',String(erasing));});
$('clear-drawing').addEventListener('click',()=>{endDrawing();if(pixels.some(p=>p)){rememberDrawing();pixels.fill(0);paintPreview();}});
$('undo-drawing').addEventListener('click',()=>{endDrawing();if(drawingHistory.length){pixels=drawingHistory.pop();paintPreview();}});
function drawingHex() {
  const bytes=new Uint8Array(WIDTH*HEIGHT/8);
  for(let i=0;i<pixels.length;i++)if(pixels[i])bytes[i>>3]|=0x80>>(i&7);
  return Array.from(bytes,b=>b.toString(16).padStart(2,'0')).join('');
}
$('send-drawing').addEventListener('click',()=>{
  endDrawing();
  return action('/api/drawing',{method:'POST',body:new URLSearchParams({bitmap:drawingHex()})},'استلم الجهاز الرسم؛ سيظهر لمدة 20 ثانية.');
});
paintPreview();
poll();
</script>
</body></html>
)LABHTML";


// ---- cyberdeck.ino ----
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_wifi.h>
#include <esp_timer.h>




Deck::Cyberdeck deck(DECK_DEMO);
Deck::OledRenderer renderer(Config::REDUCED_MOTION);
Deck::Button buttons[4];
Deck::PowerKey backPowerKey;
const uint8_t buttonPins[]={Config::UP_PIN,Config::DOWN_PIN,Config::ENTER_PIN,Config::BACK_PIN};
const char* inputNames[]={"up","down","enter","back"};
const char* buttonNames[]={"UP","DOWN","OK","BACK"};
WebServer server(80);
bool oledReady=false,apReady=false;
bool radioSuspended=false,oledSleeping=false;
uint32_t bootId=0,apiRevision=0;
uint64_t epoch=0,lastPaint=0,lastMetrics=0;
int overlayKind=0;uint64_t overlayAt=0;String message;
uint8_t drawing[1024]{};

enum class RadioJob {None,Full,Track};
RadioJob radioJob=RadioJob::None;
bool fullAttempted=false,fullPending=false;
uint64_t radioAt=0,lastFullAt=0,lastTrackAt=0,radioGuardUntil=0,scanFinishedAt=0;
uint32_t radioGeneration=0,scanGeneration=0;
int totalFound=0;Deck::Text scanErrorText,trackedBssid;
std::array<uint8_t,6> trackedMac{};
std::vector<Deck::Network> pendingNetworks;

uint64_t clockMs(){return uint64_t(esp_timer_get_time())/1000-epoch;}
bool gameActive(){return deck.page=="breach"||deck.page=="abort";}
bool uiPowerLocked(){return deck.powerLocked();}
void beginFullScan(const Deck::Text& destination="netlist"){scanGeneration++;deck.beginScan(destination);}
uint32_t scanRetryMs(uint64_t now){if(!fullAttempted||now-lastFullAt>=Config::SCAN_COOLDOWN_MS)return 0;return Config::SCAN_COOLDOWN_MS-uint32_t(now-lastFullAt);}
bool oledCommand(const uint8_t* bytes,size_t n){Wire.beginTransmission(Config::OLED_ADDRESS);Wire.write(uint8_t(0));Wire.write(bytes,n);return Wire.endTransmission()==0;}
bool beginOled(){
 Wire.begin(Config::SDA_PIN,Config::SCL_PIN);Wire.setClock(Config::I2C_HZ);Wire.setTimeOut(20);
 Wire.beginTransmission(Config::OLED_ADDRESS);if(Wire.endTransmission()!=0)return false;
 const uint8_t init[]={0xAE,0xD5,0x80,0xA8,0x3F,0xD3,0,0x40,0x8D,0x14,0x20,0,0xA1,0xC8,0xDA,0x12,0x81,0x7F,0xD9,0xF1,0xDB,0x40,0xA4,0xA6,0x2E,0xAF};
 return oledCommand(init,sizeof(init));
}
void showFrame(const Deck::Framebuffer& f){
 if(!oledReady)return;
 const uint8_t range[]={0x21,0,127,0x22,0,7};if(!oledCommand(range,sizeof(range))){oledReady=false;Serial.println("OLED disconnected; web/buttons continue");return;}uint8_t block[16];
 // Exact row-major preview pixels -> SSD1306 vertical page bytes.
 for(int page=0;page<8;page++)for(int x=0;x<128;x+=16){
  for(int dx=0;dx<16;dx++){uint8_t bits=0;for(int bit=0;bit<8;bit++)bits|=uint8_t(f.get(x+dx,page*8+bit)<<bit);block[dx]=bits;}
  Wire.beginTransmission(Config::OLED_ADDRESS);Wire.write(0x40);Wire.write(block,sizeof(block));if(Wire.endTransmission()!=0){oledReady=false;Serial.println("OLED transfer failed; check wiring then reset");return;}
 }
}
void paint(){
 const auto& frame=renderer.render(deck);
 if(deck.powerLocked()){showFrame(frame);return;}
 if(!overlayKind){showFrame(frame);return;}
 // Keep the animator's previous-frame buffer independent from phone overlays.
 static Deck::Framebuffer overlay;overlay.clear();
 if(overlayKind==2)memcpy(overlay.bits.data(),drawing,1024);
 else{overlay.text(0,4,"WEB MESSAGE");overlay.line(0,15,127,15);int x=0,y=19;for(size_t i=0;i<message.length();i++){char c=message[i];if(c=='\n'){x=0;y+=9;continue;}if(x>=126){x=0;y+=9;}if(y>=54)break;overlay.text(x,y,Deck::Text(1,c));x+=6;}}
 showFrame(overlay);
}
Deck::Text encryptionName(wifi_auth_mode_t auth){
 switch(auth){case WIFI_AUTH_OPEN:return "OPEN";case WIFI_AUTH_WEP:return "WEP";case WIFI_AUTH_WPA_PSK:return "WPA-PSK";case WIFI_AUTH_WPA2_PSK:return "WPA2-PSK";case WIFI_AUTH_WPA_WPA2_PSK:return "WPA/WPA2-PSK";case WIFI_AUTH_WPA2_ENTERPRISE:return "WPA2-ENTERPRISE";case WIFI_AUTH_WPA3_PSK:return "WPA3-SAE";case WIFI_AUTH_WPA2_WPA3_PSK:return "WPA2/WPA3-PSK";case WIFI_AUTH_OWE:return "OWE";default:return "AUTH "+Deck::num(int(auth));}
}
std::vector<Deck::Network> collectNetworks(int found){
 std::vector<Deck::Network> values;values.reserve(std::min(found,Deck::MAX_NETWORKS));
 // Keep the strongest 32 even if the scan provider returns an unsorted list.
 for(int i=0;i<found;i++){
  Deck::Network n;n.rawSsid=WiFi.SSID(i).c_str();n.ssid=Deck::safeAscii(n.rawSsid);n.bssid=WiFi.BSSIDstr(i).c_str();n.rssi=WiFi.RSSI(i);n.channel=WiFi.channel(i);n.encryption=encryptionName(WiFi.encryptionType(i));WiFi.BSSID(i,n.mac.data());
  if(n.channel<1||n.channel>14)continue;
  values.push_back(std::move(n));std::stable_sort(values.begin(),values.end(),[](const Deck::Network& a,const Deck::Network& b){return a.rssi>b.rssi;});if(values.size()>Deck::MAX_NETWORKS)values.pop_back();
 }return values;
}
void finishRadio(int result,uint64_t now){
 RadioJob done=radioJob;radioJob=RadioJob::None;
 if(result<0){esp_wifi_scan_stop();radioGuardUntil=now+500;}
 if(done==RadioJob::Full){
  if(deck.page=="scan"&&radioGeneration==scanGeneration){
   if(result>=0){pendingNetworks=collectNetworks(result);totalFound=result;deck.scanCount=result;fullPending=true;scanErrorText.clear();}
   else{scanErrorText="Scan failed or timed out";deck.finishScan({},true);apiRevision++;Serial.println("SCAN FAILED: retained previous results");}
  }
 }else if(done==RadioJob::Track&&deck.page=="tracker"&&deck.node().bssid==trackedBssid){
  bool matched=false;for(int i=0;i<result;i++)if(trackedBssid==WiFi.BSSIDstr(i).c_str()){
   int rssi=WiFi.RSSI(i);deck.networks[deck.nodeIndex].rssi=rssi;deck.networks[deck.nodeIndex].channel=WiFi.channel(i);deck.addSample(rssi);matched=true;break;
  }
  deck.trackerMissing=!matched;apiRevision++;
 }
 WiFi.scanDelete();
}
void serviceRadio(uint64_t now){
 if(DECK_DEMO||radioSuspended||deck.powerLocked())return;
 if(radioJob!=RadioJob::None){
  int result=WiFi.scanComplete();
  if(result!=WIFI_SCAN_RUNNING)finishRadio(result,now);
  else if(now-radioAt>=Config::SCAN_TIMEOUT_MS)finishRadio(WIFI_SCAN_FAILED,now);
  return;
 }
 if(fullPending){
  if(deck.page!="scan"||radioGeneration!=scanGeneration){fullPending=false;pendingNetworks.clear();}
  else if(now-deck.scanStartedAt>=Deck::SCAN_MS){deck.finishScan(std::move(pendingNetworks));fullPending=false;scanFinishedAt=now;apiRevision++;Serial.printf("SCAN DONE real=%d retained=%u\n",totalFound,unsigned(deck.networks.size()));}
  return;
 }
 if(now<radioGuardUntil)return;
 if(deck.page=="scan"&&deck.scanRequested){
  uint32_t wait=scanRetryMs(now);deck.scanWaiting=wait>0;deck.scanWaitSeconds=(wait+999)/1000;if(wait)return;
  deck.scanWaiting=false;deck.scanRequested=false;deck.scanStartedAt=now;scanErrorText.clear();fullAttempted=true;lastFullAt=radioAt=now;radioGeneration=scanGeneration;radioJob=RadioJob::Full;
  Serial.println("SCAN START: 2.4GHz AP+STA, no network login");
  int result=WiFi.scanNetworks(true,true,false,120);if(result!=WIFI_SCAN_RUNNING)finishRadio(result,now);return;
 }
 if(deck.page=="tracker"&&!deck.networks.empty()&&(trackedBssid!=deck.node().bssid||now-lastTrackAt>=Config::TRACK_PERIOD_MS||deck.history.empty())){
  // Retry missing nodes at 1 Hz too; never draw invented RSSI samples.
  if(trackedBssid==deck.node().bssid&&now-lastTrackAt<Config::TRACK_PERIOD_MS)return;
  trackedBssid=deck.node().bssid;trackedMac=deck.node().mac;lastTrackAt=radioAt=now;radioJob=RadioJob::Track;
  int result=WiFi.scanNetworks(true,true,false,120,uint8_t(deck.node().channel),nullptr,trackedMac.data());if(result!=WIFI_SCAN_RUNNING)finishRadio(result,now);
 }
}
void cancelRadioJobs(){
 scanGeneration++;if(!DECK_DEMO&&radioJob!=RadioJob::None)esp_wifi_scan_stop();
 if(!DECK_DEMO)WiFi.scanDelete();radioJob=RadioJob::None;fullPending=false;pendingNetworks.clear();
 deck.scanRequested=false;deck.scanWaiting=false;trackedBssid.clear();
}
void startNetwork(){
 WiFi.persistent(false);WiFi.setHostname("cyberdeck");WiFi.mode(WIFI_AP_STA);WiFi.setAutoReconnect(false);WiFi.setSleep(false);WiFi.setScanTimeout(Config::SCAN_TIMEOUT_MS);WiFi.setScanActiveMinTime(100);
 IPAddress address(192,168,4,1),mask(255,255,255,0);apReady=WiFi.softAPConfig(address,address,mask)&&WiFi.softAP(Config::AP_SSID,Config::AP_PASSWORD,6,false,4);
 Serial.printf("AP %s http://%s\n",apReady?"READY":"FAILED",WiFi.softAPIP().toString().c_str());
}
void toggleDevicePower(uint64_t now){
 if(deck.page=="shutdown")return;
 overlayKind=0;cancelRadioJobs();deck.togglePower(now);
 Serial.printf("POWER %s\n",deck.page.c_str());
}
void servicePowerState(){
 if(deck.page=="off"&&!radioSuspended){
  cancelRadioJobs();showFrame(renderer.render(deck));
  if(oledReady){const uint8_t off[]={0xAE};oledCommand(off,sizeof(off));}oledSleeping=true;
  if(!DECK_DEMO){server.stop();WiFi.softAPdisconnect(true);WiFi.mode(WIFI_OFF);}apReady=false;radioSuspended=true;
  Serial.println("SOFT OFF: hold BACK 2s to wake; board still powered");
 }else if(deck.page=="boot"&&radioSuspended){
  if(oledReady){const uint8_t on[]={0xAF};if(!oledCommand(on,sizeof(on)))oledReady=false;}
  if(!oledReady)oledReady=beginOled();oledSleeping=false;
  fullAttempted=false;fullPending=false;lastFullAt=lastTrackAt=radioAt=radioGuardUntil=scanFinishedAt=0;totalFound=0;scanErrorText.clear();apiRevision++;
  if(!DECK_DEMO){startNetwork();server.begin();}radioSuspended=false;
 }
}
void dispatchInput(int index,uint64_t now){
 if(overlayKind&&!deck.powerLocked()){if(index>=2)overlayKind=0;return;}
 uint32_t previousScan=deck.revision;Deck::Text previousPage=deck.page;
 deck.input(inputNames[index],now);
 if(deck.page=="scan"&&(previousPage!="scan"||deck.revision!=previousScan))scanGeneration++;
 Serial.printf("PAGE %s\n",deck.page.c_str());
}
void serviceButtons(uint64_t now){
 bool backEdge=false;
 for(int i=0;i<4;i++)if(buttons[i].update(digitalRead(buttonPins[i])==LOW,millis())){
  Serial.printf("BUTTON %s count=%lu\n",inputNames[i],(unsigned long)buttons[i].count);
  if(i==3)backEdge=true;else dispatchInput(i,now);
 }
 auto action=backPowerKey.update(buttons[3].pressed,backEdge,now,!deck.powerLocked(),Config::POWER_HOLD_MS);
 if(action==Deck::PowerKey::Action::Toggle)toggleDevicePower(now);
 else if(action==Deck::PowerKey::Action::Back)dispatchInput(3,now);
}



// The existing phone drawing/text page is retained; OLED navigation is V4.2.
String jsonString(const String& value){
 String out="\"";for(size_t i=0;i<value.length();i++){uint8_t c=value[i];if(c=='"'||c=='\\'){out+='\\';out+=char(c);}else if(c>=32&&c<=126)out+=char(c);else{char escaped[8];if(c<32)snprintf(escaped,sizeof(escaped),"\\u%04x",c);else snprintf(escaped,sizeof(escaped),"\\\\x%02X",c);out+=escaped;}}return out+'"';
}
String jsonText(const Deck::Text& text){return jsonString(String(text.c_str()));}
void sendJson(int code,const String& body){server.sendHeader("Cache-Control","no-store");server.send(code,"application/json; charset=utf-8",body);}
void handleStatus(){
 uint64_t now=clockMs();uint32_t remaining=overlayKind&&now-overlayAt<Config::DISPLAY_MS?Config::DISPLAY_MS-uint32_t(now-overlayAt):0;
 String body="{\"project\":\"CYBERDECK\",\"version\":"+jsonString(Config::VERSION)+",\"demo\":";body+=DECK_DEMO?"true":"false";
 body+=",\"apReady\":";body+=apReady?"true":"false";body+=",\"apIp\":"+jsonString(WiFi.softAPIP().toString());body+=",\"apSsid\":"+jsonString(Config::AP_SSID);
 body+=",\"clients\":"+String(WiFi.softAPgetStationNum())+",\"staConnected\":";body+=WiFi.status()==WL_CONNECTED?"true":"false";body+=",\"staIp\":"+jsonString(WiFi.localIP().toString());
 body+=",\"chip\":"+jsonString(ESP.getChipModel())+",\"revision\":"+String(ESP.getChipRevision())+",\"cores\":"+String(ESP.getChipCores())+",\"cpuMHz\":"+String(ESP.getCpuFreqMHz());
 body+=",\"flashBytes\":"+String(ESP.getFlashChipSize())+",\"heapBytes\":"+String(ESP.getFreeHeap())+",\"uptimeMs\":"+String((unsigned long)(now&0xffffffffu))+",\"bootId\":"+String(bootId);
 body+=",\"oledReady\":";body+=oledReady?"true":"false";
 body+=",\"page\":"+jsonText(overlayKind==1?"message":overlayKind==2?"drawing":deck.page)+",\"selected\":"+String(deck.networks.empty()?-1:deck.nodeIndex);
 body+=",\"displayRemainingMs\":"+String(remaining)+",\"messageRemainingMs\":"+String(overlayKind==1?remaining:0)+",\"buttons\":{";
 for(int i=0;i<4;i++){if(i)body+=',';body+=jsonString(buttonNames[i])+":{\"pressed\":";body+=buttons[i].pressed?"true":"false";body+=",\"count\":"+String(buttons[i].count)+'}';}
 body+="},\"scan\":{\"running\":";body+=radioJob==RadioJob::Full||fullPending?"true":"false";body+=",\"queued\":";body+=deck.scanRequested?"true":"false";
 body+=",\"hasResults\":";body+=fullAttempted&&scanFinishedAt>0?"true":"false";
 body+=",\"revision\":"+String(apiRevision)+",\"retryAfterMs\":"+String(scanRetryMs(now))+",\"ageMs\":"+String((unsigned long)(scanFinishedAt?now-scanFinishedAt:0));
 body+=",\"count\":"+String(deck.networks.size())+",\"totalFound\":"+String(totalFound)+",\"error\":"+jsonText(scanErrorText)+"}}";sendJson(200,body);
}
void handleNetworks(){
 String body="{\"revision\":"+String(apiRevision)+",\"demo\":";body+=DECK_DEMO?"true":"false";body+=",\"networks\":[";
 for(size_t i=0;i<deck.networks.size();i++){const auto& n=deck.networks[i];if(i)body+=',';body+="{\"ssid\":"+jsonText(n.rawSsid.empty()?n.ssid:n.rawSsid)+",\"bssid\":"+jsonText(n.bssid)+",\"rssi\":"+String(n.rssi)+",\"channel\":"+String(n.channel)+",\"encryption\":"+jsonText(n.encryption)+'}';}sendJson(200,body+"]}");
}
bool powerMutationAllowed(){if(uiPowerLocked()){sendJson(423,"{\"error\":\"Device is starting or powered off. Wait for the main menu.\"}");return false;}return true;}
bool overlayAllowed(){if(!powerMutationAllowed())return false;if(gameActive()){sendJson(409,"{\"error\":\"Finish or exit Breach before changing the OLED.\"}");return false;}return true;}
void handleText(){
 if(!overlayAllowed())return;if(!server.hasArg("text")){sendJson(400,"{\"error\":\"Missing text field\"}");return;}
 String value=server.arg("text");value.replace("\r\n","\n");value.replace("\r","\n");
 if(!Lab::validMessage(value.c_str(),value.length())){sendJson(400,"{\"error\":\"Use printable English ASCII, at most 84 characters fitting 4 rows of 21.\"}");return;}
 message=value;overlayKind=1;overlayAt=clockMs();sendJson(200,"{\"ok\":true,\"durationMs\":20000}");
}
void handleDrawing(){
 if(!overlayAllowed())return;if(!server.hasArg("bitmap")){sendJson(400,"{\"error\":\"Missing bitmap field\"}");return;}
 String value=server.arg("bitmap");if(!Lab::decodeDrawing(value.c_str(),value.length(),drawing)){sendJson(400,"{\"error\":\"Expected 2048 hexadecimal characters for 128x64\"}");return;}
 overlayKind=2;overlayAt=clockMs();sendJson(200,"{\"ok\":true,\"durationMs\":20000}");
}
void handleScan(){
 if(!powerMutationAllowed())return;
 if(gameActive()){sendJson(409,"{\"error\":\"Exit Breach before starting a full survey.\"}");return;}
 if(radioJob==RadioJob::Full||fullPending||deck.scanRequested){sendJson(409,"{\"error\":\"Scan already running or queued\"}");return;}
 if(scanRetryMs(clockMs())){sendJson(429,"{\"error\":\"Wait for the 10 second scan cooldown\"}");return;}
 beginFullScan();sendJson(202,"{\"ok\":true,\"queued\":true}");
}
void setupWeb(){
 server.on("/",HTTP_GET,[]{server.sendHeader("Cache-Control","no-cache");server.sendHeader("X-Content-Type-Options","nosniff");server.send_P(200,"text/html; charset=utf-8",WEB_PAGE);});
 server.on("/api/status",HTTP_GET,handleStatus);server.on("/api/networks",HTTP_GET,handleNetworks);server.on("/api/text",HTTP_POST,handleText);server.on("/api/drawing",HTTP_POST,handleDrawing);server.on("/api/scan",HTTP_POST,handleScan);
 server.on("/api/display/reset",HTTP_POST,[]{overlayKind=0;sendJson(200,"{\"ok\":true}");});
 server.onNotFound([]{sendJson(404,"{\"error\":\"Route or method not found\"}");});server.begin();
}

#if DECK_SELF_TEST

#endif

void setup(){
 Serial.begin(115200);epoch=uint64_t(esp_timer_get_time())/1000;
 for(int i=0;i<4;i++){pinMode(buttonPins[i],INPUT_PULLUP);buttons[i].begin(digitalRead(buttonPins[i])==LOW,millis());}
 oledReady=beginOled();Serial.println(oledReady?"OLED 0x3C READY":"OLED missing: check SDA21 SCL22 3V3 GND");
 bootId=esp_random();deck.seed=DECK_DEMO?2077:bootId;
 if(!DECK_DEMO){
  startNetwork();setupWeb();
 }else Serial.println("DEMO BUILD: invented networks/metrics, radio and HTTP not exercised");
#if DECK_SELF_TEST
 runSelfTests();
 // A complete fresh boot after automated validation; no test data leaks.
 deck=Deck::Cyberdeck(DECK_DEMO,2077);renderer=Deck::OledRenderer(Config::REDUCED_MOTION);epoch=uint64_t(esp_timer_get_time())/1000;
#endif
 epoch=uint64_t(esp_timer_get_time())/1000;
 paint();Serial.println("CYBERDECK V4.2.4 HW2: 10s boot / BACK hold 2s power / UP14 DOWN13 ENTER12 BACK27");
}
void loop(){
 uint64_t now=clockMs();deck.tick(now);serviceButtons(now);servicePowerState();
 if(overlayKind&&now-overlayAt>=Config::DISPLAY_MS)overlayKind=0;
 if(!DECK_DEMO&&!radioSuspended)server.handleClient();serviceRadio(clockMs());
 now=clockMs();deck.tick(now);servicePowerState();
 if(now-lastMetrics>=1000){lastMetrics=now;deck.system.heap=ESP.getFreeHeap()/1024;deck.system.total=std::max(1,int(ESP.getHeapSize()/1024));deck.system.ap=apReady?"ON":"OFF";deck.system.sta=radioJob!=RadioJob::None?"SCAN":WiFi.status()==WL_CONNECTED?"LINK":"IDLE";}
 if(!oledSleeping&&now-lastPaint>=Config::FRAME_MS){lastPaint=now;paint();}
 delay(oledSleeping?5:1);
}
