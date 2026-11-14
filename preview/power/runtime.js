const root=document.getElementById('cyberdeck-power-preview');
const canvas=root.querySelector('canvas'),context=canvas.getContext('2d',{alpha:false});context.imageSmoothingEnabled=false;
const status=root.querySelector('#deck-status-v42'),reader=root.querySelector('#deck-reader-v42'),power=root.querySelector('#deck-reboot-v42');
const buttons=Object.fromEntries([...root.querySelectorAll('[data-key]')].map(button=>[button.dataset.key,button]));
const reduced=typeof matchMedia==='function'&&matchMedia('(prefers-reduced-motion: reduce)').matches;
const model=new PowerCyberdeck(),renderer=new PowerRenderer(ASSETS,{reducedMotion:reduced});
let epoch=performance.now(),lastPaint=-100,lastDescription='',disposed=false,hasInput=false;
function restore(saved){const index=saved?.privateContent?.menuIndex;if(!hasInput&&['off','boot'].includes(model.page)&&Number.isInteger(index))model.menuIndex=clamp(index,0,MENU.length-1);}
restore(window.openai?.widgetState);
function receive(event){restore(event.detail?.globals?.widgetState);}
window.addEventListener('openai:set_globals',receive);
function remember(){
  if(typeof window.openai?.setWidgetState==='function')window.openai.setWidgetState({modelContent:{preview:'Cyberdeck — power preview',selectedModule:MENU[model.menuIndex],power:model.page==='off'?'off':'on'},privateContent:{menuIndex:model.menuIndex}}).catch(()=>{});
}
function describe(){
  const locked=['off','boot','shutdown'].includes(model.page);
  for(const button of Object.values(buttons))button.disabled=locked;
  power.disabled=model.page==='shutdown';power.textContent=model.page==='off'?'تشغيل الجهاز':'إطفاء الجهاز';
  power.setAttribute('aria-label',power.textContent);power.setAttribute('aria-pressed',String(model.page!=='off'));
  root.dataset.page=model.page;
  let text=model.description;
  if(['netlist','trackerSelect','securitySelect','details','tracker','security'].includes(model.page)&&model.networks.length)text+=' — '+nodeName(model.node());
  if(model.page==='breach'){const g=model.game;text+=` — ${g.axis==='row'?'صف':'عمود'} ${g.axisIndex+1}: ${g.matrix[g.cursor]} — Buffer: ${g.buffer.join(' ')||'فارغ'}`;}
  if(text!==lastDescription){status.textContent=text;reader.textContent=text;canvas.setAttribute('aria-label',text);lastDescription=text;}
}
function paint(){context.putImageData(new ImageData(renderer.render(model).rgba(),128,64),0,0);describe();}
function input(key){hasInput=true;model.input(key,performance.now()-epoch);paint();remember();const button=buttons[key];button.classList.add('pressed');setTimeout(()=>button.classList.remove('pressed'),120);}
for(const [key,button]of Object.entries(buttons))button.addEventListener('click',()=>input(key));
power.addEventListener('click',()=>{hasInput=true;model.togglePower(performance.now()-epoch);paint();remember();});
function keyboard(event){
  if(!root.isConnected)return;
  const active=document.activeElement;
  if(active&&active!==document.body&&!root.contains(active))return;
  if(active===power||active?.tagName==='SUMMARY')return;
  const key={ArrowUp:'up',ArrowDown:'down',Enter:'enter',Escape:'back'}[event.key];if(!key)return;
  event.preventDefault();if(!event.repeat)input(key);
}
document.addEventListener('keydown',keyboard);
function animate(timestamp){
  if(disposed)return;
  if(!root.isConnected){disposed=true;document.removeEventListener('keydown',keyboard);window.removeEventListener('openai:set_globals',receive);return;}
  model.tick(timestamp-epoch);if(timestamp-lastPaint>=1000/12){paint();lastPaint=timestamp;}requestAnimationFrame(animate);
}
paint();requestAnimationFrame(animate);
