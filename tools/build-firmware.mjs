import fs from 'node:fs';
import {ASSETS} from '../preview/base/assets.mjs';
import './generate-reference-frames.mjs';
const root=new URL('../',import.meta.url),sketch=new URL('firmware/cyberdeck/',root);
let header='#pragma once\n#include <stdint.h>\nnamespace Deck { namespace Assets {\nstruct Bitmap {int w,h,stride;const uint8_t* data;};\n';
header+=`static const uint8_t font[]={${ASSETS.font.join(',')}};\n`;
for(const name of ['logo','smallLogo','city']){const a=ASSETS[name];header+=`static const uint8_t ${name}Data[]={${a.data.join(',')}};\nstatic const Bitmap ${name}={${a.w},${a.h},${a.stride},${name}Data};\n`;}
header+='}}\n';fs.writeFileSync(new URL('deck_assets.h',sketch),header);
const html=fs.readFileSync(new URL('web/index.html',root),'utf8');
if(html.includes(')LABHTML"'))throw Error('Invalid raw-string terminator in web source');
fs.writeFileSync(new URL('web_page.h',sketch),'#pragma once\n#include <Arduino.h>\nstatic const char WEB_PAGE[] PROGMEM=R"LABHTML('+html+')LABHTML";\n');
const files=['config.h','deck_assets.h','cyberdeck_core.h','oled_renderer.h','lab_core.h','web_page.h','cyberdeck.ino'];
let production='#include <Arduino.h>\n#include <Wire.h>\n#include <WiFi.h>\n#include <WebServer.h>\n#include <esp_wifi.h>\n#include <esp_timer.h>\n';
for(const file of files){let content=fs.readFileSync(new URL(file,sketch),'utf8');if(file==='cyberdeck.ino')content=content.replace('#include "web_handlers.h"',fs.readFileSync(new URL('web_handlers.h',sketch),'utf8'));content=content.replace(/^#pragma once\s*$/gm,'').replace(/^#include "[^"\n]+"\s*$/gm,'');production+=`\n// ---- ${file} ----\n${content}\n`;}
fs.mkdirSync(new URL('firmware/Cyberdeck_Hardware/',root),{recursive:true});
fs.writeFileSync(new URL('firmware/Cyberdeck_Hardware/Cyberdeck_Hardware.ino',root),production.trimEnd()+'\n');
console.log('Built complete production sketch; DECK_DEMO defaults to 0.');
