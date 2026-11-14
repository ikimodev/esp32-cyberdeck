#pragma once
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
