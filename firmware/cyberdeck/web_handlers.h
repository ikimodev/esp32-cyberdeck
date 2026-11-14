#pragma once
#include "web_page.h"
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
