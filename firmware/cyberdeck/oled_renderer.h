#pragma once
#include "cyberdeck_core.h"
#include "deck_assets.h"
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
