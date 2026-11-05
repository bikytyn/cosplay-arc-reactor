#pragma once
#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="cs"><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1">
<title>MK II // Arc Reactor</title>
<style>
:root{--c:#2a7bff;--h:#bfe8ff;--bg:#05080d;--p:#0c1420;--l:#1b2a3d;--t:#cfe6ff;--d:#6c87a6}
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
body{margin:0;background:radial-gradient(circle at 50% 0,#0b1a2e,var(--bg) 60%);color:var(--t);
 font:15px/1.4 system-ui,-apple-system,Segoe UI,Roboto,sans-serif;min-height:100vh}
main{max-width:460px;margin:0 auto;padding:18px 16px 40px}
header{display:flex;justify-content:space-between;align-items:baseline;letter-spacing:.18em;text-transform:uppercase}
h1{font-size:15px;margin:0;font-weight:700}
h1 b{color:var(--c);text-shadow:0 0 12px var(--c)}
#st{font-size:11px;color:var(--d)}
#st.on{color:#5dffb0}
.rx{position:relative;width:200px;height:200px;margin:26px auto 22px;border-radius:50%;
 background:radial-gradient(circle,#000 52%,var(--l) 53%,#000 56%,#000 70%,var(--l) 71%,#000 74%);
 box-shadow:0 0 0 2px var(--l),0 0 60px -10px var(--c);transition:box-shadow .4s,opacity .4s}
.ring{position:absolute;inset:16%;border-radius:50%;border:12px solid var(--c);
 box-shadow:0 0 18px var(--c),inset 0 0 18px var(--c);opacity:.9}
.dot{position:absolute;inset:40%;border-radius:50%;background:var(--h);box-shadow:0 0 25px 8px var(--h);
 animation:br 3s ease-in-out infinite}
.off .ring,.off .dot{opacity:.06;box-shadow:none;animation:none}
.rx.off{box-shadow:0 0 0 2px var(--l)}
@keyframes br{50%{opacity:.75}}
.m-1 .ring{border-color:transparent;border-top-color:var(--c);animation:sp .7s linear infinite}
.m-3 .ring{border-color:#f00 #ff0 #0f8 #08f;animation:sp 2s linear infinite}
.m-4 .ring,.m-4 .dot{animation:rv .25s steps(2) infinite}
.m-5 .ring{border-color:#ff3a10;box-shadow:0 0 18px #f30;animation:fl .15s infinite}
.m-2 .ring,.m-2 .dot{animation:hb 1s infinite}
@keyframes sp{to{transform:rotate(360deg)}}
@keyframes rv{50%{filter:hue-rotate(180deg) brightness(1.8)}}
@keyframes fl{50%{opacity:.45}}
@keyframes hb{0%,40%,100%{opacity:.35}10%,30%{opacity:1}}
section{background:var(--p);border:1px solid var(--l);border-radius:14px;padding:14px;margin:12px 0}
h2{font-size:11px;letter-spacing:.2em;text-transform:uppercase;color:var(--d);margin:0 0 10px;font-weight:600}
.g{display:grid;grid-template-columns:repeat(3,1fr);gap:8px}
.g2{grid-template-columns:1fr 1fr}
button{background:#0f1b2a;color:var(--t);border:1px solid var(--l);border-radius:10px;padding:12px 6px;
 font:600 13px system-ui;letter-spacing:.04em;cursor:pointer;transition:.15s}
button:active{transform:scale(.96)}
button.a{border-color:var(--c);box-shadow:0 0 14px -3px var(--c),inset 0 0 12px -6px var(--c);color:#fff}
#pw{width:100%;padding:14px;font-size:14px;letter-spacing:.2em}
#pw.a{background:linear-gradient(#123,#0a1626)}
.big{background:linear-gradient(135deg,#132a44,#0b1626)}
#uni{background:linear-gradient(135deg,#3a2a08,#1a1206);border-color:#6a4d14;color:#ffd98a}
.row{display:flex;align-items:center;gap:12px;margin:8px 0}
.row label{flex:0 0 74px;font-size:13px;color:var(--d)}
.row output{flex:0 0 38px;text-align:right;font-variant-numeric:tabular-nums;font-size:13px}
input[type=range]{flex:1;accent-color:var(--c);height:28px}
input[type=color]{flex:1;height:38px;border:1px solid var(--l);border-radius:10px;background:none;padding:2px}
.ps{display:flex;flex-wrap:wrap;gap:8px;margin-top:10px}
.ps button{flex:1 1 30%;padding:9px 4px;font-size:12px;display:flex;align-items:center;gap:6px;justify-content:center}
.ps i{width:12px;height:12px;border-radius:50%;display:inline-block}
.warn{font-size:11px;color:#c98a3a;margin:2px 0 0;min-height:14px}
footer{text-align:center;font-size:11px;color:var(--d);letter-spacing:.15em;margin-top:20px}
</style></head><body><main>
<header><h1>Mark <b>II</b> // Arc Reactor</h1><span id="st">…</span></header>

<div class="rx" id="rx"><div class="ring"></div><div class="dot"></div></div>

<button id="pw">Power</button>

<section><h2>Režim</h2>
<div class="g" id="modes">
<button data-m="0">Reactor</button><button data-m="1">Spin</button><button data-m="2">Pulse</button>
<button data-m="3">Rainbow</button><button data-m="4">Rave</button><button data-m="5">Overload</button>
</div></section>

<section><h2>Akce</h2>
<div class="g g2"><button class="big" id="boot">⟳ Boot sekvence</button><button id="uni">✦ Unibeam</button></div>
<div class="row"><label>Délka bootu</label><input type="range" id="bl" min="2" max="30"><output id="blo"></output></div>
</section>

<section><h2>Barvy</h2>
<div class="row"><label>Ring</label><input type="color" id="ring"></div>
<div class="row"><label>Střed</label><input type="color" id="center"></div>
<div class="ps" id="ps"></div>
</section>

<section><h2>Nastavení</h2>
<div class="row"><label>Rychlost</label><input type="range" id="speed" min="1" max="100"><output id="so"></output></div>
<div class="row"><label>Jas</label><input type="range" id="bri" min="5" max="255"><output id="bo"></output></div>
<p class="warn" id="bw"></p>
</section>

<footer>J.A.R.V.I.S. link // mk2.local</footer>
</main>
<script>
const $=id=>document.getElementById(id);
const P=[["Mark II","2a7bff","bfe8ff"],["New Element","00d5ff","ffffff"],["Vibranium","9b3bff","e6c8ff"],
 ["Fialová","a000ff","ff60ff"],["Hulkbuster","ff7a00","ffe0a0"],["Stealth","ff0018","ff6060"]];
let S={};
P.forEach(p=>{const b=document.createElement('button');
 b.innerHTML=`<i style="background:#${p[1]};box-shadow:0 0 8px #${p[1]}"></i>${p[0]}`;
 b.onclick=()=>set({ring:p[1],center:p[2]});$('ps').appendChild(b)});
function ui(){
 $('st').textContent=S.power?'online':'standby';$('st').className=S.power?'on':'';
 $('pw').textContent=S.power?'⏻  Online':'⏻  Standby';$('pw').classList.toggle('a',!!S.power);
 document.querySelectorAll('#modes button').forEach(b=>b.classList.toggle('a',+b.dataset.m===S.mode));
 if(document.activeElement!==$('ring'))$('ring').value='#'+S.ring;
 if(document.activeElement!==$('center'))$('center').value='#'+S.center;
 if(document.activeElement!==$('speed'))$('speed').value=S.speed;
 if(document.activeElement!==$('bri'))$('bri').value=S.bri;
 if(document.activeElement!==$('bl'))$('bl').value=S.boot;$('blo').textContent=S.boot+' s';
 $('so').textContent=S.speed;$('bo').textContent=S.bri;
 $('bw').textContent=S.bri>150?'⚠ Vysoký jas = velký odběr (až ~1,6 A), baterie poteče rychle':'';
 const r=document.documentElement.style;r.setProperty('--c','#'+S.ring);r.setProperty('--h','#'+S.center);
 $('rx').className='rx m-'+S.mode+(S.power?'':' off');
}
function set(p){Object.assign(S,p);ui();
 return fetch('/api/set?'+new URLSearchParams(p)).then(r=>r.json()).then(j=>{S=j;ui()}).catch(()=>{$('st').textContent='offline';$('st').className=''})}
const T={};function th(k,p){clearTimeout(T[k]);T[k]=setTimeout(()=>set(p),70)}
$('pw').onclick=()=>set({power:S.power?0:1});
document.querySelectorAll('#modes button').forEach(b=>b.onclick=()=>set({mode:+b.dataset.m,power:1}));
$('boot').onclick=()=>fetch('/api/boot').then(r=>r.json()).then(j=>{S=j;ui()});
$('uni').onclick=()=>fetch('/api/unibeam');
$('ring').oninput=e=>th('r',{ring:e.target.value.slice(1)});
$('center').oninput=e=>th('c',{center:e.target.value.slice(1)});
$('speed').oninput=e=>{$('so').textContent=e.target.value;th('s',{speed:e.target.value})};
$('bl').oninput=e=>{$('blo').textContent=e.target.value+' s';th('l',{boot:e.target.value})};
$('bri').oninput=e=>{$('bo').textContent=e.target.value;th('b',{bri:e.target.value})};
fetch('/api/state').then(r=>r.json()).then(j=>{S=j;ui()});
</script></body></html>)rawliteral";
