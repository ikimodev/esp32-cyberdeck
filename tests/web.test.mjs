import {readFile} from 'node:fs/promises';
import vm from 'node:vm';
import assert from 'node:assert/strict';
const html=await readFile(new URL('../web/index.html',import.meta.url),'utf8');
const script=html.match(/<script>([\s\S]*?)<\/script>/)[1].replace(/poll\(\);\s*$/,'');
let passed=0;
async function test(name, fn){await fn();++passed;console.log('PASS '+name);}
class Element {
  constructor(tag='div'){this.tag=tag;this.children=[];this.textContent='';this.value='';this.hidden=false;this.disabled=false;this.dataset={};this.events={};this.classes=new Set();this.parts={};this.classList={add:(n)=>this.classes.add(n),remove:(n)=>this.classes.delete(n),toggle:(n,v)=>v?this.classes.add(n):this.classes.delete(n)};}
  appendChild(c){this.children.push(c);return c;}
  append(...c){this.children.push(...c);}
  replaceChildren(fragment){this.children=fragment.children;}
  querySelector(selector){return this.parts[selector]??=new Element();}
  addEventListener(name,fn){this.events[name]=fn;}
  setAttribute(name,value){this[name]=value;}
  getContext(){return {fillStyle:'',fillRect(){}};}
  getBoundingClientRect(){return {left:10,top:20,width:256,height:128};}
  setPointerCapture(id){this.captured=id;}
  hasPointerCapture(id){return this.captured===id;}
  releasePointerCapture(){this.captured=null;}
}
const elements={},timers=new Map(),requests=[];
let timerId=0, queue=[];
const document={getElementById:id=>elements[id]??=new Element(),createElement:tag=>new Element(tag),createDocumentFragment:()=>new Element('fragment')};
const context=vm.createContext({document,AbortController,URLSearchParams,console,setTimeout:(fn,delay)=>{timers.set(++timerId,{fn,delay});return timerId;},clearTimeout:id=>timers.delete(id),fetch:async(path,options)=>{
  requests.push({path,options});
  const next=queue.shift();
  if(next instanceof Error)throw next;
  if(!next)throw Error('Unexpected fetch '+path);
  return {ok:next.code<400,status:next.code,json:async()=>next.body};
}});
new vm.Script(script).runInContext(context);
document.getElementById('brush').value='1';
const run=(code)=>vm.runInContext(code,context);
const input=s=>{context.input=s;return run('validText(input)');};
const status=(overrides={})=>({bootId:1,apReady:true,apSsid:'ESP32-LAB',apIp:'192.168.4.1',clients:1,chip:'ESP32-D0WDQ6',revision:1,cores:2,cpuMHz:240,flashBytes:4194304,heapBytes:150000,uptimeMs:12345,staConnected:false,oledReady:true,page:'list',selected:0,messageRemainingMs:0,demo:false,buttons:{UP:{pressed:false,count:0},DOWN:{pressed:false,count:0},OK:{pressed:false,count:0},BACK:{pressed:false,count:0}},scan:{revision:1,running:false,queued:false,hasResults:true,retryAfterMs:0,ageMs:1234,count:2,totalFound:2,error:''},...overrides});
const networkData={revision:1,networks:[{ssid:'<img src=x onerror=alert(1)>',bssid:'00:00:00:00:00:01',rssi:-40,channel:6},{ssid:'',bssid:'00:00:00:00:00:02',rssi:-90,channel:11}]};
await test('HTML has mobile viewport, RTL UI and no external dependencies',()=>{
  assert.match(html,/name="viewport"/);assert.match(html,/<html lang="ar" dir="rtl">/);
  assert.doesNotMatch(html,/<script[^>]+src=|<link[^>]+href=|@import|url\(https?:/);
});
await test('text validation: ASCII, CR controls, Arabic, blank, 84/85 chars and row bounds',()=>{
  assert.equal(input('Hello\nESP32'),true);assert.equal(input('A'.repeat(84)),true);assert.equal(input('A'.repeat(85)),false);
  for(const s of ['','   ','عربي','a\tb','a\rb','a\nb\nc\nd\ne','a\nb\nc\n'+'d'.repeat(22)])assert.equal(input(s),false,s);
  assert.equal(input('a'.repeat(21)+'\n'+'b'.repeat(62)),true);
});
await test('2500 generated text cases agree with independent row-layout reference',()=>{
  let seed=71;
  for(let i=0;i<2500;i++){
    seed=(Math.imul(seed,1664525)+1013904223)>>>0;const n=seed%100;let s='';
    for(let j=0;j<n;j++){seed=(Math.imul(seed,1664525)+1013904223)>>>0;s+=seed%17===0?'\n':seed%5===0?' ':'A';}
    const expected=s.length>0&&s.length<=84&&/[^ \n]/.test(s)&&s.split('\n').reduce((sum,line)=>sum+Math.max(1,Math.ceil(line.length/21)),0)<=4;
    assert.equal(input(s),expected,JSON.stringify(s));
  }
});
await test('poll loads status and results; SSID is literal text and hidden SSID labelled',async()=>{
  queue=[{code:200,body:status()},{code:200,body:networkData}];await run('poll()');
  assert.equal(requests.at(-2).path,'/api/status');assert.equal(requests.at(-1).path,'/api/networks');
  const rows=elements.networks.children;
  assert.equal(rows.length,2);assert.equal(rows[0].children[0].textContent,networkData.networks[0].ssid);
  assert.equal(rows[0].children[0].children.length,0);assert.equal(rows[1].children[0].textContent,'<hidden>');
  assert.equal(elements.send.disabled,false);assert.equal(rows[0].classes.has('selected'),true);
});
await test('same scan revision avoids result fetch; button count pulse and details state update',async()=>{
  const s=status({page:'details',selected:1});s.buttons.DOWN={pressed:true,count:1};
  queue=[{code:200,body:s}];const before=requests.length;await run('poll()');
  assert.equal(requests.length-before,1);assert.equal(elements['button-DOWN'].classes.has('pressed'),true);assert.equal(elements['button-DOWN'].classes.has('pulse'),true);
  assert.match(elements['screen-state'].textContent,/تفاصيل/);assert.equal(elements.networks.children[1].classes.has('selected'),true);
});
await test('BACK has a real HTML control and reports press state/count',()=>{
  assert.match(html,/id="button-BACK"/);
  context.sample=status();context.sample.buttons.BACK={pressed:true,count:2};run('renderStatus(sample)');
  assert.equal(elements['button-BACK'].classes.has('pressed'),true);
  assert.equal(elements['button-BACK'].querySelector('.count').textContent,'2 ضغطات');
});
await test('network failure disables writes and marks button samples stale',async()=>{
  queue=[new Error('radio gap')];await run('poll()');
  assert.equal(elements.send.disabled,true);assert.equal(elements.scan.disabled,true);
  assert.equal(elements['button-DOWN'].classes.has('pressed'),false);assert.equal(elements['button-DOWN'].querySelector('.state').textContent,'غير محدث');
  assert.ok([...timers.values()].some(t=>t.delay===1500));
});
await test('reconnection and ESP reboot with same revision refresh networks',async()=>{
  const before=requests.length;queue=[{code:200,body:status({bootId:2})},{code:200,body:networkData}];await run('poll()');
  assert.equal(requests.length-before,2);assert.equal(elements.send.disabled,false);
});
await test('scan running and cooldown disable scan control; OLED failure and DEMO are visible',()=>{
  context.sample=status({demo:true,oledReady:false});context.sample.scan.running=true;run('renderStatus(sample)');
  assert.equal(elements.demo.hidden,false);assert.equal(elements.scan.disabled,true);assert.match(elements['screen-state'].textContent,/غير متاحة/);
  context.sample.scan.running=false;context.sample.scan.retryAfterMs=500;run('renderStatus(sample)');assert.equal(elements.scan.disabled,true);
  context.sample.scan.retryAfterMs=0;run('renderStatus(sample)');assert.equal(elements.scan.disabled,false);
});
await test('text POST uses form encoding; HTTP errors do not claim success or retry POST',async()=>{
  queue=[{code:200,body:{ok:true}}];await run("action('/api/text',{method:'POST',body:new URLSearchParams({text:'A & B'})},'received')");
  assert.equal(requests.at(-1).options.method,'POST');assert.equal(requests.at(-1).options.body.toString(),'text=A+%26+B');assert.equal(elements.feedback.textContent,'received');
  const before=requests.length;queue=[{code:429,body:{error:'cooldown'}}];await run("action('/api/scan',{method:'POST'},'success')");
  assert.equal(requests.length-before,1);assert.match(elements.feedback.textContent,/cooldown/);assert.equal(elements.feedback.classes.has('error'),true);
});
await test('invalid UI submission is rejected before fetch',()=>{
  document.getElementById('text').value='عربي';const before=requests.length;elements['text-form'].events.submit({preventDefault(){}});
  assert.equal(requests.length,before);assert.equal(elements.feedback.classes.has('error'),true);
});
await test('drawing touch coordinates scale and clamp to 128x64 pixels',()=>{
  context.event={clientX:266,clientY:148};
  assert.deepEqual(JSON.parse(run('JSON.stringify(drawingPoint(event))')),{x:127,y:63});
  context.event={clientX:-100,clientY:-100};
  assert.deepEqual(JSON.parse(run('JSON.stringify(drawingPoint(event))')),{x:0,y:0});
  context.event={clientX:138,clientY:84};
  assert.deepEqual(JSON.parse(run('JSON.stringify(drawingPoint(event))')),{x:64,y:32});
});
await test('drawing packs corners, byte boundary and next row in GFX MSB-first order',()=>{
  run('pixels.fill(0); [0,7,8,127,128,8191].forEach(i=>pixels[i]=1)');
  const hex=run('drawingHex()');assert.equal(hex.length,2048);
  assert.equal(hex.slice(0,4),'8180');assert.equal(hex.slice(30,34),'0180');assert.equal(hex.slice(-2),'01');
});
await test('continuous strokes, brush sizes and eraser use same transmitted pixel model',()=>{
  run('pixels.fill(0); erasing=false; drawLine({x:0,y:0},{x:127,y:63})');
  assert.equal(run('pixels[0]'),1);assert.equal(run('pixels[8191]'),1);assert.equal(run('pixels.reduce((a,b)=>a+b,0)'),128);
  run('pixels.fill(0)');elements.brush.value='3';run('dab(30,30)');assert.equal(run('pixels.reduce((a,b)=>a+b,0)'),9);
  run('erasing=true; dab(30,30)');assert.equal(run('pixels.some(p=>p)'),false);
  elements.brush.value='1';run('erasing=false');
});
await test('touch press draws a dot, captures pointer and ignores second finger',()=>{
  run('pixels.fill(0); drawingHistory=[]');const canvas=elements.drawing;
  const event={pointerId:7,button:0,clientX:10,clientY:20,preventDefault(){}};
  canvas.events.pointerdown(event);assert.equal(run('pixels[0]'),1);assert.equal(canvas.captured,7);
  canvas.events.pointerdown({...event,pointerId:8,clientX:266,clientY:148});assert.equal(run('pixels[8191]'),0);
  canvas.events.pointermove({...event,clientX:26,clientY:20});assert.equal(run('pixels[8]'),1);
  canvas.events.pointercancel(event);assert.equal(run('pointer'),null);assert.equal(canvas.captured,null);
});
await test('undo and clear preserve editable drawing; history is bounded',()=>{
  elements['clear-drawing'].events.click();assert.equal(run('pixels.some(p=>p)'),false);
  elements['undo-drawing'].events.click();assert.equal(run('pixels[0]'),1);
  elements['undo-drawing'].events.click();assert.equal(run('pixels.some(p=>p)'),false);
  run('for(let i=0;i<30;i++)rememberDrawing()');assert.equal(run('drawingHistory.length'),20);
});
await test('drawing POST sends exactly 1024 bitmap bytes as hex and shows 20s acknowledgement',async()=>{
  queue=[{code:200,body:{ok:true,durationMs:20000}}];
  await elements['send-drawing'].events.click();
  const req=requests.at(-1);assert.equal(req.path,'/api/drawing');assert.equal(req.options.method,'POST');assert.equal(req.options.body.get('bitmap').length,2048);
  assert.match(elements.feedback.textContent,/20/);
  context.sample=status({page:'drawing',displayRemainingMs:20000});run('renderStatus(sample)');assert.match(elements['screen-state'].textContent,/الرسم.*20/);
});
await test('embedded HTML header exactly matches standalone source',async()=>{
  const header=await readFile(new URL('../firmware/cyberdeck/web_page.h',import.meta.url),'utf8');
  assert.equal(header.split('R"LABHTML(')[1].split(')LABHTML";')[0],html);
});
console.log(`Web tests: ${passed} passed, 0 failed (Node DOM/fetch test doubles; no ESP32 runtime claims).`);
