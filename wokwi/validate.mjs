import fs from 'node:fs';
import assert from 'node:assert/strict';
const d=JSON.parse(fs.readFileSync(new URL('diagram.json',import.meta.url)));
function netlist(pressed){
 const p=new Map();
 const find=x=>{if(!p.has(x))p.set(x,x);if(p.get(x)!==x)p.set(x,find(p.get(x)));return p.get(x);};
 const join=(a,b)=>p.set(find(a),find(b));
 for(let c=1;c<=63;c++)for(const [bank,rows] of [['t','abcde'],['b','fghij']])for(const r of rows)join(`bb1:${c}${bank}.${r}`,`bb1:${c}${bank}.${rows[0]}`);
 // Model each full-size power rail as two separate halves.
 for(const rail of ['tp','tn','bp','bn'])for(let h=1;h<=50;h++)join(`bb1:${rail}.${h}`,`bb1:${rail}.${h<=25?1:26}`);
 for(const id of ['up','down','enter','back']){join(`${id}:1.l`,`${id}:1.r`);join(`${id}:2.l`,`${id}:2.r`);if(id===pressed)join(`${id}:1.l`,`${id}:2.l`);}
 for(const [a,b] of d.connections)join(a,b);
 return (a,b)=>find(a)===find(b);
}
const tied=netlist();
const ports=['esp:GND.2','esp:3V3','esp:D21','esp:D22','esp:D12','esp:D27','esp:D14','esp:D13'];
for(let i=0;i<8;i++)assert(tied(`bb1:${i+1}t.a`,ports[i]),`Remote input ${i+1} disconnected`);
for(let i=0;i<8;i++)for(let j=i+1;j<8;j++)assert(!tied(ports[i],ports[j]),`Shorted ports ${i+1},${j+1}`);
assert(tied('oled:GND','esp:GND.2'));assert(tied('oled:VCC','esp:3V3'));assert(tied('oled:SCL','esp:D22'));assert(tied('oled:SDA','esp:D21'));
const buttons=[['up',14],['down',13],['enter',12],['back',27]];
for(const [id,gpio] of buttons){
 assert(!tied(`esp:D${gpio}`,'esp:GND.2'));
 const on=netlist(id);
 for(const [other,pin] of buttons)assert.equal(on(`esp:D${pin}`,'esp:GND.2'),other===id,`${id} press must affect only its own GPIO`);
}
const lowerColumns=new Set([13,15,46,48]);
for(const [a,b,color] of d.connections){if(!color)continue;for(const end of [a,b]){const m=/^bb1:(\d+)b\.([f-j])$/.exec(end);if(m&&lowerColumns.has(Number(m[1])))assert.equal(m[2],'f','Lower button jumpers must enter ABOVE the cap, never below');}}
console.log('PASS: eight isolated remote inputs, OLED, all four button closures, and relocated lower jumper holes.');
