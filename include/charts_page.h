#pragma once
#include <Arduino.h>

const char CHARTS_PAGE[] PROGMEM = R"html(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Sensor charts</title>
<style>
*{box-sizing:border-box}body{margin:0;background:#f2f3f5;color:#18334b;font:16px system-ui,sans-serif}
main{max-width:1200px;margin:auto;padding:24px}a{color:#006caa}h1{margin-bottom:8px}
.charts{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:20px}
section{background:white;padding:20px;border-radius:16px;box-shadow:0 4px 20px #18334b12}
h2{font-size:20px;margin:0 0 8px}.value{font-size:24px;font-weight:700;min-height:36px}
canvas{display:block;width:100%;height:280px}#connection{min-height:24px}
@media(max-width:720px){.charts{grid-template-columns:1fr}main{padding:16px}}
</style>
</head>
<body><main>
<a href="/">&larr; Dashboard</a><h1>Live sensor charts</h1>
<p>Temperature and humidity sampled about every 3 seconds. Last 100 samples shown; history resets when this page reloads.</p>
<p id="connection" role="status">Waiting for sensor data...</p>
<div class="charts">
<section><h2>Temperature (&deg;C)</h2><div class="value" id="temperature">Waiting...</div>
<canvas id="tempChart" role="img" aria-label="Temperature over time"></canvas></section>
<section><h2>Humidity (%)</h2><div class="value" id="humidity">Waiting...</div>
<canvas id="humChart" role="img" aria-label="Humidity over time"></canvas></section>
</div>
</main>
<script>
const samples=[];
function draw(id,key,color){
  const canvas=document.getElementById(id), ctx=canvas.getContext('2d');
  const w=canvas.clientWidth,h=canvas.clientHeight,ratio=window.devicePixelRatio||1;
  canvas.width=Math.round(w*ratio);canvas.height=Math.round(h*ratio);ctx.scale(ratio,ratio);
  const left=52,right=w-16,top=20,bottom=h-44;
  const values=samples.map(s=>s[key]).filter(Number.isFinite);
  let low=key==='hum'?0:Math.floor(Math.min(18,...values)-2);
  let high=key==='hum'?100:Math.ceil(Math.max(30,...values)+2);
  const y=v=>bottom-(v-low)/(high-low)*(bottom-top);
  const first=samples.length?samples[0].time:0,last=samples.length?samples[samples.length-1].time:0;
  const x=t=>last===first?(left+right)/2:left+(t-first)/(last-first)*(right-left);
  ctx.font='12px system-ui';ctx.lineWidth=1;
  for(let i=0;i<=4;i++){
    const v=low+(high-low)*i/4,py=y(v);
    ctx.strokeStyle='#e2e8ef';ctx.beginPath();ctx.moveTo(left,py);ctx.lineTo(right,py);ctx.stroke();
    ctx.fillStyle='#536779';ctx.textAlign='right';ctx.fillText(v.toFixed(0),left-8,py+4);
  }
  if(!values.length){ctx.textAlign='center';ctx.fillText('Waiting for valid readings', (left+right)/2, h/2);return;}
  ctx.strokeStyle=color;ctx.lineWidth=2;ctx.beginPath();let connected=false;
  for(const s of samples){
    if(!Number.isFinite(s[key])){connected=false;continue;}
    if(connected)ctx.lineTo(x(s.time),y(s[key]));else ctx.moveTo(x(s.time),y(s[key]));
    connected=true;
  }
  ctx.stroke();ctx.fillStyle=color;
  for(const s of samples){if(Number.isFinite(s[key])){ctx.beginPath();ctx.arc(x(s.time),y(s[key]),3,0,Math.PI*2);ctx.fill();}}
  ctx.fillStyle='#536779';ctx.textAlign='left';ctx.fillText(new Date(first).toLocaleTimeString(),left,h-20);
  if(last!==first){ctx.textAlign='right';ctx.fillText(new Date(last).toLocaleTimeString(),right,h-20);}
}
function redraw(){draw('tempChart','temp','#d65c28');draw('humChart','hum','#007ca8');}
async function poll(){
  try{
    const response=await fetch('/sensors',{cache:'no-store',signal:AbortSignal.timeout(8000)});
    if(!response.ok)throw new Error('Sensor unavailable');
    const data=await response.json();
    if(!Number.isFinite(data.temp)||!Number.isFinite(data.hum))throw new Error('Invalid reading');
    samples.push({time:Date.now(),temp:data.temp,hum:data.hum});
    document.getElementById('temperature').textContent=data.temp.toFixed(2)+' °C · '+data.status;
    document.getElementById('humidity').textContent=data.hum.toFixed(2)+' %';
    document.getElementById('connection').textContent='Live · Updated '+new Date().toLocaleTimeString();
  }catch(error){
    samples.push({time:Date.now(),temp:null,hum:null});
    document.getElementById('temperature').textContent='Unavailable';
    document.getElementById('humidity').textContent='Unavailable';
    document.getElementById('connection').textContent='Sensor or connection unavailable. Retrying...';
  }finally{
    if(samples.length>100)samples.shift();
    redraw();setTimeout(poll,3000);
  }
}
window.addEventListener('resize',redraw);redraw();poll();
</script></body></html>
)html";
