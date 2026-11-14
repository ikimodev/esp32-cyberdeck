#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_wifi.h>
#include <esp_timer.h>
#include "config.h"
#include "cyberdeck_core.h"
#include "oled_renderer.h"
#include "lab_core.h"

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

#include "web_handlers.h"
#if DECK_SELF_TEST
#include "self_test.h"
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
