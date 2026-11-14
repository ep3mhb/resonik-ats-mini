#ifndef PAGE_CONTROL_H
#define PAGE_CONTROL_H

// RESONIK R4 (test): the Web Control page (/control), see WebControl.cpp

static const char webControlPage[] = R"HTML(<!DOCTYPE html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>RESONIK Web Control</title>
<style>
:root{--bg:#09202a;--card:#0f2f3c;--line:#1d4a5c;--text:#eef5f7;--muted:#8fb0bd;--accent:#06D7A0;--blue:#108AB1}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--text);font:15px/1.4 system-ui,-apple-system,Segoe UI,Roboto,sans-serif}
main{max-width:720px;margin:0 auto;padding:14px 16px 28px}
h1{font-size:18px;margin:4px 0 10px;display:flex;justify-content:space-between;align-items:center}
h1 small{font-size:12px;font-weight:600;color:var(--muted)}
#st{font-size:12px;padding:2px 8px;border-radius:9px;background:#5b2a2a}
#st.on{background:#0c5a45}
.screen{background:#000;border-radius:10px;padding:6px;border:2px solid var(--line)}
canvas{width:100%;aspect-ratio:320/170;display:block;image-rendering:pixelated;border-radius:4px}
.row{display:flex;gap:8px;margin-top:10px;flex-wrap:wrap}
button{flex:1;min-width:64px;padding:12px 6px;border:0;border-radius:10px;background:var(--card);color:var(--text);
 font:600 15px system-ui,sans-serif;cursor:pointer;touch-action:manipulation;user-select:none;-webkit-user-select:none}
button:active{background:var(--blue)}
button.big{font-size:22px;padding:14px 6px}
button.acc{background:#0c5a45}
.freq{display:flex;gap:8px;margin-top:10px}
.freq input{flex:2;min-width:0;padding:11px;border-radius:10px;border:1px solid var(--line);background:#06171e;color:var(--text);font:600 16px system-ui}
.freq button{flex:1}
.audio{margin-top:12px;padding:10px 12px;border-radius:10px;background:var(--card);font-size:13px;color:var(--muted)}
.audio b{color:var(--text)}
.hint{font-size:12px;color:var(--muted);margin-top:12px}
.panel{background:var(--card);border-radius:10px;padding:10px 14px;margin-bottom:10px}
.pf{display:flex;align-items:baseline;justify-content:space-between;gap:10px;flex-wrap:wrap}
#pfq{font:700 40px/1.1 ui-monospace,SFMono-Regular,Menlo,Consolas,monospace;letter-spacing:1px;color:#fff}
#pfq small{font-size:18px;color:var(--muted);margin-left:6px}
.tags span{display:inline-block;font-size:12px;font-weight:600;padding:2px 8px;margin:2px 0 2px 4px;border-radius:8px;background:#06171e;color:var(--muted)}
.tags span.m{background:var(--blue);color:#fff}
.sm{display:flex;align-items:center;gap:10px;margin-top:6px;font-size:13px;color:var(--muted)}
.sm .bar{flex:1;height:10px;border-radius:5px;background:#06171e;overflow:hidden}
.sm .bar i{display:block;height:100%;width:0;background:linear-gradient(90deg,var(--accent),#f5c542 70%,#ff6b6b);transition:width .2s}
#ps,#rt{white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
#ps{font-size:22px;line-height:30px;height:30px;font-weight:700;margin-top:6px;color:var(--accent)}
#rt{font-size:14px;line-height:20px;height:20px;color:var(--text);margin-top:2px}
.scope{margin-top:12px;background:var(--card);border-radius:10px;padding:10px 12px}
.scope h2{font-size:14px;margin:0 0 8px;display:flex;justify-content:space-between;align-items:center;gap:8px}
.scope h2 button{flex:0 0 auto;min-width:0;padding:6px 14px;font-size:13px}
#sc{width:100%;height:140px;aspect-ratio:auto;image-rendering:auto;background:#06171e;cursor:crosshair}
#scl{font-size:12px;color:var(--muted);margin-top:6px}
a{color:var(--accent)}
</style></head><body><main>
<h1><span><svg style="vertical-align:-6px;margin-right:8px" viewbox="0 0 53 48" width="26" height="24" aria-hidden="true"><rect x="0" y="40" width="5" height="8" rx="2" fill="#dce6ea"/><rect x="8" y="36" width="5" height="12" rx="2" fill="#dce6ea"/><rect x="16" y="28" width="5" height="20" rx="2" fill="#dce6ea"/><rect x="24" y="0" width="5" height="48" rx="2" fill="#4dabf7"/><rect x="32" y="28" width="5" height="20" rx="2" fill="#dce6ea"/><rect x="40" y="36" width="5" height="12" rx="2" fill="#dce6ea"/><rect x="48" y="40" width="5" height="8" rx="2" fill="#dce6ea"/></svg>RESONIK Web Control</span> <span id="st">offline</span></h1>
<div class="panel">
 <div class="pf"><div id="pfq">&mdash;</div><div class="tags" id="tags"></div></div>
 <div class="sm"><span id="sv" style="min-width:64px">S&ndash;</span><div class="bar"><i id="sb"></i></div><span id="sn"></span></div>
 <div id="ps"></div><div id="rt"></div>
</div>
<div class="screen"><canvas id="c" width="320" height="170"></canvas></div>
<div class="row">
 <button class="big" data-hold="r" title="Left arrow">&#9664;</button>
 <button class="big acc" data-cmd="e" title="Enter">Click</button>
 <button class="big" data-hold="R" title="Right arrow">&#9654;</button>
</div>
<div class="row">
 <button data-seek="-" title="Seek down">&#9664;&#9664; Seek</button>
 <button data-seek="+" title="Seek up">Seek &#9654;&#9654;</button>
</div>
<div class="row">
 <button data-cmd="E" title="Space">Press</button>
 <button data-hold="v" title="-">Vol &minus;</button>
 <button data-hold="V" title="+">Vol +</button>
</div>
<div class="row">
 <button data-cmd="b">Band &minus;</button><button data-cmd="B">Band +</button>
 <button data-cmd="M">Mode</button><button data-cmd="S">Step</button><button data-cmd="W">Filter</button>
</div>
<div class="freq"><input id="f" inputmode="decimal" placeholder="Frequency in kHz, e.g. 7074"><button id="go">Tune</button></div>
<div class="scope"><h2><span>Band scope</span><button id="scb" class="acc">Sweep</button></h2>
 <canvas id="sc" width="640" height="140"></canvas>
 <div id="scl">Sweep measures the signal at 41 points around the frequency (the sound is muted for 2&ndash;3 seconds).
 Click a peak to tune to it.</div></div>
<div class="audio"><button id="au" style="width:100%;margin-bottom:8px">&#128266; Listen (experimental)</button>
 <label style="display:block;margin-bottom:6px;color:var(--text)"><input type="checkbox" id="sil" checked> Silence the radio's speaker while listening</label>
 <span id="ai">Audio needs a V4B board (speaker line on GPIO11). Wi-Fi and the receiver share the processor's
 ADC, so the sound can drop out; the counter below shows how often.</span></div>
<p class="hint">Keyboard: &larr; &rarr; tune, Enter click, Space press, + &minus; volume. While this page is open, the FT8 and CW pages keep Wi-Fi on, so you can use them from
here (decoding can be a little weaker with Wi-Fi on). <a href="/">Status</a></p>
</main>
<script>
const cv=document.getElementById('c'),cx=cv.getContext('2d'),img=cx.createImageData(320,170),px=img.data;
for(let i=3;i<px.length;i+=4)px[i]=255;
const st=document.getElementById('st'),ai=document.getElementById('ai');
let ws=null,dirty=false;
function connect(){
 ws=new WebSocket((location.protocol==='https:'?'wss://':'ws://')+location.host+'/ws');
 ws.binaryType='arraybuffer';
 ws.onopen=()=>{st.textContent='live';st.className='on';if(listening){ws.send('!audio1');if(document.getElementById('sil').checked)ws.send('!mute1');}};
 ws.onclose=()=>{st.textContent='offline';st.className='';setTimeout(connect,2000);};
 ws.onmessage=e=>{
  if(typeof e.data==='string'){const j=JSON.parse(e.data);if(j.p)panel(j);else if(j.scope)scope(j);else status(j);return;}
  const b=new Uint8Array(e.data);
  if(b[0]===1){const y=b[1],n=b[2];let o=3,p=y*320*4;
   for(let i=0;i<n*320;i++,o+=2,p+=4){const v=(b[o]<<8)|b[o+1];
    px[p]=((v>>11)&31)*255/31;px[p+1]=((v>>5)&63)*255/63;px[p+2]=(v&31)*255/31;}
   dirty=true;}
  else if(b[0]===3){const y=b[1];let o=3,p=y*320*4;
   while(o+2<b.length){const c=b[o],v=(b[o+1]<<8)|b[o+2];o+=3;
    const r=((v>>11)&31)*255/31,g=((v>>5)&63)*255/63,bl=(v&31)*255/31;
    for(let i=0;i<c;i++,p+=4){px[p]=r;px[p+1]=g;px[p+2]=bl;}}
   dirty=true;}
  else if(b[0]===2)play(new Int16Array(e.data,2,(e.data.byteLength-2)>>1));
  else if(b[0]===4){const m=new Int16Array(b.length-2);for(let i=2;i<b.length;i++)m[i-2]=ulaw[b[i]];play(m);}
 };
}
function draw(){if(dirty){cx.putImageData(img,0,0);dirty=false;}requestAnimationFrame(draw);}
requestAnimationFrame(draw);
function send(c){if(ws&&ws.readyState===1)ws.send(c);}
// Buttons: one command per tap; hold buttons repeat
document.querySelectorAll('[data-cmd]').forEach(b=>b.addEventListener('click',()=>send(b.dataset.cmd)));
document.querySelectorAll('[data-hold]').forEach(b=>{let t=null,d=null;const c=b.dataset.hold;
 const stop=()=>{clearTimeout(d);clearInterval(t);t=d=null;};
 b.addEventListener('pointerdown',e=>{e.preventDefault();send(c);d=setTimeout(()=>{t=setInterval(()=>send(c),90);},400);});
 ['pointerup','pointerleave','pointercancel'].forEach(x=>b.addEventListener(x,stop));});
document.addEventListener('keydown',e=>{if(e.target.tagName==='INPUT')return;
 const k={ArrowLeft:'r',ArrowRight:'R',Enter:'e',' ':'E','+':'V','=':'V','-':'v'}[e.key];
 if(k){e.preventDefault();send(k);}});
document.querySelectorAll('[data-seek]').forEach(b=>b.addEventListener('click',()=>send('!seek'+b.dataset.seek)));
// Panel: drawn here from the radio's numbers
const $=id=>document.getElementById(id);let P=null;
// Modes as in Common.h: 0 FM, 1 LSB, 2 USB, 3 AM, 4 CW
function fmtF(f,m){if(m===0)return (f/1e6).toFixed(2)+'<small>MHz</small>';
 if(m===3)return Math.round(f/1000)+'<small>kHz</small>';return (f/1000).toFixed(2)+'<small>kHz</small>';}
function esc(t){return t.replace(/[&<>]/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;'}[c]));}
function panel(j){P=j;$('pfq').innerHTML=fmtF(j.f,j.m);
 $('tags').innerHTML=(j.band!==j.mode?'<span>'+esc(j.band)+'</span>':'')+'<span class="m">'+esc(j.mode)+'</span><span>Step '+esc(j.step)+
  '</span><span>BW '+esc(j.bw)+'</span><span>Vol '+j.vol+'</span><span>'+j.bat.toFixed(2)+' V</span>';
 // S-meter: S9 = 34 dBuV (50 ohm), 6 dB per S unit; FM in dBuV
 let lab;if(j.m===0)lab=j.rssi+' dB&micro;V';else{const s=(j.rssi-34)/6+9;
  lab=s<=9?'S'+Math.max(0,Math.round(s)):'S9+'+Math.round(j.rssi-34);}
 $('sv').innerHTML=lab;$('sb').style.width=Math.min(100,j.rssi/80*100)+'%';$('sn').textContent='SNR '+j.snr+' dB';
 $('ps').textContent=j.ps.trim();$('rt').textContent=j.rt.trim();drawScope();}
// Band scope
let S=null;const sc=$('sc'),sx=sc.getContext('2d');
$('scb').onclick=()=>{$('scb').textContent='Sweeping…';send('!scope');};
function scope(j){S=j;$('scb').textContent='Sweep';drawScope();
 const u=j.fm?1e6:1e3,d=j.fm?2:(j.st%1000?1:0);
 $('scl').textContent=(j.f0/u).toFixed(d)+' – '+((j.f0+j.st*(j.r.length-1))/u).toFixed(d)+(j.fm?' MHz':' kHz')+
  ', step '+(j.st/1000)+' kHz. Click a peak to tune to it.';}
function drawScope(){if(!S||!S.r.length)return;const W=sc.width,H=sc.height,n=S.r.length,mx=Math.max(30,...S.r);
 sx.clearRect(0,0,W,H);sx.strokeStyle='#1d4a5c';sx.lineWidth=1;
 for(let i=1;i<4;i++){sx.beginPath();sx.moveTo(0,H*i/4);sx.lineTo(W,H*i/4);sx.stroke();}
 const bw=W/n;
 for(let i=0;i<n;i++){const h=S.r[i]/mx*(H-14),q=Math.min(1,S.s[i]/20);
  sx.fillStyle='rgb('+Math.round(16+q*(6-16))+','+Math.round(138+q*(215-138))+','+Math.round(177+q*(160-177))+')';
  sx.fillRect(i*bw+1,H-h,bw-2,h);}
 if(hov>=0&&hov<n){sx.fillStyle='rgba(255,255,255,.15)';sx.fillRect(hov*bw,0,bw,H);
  const u=S.fm?1e6:1e3,d=S.fm?2:(S.st%1000?1:0),t=((S.f0+hov*S.st)/u).toFixed(d)+(S.fm?' MHz':' kHz');
  sx.font='bold 22px system-ui,sans-serif';const tw=sx.measureText(t).width+16;let x=hov*bw+bw/2-tw/2;
  x=Math.max(2,Math.min(W-tw-2,x));sx.fillStyle='rgba(6,23,30,.9)';sx.fillRect(x,4,tw,30);
  sx.fillStyle='#fff';sx.fillText(t,x+8,27);}
 if(P){const k=(P.f-S.f0)/S.st;if(k>=-0.5&&k<=n-0.5){const x=(k+0.5)*bw;sx.strokeStyle='#ff6b6b';sx.lineWidth=2;
  sx.beginPath();sx.moveTo(x,0);sx.lineTo(x,H);sx.stroke();}}}
sc.addEventListener('click',e=>{if(!S)return;const r=sc.getBoundingClientRect(),n=S.r.length;
 // Exactly the bar under the pointer (test 10 snapped to a stronger
 // neighbour, which picked the wrong bar)
 const b=Math.floor((e.clientX-r.left)/r.width*n);
 if(b>=0&&b<n)send('F'+(S.f0+b*S.st)+'\n');});
// Hover: highlight the bar and show its frequency
let hov=-1;
sc.addEventListener('mousemove',e=>{if(!S)return;const r=sc.getBoundingClientRect(),n=S.r.length;
 const b=Math.floor((e.clientX-r.left)/r.width*n);if(b!==hov){hov=b;drawScope();}});
sc.addEventListener('mouseleave',()=>{hov=-1;drawScope();});
function tune(){const v=parseFloat(document.getElementById('f').value.replace(',','.'));
 if(v>0)send('F'+Math.round(v*1000)+'\n');}
document.getElementById('go').onclick=tune;
document.getElementById('f').addEventListener('keydown',e=>{if(e.key==='Enter')tune();});
// Audio: 8 kHz samples scheduled with a small buffer
let ac=null,next=0,listening=false,gain=null,gaps=0,gapMs=0,reserve=0.35,lastS=0;
// mu-law to 16-bit
const ulaw=new Int16Array(256);for(let i=0;i<256;i++){const u=~i&255,s=u&128,e=(u>>4)&7,m=u&15;
 let v=((m<<3)+0x84)<<e;v-=0x84;ulaw[i]=s?-v:v;}
document.getElementById('au').onclick=function(){
 listening=!listening;this.textContent=listening?'\u{1F507} Stop listening':'\u{1F50A} Listen (experimental)';
 if(listening&&!ac){ac=new(window.AudioContext||window.webkitAudioContext)();gain=ac.createGain();gain.gain.value=1;gain.connect(ac.destination);}
 if(ac)ac.resume();send(listening?'!audio1':'!audio0');send(listening&&document.getElementById('sil').checked?'!mute1':'!mute0');next=0;};
document.getElementById('sil').onchange=function(){if(listening)send(this.checked?'!mute1':'!mute0');};
function play(s){if(!listening||!ac)return;
 // Safari does not accept 8 kHz buffers: resample to the device rate
 // (linear, carrying the last sample over so packets join smoothly)
 const r=ac.sampleRate/8000,len=Math.floor(s.length*r),buf=ac.createBuffer(1,len,ac.sampleRate),ch=buf.getChannelData(0);
 let peak=0;
 for(let i=0;i<len;i++){const x=i/r,k=Math.floor(x),f=x-k,a=k?s[k-1]:lastS,b=s[Math.min(k,s.length-1)];
  const v=(a+(b-a)*f)/32768;ch[i]=v;const m=Math.abs(v);if(m>peak)peak=m;}
 lastS=s[s.length-1];
 if(quiet)gain.gain.value=0;
 else if(gain.gain.value===0)gain.gain.value=1;
 if(!quiet&&peak>0.003)gain.gain.value=Math.min(40,Math.max(0.5,gain.gain.value*0.9+0.1*(0.5/peak)));
 const src=ac.createBufferSource();src.buffer=buf;src.connect(gain);
 const t=ac.currentTime;
 // Ran dry: the sound stopped until this packet (a gap you hear)
 // After a gap keep a bigger reserve (up to 1 s)
 if(next<t+0.02){if(next>0){gaps++;gapMs+=Math.round((t+reserve-next)*1000);reserve=Math.min(1,reserve+0.15);}next=t+reserve;}
 else if(next>t+2)next=t+reserve;
 src.start(next);next+=buf.duration;}
let quiet=false;
function status(j){
 // No input on GPIO11 (V4B with headphones plugged in: the speaker line is
 // cut): play silence instead of the ADC noise turned up 40 times
 quiet=!!(j.audio&&j.level<3);
 if(quiet&&gain)gain.gain.value=0;
 if(quiet){ai.innerHTML='<b>No audio on GPIO11</b> &middot; headphones plugged into the radio? On the V4B they cut the line the web audio comes from. '+
  'Unplug them and use &ldquo;Silence the radio&rsquo;s speaker&rdquo; instead.';return;}
 if(!j.audio){ai.innerHTML='Audio off'+(j.level<2?' &middot; <b>no audio on GPIO11 (V4B board needed)</b>':'');return;}
 ai.innerHTML='<b>Streaming</b> 8 kHz &middot; level '+j.level+' &middot; '+j.cpu+' MHz &middot; speaker '+
  ({on:'on',testing:'testing\u2026',off:'off',quiet:'quieter'}[j.spk]||j.spk)+'<br>'+
  'Radio: ADC failed '+j.adcErrors+', dropped '+j.late+', filled in '+j.caughtUp+', ring overrun '+j.overrun+
  ', read '+j.readUs+'/'+j.readMax+' &micro;s<br>'+
  'Network: busy '+j.waits+'<br>Browser: gaps '+gaps+' ('+gapMs+' ms)';}
connect();
</script></body></html>)HTML";


#endif // PAGE_CONTROL_H
