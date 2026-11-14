import {OledRenderer} from '../base/renderer.mjs';
import {POWER_OFF_MS} from './power-model.mjs';

const unit=(value)=>Math.max(0,Math.min(1,value));
const ease=value=>{const p=unit(value);return p*p*(3-2*p);};
export const LOGO_GLITCH_PERIOD_MS=2000, LOGO_GLITCH_BURST_MS=320;
export class PowerRenderer extends OledRenderer {
  constructor(...args){super(...args);this.shutdownFrame=null;}
  pixel(bits,x,y){return x>=0&&x<128&&y>=0&&y<64?(bits[y*16+(x>>3)]>>(7-(x&7)))&1:0;}
  glitchLogo(asset,x,y,time,reveal=1){
    const f=this.f,phase=((time%LOGO_GLITCH_PERIOD_MS)+LOGO_GLITCH_PERIOD_MS)%LOGO_GLITCH_PERIOD_MS;
    f.bitmap(asset,x,y,{beam:this.reducedMotion?-100:phase/LOGO_GLITCH_PERIOD_MS*(asset.w+18)-9,reveal});
    if(this.reducedMotion||phase>=LOGO_GLITCH_BURST_MS)return;
    // Brief horizontal tears and a sparse echo; never invert/flash the whole display.
    const source=f.bits.slice(),magnitude=asset.w>100?6:3;
    const direction=phase<100?1:phase<220?-1:1;
    const strips=[[.28,2,direction*magnitude],[.56,asset.h>15?3:2,-direction*(magnitude-1)],[.8,1,direction*(magnitude+1)]];
    for(const [fraction,height,shift]of strips){
      const start=y+Math.floor(asset.h*fraction);
      for(let yy=start;yy<Math.min(y+asset.h,start+height);yy++)for(let xx=x;xx<x+asset.w;xx++){
        const from=xx-shift,ghost=xx+shift;
        const lit=from>=x&&from<x+asset.w&&this.pixel(source,from,yy);
        const echo=phase<160&&(xx+yy)%3===0&&ghost>=x&&ghost<x+asset.w&&this.pixel(source,ghost,yy);
        f.set(xx,yy,lit||echo?1:0);
      }
    }
  }
  fullLogo(time,reveal=1){this.glitchLogo(this.assets.logo,1,0,time,reveal);}
  header(title){
    const f=this.f;f.text(2,4,title);this.glitchLogo(this.assets.smallLogo,75,2,this.logoTime??0);
    f.line(0,15,85,15);f.line(90,15,127,15);
  }
  scaleFrame(bits,sx,sy){
    const f=this.f;f.bits.fill(0);
    if(sx<=0||sy<=0)return;
    const halfX=64*sx,halfY=32*sy;
    for(let y=0;y<64;y++)for(let x=0;x<128;x++){
      if(Math.abs(x-63.5)>halfX||Math.abs(y-31.5)>halfY)continue;
      const xx=Math.round((x-63.5)/sx+63.5),yy=Math.round((y-31.5)/sy+31.5);
      if(this.pixel(bits,xx,yy))f.set(x,y);
    }
  }
  startup(s){
    const f=this.f,t=s.now-s.bootStartedAt;
    if(t<650)return;
    if(this.reducedMotion){
      if(t<8800){this.city(0);this.fullLogo(2000);}else{const menu=Object.create(s);menu.page='menu';super.renderPage(menu);}return;
    }
    // A brief power seam opens into the image; it is not a progress meter.
    if(t<1600){
      const half=1+Math.round(ease((t-650)/950)*39);
      f.line(64-half,31,63+half,31);if(t>1200){f.set(63,30);f.set(64,32);}return;
    }
    this.city(t-1600);this.fullLogo(t-1600);
    if(t<3600){
      const frame=f.bits.slice(),p=ease((t-1600)/2000);
      this.scaleFrame(frame,.62+.38*p,.025+.975*p);return;
    }
    if(t<8800)return;
    // The menu unfolds from the same center, keeping the original pixels/identity.
    const city=f.bits.slice(),menu=Object.create(s);menu.page='menu';f.clear();super.renderPage(menu);
    const next=f.bits.slice(),p=ease((t-8800)/1200),hy=Math.ceil(32*p),hx=Math.ceil(64*Math.min(1,p*1.4));
    f.bits.set(city);
    for(let y=0;y<64;y++)for(let x=0;x<128;x++)if(Math.abs(y-31.5)<=hy&&Math.abs(x-63.5)<=hx)f.set(x,y,this.pixel(next,x,y));
  }
  render(s){
    this.logoTime=Math.max(0,s.now-(s.bootStartedAt??0)-1600);
    const powerPage=['off','boot','shutdown'].includes(s.page);
    if(!powerPage){
      if(['off','boot','shutdown'].includes(this.lastPage)){this.lastPage=s.page;this.previous=null;}
      return super.render(s);
    }
    const entered=this.lastPage!==s.page;
    if(entered&&s.page==='shutdown')this.shutdownFrame=this.f.bits.slice();
    this.lastPage=s.page;this.f.clear();
    if(s.page==='boot')this.startup(s);
    if(s.page==='shutdown'&&!this.reducedMotion){
      const t=s.now-s.powerAt;
      if(t<510){const p=ease(t/510);this.scaleFrame(this.shutdownFrame??new Uint8Array(1024),1-p*.15,Math.max(.015,1-p));}
      else if(t<760){const half=Math.round((1-ease((t-510)/250))*53);if(half) this.f.line(64-half,31,63+half,31);}
      else if(t<POWER_OFF_MS-40)this.f.set(64,31);
    }
    this.lastTime=s.now;return this.f;
  }
}
