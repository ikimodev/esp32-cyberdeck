import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile} from 'node:fs/promises';
import vm from 'node:vm';
import {ASSETS} from '../base/assets.mjs';
import {PowerCyberdeck,POWER_BOOT_MS,POWER_OFF_MS} from './power-model.mjs';
import {PowerRenderer,LOGO_GLITCH_PERIOD_MS,LOGO_GLITCH_BURST_MS} from './power-renderer.mjs';

test('Logo tear repeats every two seconds, is clearly visible, and affects only logo rows',()=>{
  assert.equal(LOGO_GLITCH_PERIOD_MS,2000);assert.equal(LOGO_GLITCH_BURST_MS,320);
  const r=new PowerRenderer(ASSETS),clean=new PowerRenderer(ASSETS,{reducedMotion:true});
  const logo=time=>{r.f.clear();r.fullLogo(time);return r.f.bits.slice();};
  const first=logo(40),repeat=logo(2040);assert.deepEqual(first,repeat);
  clean.f.clear();clean.fullLogo(40);let changed=0;
  for(let y=0;y<64;y++)for(let x=0;x<128;x++)if(r.pixel(first,x,y)!==clean.f.get(x,y)){changed++;assert.ok(y<25);}
  assert.ok(changed>70,`Visible shifted pixels: ${changed}`);
  assert.notDeepEqual(first,logo(400));
  r.f.clear();r.logoTime=40;r.header('CYBERDECK');const glitch=r.f.bits.slice();
  r.f.clear();r.logoTime=400;r.header('CYBERDECK');
  for(let y=0;y<64;y++)for(let x=0;x<128;x++)if(r.pixel(glitch,x,y)!==r.f.get(x,y))assert.ok(x>=75&&y>=2&&y<12);
  clean.f.clear();clean.fullLogo(2040);const a=clean.f.bits.slice();clean.f.clear();clean.fullLogo(400);assert.deepEqual(a,clean.f.bits);
});

test('Starts off; startup is exactly ten seconds and menu keys do not skip it',()=>{
  const s=new PowerCyberdeck();s.tick(90000);assert.equal(s.page,'off');s.togglePower();assert.equal(s.page,'boot');
  for(const key of ['enter','up','down','back'])s.input(key,95000);assert.equal(s.page,'boot');assert.equal(s.menuIndex,0);
  s.tick(90000+POWER_BOOT_MS-1);assert.equal(s.page,'boot');s.tick(90000+POWER_BOOT_MS);assert.equal(s.page,'menu');
  assert.equal(s.lastInputAt,s.now);s.tick(s.now+100);assert.equal(s.page,'menu');
});
test('Power off animates, stays black, and the next boot has its own deadline',()=>{
  const s=new PowerCyberdeck();s.togglePower(100);s.tick(10100);s.input('down');s.togglePower(11000);
  assert.equal(s.page,'shutdown');s.input('enter');s.togglePower(11100);assert.equal(s.page,'shutdown');
  s.tick(11000+POWER_OFF_MS);assert.equal(s.page,'off');s.tick(50000);assert.equal(s.page,'off');
  s.togglePower(50000);s.tick(59999);assert.equal(s.page,'boot');s.tick(60000);assert.equal(s.page,'menu');assert.equal(s.menuIndex,1);
});
test('Power can cancel startup and scanning without later navigation',()=>{
  const s=new PowerCyberdeck();s.togglePower();s.tick(2500);s.togglePower();s.tick(3400);assert.equal(s.page,'off');s.tick(50000);assert.equal(s.page,'off');
  s.togglePower();s.tick(60000);s.input('enter');assert.equal(s.page,'scan');s.togglePower();s.tick(61000);s.tick(80000);assert.equal(s.page,'off');
});
test('All opening and closing frames are one bit and use physical-row colors',()=>{
  const s=new PowerCyberdeck(),r=new PowerRenderer(ASSETS);assert.ok(r.render(s).bits.every(b=>b===0));s.togglePower();
  const frames=[];
  for(const time of [0,400,650,1000,1500,1600,2100,2800,3600,5500,8799,9000,9600,9999,10000,10200]){
    s.tick(time);const f=r.render(s);assert.equal(f.bits.length,1024);assert.equal(f.rgba().length,32768);frames.push(Buffer.from(f.bits).toString('base64'));
    for(const box of f.textBounds)assert.ok(box.x>=0&&box.y>=0&&box.x+box.w<=128&&box.y+box.h<=64,box.label);
    const rgba=f.rgba();for(let y=0;y<64;y++)for(let x=0;x<128;x++)if(f.get(x,y)){const at=(y*128+x)*4;assert.equal(rgba[at],y<16?255:43);assert.equal(rgba[at+2],y<16?55:255);}
  }
  assert.ok(new Set(frames).size>=9);s.togglePower();
  for(const dt of [0,180,510,640,780,899,900,20000]){s.tick(10200+dt);const f=r.render(s);assert.equal(f.bits.length,1024);if(dt>=900)assert.ok(f.bits.every(b=>b===0));}
});
test('Reduced motion still respects duration and power state',()=>{
  const s=new PowerCyberdeck(),r=new PowerRenderer(ASSETS,{reducedMotion:true});s.togglePower();s.tick(1000);assert.ok(r.render(s).bits.some(Boolean));s.tick(9999);assert.equal(s.page,'boot');s.tick(10000);assert.equal(s.page,'menu');
  s.togglePower();assert.ok(r.render(s).bits.every(b=>b===0));s.tick(10900);assert.equal(s.page,'off');
});
test('Existing scanner/details and Breach remain usable after startup',()=>{
  const s=new PowerCyberdeck();s.togglePower();s.tick(10000);s.input('enter');s.tick(12600);assert.equal(s.page,'netlist');s.input('enter');assert.equal(s.page,'details');s.input('back');s.input('back');s.menuIndex=4;s.input('enter');s.input('enter');
  for(const key of ['down','enter','enter','down','enter'])s.input(key);assert.equal(s.page,'success');
});
test('Bundled controls power on/off, lock inputs, and work without a server',async()=>{
  const html=await readFile(new URL('power-preview.fragment.html',import.meta.url),'utf8');
  assert.doesNotMatch(html,/\bfetch\s*\(|\bXMLHttpRequest\b|\bWebSocket\b|<script[^>]+src=/);
  assert.doesNotMatch(html,/إعادة الإقلاع|لتحميل|دخول لتجاوز/);
  const script=html.match(/<script>([\s\S]*?)<\/script>/)[1];let clock=0,raf;
  const element=()=>({textContent:'',disabled:false,dataset:{},events:{},classList:{add(){},remove(){}},setAttribute(k,v){this[k]=v;},addEventListener(k,fn){this.events[k]=fn;}});
  const canvas=element();canvas.getContext=()=>({putImageData(){}});
  const status=element(),reader=element(),power=element(),buttons=Object.fromEntries(['up','down','enter','back'].map(k=>{const b=element();b.dataset.key=k;return[k,b];}));
  const root={isConnected:true,dataset:{},querySelector:s=>({'canvas':canvas,'#deck-status-v42':status,'#deck-reader-v42':reader,'#deck-reboot-v42':power}[s]),querySelectorAll:()=>Object.values(buttons)};
  const document={getElementById:()=>root,addEventListener(){},removeEventListener(){}};
  const context=vm.createContext({document,window:{addEventListener(){},removeEventListener(){}},performance:{now:()=>clock},matchMedia:()=>({matches:false}),ImageData:class{},requestAnimationFrame:fn=>raf=fn,setTimeout(){}});
  new vm.Script(script).runInContext(context);assert.equal(root.dataset.page,'off');assert.equal(power.textContent,'تشغيل الجهاز');assert.ok(buttons.enter.disabled);
  power.events.click();assert.equal(root.dataset.page,'boot');assert.equal(power.textContent,'إطفاء الجهاز');
  clock=9999;raf(clock);assert.equal(root.dataset.page,'boot');clock=10100;raf(clock);assert.equal(root.dataset.page,'menu');assert.equal(buttons.enter.disabled,false);
  power.events.click();assert.equal(root.dataset.page,'shutdown');assert.equal(power.disabled,true);
  clock=11100;raf(clock);assert.equal(root.dataset.page,'off');assert.equal(power.textContent,'تشغيل الجهاز');
});
