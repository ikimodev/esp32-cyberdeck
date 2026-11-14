// Pure simulation state. No DOM, radio, network APIs or hardware access.
export const VERSION='4.2.3';
export const DEMO_NETWORKS=Object.freeze([
  {ssid:'NEON-HOME',bssid:'02:A7:11:9C:40:01',rssi:-42,channel:1,encryption:'WPA2-PSK'},
  {ssid:'WORKSHOP',bssid:'02:A7:11:9C:40:02',rssi:-51,channel:6,encryption:'WPA2/WPA3-PSK'},
  {ssid:'GUEST_24G',bssid:'02:A7:11:9C:40:03',rssi:-59,channel:6,encryption:'OPEN'},
  {ssid:'GHOST-LINK',bssid:'02:A7:11:9C:40:04',rssi:-67,channel:11,encryption:'WPA3-SAE'},
  {ssid:'',bssid:'02:A7:11:9C:40:05',rssi:-78,channel:3,encryption:'WPA2-PSK'},
  {ssid:'NIGHT-CITY-MEGABUILDING-LOCAL-24G',bssid:'02:A7:11:9C:40:06',rssi:-86,channel:13,encryption:'WPA2-ENTERPRISE'}
]);
export const MENU=Object.freeze(['NET SCANNER','SIGNAL TRACKER','SECURITY PROFILE','SYSTEM MONITOR','BREACH PROTOCOL']);
export const TOKENS=Object.freeze(['BD','55','1C','E9']);
export const CAPACITY=4, BREACH_MS=30000, BOOT_MS=4000, SCAN_MS=2600;
export function clamp(n,lo,hi){return Math.max(lo,Math.min(hi,n));}
export function signalQuality(rssi){return clamp(Math.round((rssi+100)*100/60),0,100);}
export function signalLabel(rssi){const q=signalQuality(rssi);return q>=80?'STRONG':q>=50?'GOOD':q>=25?'WEAK':'POOR';}
export function nodeName(n){return n.ssid||'<hidden>';}
export function elapsedText(ms){const sec=Math.floor(ms/1000);const h=Math.floor(sec/3600);return `${String(h).padStart(2,'0')}:${String(Math.floor(sec/60)%60).padStart(2,'0')}:${String(sec%60).padStart(2,'0')}`;}

export function makePuzzle(seed=2077,round=0){
  if(round===0)return {matrix:['1C','BD','E9','BD','55','E9','55','1C','BD'],target:['BD','55','E9'],capacity:CAPACITY,duration:BREACH_MS};
  let state=(seed+Math.imul(round,2654435761))>>>0;
  const rand=n=>{state=(Math.imul(state,1664525)+1013904223)>>>0;return (state>>>8)%n;};
  const matrix=Array.from({length:9},()=>TOKENS[rand(TOKENS.length)]);
  const path=[rand(3)];
  let axis='column';
  for(let k=1;k<4;k++){
    const last=path.at(-1);
    const candidates=Array.from({length:9},(_,i)=>i).filter(i=>!path.includes(i)&&(axis==='column'?i%3===last%3:Math.floor(i/3)===Math.floor(last/3)));
    path.push(candidates[rand(candidates.length)]);axis=axis==='column'?'row':'column';
  }
  const pool=[...TOKENS],target=[];
  for(let i=0;i<3;i++)target.push(pool.splice(rand(pool.length),1)[0]);
  const offset=round%2;
  for(let i=0;i<3;i++)matrix[path[i+offset]]=target[i];
  return {matrix,target,capacity:CAPACITY,duration:BREACH_MS};
}

export class BreachGame {
  constructor(puzzle=makePuzzle()){
    this.matrix=[...puzzle.matrix];this.target=[...puzzle.target];
    this.capacity=puzzle.capacity??CAPACITY;this.duration=puzzle.duration??BREACH_MS;
    this.used=[];this.buffer=[];this.cursor=0;this.axis='row';this.axisIndex=0;
    this.startedAt=null;this.finishedAt=null;this.result=null;this.reason='';
  }
  legal(){return Array.from({length:9},(_,i)=>i).filter(i=>!this.used.includes(i)&&(this.axis==='row'?Math.floor(i/3)===this.axisIndex:i%3===this.axisIndex));}
  remaining(now){return this.startedAt===null?this.duration:Math.max(0,this.duration-((this.finishedAt??now)-this.startedAt));}
  tick(now){if(!this.result&&this.startedAt!==null&&this.remaining(now)<=0)this.finish('failure','TIME EXPIRED',now);}
  move(direction,now){this.tick(now);if(this.result)return false;const legal=this.legal();if(!legal.length)return false;let p=legal.indexOf(this.cursor);if(p<0)p=0;this.cursor=legal[(p+direction+legal.length)%legal.length];return true;}
  finish(result,reason,now){this.result=result;this.reason=reason;this.finishedAt=now;}
  matches(){for(let i=0;i<=this.buffer.length-this.target.length;i++)if(this.target.every((v,j)=>v===this.buffer[i+j]))return true;return false;}
  pick(now,index=this.cursor){
    this.tick(now);if(this.result||!this.legal().includes(index))return false;
    if(this.startedAt===null)this.startedAt=now;
    this.used.push(index);this.buffer.push(this.matrix[index]);
    if(this.matches()){this.finish('success','SEQUENCE MATCH',now);return true;}
    if(this.buffer.length>=this.capacity){this.finish('failure','BUFFER FULL',now);return true;}
    this.axis=this.axis==='row'?'column':'row';
    this.axisIndex=this.axis==='column'?index%3:Math.floor(index/3);
    const legal=this.legal();
    if(!legal.length)this.finish('failure','NO ROUTE',now);else this.cursor=legal[0];
    return true;
  }
  progress(){let best=0;for(let n=1;n<=Math.min(this.target.length,this.buffer.length);n++)if(this.target.slice(0,n).every((v,i)=>v===this.buffer[this.buffer.length-n+i]))best=n;return best;}
}

export class Cyberdeck {
  constructor({networks=DEMO_NETWORKS,seed=2077}={}){
    this.provider=networks.map(n=>({...n}));this.seed=seed;
    this.now=0;this.page='boot';this.pageAt=0;this.lastInputAt=0;this.revision=0;
    this.menuIndex=0;this.networks=[];this.nodeIndex=0;this.selectKind='netlist';
    this.profileReturn='netlist';this.dataTab=0;
    this.scanReturn='netlist';this.scanStartedAt=0;this.scanCount=0;
    this.sampleAt=0;this.history=[];this.liveRssi=-60;
    this.round=0;this.game=new BreachGame(makePuzzle(seed,0));this.abortIndex=0;
  }
  go(page){if(this.page!==page){this.page=page;this.pageAt=this.now;this.revision++;}}
  node(){return this.networks[this.nodeIndex]??this.provider[0]??{ssid:'NO TARGET',bssid:'--:--:--:--:--:--',rssi:-100,channel:0,encryption:'N/A'};}
  beginScan(destination='netlist'){this.scanReturn=destination;this.scanStartedAt=this.now;this.scanCount=0;this.go('scan');}
  ensureNodes(destination){if(this.networks.length){this.nodeIndex=clamp(this.nodeIndex,0,this.networks.length-1);this.go(destination);}else this.beginScan(destination);}
  startTracker(){this.history=[];this.sampleAt=this.now;this.liveRssi=this.node().rssi;this.go('tracker');this.sample(this.now);}
  sample(t){const idx=this.nodeIndex;const wave=Math.sin(t/1700+idx)*3.8+Math.sin(t/430+idx*2)*1.9;this.liveRssi=clamp(Math.round(this.node().rssi+wave),-100,-30);this.history.push(this.liveRssi);if(this.history.length>64)this.history.shift();}
  tick(time){
    if(!Number.isFinite(time))return;
    this.now=Math.max(this.now,time);
    if(this.page==='boot'&&this.now>=BOOT_MS)this.go('menu');
    if(this.page==='scan'){
      const elapsed=this.now-this.scanStartedAt;
      this.scanCount=Math.min(this.provider.length,Math.floor(elapsed/SCAN_MS*this.provider.length));
      if(elapsed>=SCAN_MS){
        this.networks=this.provider.map(n=>({...n})).sort((a,b)=>b.rssi-a.rssi);
        this.nodeIndex=clamp(this.nodeIndex,0,Math.max(0,this.networks.length-1));
        this.scanCount=this.networks.length;this.selectKind=this.scanReturn;this.go(this.scanReturn);
      }
    }
    if(this.page==='tracker'){
      // Bound catch-up after a hidden tab: history remains at most 64 samples.
      this.sampleAt=Math.max(this.sampleAt,this.now-64*250);
      while(this.now-this.sampleAt>=250){this.sampleAt+=250;this.sample(this.sampleAt);}
    }
    if(this.page==='breach'||this.page==='abort'){
      this.game.tick(this.now);
      if(this.game.result)this.go(this.game.result==='success'?'success':'failure');
    }
    if(this.page==='menu'&&this.now-this.lastInputAt>45000)this.go('idle');
  }
  enterMenu(){
    this.lastInputAt=this.now;
    const item=this.menuIndex;
    if(item===0)this.beginScan('netlist');
    if(item===1){this.selectKind='trackerSelect';this.ensureNodes('trackerSelect');}
    if(item===2){this.selectKind='securitySelect';this.ensureNodes('securitySelect');}
    if(item===3)this.go('system');
    if(item===4)this.go('breachIntro');
  }
  newGame(){this.game=new BreachGame(makePuzzle(this.seed,this.round));}
  input(key,time=this.now){
    if(!['up','down','enter','back'].includes(key))return;
    const priorPage=this.page;this.tick(time);this.lastInputAt=this.now;
    // A confirm arriving at the time limit must show failure, not retry it.
    if((priorPage==='breach'||priorPage==='abort')&&this.page==='failure')return;
    const direction=key==='down'?1:key==='up'?-1:0;
    if(this.page==='boot'){if(key==='enter')this.go('menu');return;}
    if(this.page==='idle'){this.go('menu');return;}
    if(this.page==='menu'){
      if(direction)this.menuIndex=clamp(this.menuIndex+direction,0,MENU.length-1);
      if(key==='enter')this.enterMenu();if(key==='back')this.go('idle');return;
    }
    if(this.page==='scan'){if(key==='back')this.go('menu');return;}
    if(['netlist','trackerSelect','securitySelect'].includes(this.page)){
      if(direction)this.nodeIndex=clamp(this.nodeIndex+direction,0,Math.max(0,this.networks.length-1));
      if(key==='back')this.go('menu');
      if(key==='enter'){
        if(!this.networks.length){this.beginScan(this.page);return;}
        if(this.page==='netlist'){this.profileReturn='netlist';this.go('details');}
        else if(this.page==='trackerSelect')this.startTracker();
        else {this.profileReturn='securitySelect';this.dataTab=0;this.go('security');}
      }
      return;
    }
    if(this.page==='details'){
      if(key==='back')this.go('netlist');
      if(key==='enter'){this.profileReturn='details';this.dataTab=0;this.go('security');}
      return;
    }
    if(this.page==='tracker'){
      if(key==='back')this.go('trackerSelect');
      if(direction&&this.networks.length){this.nodeIndex=(this.nodeIndex+direction+this.networks.length)%this.networks.length;this.history=[];this.sampleAt=this.now;this.sample(this.now);}
      return;
    }
    if(this.page==='security'){
      if(key==='back')this.go(this.profileReturn);
      if(direction||key==='enter')this.dataTab=1-this.dataTab;return;
    }
    if(this.page==='system'){if(key==='back')this.go('menu');return;}
    if(this.page==='breachIntro'){
      if(key==='back')this.go('menu');
      if(key==='enter'){this.newGame();this.go('breach');}return;
    }
    if(this.page==='breach'){
      if(direction)this.game.move(direction,this.now);
      if(key==='enter')this.game.pick(this.now);
      if(key==='back'){this.abortIndex=0;this.go('abort');}
      if(this.game.result)this.go(this.game.result==='success'?'success':'failure');return;
    }
    if(this.page==='abort'){
      if(direction)this.abortIndex=1-this.abortIndex;
      if(key==='back'||key==='enter'&&this.abortIndex===0)this.go('breach');
      else if(key==='enter'){this.newGame();this.go('breachIntro');}return;
    }
    if(this.page==='success'||this.page==='failure'){
      if(key==='back')this.go('menu');
      if(key==='enter'){this.round++;this.newGame();this.go('breachIntro');}
    }
  }
  get demoSystem(){return {heap:184+Math.round(Math.sin(this.now/9000)*4),total:320,uptime:this.now,ap:'ON',sta:'IDLE',version:VERSION};}
  get description(){
    const names={boot:'تسلسل الإقلاع',menu:MENU[this.menuIndex],scan:'فحص شبكات تجريبي',netlist:'قائمة الشبكات التجريبية',trackerSelect:'اختر شبكة لمراقبة إشارتها',securitySelect:'اختر شبكة لعرض ملفها',details:'تفاصيل الشبكة',tracker:'رسم الإشارة التجريبية',security:`DATA — الصفحة ${this.dataTab+1} من 2`,system:'مراقبة النظام التجريبي',breachIntro:'Breach Protocol — الاستعداد',breach:'Breach Protocol — اللعب',abort:'تأكيد الخروج — المؤقت مستمر',success:'نجاح تسلسل Breach',failure:`فشل Breach: ${this.game.reason}`,idle:'مدينة الانتظار — أي زر يعيد القائمة'};
    return names[this.page]??this.page;
  }
}
