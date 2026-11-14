import {Cyberdeck} from '../base/core.mjs';
export const POWER_BOOT_MS=10000, POWER_OFF_MS=900;

// Preview-only power state: off -> ten-second startup -> existing interface.
export class PowerCyberdeck extends Cyberdeck {
  constructor(options={}){super(options);this.page='off';this.bootStartedAt=0;this.powerAt=0;}
  togglePower(time=this.now){
    this.tick(time);
    if(this.page==='shutdown')return;
    if(this.page==='off'){
      const now=this.now,revision=this.revision,menuIndex=this.menuIndex;
      Object.assign(this,new Cyberdeck({networks:this.provider,seed:this.seed}));
      this.now=now;this.pageAt=now;this.bootStartedAt=now;this.powerAt=now;this.lastInputAt=now;this.menuIndex=menuIndex;this.revision=revision+1;
    }else{this.powerAt=this.now;this.go('shutdown');}
  }
  tick(time){
    if(!Number.isFinite(time))return;
    this.now=Math.max(this.now,time);
    if(this.page==='off')return;
    if(this.page==='shutdown'){if(this.now-this.powerAt>=POWER_OFF_MS)this.go('off');return;}
    if(this.page==='boot'){
      if(this.now-this.bootStartedAt>=POWER_BOOT_MS){this.lastInputAt=this.now;this.go('menu');}
      return;
    }
    super.tick(this.now);
  }
  input(key,time=this.now){this.tick(time);if(['off','boot','shutdown'].includes(this.page))return;super.input(key,this.now);}
  get description(){
    if(this.page==='off')return 'الجهاز مطفأ';
    if(this.page==='boot')return 'CYBERDECK // POWER ON';
    if(this.page==='shutdown')return 'إطفاء الجهاز';
    return super.description;
  }
}
