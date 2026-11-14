import fs from 'node:fs';
import path from 'node:path';
import assert from 'node:assert/strict';
import {fileURLToPath} from 'node:url';
const root=fileURLToPath(new URL('../',import.meta.url));
const source=fs.readFileSync(path.join(root,'firmware/Cyberdeck_Hardware/Cyberdeck_Hardware.ino'),'utf8');
assert.match(source,/#define DECK_DEMO 0/);assert.doesNotMatch(source,/^#include "/m);
assert.match(source,/BOOT_MS=10000,POWER_OFF_MS=900/);assert.match(source,/GLITCH_PERIOD_MS=2000,GLITCH_BURST_MS=320/);
const ignore=new Set(['.git','qa','.cache','node_modules','build']);let checked=0,links=0;
function visit(directory){for(const entry of fs.readdirSync(directory,{withFileTypes:true})){
  if(ignore.has(entry.name))continue;const file=path.join(directory,entry.name);if(entry.isDirectory()){visit(file);continue;}
  if(!/\.(?:md|mjs|js|json|html|h|ino|cpp|yml|yaml|txt)$/.test(entry.name))continue;
  const content=fs.readFileSync(file,'utf8');checked++;
  assert.doesNotMatch(content,/[A-Za-z]:[\\/]Users[\\/]|[A-Za-z]:[\\/]Program Files[\\/]|gh[pousr]_[A-Za-z0-9]{20,}|github_pat_[A-Za-z0-9_]{20,}|-----BEGIN (?:RSA |OPENSSH )?PRIVATE KEY-----/,'Private path or credential in '+path.relative(root,file));
  if(entry.name.endsWith('.md'))for(const match of content.matchAll(/!?\[[^\]]*\]\(([^)]+)\)/g)){
    const target=match[1].split(/\s+"/)[0];if(/^(https?:|mailto:|#)/.test(target))continue;
    const relative=decodeURIComponent(target.split('#')[0]);assert.ok(fs.existsSync(path.resolve(path.dirname(file),relative)),'Broken local link: '+target+' in '+path.relative(root,file));links++;
  }
}}
visit(root);console.log(`PASS: ${checked} text files checked, ${links} local documentation links, production defaults and portable paths.`);
