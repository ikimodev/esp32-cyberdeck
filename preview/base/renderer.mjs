// All product pixels are binary. Color is applied only by the physical-row mapper.
export class Framebuffer {
  constructor(font){this.bits=new Uint8Array(1024);this.font=font;this.textBounds=[];}
  clear(){this.bits.fill(0);this.textBounds=[];}
  get(x,y){if(x<0||x>=128||y<0||y>=64)return 0;return (this.bits[y*16+(x>>3)]>>(7-(x&7)))&1;}
  set(x,y,value=1){x=Math.round(x);y=Math.round(y);if(x<0||x>=128||y<0||y>=64)return;const i=y*16+(x>>3),mask=128>>(x&7);if(value)this.bits[i]|=mask;else this.bits[i]&=255^mask;}
  rect(x,y,w,h,fill=false,value=1){x=Math.round(x);y=Math.round(y);if(fill){for(let yy=y;yy<y+h;yy++)for(let xx=x;xx<x+w;xx++)this.set(xx,yy,value);}else {this.line(x,y,x+w-1,y,value);this.line(x,y+h-1,x+w-1,y+h-1,value);this.line(x,y,x,y+h-1,value);this.line(x+w-1,y,x+w-1,y+h-1,value);}}
  line(x0,y0,x1,y1,value=1){x0=Math.round(x0);y0=Math.round(y0);x1=Math.round(x1);y1=Math.round(y1);const dx=Math.abs(x1-x0),dy=-Math.abs(y1-y0),sx=x0<x1?1:-1,sy=y0<y1?1:-1;let err=dx+dy;for(;;){this.set(x0,y0,value);if(x0===x1&&y0===y1)break;const e=err*2;if(e>=dy){err+=dy;x0+=sx;}if(e<=dx){err+=dx;y0+=sy;}}}
  text(x,y,label,value=1){
    label=String(label);x=Math.round(x);y=Math.round(y);
    this.textBounds.push({x,y,w:Math.max(0,label.length*6-1),h:8,label});
    for(const c of label){const code=c.charCodeAt(0),offset=((code>=32&&code<127?code:63)-32)*5;for(let xx=0;xx<5;xx++){const b=this.font[offset+xx];for(let yy=0;yy<8;yy++)if(b&(1<<yy))this.set(x+xx,y+yy,value);}x+=6;}
  }
  label(x,y,label,columns=21,value=1){const s=String(label);this.text(x,y,s.length>columns?s.slice(0,columns-3)+'...':s,value);}
  bitmap(asset,x=0,y=0,{beam=-100,glitch=0,reveal=1}={}){
    const data=asset.data;for(let yy=0;yy<asset.h;yy++)for(let xx=0;xx<asset.w;xx++){
      if(xx>asset.w*reveal+(yy%4)*2)continue;
      let lit=(data[yy*asset.stride+(xx>>3)]>>(7-(xx&7)))&1;
      if(!lit&&Math.abs(xx-beam)<3&&(xx+yy)%2===0&&xx>0&&xx<asset.w-1){const a=(data[yy*asset.stride+((xx-1)>>3)]>>(7-((xx-1)&7)))&1,b=(data[yy*asset.stride+((xx+1)>>3)]>>(7-((xx+1)&7)))&1;lit=a||b;}
      if(lit)this.set(x+xx+(yy>=10&&yy<12?glitch:0),y+yy);
    }
  }
  reticle(x,y,w,h){for(const [xx,sx]of[[x,1],[x+w-1,-1]])for(const[yy,sy]of[[y,1],[y+h-1,-1]]){this.line(xx,yy,xx+sx*3,yy);this.line(xx,yy,xx,yy+sy*2);}}
  rgba(){const out=new Uint8ClampedArray(128*64*4);for(let y=0;y<64;y++)for(let x=0;x<128;x++){const i=(y*128+x)*4,on=this.get(x,y);out[i]=on?(y<16?255:43):0;out[i+1]=on?(y<16?214:190):0;out[i+2]=on?(y<16?55:255):0;out[i+3]=255;}return out;}
}

export class OledRenderer {
  constructor(assets,{reducedMotion=false}={}){this.assets=assets;this.f=new Framebuffer(assets.font);this.reducedMotion=reducedMotion;this.lastPage=null;this.previous=null;this.transitionAt=0;this.cursorY=20;this.cursorAt=0;this.lastTime=0;this.seenRevision=-1;this.resolveKey=null;this.resolveAt=0;}
  header(title){const f=this.f;f.text(2,4,title);f.bitmap(this.assets.smallLogo,75,2);f.line(0,15,85,15);f.line(90,15,127,15);}
  footer(left,right=''){const f=this.f;f.line(0,54,127,54);f.label(2,56,left,20);if(right)f.text(127-right.length*6,56,right);}
  fullLogo(t,reveal=1){this.f.bitmap(this.assets.logo,1,0,{beam:this.reducedMotion?-100:(t%4200)/4200*144-9,glitch:!this.reducedMotion&&(t+650)%3800<160?2:0,reveal});}
  city(t){const f=this.f;f.bitmap(this.assets.city);const car=31+Math.floor(t/70)%89,y=55-Math.floor((car-28)*12/99);f.line(car,y-1,car+2,y-1);for(let i=0;i<4;i++){const yy=56+(i*3+Math.floor(t/500))%8,xx=39+(i*13+Math.floor(t/330))%48;if(Math.abs(xx-65)<(yy-48)*2)f.line(xx,yy,xx+2,yy);}}
  animateCursor(target,now){const dt=Math.max(0,Math.min(100,now-this.lastTime));this.cursorY=this.reducedMotion?target:this.cursorY+(target-this.cursorY)*(1-Math.exp(-dt/45));if(Math.abs(this.cursorY-target)<.15)this.cursorY=target;return Math.round(this.cursorY);}
  selectionBar(y){const f=this.f;for(let yy=y-1;yy<y+10;yy++)for(let x=0;x<128;x++)f.set(x,yy,1-f.get(x,yy));}
  decodeSelected(label,key,now){
    if(this.resolveKey!==key){this.resolveKey=key;this.resolveAt=now;}
    const elapsed=Math.max(0,now-this.resolveAt);
    if(this.reducedMotion||elapsed>=420)return label;
    const resolved=Math.floor(label.length*elapsed/420),tick=Math.floor(elapsed/(1000/12)),alphabet='01ABCDEF';
    return [...label].map((c,i)=>i<resolved||!/[A-Za-z0-9]/.test(c)?c:alphabet[(i*5+tick*3+key.length)%alphabet.length]).join('');
  }
  list(items,index,s,{network=false,title='',hint='OK:OPEN BK:MENU'}={}){
    const f=this.f,first=Math.max(0,index-2);this.header(title);
    if(!items.length){f.text(10,26,'NO NODES FOUND');f.text(10,39,'OK:RESCAN');this.footer('BK:MENU');return;}
    const selectedRow=index-first;
    const y=this.animateCursor(20+selectedRow*11,s.now);
    for(let row=0;row<3;row++){
      const i=first+row;if(i>=items.length)break;
      const yy=20+row*11;
      if(i===index)f.text(0,yy,'>');
      const raw=network?(items[i].ssid||'<hidden>'):items[i],columns=network?15:18;
      let label=raw.length>columns?raw.slice(0,columns-3)+'...':raw;
      if(i===index)label=this.decodeSelected(label,`${s.revision}:${s.page}:${index}:${raw}`,s.now);
      f.text(11,yy,label);
      if(network)f.text(108,yy,String(items[i].rssi));
    }
    // Invert the actual moving band after text, keeping every covered glyph clear.
    this.selectionBar(y);
    this.footer(hint);
  }
  renderPage(s){
    const f=this.f,t=s.now,n=s.node();
    if(s.page==='boot'){
      this.city(this.reducedMotion?0:t);
      this.fullLogo(this.reducedMotion?2000:t,this.reducedMotion?1:Math.min(1,t/780));
      f.rect(0,62,128,2,true,0);
      for(let x=0;x<128;x+=5)if(x<Math.min(1,t/4000)*128)f.line(x,63,Math.min(127,x+2),63);
      return;
    }
    if(s.page==='idle'){this.city(this.reducedMotion?0:t);this.fullLogo(this.reducedMotion?2000:t);return;}
    if(s.page==='menu'){this.list(['NET SCANNER','SIGNAL TRACKER','SECURITY PROFILE','SYSTEM MONITOR','BREACH PROTOCOL'],s.menuIndex,s,{title:'CYBERDECK',hint:`OK:OPEN    ${s.menuIndex+1}/5`});return;}
    if(s.page==='scan'){
      this.header('NET SCANNER');f.text(3,21,`FOUND: ${String(s.scanCount).padStart(2,'0')} NODES`);
      for(let x=8;x<125;x+=19){f.line(x,39,Math.min(123,x+19),39);f.line(x,39,x,48);f.rect(x-1,38,3,3);f.set(x,48);}
      const x=8+Math.floor((t-s.scanStartedAt)/180)%7*19;f.rect(x-2,37,5,5,true);this.footer('DEMO SURVEY BK:MENU');return;
    }
    if(['netlist','trackerSelect','securitySelect'].includes(s.page)){
      const hint=s.page==='netlist'?'OK:DETAIL BK:MENU':s.page==='trackerSelect'?'OK:TRACK BK:MENU':'OK:DATA BK:MENU';
      const title=s.page==='netlist'?`NET ${String(s.nodeIndex+1).padStart(2,'0')}/${String(s.networks.length).padStart(2,'0')}`:'SELECT NODE';
      this.list(s.networks,s.nodeIndex,s,{network:true,title,hint});return;
    }
    if(s.page==='details'){
      this.header('NODE / DATA');const name=n.ssid||'<hidden>';
      f.text(2,19,name.slice(0,21));f.text(2,28,name.slice(21,42));
      f.text(2,38,`RSSI: ${n.rssi} dBm`);f.text(2,46,`CH:${n.channel}  2.4GHz`);
      this.footer('OK:DATA BK:LIST');return;
    }
    if(s.page==='security'){
      this.header(`DATA ${s.dataTab+1}/2`);
      if(s.dataTab===0){const name=n.ssid||'<hidden>';f.text(2,19,name.slice(0,21));f.text(2,28,name.slice(21,42));f.text(2,37,'BSSID');f.text(2,46,n.bssid);}
      else{f.label(2,19,`ENC: ${n.encryption}`,21);f.text(2,30,`CH: ${n.channel} / 2.4GHz`);f.text(2,41,`RSSI: ${n.rssi} dBm`);}
      this.footer('OK:NEXT BK:BACK');return;
    }
    if(s.page==='tracker'){
      this.header('SIG TRACKER');f.label(2,19,n.ssid||'<hidden>',21);
      const left=2,right=125,top=29,bottom=45;
      for(let x=left;x<=right;x+=8){f.set(x,top);f.set(x,bottom);}
      for(let y=top;y<=bottom;y+=4)f.set(left,y);
      const points=s.history.slice(-64),start=Math.max(0,points.length-1);
      const yy=v=>Math.round(bottom-(Math.max(-100,Math.min(-30,v))+100)/70*(bottom-top));
      for(let i=1;i<points.length;i++){const x0=right-(start-i+1)*2,x1=right-(start-i)*2;if(x0>=left)f.line(x0,yy(points[i-1]),x1,yy(points[i]));}
      if(points.length)f.rect(right-1,yy(points.at(-1))-1,3,3,true);
      const q=Math.max(0,Math.min(100,Math.round((s.liveRssi+100)*100/60)));
      f.text(2,46,`${s.liveRssi}dBm CH${n.channel} Q${q}%`);this.footer('UP/DN:NODE BK:LIST');return;
    }
    if(s.page==='system'){
      this.header('SYS MONITOR');const sys=s.demoSystem;
      f.text(2,19,`HEAP ${sys.heap}/${sys.total} KiB`);f.rect(2,30,123,4);f.rect(4,31,Math.round(sys.heap/sys.total*119),2,true);
      const sec=Math.floor(t/1000),h=Math.floor(sec/3600),uptime=`${String(h).padStart(2,'0')}:${String(Math.floor(sec/60)%60).padStart(2,'0')}:${String(sec%60).padStart(2,'0')}`;
      f.label(2,37,`UP ${uptime}`,21);f.text(2,46,`AP ${sys.ap} / STA ${sys.sta}`);this.footer('DEMO METRICS BK:MENU');return;
    }
    if(s.page==='breachIntro'){
      this.header('BREACH');f.text(2,19,'3x3 / 4 BUFFER SLOTS');
      f.text(2,29,`SEQ: ${s.game.target.join(' ')}`);f.text(2,38,'TOP ROW FIRST');f.text(2,46,'30s ON FIRST PICK');this.footer('OK:START BK:MENU');return;
    }
    if(s.page==='abort'){
      this.header('ABORT BREACH');f.text(2,20,'TIMER KEEPS RUNNING');
      for(let i=0;i<2;i++){const label=i===0?'RESUME':'ABORT';f.text(16,33+i*11,i===s.abortIndex?this.decodeSelected(label,`${s.revision}:abort:${i}`,s.now):label);}this.selectionBar(33+s.abortIndex*11);this.footer('OK:CHOOSE BK:RESUME');return;
    }
    if(s.page==='success'||s.page==='failure'){
      const ok=s.page==='success';this.header(ok?'BREACH OK':'BREACH FAIL');
      f.reticle(3,20,122,30);f.text(10,25,ok?'DAEMON UPLOADED':'BREACH FAILED');f.text(10,38,ok?'SEQUENCE MATCH':s.game.reason);
      if(!this.reducedMotion&&t-s.pageAt<500){const edge=20+Math.floor((t-s.pageAt)/500*30);f.line(4,edge,123,edge);}
      this.footer('OK:RETRY BK:MENU');return;
    }
    if(s.page==='breach')this.breach(s);
  }
  breach(s){
    const f=this.f,g=s.game,remaining=Math.ceil(g.remaining(s.now)/1000);
    f.text(2,0,'BREACH');f.text(103,0,`${String(remaining).padStart(2,'0')}s`);
    f.text(2,8,'SEQ');f.text(49,8,g.target.join(' '));f.line(0,16,127,16);
    const legal=g.legal();
    for(let i=0;i<9;i++){
      const x=2+(i%3)*20,y=20+Math.floor(i/3)*12,used=g.used.includes(i),selected=g.cursor===i;
      if(selected){f.rect(x,y,18,11,true);f.text(x+3,y+2,g.matrix[i],0);}
      else{
        if(legal.includes(i))f.reticle(x,y,18,11);
        f.text(x+3,y+2,g.matrix[i]);
        if(used){f.line(x,y+10,x+17,y);f.set(x+17,y+10);}
      }
    }
    f.line(64,19,64,55);f.text(70,19,'BUFFER');
    for(let i=0;i<4;i++){const x=70+i%2*28,y=29+Math.floor(i/2)*10;f.text(x,y,g.buffer[i]??'--');if(i===g.buffer.length)f.line(x,y+8,x+10,y+8);}
    const p=g.progress();for(let i=0;i<g.target.length;i++)f.rect(70+i*9,50,6,4,i<p);
    f.text(105,48,`${g.buffer.length}/4`);
    f.text(2,56,`${g.axis==='row'?'ROW':'COL'} ${g.axisIndex+1}`);f.text(86,56,'OK:ADD');
  }
  render(s){
    const changed=this.lastPage!==s.page;
    if(changed){this.previous=this.lastPage===null?null:this.f.bits.slice();this.transitionAt=s.now;this.cursorY=20;this.lastPage=s.page;}
    this.f.clear();this.renderPage(s);
    const dt=s.now-this.transitionAt;
    const exempt=['boot','idle','breach','success','failure'].includes(s.page);
    if(this.previous&&!this.reducedMotion&&!exempt&&dt<180){
      const target=this.f.bits.slice(),offset=Math.max(1,Math.round((1-dt/180)*128));
      const read=(data,x,y)=>x>=0&&x<128?((data[y*16+(x>>3)]>>(7-(x&7)))&1):0;
      for(let y=16;y<64;y++)for(let x=0;x<128;x++)this.f.set(x,y,x<offset?read(this.previous,x+128-offset,y):read(target,x-offset,y));
    }
    this.lastTime=s.now;return this.f;
  }
}
