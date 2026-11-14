// Wiring test only. The V4.2 interactive preview is a separate project.
// No external libraries: Wire is included in the ESP32 Arduino core.
#include <Arduino.h>
#include <Wire.h>
#include <string.h>
const uint8_t font5x7[]={0,0,0,0,0,0,0,95,0,0,0,7,0,7,0,20,127,20,127,20,36,42,127,42,18,35,19,8,100,98,54,73,86,32,80,0,8,7,3,0,0,28,34,65,0,0,65,34,28,0,42,28,127,28,42,8,8,62,8,8,0,128,112,48,0,8,8,8,8,8,0,0,96,96,0,32,16,8,4,2,62,81,73,69,62,0,66,127,64,0,114,73,73,73,70,33,65,73,77,51,24,20,18,127,16,39,69,69,69,57,60,74,73,73,49,65,33,17,9,7,54,73,73,73,54,70,73,73,41,30,0,0,20,0,0,0,64,52,0,0,0,8,20,34,65,20,20,20,20,20,0,65,34,20,8,2,1,89,9,6,62,65,93,89,78,124,18,17,18,124,127,73,73,73,54,62,65,65,65,34,127,65,65,65,62,127,73,73,73,65,127,9,9,9,1,62,65,65,81,115,127,8,8,8,127,0,65,127,65,0,32,64,65,63,1,127,8,20,34,65,127,64,64,64,64,127,2,28,2,127,127,4,8,16,127,62,65,65,65,62,127,9,9,9,6,62,65,81,33,94,127,9,25,41,70,38,73,73,73,50,3,1,127,1,3,63,64,64,64,63,31,32,64,32,31,63,64,56,64,63,99,20,8,20,99,3,4,120,4,3,97,89,73,77,67,0,127,65,65,65,2,4,8,16,32,0,65,65,65,127,4,2,1,2,4,64,64,64,64,64,0,3,7,8,0,32,84,84,120,64,127,40,68,68,56,56,68,68,68,40,56,68,68,40,127,56,84,84,84,24,0,8,126,9,2,24,164,164,156,120,127,8,4,4,120,0,68,125,64,0,32,64,64,61,0,127,16,40,68,0,0,65,127,64,0,124,4,120,4,120,124,8,4,4,120,56,68,68,68,56,252,24,36,36,24,24,36,36,24,252,124,8,4,4,8,72,84,84,84,36,4,4,63,68,36,60,64,64,32,124,28,32,64,32,28,60,64,48,64,60,68,40,16,40,68,76,144,144,144,124,68,100,84,76,68,0,8,54,65,0,0,0,119,0,0,0,65,54,8,0,2,1,2,4,2};
uint8_t frame[1024];
bool oledReady=false;
struct Button {const char* name; uint8_t pin; bool raw, stable; uint32_t changed, presses;};
Button buttons[]={{"UP",14,true,true,0,0},{"DOWN",13,true,true,0,0},{"ENTER",12,true,true,0,0},{"BACK",27,true,true,0,0}};
void commands(const uint8_t* data,size_t n){Wire.beginTransmission(0x3c);Wire.write(0);Wire.write(data,n);Wire.endTransmission();}
void pixel(int x,int y){if(x>=0&&x<128&&y>=0&&y<64)frame[x+(y/8)*128]|=1<<(y&7);}
void text(int x,int y,const char* s){for(;*s;s++,x+=6){uint8_t c=*s;if(c<32||c>126)c='?';for(int col=0;col<5;col++)for(int row=0;row<8;row++)if(font5x7[(c-32)*5+col]&(1<<row))pixel(x+col,y+row);}}
void render(){
 memset(frame,0,sizeof(frame));text(0,3,"CYBERDECK / I-O");for(int x=0;x<128;x++)pixel(x,14);
 for(int i=0;i<4;i++){char line[24];snprintf(line,sizeof(line),"%-5s %-4s %lu",buttons[i].name,buttons[i].stable?"IDLE":"DOWN",(unsigned long)buttons[i].presses);text(0,18+i*11,line);}
 if(!oledReady)return;
 const uint8_t range[]={0x21,0,127,0x22,0,7};commands(range,sizeof(range));
 for(int start=0;start<1024;start+=16){Wire.beginTransmission(0x3c);Wire.write(0x40);Wire.write(frame+start,16);Wire.endTransmission();}
}
void setup(){
 Serial.begin(115200);for(auto& b:buttons)pinMode(b.pin,INPUT_PULLUP);
 Wire.begin(21,22);Wire.setClock(400000);Wire.beginTransmission(0x3c);oledReady=Wire.endTransmission()==0;
 Serial.println(oledReady?"OLED 0x3C OK":"OLED NOT FOUND");
 const uint8_t init[]={0xAE,0xD5,0x80,0xA8,0x3F,0xD3,0,0x40,0x8D,0x14,0x20,0,0xA1,0xC8,0xDA,0x12,0x81,0x7F,0xD9,0xF1,0xDB,0x40,0xA4,0xA6,0x2E,0xAF};
 if(oledReady)commands(init,sizeof(init));render();Serial.println("READY: UP14 DOWN13 ENTER12 BACK27");
}
void loop(){
 uint32_t now=millis();bool dirty=false;
 for(auto& b:buttons){bool value=digitalRead(b.pin);if(value!=b.raw){b.raw=value;b.changed=now;}if(value!=b.stable&&uint32_t(now-b.changed)>=30){b.stable=value;if(!value)b.presses++;Serial.printf("%s %s count=%lu\n",b.name,value?"RELEASE":"PRESS",(unsigned long)b.presses);dirty=true;}}
 if(dirty)render();delay(1);
}
