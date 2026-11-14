// Render real 128x64 preview frames. The GIF uses fictional preview networks.
import fs from 'node:fs';
import {spawnSync} from 'node:child_process';
import {deflateSync} from 'node:zlib';
import {fileURLToPath} from 'node:url';
import {ASSETS} from '../preview/base/assets.mjs';
import {Framebuffer} from '../preview/base/renderer.mjs';
import {PowerCyberdeck} from '../preview/power/power-model.mjs';
import {PowerRenderer} from '../preview/power/power-renderer.mjs';
const root=new URL('../',import.meta.url),out=new URL('docs/images/',root),temp=new URL('.cache/media/',root);
fs.mkdirSync(out,{recursive:true});fs.mkdirSync(temp,{recursive:true});
function crc(bytes){let value=0xffffffff;for(const byte of bytes){value^=byte;for(let i=0;i<8;i++)value=(value>>>1)^((value&1)?0xedb88320:0);}return(value^0xffffffff)>>>0;}
function chunk(type,data){const name=Buffer.from(type),size=Buffer.alloc(4),sum=Buffer.alloc(4);size.writeUInt32BE(data.length);sum.writeUInt32BE(crc(Buffer.concat([name,data])));return Buffer.concat([size,name,data,sum]);}
function png(width,height,pixels){const rows=Buffer.alloc(height*(width*4+1));for(let y=0;y<height;y++)rows.set(pixels.subarray(y*width*4,(y+1)*width*4),y*(width*4+1)+1);const info=Buffer.alloc(13);info.writeUInt32BE(width);info.writeUInt32BE(height,4);info[8]=8;info[9]=6;return Buffer.concat([Buffer.from([137,80,78,71,13,10,26,10]),chunk('IHDR',info),chunk('IDAT',deflateSync(rows)),chunk('IEND',Buffer.alloc(0))]);}
const model=new PowerCyberdeck(),renderer=new PowerRenderer(ASSETS);model.togglePower();
const shots=[];
function shot(label,time,page){model.tick(time);if(page)model.go(page);renderer.render(model);model.tick(model.now+500);shots.push({label,pixels:renderer.render(model).rgba()});}
shot('BOOT / PIXEL PREVIEW',7000);model.tick(10000);shot('MAIN MENU',10500);model.beginScan();model.tick(model.now+2600);shot('SCANNER / SAMPLE DATA',model.now+500);model.menuIndex=4;model.go('breachIntro');model.input('enter',model.now);shot('CODE MATRIX / GAME',model.now+300);
const width=1088,height=640,pixels=Buffer.alloc(width*height*4);for(let i=0;i<pixels.length;i+=4){pixels[i]=9;pixels[i+1]=13;pixels[i+2]=19;pixels[i+3]=255;}
for(let i=0;i<shots.length;i++){
 const tx=24+(i%2)*544,ty=20+Math.floor(i/2)*320,label=new Framebuffer(ASSETS.font);label.text(0,0,shots[i].label);const caption=label.rgba();
 for(let y=0;y<64;y++)for(let x=0;x<128;x++)for(let dy=0;dy<4;dy++)for(let dx=0;dx<4;dx++){const index=((ty+42+y*4+dy)*width+tx+x*4+dx)*4;pixels.set(shots[i].pixels.subarray((y*128+x)*4,(y*128+x)*4+4),index);}
 for(let y=0;y<8;y++)for(let x=0;x<128;x++)for(let dy=0;dy<3;dy++)for(let dx=0;dx<3;dx++){const index=((ty+y*3+dy)*width+tx+x*3+dx)*4;pixels.set(caption.subarray((y*128+x)*4,(y*128+x)*4+4),index);}
}
fs.writeFileSync(new URL('preview-overview.png',out),png(width,height,pixels));
const live=new PowerCyberdeck(),screen=new PowerRenderer(ASSETS);
const actions=[[300,'power'],[10700,'enter'],[13600,'enter'],[14500,'back'],[15100,'back'],[15600,'down'],[15800,'down'],[16000,'down'],[16200,'down'],[16900,'enter'],[17500,'enter'],[18100,'down'],[18300,'enter'],[18700,'enter'],[19100,'down'],[19300,'enter'],[20700,'power']];
let next=0;const scale=4,w=128*scale,h=64*scale,frames=264;
for(let frame=0;frame<frames;frame++){
 const time=Math.round(frame*1000/12);while(next<actions.length&&actions[next][0]<=time){const[t,key]=actions[next++];if(key==='power')live.togglePower(t);else live.input(key,t);}live.tick(time);const rgba=screen.render(live).rgba(),rgb=Buffer.alloc(w*h*3);
 for(let y=0;y<h;y++)for(let x=0;x<w;x++){const from=(Math.floor(y/scale)*128+Math.floor(x/scale))*4,to=(y*w+x)*3;rgb[to]=rgba[from];rgb[to+1]=rgba[from+1];rgb[to+2]=rgba[from+2];}
 fs.writeFileSync(new URL(String(frame).padStart(4,'0')+'.ppm',temp),Buffer.concat([Buffer.from(`P6\n${w} ${h}\n255\n`),rgb]));
}
const result=spawnSync('ffmpeg',['-hide_banner','-loglevel','error','-y','-framerate','12','-i',fileURLToPath(temp)+'%04d.ppm','-filter_complex','[0:v]split[a][b];[a]palettegen=stats_mode=diff[p];[b][p]paletteuse=dither=none','-loop','0',fileURLToPath(new URL('cyberdeck-demo.gif',out))],{encoding:'utf8'});
if(result.error||result.status!==0){console.error('PNG rendered; GIF generation needs ffmpeg in PATH. '+(result.error?.message||result.stderr));process.exitCode=1;}else console.log('Rendered overview PNG and 22-second / 12fps preview GIF.');
