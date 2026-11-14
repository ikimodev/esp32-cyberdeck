import fs from 'node:fs';
import {ASSETS} from '../preview/base/assets.mjs';
import {PowerCyberdeck as Cyberdeck} from '../preview/power/power-model.mjs';
import {PowerRenderer as OledRenderer} from '../preview/power/power-renderer.mjs';
const root=new URL('../',import.meta.url),sketch=new URL('firmware/cyberdeck/',root);
fs.mkdirSync(new URL('qa/',root),{recursive:true});
const rows=[];let state,renderer;
function scenario(commands){state=new Cyberdeck();state.togglePower(0);state.revision=0;renderer=new OledRenderer(ASSETS);commands.forEach((command,index)=>{const [time,key='']=command;if(key==='power')state.togglePower(time);else if(key)state.input(key,time);else state.tick(time);const bits=renderer.render(state).bits;let hash=2166136261;for(const byte of bits){hash^=byte;hash=Math.imul(hash,16777619)>>>0;}rows.push({reset:index===0,time,key,hash,page:state.page,hex:Buffer.from(bits).toString('hex')});});}
scenario([[0],[200],[649],[650],[1000],[1599],[1600],[2100],[2800],[3600],[3640],[3920],[5600],[5640],[5920],[7600],[7920],[8799],[8800],[9000],[9600],[9999],[10000],[10083],[10180],[10430],[10500,'down'],[10540],[10680],[11000],[11001,'back'],[12000],[12001,'up'],[13000],[59002]]);
scenario([[0],[10000],[10500,'enter'],[10800],[11600],[13100],[13140],[13280],[13600],[13700,'down'],[13800],[14200],[14300,'enter'],[14480],[14800,'enter'],[14980],[15100,'down'],[15300],[15400,'back'],[15600,'back']]);
scenario([[0],[10000],[10100,'down'],[10200,'enter'],[11000],[12800],[13000,'down'],[13500],[13600,'enter'],[13850],[14100],[15100],[21100],[30100],[30200,'down'],[30500],[31000,'back']]);
scenario([[0],[10000],[10100,'down'],[10200,'down'],[10300,'enter'],[12900],[13500,'down'],[14000],[14100,'enter'],[14300],[14400,'enter'],[14600],[14700,'back']]);
scenario([[0],[10000],[10100,'down'],[10200,'down'],[10300,'down'],[10400,'enter'],[10580],[18000],[22000],[24000,'back']]);
const intro=[[0],[10000],[10100,'down'],[10200,'down'],[10300,'down'],[10400,'down'],[10500,'enter'],[10700],[11000,'enter']];
scenario([...intro,[11100,'down'],[11200,'enter'],[11300,'back'],[11400,'down'],[11500,'up'],[11600,'enter'],[11700,'enter'],[11800,'down'],[11900,'enter'],[12100],[12600],[12700,'enter'],[12900],[13000,'enter'],[13100,'down'],[13200,'enter']]);
scenario([...intro,[11200,'enter'],[11300,'enter'],[11400,'enter'],[11500,'enter'],[11700],[12100],[12200,'back']]);
scenario([...intro,[14000,'enter'],[14100,'back'],[43999],[44000,'enter'],[44200],[45000]]);
scenario([[0],[1000,'enter'],[5000,'power'],[5300],[5510],[5600],[5799],[5900],[25000],[25001,'power'],[25651],[26601],[28601],[30641],[34999],[35001],[35500,'down'],[36000,'power'],[36510],[36900],[50000]]);
let fixture='#pragma once\nnamespace Deck {\nstruct GoldenStep {bool reset;uint64_t time;const char* key;uint32_t hash;const char* page;};\nstatic const GoldenStep goldenSteps[]={\n';
fixture+=rows.map(r=>`{${r.reset},${r.time},"${r.key}",${r.hash}u,"${r.page}"}`).join(',\n');fixture+='\n};\n}\n';
fs.writeFileSync(new URL('golden_frames.h',sketch),fixture);
fs.writeFileSync(new URL('qa/golden-frames.json',root),JSON.stringify(rows,null,2));

console.log('Generated '+rows.length+' reference frames (not a hardware comparison).');
