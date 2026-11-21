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
.panel{position:relative;background:var(--card);border-radius:10px;padding:10px 14px;margin-bottom:10px}
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
.pnav{display:flex;flex-wrap:wrap;gap:6px;margin:0 0 10px}
.pnav a,.pnav span{padding:5px 11px;border-radius:9px;background:var(--card);color:var(--muted);text-decoration:none;font-size:13px;font-weight:600}
.pnav span{background:var(--blue);color:#fff}
.card{margin-top:12px;background:var(--card);border-radius:10px;padding:10px 12px}
.card h2{font-size:14px;margin:0 0 8px;color:var(--text)}
.grid{display:grid;grid-template-columns:1fr 1fr;gap:8px}
.grid label{display:flex;flex-direction:column;gap:4px;font-size:12px;color:var(--muted)}
select,.num input{width:100%;padding:9px 8px;border-radius:8px;border:1px solid var(--line);background:#06171e;color:var(--text);font:600 14px system-ui,sans-serif}
.seg{display:flex;gap:6px;flex-wrap:wrap}
.seg button{padding:9px 6px;font-size:14px;min-width:54px}
.seg button.on{background:var(--blue);color:#fff}
.seg .st{display:inline-block;margin-left:10px;padding:1px 9px;border-radius:8px;font-size:12px;font-weight:700;letter-spacing:.04em;background:rgba(0,0,0,.25);color:#cfe3ea;vertical-align:1px}
.seg .st.on{background:var(--accent);color:#06171e}
.chips{display:flex;gap:6px;flex-wrap:wrap}
.chips button{flex:0 0 auto;min-width:0;padding:7px 10px;font-size:13px;background:#06171e}
.chips button.on{background:var(--blue)}
.vol{display:flex;align-items:center;gap:10px;margin-top:10px}
.vol span{min-width:36px;text-align:right;font-weight:700}
input[type=range]{flex:1;width:100%;accent-color:var(--accent)}
details.card>summary{cursor:pointer;font-weight:700;font-size:14px;list-style:none;display:flex;justify-content:space-between}
details.card>summary::after{content:'+';color:var(--muted)}
details.card[open]>summary::after{content:'\2212'}
details.card[open]>summary{margin-bottom:10px}
.set{display:grid;grid-template-columns:1fr 1fr;gap:10px 12px}
.set .it{display:flex;flex-direction:column;gap:4px;font-size:12px;color:var(--muted)}
.set .it.off{opacity:.4}
.set .it .rv{display:flex;align-items:center;gap:8px}
.set .it .ih{font-size:11px;line-height:1.35;opacity:.85}
.set .it .rv b{min-width:64px;text-align:right;color:var(--text);font-size:13px}
.tg{display:flex;align-items:center;gap:8px;color:var(--text);font-size:14px;padding:8px 0}
.tg input{width:20px;height:20px;accent-color:var(--accent)}
.mem{display:flex;gap:6px;align-items:center;margin:4px 0}
.mem button.t{flex:1;text-align:left;padding:8px 10px;background:#06171e;font-size:13px}
.mem button.x{flex:0 0 36px;min-width:0;padding:8px 0;background:#3a2020}
@media (max-width:520px){.set,.grid{grid-template-columns:1fr}}
.tabs{display:flex;gap:4px;margin:0 0 2px;overflow-x:auto;scrollbar-width:none}
.tabs button{flex:1;min-width:0;padding:10px 4px;font-size:13px;border-radius:9px 9px 0 0;background:#0b2631;color:var(--muted)}
.tabs button.on{background:var(--card);color:var(--text)}
section[data-tab]{display:none}section[data-tab].on{display:block}
section[data-tab]>:first-child{margin-top:0}
#map{width:100%;display:block;background:#06171e;border-radius:8px}
#map .land{fill:#1d4a5c}
#map .grid{stroke:#123744;stroke-width:.6;fill:none}
#map .stn{cursor:pointer}
#map .dot{stroke:#06171e;stroke-width:1.5}
#map .lbl{font:600 10px system-ui,sans-serif;fill:#8fb0bd}
#map .pulse{fill:none;stroke-width:2.5;transform-box:fill-box;transform-origin:center;animation:hb 1s ease-out infinite}
@keyframes hb{0%{transform:scale(.6);opacity:1}100%{transform:scale(2.6);opacity:0}}
.bfw{margin:0 0 10px;padding:8px 10px;border-radius:8px;background:#0e3a48}
.bfh{display:flex;justify-content:space-between;align-items:baseline;gap:8px;flex-wrap:wrap;font-size:13px;margin-bottom:6px}
.bfh span{color:var(--muted)}
.bfr{display:grid;grid-template-columns:minmax(0,1fr) 110px 110px;gap:8px}
.dpr{display:grid;grid-template-columns:minmax(0,1.3fr) minmax(0,1fr) 110px 110px;gap:8px}
.bfr select,.dpr select{width:100%;min-width:0}
.bfr button,.dpr button{padding:9px 6px;font-size:14px;background:#06171e}
@media(max-width:520px){.bfr{grid-template-columns:minmax(0,1fr) 68px 68px}.dpr{grid-template-columns:minmax(0,1fr) minmax(0,1fr)}}
.bfr button:disabled,.dpr button:disabled,.dpr select:disabled{opacity:.45}
.bfw .hint{margin:8px 0 0}
.dctl{display:grid;grid-template-columns:1fr 2fr 1.4fr auto;gap:8px;align-items:end;margin:10px 0 0}
.dctl label{display:flex;flex-direction:column;gap:4px;font-size:12px;color:var(--muted);min-width:0}
.dctl input,.dctl select{height:38px;box-sizing:border-box}
.dctl input{width:100%;min-width:0;padding:9px 8px;border-radius:8px;border:1px solid var(--line);background:#06171e;color:var(--text);font:600 14px system-ui,sans-serif}
.dbtn{display:flex;gap:8px}
.dbtn button{flex:0 0 auto;min-width:0;height:38px;padding:0 14px;font-size:14px;white-space:nowrap;background:#06171e}
#dtab{width:100%;font-size:13px;border-collapse:collapse;margin-top:10px}
#dtab th,#dtab td{text-align:left;padding:4px 8px 4px 0;white-space:nowrap}
#dtab th{color:var(--muted);font-size:12px;font-weight:600}
@media(max-width:560px){.dctl{grid-template-columns:1fr 1fr}.dbtn button{flex:1}#dtab th:nth-child(2),#dtab td:nth-child(2){display:none}#dtab{font-size:12px}#dtab th,#dtab td{padding-right:6px}}
.bnds{display:flex;gap:6px;flex-wrap:wrap;margin-bottom:8px}
.bnds button{flex:1;min-width:92px;padding:7px 6px;font-size:12px;background:#06171e;border:2px solid transparent;text-align:left;line-height:1.3}
.bnds button.on{border-color:currentColor}
.bnds b{display:block;font-size:13px;color:var(--text)}
.lg{display:flex;gap:14px;flex-wrap:wrap;font-size:12px;color:var(--muted);margin-top:8px}
.lg i{display:inline-block;width:10px;height:10px;border-radius:50%;margin-right:5px;vertical-align:-1px}
</style></head><body><main>
<h1><span><svg style="vertical-align:-6px;margin-right:8px" viewbox="0 0 53 48" width="26" height="24" aria-hidden="true"><rect x="0" y="40" width="5" height="8" rx="2" fill="#dce6ea"/><rect x="8" y="36" width="5" height="12" rx="2" fill="#dce6ea"/><rect x="16" y="28" width="5" height="20" rx="2" fill="#dce6ea"/><rect x="24" y="0" width="5" height="48" rx="2" fill="#4dabf7"/><rect x="32" y="28" width="5" height="20" rx="2" fill="#dce6ea"/><rect x="40" y="36" width="5" height="12" rx="2" fill="#dce6ea"/><rect x="48" y="40" width="5" height="8" rx="2" fill="#dce6ea"/></svg>RESONIK Web Control</span> <span id="st">offline</span></h1>
<nav class="pnav"><a href="/">Status</a><span>Control</span><a href="/memory">Memory</a><a href="/config">Config</a><a href="/wifi">Wi-Fi</a><a href="/update">Update</a></nav>
<div class="panel">
 <div class="pf"><div id="pfq">&mdash;</div><div class="tags" id="tags"></div></div>
 <div class="sm"><span id="sv" style="min-width:64px">S&ndash;</span><div class="bar"><i id="sb"></i></div><span id="sn"></span></div>
 <div id="ps"></div><div id="rt"></div>
 <div id="note" style="display:none;position:fixed;left:50%;bottom:14px;transform:translateX(-50%);z-index:30;width:max-content;max-width:92vw;box-sizing:border-box;padding:8px 14px;border-radius:10px;background:#06171e;color:var(--accent);font-size:13px;text-align:center;box-shadow:0 0 0 1px var(--line),0 6px 18px rgba(0,0,0,.5)"></div>
</div>
<nav class="tabs" id="tabs"><button data-t="rx" class="on">Receiver</button><button data-t="mem">Memory</button><button data-t="bcn">Beacons</button><button data-t="dec">Decodes</button><button data-t="set">Settings</button><button data-t="more">More</button></nav>
<section data-tab="rx" class="on">
<div class="row" style="margin-top:0">
 <button class="big" data-hold="r" title="Left arrow: one step down">&#9664;</button>
 <button data-seek="-" title="Seek down">&#9664;&#9664; Seek</button>
 <button data-seek="+" title="Seek up">Seek &#9654;&#9654;</button>
 <button class="big" data-hold="R" title="Right arrow: one step up">&#9654;</button>
</div>
<div class="freq"><input id="f" inputmode="decimal" placeholder="Frequency in kHz, e.g. 7074"><button id="go">Tune</button></div>
<div class="card">
 <div class="seg" id="modes"></div>
 <div class="grid" style="margin-top:10px">
  <label>Band<select id="q4096"></select></label>
  <label>Step<select id="q5120"></select></label>
  <label>Filter<select id="q4864"></select></label>
  <label>AGC / attenuator<select id="q4608"></select></label>
 </div>
 <div class="vol"><span style="min-width:0;font-weight:600;color:var(--muted)">Volume</span><input type="range" id="q4352" min="0" max="63"><span id="q4352v"></span></div>
</div>
<div class="card"><h2>Bands</h2><div class="chips" id="bands"></div></div>
<div class="scope"><h2><span>Band scope</span><button id="scb" class="acc">Sweep</button></h2>
 <canvas id="sc" width="640" height="140"></canvas>
 <div id="scl">Sweep measures the signal at 41 points around the frequency (the sound is muted for 2&ndash;3 seconds).
 Click a peak to tune to it.</div></div>
</section>
<section data-tab="mem"><div class="card"><h2>Memories</h2>
 <div id="mems"></div>
 <div class="row"><button id="msave">Save this station</button><a href="/memory" style="flex:1;text-align:center;padding:12px 0">Edit names</a></div></div></section>
<section data-tab="bcn"><div class="card"><h2>NCDXF/IARU beacons <span id="bclk" style="float:right;color:var(--muted);font-weight:600"></span></h2>
 <div class="bfw"><div class="bfh"><b>Follow a beacon</b><span id="bfolt"></span></div>
  <div class="bfr"><select id="bsel"></select><button id="bgo">Start</button><button id="bstop">Stop</button></div></div>
 <div class="bnds" id="bnds"></div>
 <svg id="map" viewBox="0 0 720 300" role="img" aria-label="World map of the 18 beacons">
  <g class="grid" id="mgrid"></g><path class="land" d="M224 288l3 1l3 0l-3 1l-2 1l-3 0l-2-1l-2 0l-3-1l-2-1l-2-2l4 1l3 1l1-2l2-1l2 0l1 3zM243 282l2 1l-4 1l-2 1l1-2l2 0zM501 279l-4 1l0-2l2 0l2 0zM651 262l2 0l2 0l2 0l0 2l-1 2l-2 1l-2 0l-1-2l-2-3l2 0zM706 262l2 0l0 2l-2 2l0 2l-3 0l-1 2l-1 2l-2 1l-2 0l-4-1l1-2l3-2l2-1l2-1l2-2l1-2l2-1zM709 252l2 2l3 2l2-1l1 2l-3 1l0 2l-2 3l-2 0l0-2l-2-2l1-3l0-3l-3-3l3 1zM694 224l-2 0l-2-2l-2-1l2 0l2 1l2 2zM717 215l-1 2l-1-2l2 0zM460 207l0 3l0 2l0 2l-1 2l-1 2l-1 3l-1 4l-1 3l-1 2l-3 1l-3-1l-1-3l0-3l1-2l1-2l-1-2l0-3l1-3l2 0l2 0l1-2l2-2l2-2l0-2l2 2zM647 208l2 0l2 2l0 3l1 3l1 2l2 1l3 2l1 2l0 2l2 0l1 2l2 2l2 2l0 2l0 2l1 3l-1 3l0 2l-1 2l-2 1l0 2l-2 2l-1 3l0 2l-3 1l-2 0l-2 2l-2-1l-2-1l-2 2l-2-1l-2 0l-2-1l-1-3l-2-2l-2 0l0-2l-2 2l1-2l1-2l-2 1l-2 2l-2-1l-1-3l-3-2l-3-1l-4 0l-3 1l-2 1l-2-1l-2 1l-2 1l-1 2l-3 0l-3 0l-2 1l-2 0l-2 1l-2 0l-2-1l-1-2l2-3l-2-3l0-2l-1-2l-1-2l-1-2l0-2l1 2l-1-3l0-2l1-2l0-2l3-1l2-2l2 0l3 0l2-1l2-1l1-2l2-1l1-3l1 2l0-2l2 0l0-2l1-2l2 0l2-1l2 1l2 1l2-3l1-2l2 0l2-1l0-2l2 2l2 0l2 0l2 0l0 2l-1 2l-1 2l2 1l2 1l2 2l2 1l3 0l1-2l0-3l0-3l1-2l0-2l1-2l1 3l1 2l0 2zM601 200l-3-1l2 0l2 1zM682 200l-2 0l-1-2l2 1zM609 200l-2 1l1-2l2-1l2-1l2 0l-2 1l-2 1zM596 196l2 1l-2 1l-3 0l2-1zM606 196l-3 2l-3 0l1-2l2 1l3-1zM680 197l-2-1l-2-1l2 0l2 2zM577 194l4 0l4 0l4 2l2 1l-2 1l-2-1l-2 0l-2 0l-2-1l-2-1l-2 0l-4 0l-2-1l1-2l3 0l2 1zM672 194l-2-1l-1-3l2 2l1 2zM664 191l-2 1l-2 1l-2-1l2-1l2 0l1-3l2 1l-1 2zM621 186l1 2l-2-1l-2 0l-2 0l3-1l2 0zM666 189l-2-2l-3-2l2 1l2 1l1 2zM628 182l1 4l2 1l2-2l2-2l2 0l3 2l2 0l3 2l4 1l2 1l1 2l3 1l-1 3l2 1l1 2l2 0l0 2l3 1l-2 0l-2 0l-2-1l-2-1l-2-3l-3-1l-2 1l0 2l-2 1l-3-1l-2-1l-2-1l-3 1l1-2l1-3l-5-3l-2 0l-3-2l-3-1l2-1l2 0l-4-1l-2-1l2-2l4 1zM610 177l-1 2l-2 1l-2-1l-3 0l-2 1l2 3l5-2l-2 2l-2 1l2 2l1 3l0 2l-2 0l1-2l-2 1l0-2l-1-3l-1 3l0 3l-2 0l0-2l0-2l-1-3l2-4l2-3l4 1l2 0l2-1zM617 178l-1 3l-1-3l0-2l2 1zM572 192l-3 0l-1-2l-3-2l-2-2l-1-2l-2-3l-2-3l-1-2l-2-1l-1-2l-3-3l4 0l2 1l1 2l3 3l2 0l2 1l1 2l2 1l0 2l1 2l2 1l1 4l0 3zM596 176l2 2l-2 0l-1 2l0 2l-2 1l0 2l-1 3l-2 0l-2-1l-4 0l-2-1l-2 0l0-3l-2-2l0-2l0-2l3 0l2-2l3-1l1-2l2-2l2-1l2-3l2 1l2 2l0 2l-2 3l1 2zM613 163l0 3l-2 0l0 2l-3 0l0-2l-1-2l-3 2l1-2l2-1l2 0l2-1l0-2l2 2zM522 168l-2-2l-1-2l1-4l2 1l1 2l1 2l-1 2zM238 160l-2 0l1-2l1 2zM608 159l-1 2l-2 0l1-2l2-1zM597 161l-3 2l3-2l1-2l1-2l0 2l-2 2zM604 156l2 1l-1 2l-1-2zM611 156l1 2l-2-1l1 2l-2-1l1-2zM603 154l0 2l-2-3l2 1zM603 143l2 1l0 2l-2 2l0 2l2 2l3 0l0 2l-2-1l-2-1l-2 1l-1-2l-1-2l0-2l1-2l0-2l2 0zM206 144l-2 0l2-1l2 1l-2 0zM215 140l2 1l3 0l2 0l1 2l-2 0l-2 1l-2 1l-2-1l-2 0l-2-1l2 0l2 0l-1-2zM581 143l-2 1l-2-1l0-2l3-1l2 0l-1 3zM49 142l-1-2l2 0l0 2zM201 134l2 1l3 2l2 1l2 1l2 0l-2 1l-3 0l-3 0l2-1l-2 0l-1-2l-2 0l-2-1l-3 0l-2-1l-2 1l-2 0l2-1l3-1l2 0l2 0l2 0zM602 134l-1 2l-1-2l1-3l2-2l1 2l-2 3zM629 112l-1 2l-2 1l-1-2l2-1l2 0zM429 109l-3 2l0-2l3 0zM407 109l3 0l2 0l-3 1l-2-1zM391 104l0 2l-2 0l-4-1l2-1l3 0zM378 98l2 1l-1 3l-2 0l0-3zM642 106l0 2l-1 2l-3 1l-4 0l-2 2l-2-1l-3-1l-3 1l-2 0l2 2l-1 3l-2 1l0-3l-2-2l2 0l3-3l5-1l2 0l2-4l2 1l3-2l2-3l0-2l3-2l1 3l0 2l-2 2l0 2zM379 96l-2-1l2-1l0 2zM648 92l3-1l0 2l-3 1l-2 2l-3-1l-1 2l-2 0l0-2l1-2l2 0l0-3l3 1l2 1zM233 87l3 0l-2 1l-2-1zM236 82l-3-1l-2-1l3 1l2 0zM113 83l-4-1l-3-1l-2-1l-1-2l2 1l2 0l2 1l3 2zM248 79l-2 1l2 0l2 1l3 1l1 2l0 3l-2-1l0-2l-3 2l-2-1l-2 0l-4 0l1-2l3-4l2-2l2 0l-1 2zM95 72l2 0l-1 2l2 2l-2-1l-2-2zM647 79l2 3l-3-1l-1 3l2 2l0 2l-2-1l-1-3l0-2l0-4l-1-2l0-3l2-1l2 3l0 4zM346 75l-3 2l-3-1l2-2l-1-2l2-1l2-1l2 0l2 1l-1 3l-2 1zM54 66l-2 1l-1-2l3-1l2 1l-2 1zM354 63l-2 2l2 0l2 0l-2 3l2 0l2 3l2 2l3 2l0 2l-2 1l-3 0l-3 1l-2 1l-2-1l-3 1l3-2l2-1l-3 0l2-2l-2-1l4-1l-1-2l-3-1l0-2l-2-2l0-2l2-1l2 0l2 0zM196 55l-2 1l-2 0l2-2l2 0zM17 52l2 1l2 0l2 0l-2 1l-2 0l-2-1zM190 49l2 1l2 0l3 1l3 2l-2 0l-3 0l-3 0l-3 1l-3-1l2-3l2-1zM331 47l2 3l-3 1l-6 2l-4 0l-6-1l2-1l-4-1l4 0l-5-1l2-2l3 0l3 2l3-2l2 1l4-1l3 0zM208 46l-2 0l0-2l2-1l2 1l-2 2zM0 42l5 2l5 2l6 0l4 2l-2 1l-3 0l0 2l-3 0l-4-1l-2-1l-3 0l-3-1l0 2zM720 50l-3 1l-2 0l2 1l2 2l-4 1l-6 1l-2 1l-3 1l-3 1l-3 0l-5 1l-3 1l-3-1l-1 2l-2 2l2 1l0 3l-2 0l0 2l-3 1l-1 3l-3 0l-1 2l-2 2l-2-4l-1-5l1-3l2-1l3-1l3-3l4-2l3-1l2-3l-2 0l-2 2l-5 2l-1-3l-6 1l-5 3l2 2l-4 0l-3 0l-3-1l-3 1l-6-1l-7 1l-6 4l-8 5l3 0l3 1l2-1l2 1l3 2l0 2l-2 2l0 2l-1 3l-3 3l-3 4l-3 2l-4 2l-2-1l-3 2l-2 1l-1 2l-2 2l-2 0l0 2l2 1l1 2l1 4l-3 1l-3 1l0-2l-1-2l2-1l-2-1l-2-1l0-2l-1-2l-3 1l-2 1l-2 0l1-2l0-2l-4 2l-3 2l0 2l2 0l1 2l3-2l3 1l-3 2l-3 2l1 2l1 2l1 2l2 2l-1 2l1 3l-1 2l-2 2l-2 3l-2 2l-2 2l-3 1l-2 1l-2 1l-2 0l-2 1l-2 0l-1 2l-2-1l-2-1l-4 2l-2 3l2 2l2 3l2 1l2 5l-1 4l-4 2l-1 2l-3 2l0-3l-3-1l-2-3l-2-1l-1-2l-2 0l0 2l-1 3l-1 2l0 2l2 0l1 3l2 2l2 1l2 2l0 2l0 2l1 3l-3-1l-2-2l-2-2l0-2l0-2l-2-2l-1-2l-2-1l1-3l1-3l-1-3l-1-4l-1-2l-1-2l-3 3l-3-1l1-3l-2-4l-2-2l-1-2l-1-3l-2 0l0 2l-2 0l-3 1l-2 0l0 2l-4 2l-2 2l-2 2l-2 1l-2 2l-2 2l0 2l1 2l-1 2l0 3l-2 2l-2 2l-3-1l-1-3l-1-2l-1-2l-1-3l-2-4l-1-4l0-2l0-3l-1-2l-3 1l-4-2l1-2l-3-1l-2-2l-1-2l-4 1l-3 0l-3 0l-4-1l-2 0l-2 0l-1-3l-3 0l-2 1l-2-1l-2-1l-2-1l-1-2l-2-2l-2-1l-2 1l1 3l1 2l2 2l0 2l2 1l0-2l1 2l0 2l2 0l2 0l2-2l2-1l2-2l0 2l1 3l2 1l2 1l2 1l-1 2l-2 2l-2 2l-2 2l-2 1l0 2l-3 0l-2 1l-2 1l-2 2l-3 1l-2 1l-2 1l-2 0l-2 0l-1 2l-2 0l-2-1l1-2l-1-2l0-2l-1-2l-1-2l-2-1l0-2l-2-1l-2-3l0-2l-1-2l-2-2l-1-2l-1-2l-2-2l-2-1l1-2l-2 2l-2-1l-1-3l0 3l2 2l1 3l2 2l1 2l0 2l2 2l1 2l0 2l1 3l2 1l1 2l1 2l3 3l3 2l1 2l-1 2l2 0l1 2l3 0l2-1l2 0l2-1l2 0l2 0l1 2l-1 3l-1 2l-1 2l-2 3l-2 3l-2 2l-2 2l-3 2l-2 1l-2 3l-2 2l-2 3l-1 2l-2 3l2 2l0 2l0 2l1 2l1 2l0 2l0 4l0 3l-2 2l-2 1l-2 1l-2 2l-3 2l-1 2l1 2l1 3l0 2l-3 2l-2 1l0 2l-1 2l0 2l-2 2l-2 2l-3 3l-2 2l-3 1l-3 1l-3 0l-2 0l-2 1l-2 0l-2 1l-2-2l-1-3l0-2l-1-2l-2-4l-2-1l-1-2l0-2l-1-4l0-3l-1-2l-2-4l-2-3l-1-3l1-3l1-3l2-2l0-2l0-2l-1-3l0-3l-2-2l0-3l-2-2l-2-2l-1-2l-1-2l1-3l0-4l0-2l-2-2l-2 0l-2 1l-2-2l-2-3l-2 0l-2 0l-3 1l-3 1l-3 2l-2-1l-2 0l-4 0l-3 1l-3-1l-2-1l-2-1l-1-2l-2-1l-1-3l-2-2l-2-2l-2-1l-1-2l0-2l-1-2l1-2l0-2l1-3l-1-2l0-2l-1-2l0-2l1-2l2-3l1-2l1-2l3-2l3-1l1-2l3-2l-1-2l1-2l2-2l2-1l3-3l0-2l2 0l3 1l2 1l3-1l2-1l3-1l3-1l4 0l3 0l2 0l2 0l2-1l1 2l2-1l-1 2l1 2l-2 1l2 1l1 2l2 0l3 1l2 0l1 2l2 1l3 0l2 1l2-1l0-2l2-1l4 0l2 1l2 0l3 1l2 0l2 1l2 0l3-1l2 1l2 0l2 0l2-2l0-2l1-2l1-3l0-2l-3-1l-1 2l-3 0l-2-1l-2 0l-2 1l-2-1l-2 0l-1-2l0-3l-2-1l3-2l3 0l4-1l3-1l2-1l3 0l4 1l3 1l2 0l2 0l2-1l0-2l-2-1l-4-3l-2 0l-2-1l2-1l0-2l3-1l-2 0l-2 1l-2 0l-2 0l0 2l3 0l-3 1l-2 1l-3-2l2-1l-4-1l-2 0l-2 2l-2 4l-2 2l1 2l2 0l-3 1l-2 2l-1-2l-2 0l-3 1l2 1l-2 0l-2-1l1 2l1 2l1 2l-2-1l0 3l-3-1l-1-3l-2-2l-1-2l0-2l-1-2l-3-1l-3-1l-2-1l0-2l-2 0l-2-1l-1 2l2 2l3 3l2 0l2 2l3 1l-2 0l-2 1l1 2l-2 2l0-2l-1-2l-2-1l-2-1l-3-1l-2-2l-2-3l-2-1l-2 1l-3 2l-4-1l-3 1l0 2l-2 2l-2 0l-2 2l0 3l-1 2l-2 0l-1 2l-3 0l-2 0l-2 1l-2-2l-2 0l-2 0l-1-3l-1-2l1-3l0-2l-1-2l3-1l2 0l3 0l2 0l2 0l3 0l2-5l-2-2l-2-1l-3-1l2-2l4 1l-1-3l2 1l5-1l0-2l2 0l2-1l2-3l3-1l2 0l2 0l2-1l-1-2l-1-3l3-1l2-1l0 2l-2 2l1 2l2 0l2 1l3 0l3 0l3-1l2-1l2 1l2 0l4-1l-1-2l0-2l3-2l2 2l2-2l-2-1l2-2l3 0l2 0l2 0l2-1l-2-1l-3 0l-4 1l-3 0l-3-1l0-2l-1-2l3-3l4-2l2 0l-3-2l-4 1l-2 1l-2 3l-4 2l-2 2l2 2l2 1l-2 2l-2 1l-1 3l-1 2l-3 0l-3 1l-1-2l-1-2l-2-3l-5 1l-3 1l-3-1l0-2l-1-5l2-1l5-2l4-2l4-3l5-4l3-1l5-3l5-1l3 1l3-2l4 0l3 0l7 1l-3 1l2 1l2-1l4 1l5 1l8 2l1 2l-2 1l-3 1l-9-2l-2 1l4 1l0 3l2 1l2 0l-1-2l6 1l2-1l3-3l2 0l2 1l1-2l-2-1l0-2l6 1l-2 1l2 2l3-1l4-2l7-2l2 0l-2 2l2 0l2-1l4 0l3-1l2 1l2-1l-2-1l7 0l3 1l7 2l-1-2l-2-1l-1-2l4-3l1-2l2 0l5 0l1 2l-2 1l2 2l-1 3l2 1l-1 2l-3 2l2 1l3-2l2-2l0-2l-2 0l1-3l-3-2l4-1l-1-2l2 1l0 2l2 1l-1-2l3-1l4 0l4 2l-2-2l0-2l4-1l4 0l5 0l-2-1l2-1l3 0l4-1l5-1l6 0l6-1l4 0l2-2l5 0l3 0l-3 1l5 0l2 1l6 0l5 1l-1 2l-6 1l4 1l2 0l2 1l3-1l7 1l8 1l5-1l3 0l3 1l0 2l2 2l4 0l2-2l3 1l3 0l4 0l5 0l-2-2l3-1l18 2l2 1l5 1l8 0l4 0l2 1l-1 2l3 0l3 0l3 0l4 0l4 0l3 2l3-1l-2-1l7-1l4 0l6 1zM458 97l1 2l2 0l-2 1l-1 2l0 3l2 0l2 1l3 1l3-1l-1-2l1-2l-2-1l0-3l2 1l0-2l-2-1l0 2l-1-2l0-2l-2 0l-1-2l1-2l3-1l0-3l-2 0l-2 0l-2 1l-2 0l-3 2l-2 2l2 2l2 3zM169 42l-2 0l-2 0l-2 0l-3-1l2 0l2-1l2 0l3 2zM0 37l2 0l3 0l-2 1l-3 0zM179 41l0 2l3-2l2 2l1 3l2-2l2-2l0-2l3 0l3 1l2 1l-1 2l1 2l-4 1l-2 0l-3 0l-2 3l-3 2l-3 0l-2 2l-2 0l-2 2l-2 2l-1 2l0 2l3 0l0 2l1 2l3-1l4 1l2 1l4 2l2 0l3 1l2 0l1 3l1 3l3 2l2-1l1-2l-1-3l-2-1l4-1l2-2l1-2l-2-2l-2-2l2-2l-1-2l0-3l5 0l2 1l3 0l3 1l4 1l0 2l0 2l2 0l2 2l3-2l2-2l2 1l3 3l2 2l2 2l2 2l3 0l2 2l2 1l1 3l-2 0l-5 2l-2 2l-3 0l-5-1l-3 0l-2 1l-4 2l-3 3l-2 1l5-3l4-1l3 0l2 1l-2 1l0 2l1 2l3 1l3-1l2-2l1 2l-2 1l-5 2l-4 2l-1-2l3-2l-3 0l-2 1l-2 1l-2 1l-2 1l-2 1l1 2l-3 1l-2 1l2 0l-3 1l-2 0l1 2l-2 1l-1 2l-1 2l1-2l-2-2l0 2l0 2l2 3l-2 1l-2 1l-2 1l-1 2l-3 1l-2 2l0 2l1 3l1 2l1 2l0 2l-1 2l-2-1l-1-2l-1-2l0-2l-2-3l-3 1l-2-1l-3-1l-2 0l-2 1l0 2l-3 0l-3-1l-3 0l-3 2l-2 0l-2 2l0 2l0 3l-1 4l1 2l1 3l1 2l3 2l3 0l3 0l2-2l1-2l2-2l2 0l2 0l0 2l0 2l-1 2l-1 2l0 2l0 2l2-1l2 0l2 0l2 0l2 1l0 2l0 2l0 2l0 2l0 2l2 2l2 1l2 0l2-1l2 0l2 1l3-1l1-2l2-1l2 0l2-1l1-2l2 1l-2 1l1 2l0 3l1-2l-1-2l3-1l2 0l2 1l4 1l2 1l4-1l2 0l2 2l1 2l3 1l1 2l2 1l3 1l2 0l2 0l2 1l2 2l2 2l1 3l1 2l-1 2l4 0l0 2l4 0l3 1l1 2l2 0l4 1l3 0l3 1l3 3l3 0l1 3l1 2l-1 3l-4 4l-1 2l-2 2l-1 2l0 3l0 3l-1 2l-1 3l-2 3l0 2l-2 1l-2 1l-3 1l-2 1l-2 0l-2 2l-2 2l0 2l0 2l-2 2l-2 4l-2 2l-2 0l0 2l-2 2l-3 2l-2 0l-2-1l-2 0l2 2l1 2l-2 3l-3 1l-4 1l-3 0l0 2l0 2l-3 0l-2 0l0 2l2 0l-1 2l-2 3l-2 1l-2 1l0 2l2 1l2 0l-1 2l-2 1l-2 3l-2 1l0 3l2 1l-3 0l-3 1l0 2l-3-1l-2-1l-3-1l-1-2l0-2l0-4l1-2l2-1l-3-1l2-1l0-4l3 1l1-4l-2-1l0 3l-2-1l1-2l1-4l0-3l0-3l2-3l1-3l1-3l0-3l0-2l0-2l1-3l1-4l0-4l1-4l0-3l-1-3l-2-1l-4-3l-3-2l-2-2l-1-2l-1-3l-2-3l-2-4l-2-3l-3-2l0-3l2-2l1-2l-2-1l0-2l2-3l2-2l1-2l2-2l0-2l0-2l0-2l-1-2l0-2l-2-1l-2 1l0 2l-2 1l0-2l-2 0l-2-1l-2-2l-2-1l-2 0l1-2l-2-2l-2-2l-2 0l-2-1l-2-1l-2 0l-4-3l-2-1l-2 0l-2 1l-2-1l-3-1l-4-1l-2-2l-3-1l-3-2l-1-2l0-2l0-2l-3-3l-2-1l-3-2l0-2l-2-1l0-2l-3-1l-1-2l-1-3l-2-1l-2-1l0 2l1 2l2 2l1 2l2 2l1 3l2 1l1 2l1 2l-2-1l-2-2l-1-2l-2-2l-2-1l-2-1l2-1l-2-3l-2-1l-1-2l-1-3l-2-2l-2-1l-3-1l-2-3l-2-3l-2-3l-1-2l0-2l0-2l-1-2l1-5l0-3l-1-2l3 0l1 2l0-2l-1-2l-4-2l-5-2l-1-3l-2-1l-3-3l-3-2l0-2l-3-1l-1-2l-2 0l-3 0l-3-1l-4-2l-2 0l-3-1l-3 0l-4-1l-2-1l-2 1l-3 2l-2 0l-2 1l0-3l2-1l-3 1l-4 2l0 3l-3 1l-2 0l-3 2l-3 2l-2 0l-3 1l-2 1l-4 0l2-1l2-1l2-1l3 0l4-2l2-1l0-2l-2-1l-2 0l-2 0l-2 1l-1-2l-4-1l-3-1l-1-2l2-2l2-1l2 0l3-1l0-2l-2 0l-2 0l-3 0l-3 0l-3-2l3-1l4-1l2 0l4 1l-2-1l-2-1l-2-1l-2-1l-3-1l2-1l3 0l3-1l2-2l2 0l4-1l2 0l3-1l3 1l2 0l4 0l3 1l2 0l4 1l4 0l4 0l3 0l2 1l4 0l3 1l2 0l2-1l2 0l3 0l3-1l3 0l2 0l2-1l4 2l3-1l3 1l3-1l3 1l5 1l3 0l2 0l2 1l-3 1l4 1l5-1l2 0l2 1l2-1l-2-1l2 0l2 0l2-1l3 2l3 0l3 1l3-1l3 0l2-1l3 1l3 0l1-2l-3-1l-2-1l0-2l3-2l2 0l2 1l3 3l-2 1l4 0zM132 34l3 0l3 1l2-1l2 1l2 2l-1-3l2 0l2 0l2 1l1 2l4 2l4 1l-3 1l-3 1l-4 0l-2 0l-4 0l-6 1l-3 0l-3-2l-2 1l-3-2l2 0l3 0l3 0l2-1l-4 0l-4 0l-3 0l4-2l-3 0l-4 0l2-2l7-2l2 1zM151 33l-2 1l-3-1l3 0l2 0zM207 34l-2 0l-2 0l-2 1l-3-2l6 0l3 1zM187 34l3-1l5-1l4 3l3 0l2-1l5 2l3 0l4 1l2 1l4 1l2 1l2 2l-4 1l5 1l3 0l3 2l3 0l0 2l-4 2l-2-1l-3-2l-3 0l0 2l2 1l3 1l2 2l-1 2l-3-1l-5-1l3 1l2 1l-5 0l-4-1l-2-1l-3-1l-3-1l-5 1l-2-1l1-2l4 0l4 0l0-2l3-2l-2-1l-3-1l-4-1l2 0l-3-2l-3 0l-5 1l-7-1l-4-1l-3 0l-2-1l2 0l-3 0l0-2l1-2l2-1l5-1l-1 2zM159 32l3 1l3-1l-1 2l3 1l0 2l-4 0l-2 0l-6-2l4 0l-2-2l2-1zM647 34l-3 0l-4-1l2-1l2 0l3 1zM174 34l-3 2l-2 0l-1-2l1-2l2 0l4 0l4 0l-3 2l-2 0zM119 37l-5 1l-6-2l2-2l2-1l-2-2l7 0l3 1l5 0l2 0l2 1l-3 1l-4 1l-3 1zM661 30l-2 1l-3-1l-4 0l4-1l5 1zM173 30l-4 1l-3-1l4-1l2 0zM650 29l-9 1l-3 1l-4-2l4-1l5 0l7 1zM163 27l2 0l0 2l-5 1l-2 0l-3-1l0-2l2 0l3 0l3 0zM144 28l2 0l2 0l-1 2l-6 0l-5 1l-3 0l3-1l-9 0l-2 0l2-2l2-1l6 1l3 1l4 0l-3-2l2-1l2 1zM475 39l-8-1l-4-1l2-1l0-2l4-1l-2-1l5-1l4-2l6-2l7 0l3-1l4 0l2 1l-2 1l-7 1l-6 0l-6 2l-3 2l-3 2l0 2l4 2zM171 26l2 0l4 0l2 1l3 2l2 0l3 0l3 0l4-1l4 1l2 0l-1 2l-3 0l-2 0l-6 0l-4 0l-4 0l-5-1l-1-2l-2-1l-4 0l-2-1l5 0zM128 25l-2 2l-2 0l-4 1l-3 0l-3 0l4-2l4-1l3 0l3 0zM172 25l-4 0l3-1l2 1zM140 25l-4 0l-3 0l2-1l2 0l3 0zM409 24l-4 1l-4 0l2-1l3-1l3 1zM141 23l-3 0l-3 0l2-1l4 1zM168 24l-3 0l-2-1l2-1l4 1zM160 23l-3 1l-3-1l-4 0l2 0l-3-1l4 0l5 0l2 1zM570 23l-11 1l4-2l3-1l5 2zM397 21l6 1l-5 1l-3 2l-3 1l-4-1l-2-1l-4-2l4-2l4 1l4-1l3 1zM411 19l4 1l-3 1l-6 0l-6 0l-3-1l-2-1l6 0l3 0l2 0l5 0zM462 19l-2 0l-2 0l-3 1l-2 0l-3-1l4-1l3 0l3 0l3 1zM560 22l-4 0l-6 0l-3-1l-2-1l-3-1l6-1l4-1l4 2l4 1l0 2zM186 21l2 0l-2 1l-4 1l-4 1l-4-1l-2-1l2-1l-4 0l-2 0l0-2l3-1l4-1l3 2l3 0l3 0l2 2zM223 14l5 0l5 0l3 1l-5 1l-5 1l3 0l-5 1l-3 1l-3 1l-4 1l-2 0l-6 0l3 1l-2 2l-3 0l-4 2l4 0l-5 2l-5-1l-6 0l-3 0l-4 0l3-1l-1-2l2 0l5 1l-3-1l-3-1l2-1l3 0l-2-1l-1-2l6 1l4-1l-4 0l-7 0l-4-1l-4-1l3-1l2 0l4-1l3 0l2 0l3 0l4-1l3 0l6 0l2 0l5 0l5 0l4 0zM306 13l12 2l-3 0l-8 0l-11 1l8 0l6 0l4 0l2 1l-2 1l5-1l9-1l6 1l2 0l-9 2l-7 1l5 0l-3 1l-1 4l2 1l-3 0l-3 1l3 1l1 2l-2 0l2 1l-4 1l2 0l-3 1l-3 0l2 2l-4 0l2 1l3 1l0 2l-3 0l-2-1l-2-1l-2 3l6 0l2 0l-5 1l-5 2l-6 1l-3 0l-2 1l-2 2l-5 1l-4 1l-3 0l-1 3l-5 3l0 3l-1 2l-3 0l-3-2l-4 0l-3-3l-3-2l-1-2l-3-3l0-2l1-3l3 0l1-3l-2 1l-3 0l-2 0l-1-2l3-1l4 1l-3-1l-2-1l-2 0l-2 0l3-2l-2-1l-4-3l-2-1l-6-2l-4 0l-5 0l-5 0l-2-1l-4-1l5-1l4 0l-8 0l-5-1l8-2l8-1l-5-1l2-1l7-1l3-1l3-1l7 0l6 0l2 0l5-1l5 1l3 0l4 1l-5-1l7-2l7 0l3-1l7 0l16 0z"/><g id="mstn"></g></svg>
 <div class="lg"><span><i style="background:#06D7A0"></i>Heard well</span><span><i style="background:#f5c542"></i>Heard weakly</span><span><i style="background:#5b6b72"></i>Not heard</span><span><i style="background:transparent;border:1.5px solid #dce6ea"></i>Not checked yet</span></div>
 <p class="hint" id="bhint">Pulsing rings show the five beacons on the air now, one per frequency, each for 10 seconds. Tap a frequency to tune there in CW: the radio then checks every beacon slot and colours what it hears. To follow one beacon across the bands, choose it at the top and press <b>Start</b> (or tap it on the map): the radio goes to that beacon in CW at once. It changes band by itself every 10 seconds while the beacon is on the air and waits on 14.100 MHz while it is silent (130 of every 180 seconds), until you press <b>Stop</b>, which takes the radio back to where it was before, or tune away or choose another band, which ends it where you are. The line beside the buttons counts down to its next turn. The radio's clock must be set.</p></div></section>
<section data-tab="dec"><div class="card"><h2>Decodes <span id="dmode" style="float:right;color:var(--muted);font-weight:600">Choose a decoder and press Start</span></h2>
<div class="bfw"><div class="bfh"><b>Decoder on the radio</b><span id="dpst"></span></div>
 <div class="dpr"><select id="dpsel" aria-label="Decoder"></select><select id="dpch" aria-label="Band"></select><button id="dpgo">Start</button><button id="dpstop">Stop</button></div>
 <p class="hint"><b>Start</b> opens the decoder on the radio, as its menu does. FT8 and FT4 go to the band you choose, NAVTEX to 518 kHz; CW, RTTY, PSK31 and WEFAX decode where the radio is, so tune first. <b>Stop</b> closes it and the radio goes back to where it was.</p></div>
<canvas id="dwf" width="600" height="110" style="width:100%;height:110px;background:#041F29;border-radius:8px"></canvas>
<canvas id="dfax" width="600" height="400" style="width:100%;display:none;background:#fff;border-radius:8px;margin-top:8px"></canvas>
<p id="dfaxsave" class="dbtn" style="display:none;margin:8px 0 0"><button onclick="(()=>{const a=document.createElement('a');a.href=$('dfax').toDataURL('image/png');a.download='resonik-wefax.png';a.click();})()">Save picture</button></p>
<svg id="dmap" viewBox="0 0 720 300" role="img" aria-label="Map of the stations heard" style="width:100%;margin-top:8px"></svg>
<div id="dtxt" style="font-family:monospace;white-space:pre-wrap;word-break:break-all;min-height:1.2em;margin:8px 0"></div>
<div class="dctl"><label>My locator<input id="dqth" maxlength="6" placeholder="LL58"></label>
<label>DX alarm<input id="dalarm" placeholder="call or country"></label>
<label>Decoder audio<select id="daud" onchange="setv(19712,this.value)"><option value="0">3 kHz (fast)</option><option value="1">6 kHz (full)</option></select></label>
<div class="dbtn"><button id="dsave">Save log</button><button id="dclr">Clear</button></div></div>
<p class="hint"><b>Decoder audio.</b> <b>3 kHz</b> holds everything FT8, FT4, RTTY, NAVTEX, PSK31 and WEFAX use, and keeps the radio and this page quick: leave it there. <b>6 kHz</b> takes in twice as much of the radio's sound, as versions before R5 did; the radio works twice as hard and this page gets slower, so choose it only if you get fewer decodes than you expect, and compare. The change applies at once and the waterfall starts again. On the radio: Menu &rarr; Ham &amp; SWL &rarr; Decoder Audio.</p>
<div style="overflow-x:auto"><table id="dtab"><thead><tr><th>UTC</th><th>Mode</th><th>dB</th><th>Call</th><th>Country</th><th>Message</th></tr></thead><tbody></tbody></table></div>
</div></section>
<section data-tab="set"><div id="sets"></div></section>
<section data-tab="more">
<details class="card" id="scr"><summary>Radio screen</summary>
 <p class="hint" style="margin-top:0">The radio's own screen, for its menus and the FT8 and CW pages. Only sent while this is open.</p>
 <div class="screen"><canvas id="c" width="320" height="170"></canvas></div>
 <div class="row">
  <button class="big" data-hold="r">&#9664;</button><button class="big acc" data-cmd="e" title="Enter">Click</button><button class="big" data-hold="R">&#9654;</button>
 </div>
 <div class="row"><button data-cmd="E" title="Space">Press</button></div>
</details>
<details class="card audio" id="aud"><summary>Listen (experimental)</summary><button id="au" style="width:100%;margin-bottom:8px">&#128266; Listen</button>
 <label style="display:block;margin-bottom:6px;color:var(--text)">Radio speaker while listening
  <select id="sil" style="margin-left:6px;padding:4px 6px;border-radius:6px;background:#06171e;color:var(--text);border:1px solid var(--line)">
   <option value="0" selected>Normal (best web sound)</option><option value="4">Quieter</option><option value="8">Quietest</option></select></label>
 <label style="display:block;margin-bottom:6px;color:var(--text)">Hiss filter
  <select id="hf" style="margin-left:6px;padding:4px 6px;border-radius:6px;background:#06171e;color:var(--text);border:1px solid var(--line)">
   <option value="0">Off</option><option value="3400">Light (3.4 kHz)</option><option value="2800" selected>Medium (2.8 kHz)</option><option value="2200">Strong (2.2 kHz)</option></select></label>
 <span id="ai">Audio needs a V4B board (speaker line on GPIO11). Wi-Fi and the receiver share the processor's
 ADC, so the sound is phone quality and can drop out. Headphones in the radio cut it.</span></details>
</section>
<p class="hint">Keyboard: &larr; &rarr; tune, + &minus; volume. Settings change on the radio at once and are saved as from its own menu.
Bluetooth, USB and Wi-Fi modes and firmware updates stay on the radio. <a href="/">Status</a></p>
</main>
<script>
const cv=document.getElementById('c'),cx=cv.getContext('2d'),img=cx.createImageData(320,170),px=img.data;
for(let i=3;i<px.length;i+=4)px[i]=255;
const st=document.getElementById('st'),ai=document.getElementById('ai');
let ws=null,dirty=false;
// R5: the radio sends its status every second; silence for 8 s means the
// connection is stuck (it can stay open while nothing gets through), so the
// page closes it and connects again instead of freezing
let lastMsg=Date.now();
setInterval(()=>{if(ws&&ws.readyState===1&&Date.now()-lastMsg>8000){st.textContent='reconnecting';st.className='';const old=ws;old.onclose=null;old.onmessage=null;try{old.close();}catch(e){}setTimeout(connect,500);}},2000);
function connect(){
 lastMsg=Date.now();
 ws=new WebSocket((location.protocol==='https:'?'wss://':'ws://')+location.host+'/ws');
 ws.binaryType='arraybuffer';
 ws.onopen=()=>{st.textContent='live';st.className='on';if(scrOpen)ws.send('!screen1');if(document.querySelector('section[data-tab="dec"].on'))ws.send('!dec1');if(listening){ws.send('!audio1');ws.send('!mute'+document.getElementById('sil').value);}};
 ws.onclose=()=>{st.textContent='offline';st.className='';setTimeout(connect,2000);};
 ws.onmessage=e=>{lastMsg=Date.now();
  if(typeof e.data==='string'){const j=JSON.parse(e.data);
   if(j.dec||j.dg||j.sp){if(j.dec)dDecodes(j.dec);if(j.dg)dText(j.dg);if(j.sp)dSpec(j.sp);if(j.fx)dFax(j.fx);}else if(j.p)panel(j);else if(j.bcn)beaconReport(j);else if(j.seekf)seekShow(j.seekf);else if(j.note)note(j.note);else if(j.scope)scope(j);else if(j.def){if(j.dl)dLists(j.dl);buildSets(j.def);}else if(j.v)vals(j);else if(j.mem)mems(j.mem);else status(j);return;}
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
// FM: stereo shown in the FM button (mono when the pilot is not received)
function stereo(){const b=$('stb');if(!b||!P)return;b.className=P.st?'st on':'st';b.textContent=P.st?'Stereo':'Mono';}
function panel(j){P=j;dpDraw();stereo();$('pfq').innerHTML=fmtF(j.f,j.m);
 $('tags').innerHTML=(j.band!==j.mode?'<span>'+esc(j.band)+'</span>':'')+'<span class="m">'+esc(j.mode)+'</span><span>Step '+esc(j.step)+
  '</span><span>BW '+esc(j.bw)+'</span>'+(j.mem?'<span class="m" title="Tuned to a saved memory">Memory '+j.mem+'</span>':'')+
  '<span title="'+(j.bmin>0?'About '+Math.floor(j.bmin/60)+' h '+(j.bmin%60)+' min left':'')+'">'+(j.chg?'USB power':j.bat.toFixed(2)+' V &middot; '+j.batp+'%')+'</span>';
 if(j.lo!==undefined){const fm=j.m===0,u=fm?1e6:1e3,d=fm?1:0;$('f').placeholder=(fm?'MHz':'kHz')+' '+(j.lo/u).toFixed(d)+'\u2013'+(j.hi/u).toFixed(d);}
 // S-meter: S9 = 34 dBuV (50 ohm), 6 dB per S unit; FM in dBuV
 let lab;if(j.m===0)lab=j.rssi+' dB&micro;V';else{const s=(j.rssi-34)/6+9;
  lab=s<=9?'S'+Math.max(0,Math.round(s)):'S9+'+Math.round(j.rssi-34);}
 $('sv').innerHTML=lab;$('sb').style.width=Math.min(100,j.rssi/80*100)+'%';$('sn').textContent='SNR '+j.snr+' dB';
 $('ps').textContent=j.ps.trim();$('rt').textContent=j.rt.trim();drawScope();}
// Band scope
let S=null;const sc=$('sc'),sx=sc.getContext('2d');
$('scb').onclick=()=>{$('scb').textContent='Sweeping…';send('!scope');};
function scope(j){S=j;$('scb').textContent='Sweep';drawScope();
 if(j.seek){$('scl').textContent='Last Seek: signal strength at each kHz it checked ('+(j.f0/1e3).toFixed(0)+'\u2013'+((j.f0+j.st*(j.r.length-1))/1e3).toFixed(0)+
  ' kHz). It stops at the first bar above the yellow line. Click a bar to tune there.';return;}
 const u=j.fm?1e6:1e3,d=j.fm?2:(j.st%1000?1:0);
 $('scl').textContent=(j.f0/u).toFixed(d)+' – '+((j.f0+j.st*(j.r.length-1))/u).toFixed(d)+(j.fm?' MHz':' kHz')+
  ', step '+(j.st/1000)+' kHz. Click a peak to tune to it.';}
function drawScope(){if(!S||!S.r.length)return;const W=sc.width,H=sc.height,n=S.r.length,mx=Math.max(30,...S.r,S.thr||0);
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
 if(S.thr){const ty=H-S.thr/mx*(H-14);sx.strokeStyle='#f5c542';sx.setLineDash([6,4]);sx.lineWidth=1.5;
  sx.beginPath();sx.moveTo(0,ty);sx.lineTo(W,ty);sx.stroke();sx.setLineDash([]);}
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
// Settings: built from the radio's own list, values kept up to date
let DEF=[],V=null,scrOpen=false;const GROUPS=['Tuning','Audio','Receiver','Display','Power and time'];
const QUICK={4096:1,5376:1,5120:1,4864:1,4608:1,4352:1};
function opt(sel,list,cur){const k=list.join('|');if(sel.dataset.k!==k){sel.innerHTML=list.map((o,i)=>'<option value="'+i+'">'+esc(o)+'</option>').join('');sel.dataset.k=k;}
 if(document.activeElement!==sel)sel.value=cur;}
function agcList(m){const a=['AGC on','AGC off'];for(let i=2;i<=m;i++)a.push('Att '+(i-1));return a;}
function stepLbl(s){return /^\d+$/.test(s)?s+' Hz':s;}
function setv(c,v){send('!set '+c+' '+v);}
function buildSets(def){DEF=def;const box=$('sets');box.innerHTML='';
 const by={};def.forEach(d=>{if(QUICK[d.c])return;(by[d.g]=by[d.g]||[]).push(d);});
 GROUPS.concat(Object.keys(by).filter(g=>!GROUPS.includes(g))).forEach(g=>{if(!by[g])return;
  const det=document.createElement('details');det.className='card';det.innerHTML='<summary>'+esc(g)+'</summary><div class="set"></div>';
  const grid=det.querySelector('.set');
  by[g].forEach(d=>{const it=document.createElement('div');it.className='it';it.id='it'+d.c;
   if(d.t==='b'){it.innerHTML='<label class="tg"><input type="checkbox" id="s'+d.c+'"> '+esc(d.n)+'</label>';
    it.querySelector('input').onchange=e=>setv(d.c,e.target.checked?1:0);}
   else if(d.t==='n'){it.innerHTML=esc(d.n)+'<div class="rv"><input type="range" id="s'+d.c+'" min="'+d.min+'" max="'+d.max+'" step="'+d.s+'"><b id="s'+d.c+'v"></b></div>';
    const r=it.querySelector('input'),out=it.querySelector('b');let t=0;
    r.oninput=()=>{out.textContent=fmtN(d,+r.value);clearTimeout(t);t=setTimeout(()=>setv(d.c,r.value),120);};}
   else{it.innerHTML=esc(d.n)+'<select id="s'+d.c+'"></select>';const sel=it.querySelector('select');
    if(d.o)opt(sel,d.o,0);sel.onchange=()=>setv(d.c,sel.value);}
   if(d.h){const h=document.createElement('small');h.className='ih';h.textContent=d.h;it.appendChild(h);}
   grid.appendChild(it);});
  box.appendChild(det);});
 // Quick controls
 const b=def.find(d=>d.c===4096);if(b){$('bands').innerHTML=b.o.map((o,i)=>'<button data-b="'+i+'">'+esc(o)+'</button>').join('');
  $('bands').querySelectorAll('button').forEach(x=>x.onclick=()=>setv(4096,x.dataset.b));opt($('q4096'),b.o,0);}
 if(V)vals(V);}
function fmtN(d,v){if(d.c===8704&&v>0)return '+'+v+' Hz';if((d.c===10752||d.c===13824)&&!v)return 'Off';if(d.c===7168&&!v)return 'Off';return v+(d.u?' '+d.u:'');}
['q4096','q5120','q4864','q4608'].forEach(id=>{$(id).onchange=e=>setv(+id.slice(1),e.target.value);});
{let t=0;const r=$('q4352');r.oninput=()=>{$('q4352v').textContent=r.value;clearTimeout(t);t=setTimeout(()=>setv(4352,r.value),100);};}
function vals(j){const wasFol=V&&V.v&&V.v[15360]>=2;V=j;const v=j.v,x=j.x||[];if(v[15360]>=2)bOk=true;if(wasFol&&!(v[15360]>=2))note('Beacon follow off');if($('daud')&&v[19712]!==undefined&&document.activeElement!==$('daud'))$('daud').value=v[19712];
 // Quick
 opt($('q4096'),$('q4096').dataset.k?$('q4096').dataset.k.split('|'):[],v[4096]);
 $('bands').querySelectorAll('button').forEach(b=>b.classList.toggle('on',+b.dataset.b===v[4096]));
 const ml=j.d[5376]||[];$('modes').innerHTML=ml.map((m,i)=>'<button data-m="'+i+'"'+(i===v[5376]?' class="on"':'')+'>'+esc(m)+(m==='FM'?'<span id="stb"></span>':'')+'</button>').join('');stereo();
 $('modes').querySelectorAll('button').forEach(b=>b.onclick=()=>setv(5376,b.dataset.m));
 opt($('q5120'),(j.d[5120]||[]).map(stepLbl),v[5120]);opt($('q4864'),j.d[4864]||[],v[4864]);opt($('q4608'),agcList(j.amax),v[4608]);
 const r=$('q4352');if(document.activeElement!==r){r.value=v[4352];$('q4352v').textContent=v[4352];}
 // Settings
 DEF.forEach(d=>{if(QUICK[d.c])return;const el=$('s'+d.c);if(!el)return;const it=$('it'+d.c);
  const off=x.includes(d.c);it.classList.toggle('off',off);el.disabled=off;
  if(document.activeElement===el)return;
  if(d.t==='b')el.checked=!!v[d.c];
  else if(d.t==='n'){el.value=v[d.c];$('s'+d.c+'v').textContent=fmtN(d,v[d.c]);}
  else if(d.o)opt(el,d.o,v[d.c]);});}
function mems(list){$('mems').innerHTML=list.length?list.map(m=>'<div class="mem"><button class="t" data-i="'+m.i+'"><b>'+(m.i+1)+'</b> &nbsp;'+
  fmtF(m.f,['FM','LSB','USB','AM','CW'].indexOf(m.m)).replace(/<\/?small>/g,' ')+' '+esc(m.m)+(m.n?' &middot; '+esc(m.n):'')+'</button><button class="x" data-i="'+m.i+'" title="Delete">&times;</button></div>').join(''):
  '<p class="hint" style="margin:0">No memories yet.</p>';
 $('mems').querySelectorAll('.t').forEach(b=>b.onclick=()=>send('!mem t '+b.dataset.i));
 $('mems').querySelectorAll('.x').forEach(b=>b.onclick=()=>{if(confirm('Delete memory '+(+b.dataset.i+1)+'?'))send('!mem d '+b.dataset.i);});}
$('msave').onclick=()=>send('!mem s');
$('scr').addEventListener('toggle',e=>{scrOpen=$('scr').open;send(scrOpen?'!screen1':'!screen0');});
// Tabs (remembered in this browser)
function showTab(t){document.querySelectorAll('#tabs button').forEach(b=>b.classList.toggle('on',b.dataset.t===t));
 document.querySelectorAll('section[data-tab]').forEach(x=>x.classList.toggle('on',x.dataset.tab===t));
 try{localStorage.setItem('rtab',t);}catch(e){}if(t==='bcn')drawBeacons();try{if(ws&&ws.readyState===1)ws.send(t==='dec'?'!dec1':'!dec0');}catch(e){}}
document.querySelectorAll('#tabs button').forEach(b=>b.onclick=()=>showTab(b.dataset.t));
try{const t=localStorage.getItem('rtab');if(t&&document.querySelector('section[data-tab="'+t+'"]'))showTab(t);}catch(e){}
// Beacons: schedule from the clock, as on the radio
const BC=[['4U1UN','New York',40.75,-73.97],['VE8AT','Eureka, Canada',79.99,-85.9],['W6WX','California',37.16,-121.9],['KH6RS','Hawaii',20.75,-156.4],
 ['ZL6B','New Zealand',-41.06,175.6],['VK6RBP','Australia',-32.1,116.0],['JA2IGY','Japan',34.46,136.78],['RR9O','Siberia',54.98,82.9],['VR2B','Hong Kong',22.27,114.15],
 ['4S7B','Sri Lanka',6.9,79.87],['ZS6DN','South Africa',-25.9,28.3],['5Z4B','Kenya',-1.25,36.67],['4X6TU','Israel',32.06,34.77],['OH2B','Finland',60.32,24.83],
 ['CS3B','Madeira',32.83,-16.9],['LU4AA','Argentina',-34.6,-58.4],['OA4B','Peru',-12.07,-77.0],['YV5B','Venezuela',10.4,-66.9]];
const BF=[14100,18110,21150,24930,28200],BCOL=['#4dabf7','#06D7A0','#f5c542','#ff8c42','#ff6b6b'];
// Follow a beacon: the radio's answer is shown beside the buttons (and as a note)
let bOk=false,bMsg='',bMsgT=0,bWait=0;function bSay(t){bMsg=t;bMsgT=Date.now();drawBeacons();}
function bStart(i){bOk=false;bMsg='';bWait=Date.now();setv(15360,i+2);drawBeacons();
 setTimeout(()=>{if(!bOk&&!bMsg)bSay(ws&&ws.readyState===1?'No answer from the radio: press Start again':'Not connected to the radio: wait for "live" at the top, then Start');else drawBeacons();},2500);}
let bSel=0;const heard=[{},{},{},{},{}];   // band -> beacon -> {q,r,t}
function bOn(b,utc){return((Math.floor((utc%180)/10)-b+18)%18);}
(function(){let g='';for(let x=0;x<=720;x+=60)g+='<line x1="'+x+'" y1="0" x2="'+x+'" y2="300"/>';for(let y=0;y<=300;y+=60)g+='<line x1="0" y1="'+y+'" x2="720" y2="'+y+'"/>';$('mgrid').innerHTML=g;
 $('mstn').innerHTML=BC.map((c,i)=>{const x=(c[3]+180)*2,y=(90-c[2])*2,ly=y<20?y+16:y-8;
  return '<g class="stn" data-i="'+i+'"><title></title><circle class="pulse" cx="'+x+'" cy="'+y+'" r="7" style="display:none"/><circle cx="'+x+'" cy="'+y+'" r="13" fill="transparent"/><circle class="dot" cx="'+x+'" cy="'+y+'" r="5"/><text class="lbl" x="'+(x+7)+'" y="'+ly+'">'+c[0]+'</text></g>';}).join('');
 $('mstn').querySelectorAll('.stn').forEach(g=>g.onclick=()=>{const i=+g.dataset.i;if(confirm('Follow '+BC[i][0]+' ('+BC[i][1]+') across the bands?\nThe radio changes band by itself until you press Stop (back to where it is now), tune away or choose another band.'))bStart(i);});
 $('bsel').innerHTML=BC.map((c,i)=>'<option value="'+i+'">'+c[0]+' \u00b7 '+c[1]+'</option>').join('');$('bgo').onclick=()=>bStart(+$('bsel').value);$('bstop').onclick=()=>setv(15360,-1);
 $('bnds').innerHTML=BF.map((f,b)=>'<button data-b="'+b+'" style="color:'+BCOL[b]+'"><b></b><span></span></button>').join('');
 $('bnds').querySelectorAll('button').forEach(x=>x.onclick=()=>{bSel=+x.dataset.b;send('F'+BF[bSel]*1000+'\n');setTimeout(()=>{const m=V&&V.d&&V.d[5376];if(m&&m.indexOf('CW')>=0)setv(5376,m.indexOf('CW'));},600);drawBeacons();});})();
// RESONIK R5: the decoder pages of the radio, started and stopped from here.
// DL: what there is (FT8/FT4 channels, Digital modes); P.dp, P.dc: what runs
let DL=null,dpWait=0,dpMsg='',dpMsgT=0,dpLast='';
function dLists(l){DL=l;$('dpsel').innerHTML='<option value="f">FT8</option><option value="4">FT4</option><option value="c">CW</option>'+l.dg.map((n,i)=>'<option value="d'+i+'">'+esc(n)+'</option>').join('');dpBands();dpLast='';dpDraw();}
function dpBands(){if(!DL)return;const k=$('dpsel').value,c=$('dpch');
 if(k==='f'||k==='4'){const ft4=k==='4'?1:0,keep=!c.disabled&&c.selectedIndex>=0?c.options[c.selectedIndex].text:'20M';
  c.innerHTML=DL.ft.map((x,i)=>x[1]===ft4?'<option value="'+i+'">'+esc(x[0])+'</option>':'').join('');c.disabled=false;
  const o=[...c.options].find(o=>o.text===keep)||[...c.options].find(o=>o.text==='20M');if(o)c.value=o.value;}
 else{c.innerHTML='<option>'+(k[0]==='d'&&DL.dg[+k.slice(1)]==='NAVTEX'?'518 kHz':"radio's frequency")+'</option>';c.disabled=true;}}
function dpStart(){const k=$('dpsel').value;if(!k)return;dpMsg='';dpWait=Date.now();
 send(k==='f'||k==='4'?'!page 1 '+$('dpch').value:k==='c'?'!page 2 0':'!page 3 '+k.slice(1));dpDraw();
 setTimeout(()=>{if(!(P&&P.dp)&&!dpMsg){dpMsg=ws&&ws.readyState===1?'No answer from the radio: press Start again':'Not connected to the radio';dpMsgT=Date.now();}dpDraw();},3000);}
function dpDraw(){const e=$('dpst');if(!e)return;const p=P&&P.dp||0,c=P&&P.dp?P.dc:-1,now=Date.now();let t='Not running',hot=false;
 if(p===1)t=c>=0&&DL&&DL.ft[c]?(DL.ft[c][1]?'FT4':'FT8')+' on '+DL.ft[c][0]+' is running':'FT8 is starting\u2026';
 else if(p===2)t='CW is running';
 else if(p===3)t=c>=0&&DL&&DL.dg[c]?DL.dg[c]+' is running':'Digital is starting\u2026';
 else if(dpMsg&&now-dpMsgT<8000){t=dpMsg;hot=true;}
 else if(dpWait&&now-dpWait<3000)t='Starting\u2026';
 e.textContent=t;e.style.color=hot?'var(--accent)':'';$('dpstop').disabled=!p;
 // The two lists follow the radio when what runs there changes, not while you choose
 const key=p+':'+c;if(key===dpLast||!DL||(p&&p!==2&&c<0))return;dpLast=key;
 if(p===1){$('dpsel').value=DL.ft[c][1]?'4':'f';dpBands();$('dpch').value=c;}
 else if(p===2){$('dpsel').value='c';dpBands();}
 else if(p===3){$('dpsel').value='d'+c;dpBands();}
 else $('dmode').textContent='Choose a decoder and press Start';}
$('dpsel').onchange=dpBands;$('dpgo').onclick=dpStart;$('dpstop').onclick=()=>{dpWait=0;send('!page 0 0');};setInterval(dpDraw,1000);
// RESONIK R5: Decodes tab: FT8/FT4 decodes with country and map, CW and
// digital text, a waterfall from the radio's decoder page, DX alarm, log
const PFX=[['EP','Iran','IR'],['EQ','Iran','IR'],['A6','UAE','AE'],['A4','Oman','OM'],['A7','Qatar','QA'],['A9','Bahrain','BH'],['9K','Kuwait','KW'],['HZ','Saudi Arabia','SA'],['7Z','Saudi Arabia','SA'],['YI','Iraq','IQ'],['4X','Israel','IL'],['4Z','Israel','IL'],['JY','Jordan','JO'],['OD','Lebanon','LB'],['YK','Syria','SY'],['TA','Turkey','TR'],['TC','Turkey','TR'],['YM','Turkey','TR'],['SU','Egypt','EG'],['AP','Pakistan','PK'],['VU','India','IN'],['4S','Sri Lanka','LK'],['YA','Afghanistan','AF'],['EX','Kyrgyzstan','KG'],['EY','Tajikistan','TJ'],['EZ','Turkmenistan','TM'],['UK','Uzbekistan','UZ'],['UN','Kazakhstan','KZ'],['4L','Georgia','GE'],['EK','Armenia','AM'],['4J','Azerbaijan','AZ'],['4K','Azerbaijan','AZ'],
['UA9','Asiatic Russia','RU'],['UA0','Asiatic Russia','RU'],['RA9','Asiatic Russia','RU'],['RA0','Asiatic Russia','RU'],['R9','Asiatic Russia','RU'],['R0','Asiatic Russia','RU'],['UA','Russia','RU'],['RA','Russia','RU'],['RU','Russia','RU'],['RV','Russia','RU'],['RW','Russia','RU'],['RX','Russia','RU'],['RZ','Russia','RU'],['R','Russia','RU'],['UR','Ukraine','UA'],['UT','Ukraine','UA'],['US','Ukraine','UA'],['UX','Ukraine','UA'],['UY','Ukraine','UA'],['UZ','Ukraine','UA'],['EU','Belarus','BY'],['EW','Belarus','BY'],['ER','Moldova','MD'],
['SP','Poland','PL'],['SQ','Poland','PL'],['SN','Poland','PL'],['OK','Czechia','CZ'],['OL','Czechia','CZ'],['OM','Slovakia','SK'],['HA','Hungary','HU'],['HG','Hungary','HU'],['YO','Romania','RO'],['YP','Romania','RO'],['LZ','Bulgaria','BG'],['SV','Greece','GR'],['SX','Greece','GR'],['5B','Cyprus','CY'],['C4','Cyprus','CY'],['9A','Croatia','HR'],['S5','Slovenia','SI'],['E7','Bosnia','BA'],['YU','Serbia','RS'],['YT','Serbia','RS'],['4O','Montenegro','ME'],['Z3','North Macedonia','MK'],['ZA','Albania','AL'],
['I','Italy','IT'],['IS0','Sardinia','IT'],['IT9','Sicily','IT'],['9H','Malta','MT'],['HB0','Liechtenstein','LI'],['HB','Switzerland','CH'],['OE','Austria','AT'],['DL','Germany','DE'],['DK','Germany','DE'],['DJ','Germany','DE'],['DF','Germany','DE'],['DG','Germany','DE'],['DH','Germany','DE'],['DB','Germany','DE'],['DC','Germany','DE'],['DD','Germany','DE'],['DM','Germany','DE'],['DO','Germany','DE'],['DA','Germany','DE'],['F','France','FR'],['TM','France','FR'],['ON','Belgium','BE'],['OO','Belgium','BE'],['OT','Belgium','BE'],['PA','Netherlands','NL'],['PB','Netherlands','NL'],['PD','Netherlands','NL'],['PE','Netherlands','NL'],['PH','Netherlands','NL'],['LX','Luxembourg','LU'],
['G','England','GB'],['M','England','GB'],['2E','England','GB'],['GW','Wales','GB'],['MW','Wales','GB'],['GM','Scotland','GB'],['MM','Scotland','GB'],['GI','Northern Ireland','GB'],['MI','Northern Ireland','GB'],['EI','Ireland','IE'],['EJ','Ireland','IE'],['EA8','Canary Islands','ES'],['EA6','Balearic Islands','ES'],['EA','Spain','ES'],['EB','Spain','ES'],['EC','Spain','ES'],['ED','Spain','ES'],['EE','Spain','ES'],['CT3','Madeira','PT'],['CU','Azores','PT'],['CT','Portugal','PT'],['CS','Portugal','PT'],
['OH','Finland','FI'],['OG','Finland','FI'],['SM','Sweden','SE'],['SA','Sweden','SE'],['SE','Sweden','SE'],['SK','Sweden','SE'],['LA','Norway','NO'],['LB','Norway','NO'],['OZ','Denmark','DK'],['OU','Denmark','DK'],['OX','Greenland','GL'],['TF','Iceland','IS'],['ES','Estonia','EE'],['YL','Latvia','LV'],['LY','Lithuania','LT'],
['K','USA','US'],['W','USA','US'],['N','USA','US'],['AA','USA','US'],['AB','USA','US'],['AC','USA','US'],['AD','USA','US'],['AE','USA','US'],['AF','USA','US'],['AG','USA','US'],['AI','USA','US'],['AJ','USA','US'],['AK','USA','US'],['KH6','Hawaii','US'],['KL','Alaska','US'],['KP4','Puerto Rico','PR'],['VE','Canada','CA'],['VA','Canada','CA'],['VO','Canada','CA'],['VY','Canada','CA'],['XE','Mexico','MX'],['XF','Mexico','MX'],
['PY','Brazil','BR'],['PP','Brazil','BR'],['PQ','Brazil','BR'],['PR','Brazil','BR'],['PS','Brazil','BR'],['PT','Brazil','BR'],['PU','Brazil','BR'],['LU','Argentina','AR'],['LW','Argentina','AR'],['CE','Chile','CL'],['CX','Uruguay','UY'],['ZP','Paraguay','PY'],['OA','Peru','PE'],['HK','Colombia','CO'],['YV','Venezuela','VE'],['HC','Ecuador','EC'],['CP','Bolivia','BO'],['TI','Costa Rica','CR'],['HP','Panama','PA'],['CO','Cuba','CU'],['CM','Cuba','CU'],['HI','Dominican Rep.','DO'],
['JA','Japan','JP'],['JE','Japan','JP'],['JF','Japan','JP'],['JG','Japan','JP'],['JH','Japan','JP'],['JI','Japan','JP'],['JJ','Japan','JP'],['JK','Japan','JP'],['JL','Japan','JP'],['JM','Japan','JP'],['JN','Japan','JP'],['JO','Japan','JP'],['JP','Japan','JP'],['JQ','Japan','JP'],['JR','Japan','JP'],['JS','Japan','JP'],['7J','Japan','JP'],['7K','Japan','JP'],['7L','Japan','JP'],['7M','Japan','JP'],['7N','Japan','JP'],['8J','Japan','JP'],['HL','South Korea','KR'],['DS','South Korea','KR'],['6K','South Korea','KR'],['BV','Taiwan','TW'],['BY','China','CN'],['BG','China','CN'],['BH','China','CN'],['BD','China','CN'],['BA','China','CN'],['BI','China','CN'],['VR','Hong Kong','HK'],['XX9','Macao','MO'],['HS','Thailand','TH'],['E2','Thailand','TH'],['9M','Malaysia','MY'],['9V','Singapore','SG'],['YB','Indonesia','ID'],['YC','Indonesia','ID'],['YD','Indonesia','ID'],['DU','Philippines','PH'],['DV','Philippines','PH'],['DW','Philippines','PH'],['XV','Vietnam','VN'],['3W','Vietnam','VN'],['XU','Cambodia','KH'],['S2','Bangladesh','BD'],['9N','Nepal','NP'],
['VK','Australia','AU'],['ZL','New Zealand','NZ'],['ZS','South Africa','ZA'],['ZR','South Africa','ZA'],['ZU','South Africa','ZA'],['5Z','Kenya','KE'],['5H','Tanzania','TZ'],['5X','Uganda','UG'],['9J','Zambia','ZM'],['Z2','Zimbabwe','ZW'],['V5','Namibia','NA'],['A2','Botswana','BW'],['7Q','Malawi','MW'],['C9','Mozambique','MZ'],['5R','Madagascar','MG'],['3B8','Mauritius','MU'],['FR','Reunion','RE'],['ET','Ethiopia','ET'],['ST','Sudan','SD'],['5A','Libya','LY'],['3V','Tunisia','TN'],['7X','Algeria','DZ'],['CN','Morocco','MA'],['6W','Senegal','SN'],['9G','Ghana','GH'],['5N','Nigeria','NG'],['TJ','Cameroon','CM'],['D2','Angola','AO'],['9Q','DR Congo','CD']];
const FLAG=c=>c?String.fromCodePoint(...[...c].map(x=>127397+x.charCodeAt(0))):'';
function country(call){if(!call)return null;call=call.replace(/[<>]/g,'');let base=call;const p=call.split('/');
 if(p.length>1){const longest=p.reduce((x,y)=>x.length>=y.length?x:y);const pre=p.find(x=>x!==longest&&x.length<=4&&!/^(P|M|MM|AM|QRP|A|\d)$/.test(x));base=pre||longest;}
 let best=null;for(const e of PFX)if(base.startsWith(e[0])&&(!best||e[0].length>best[0].length))best=e;return best;}
function grid2ll(g){if(!/^[A-R]{2}\d{2}/i.test(g))return null;g=g.toUpperCase();let lon=(g.charCodeAt(0)-65)*20-180+(+g[2])*2+1,lat=(g.charCodeAt(1)-65)*10-90+(+g[3])+0.5;return[lat,lon];}
function parseMsg(t){const w=t.trim().split(/\s+/);let grid=null,call=null;const last=w[w.length-1];if(/^[A-R]{2}\d{2}$/.test(last)&&last!=='RR73')grid=last;
 if(w[0]==='CQ'){call=grid?w[w.length-2]:w[w.length-1];if(/^(DX|POTA|SOTA|NA|EU|AS|AF|OC|SA|TEST)$/.test(call))call=null;}else if(w.length>=2)call=w[1];
 return{call,grid};}
const DLOG=[];let dmapReady=false,dSlot=0;
function dInit(){if(dmapReady)return;dmapReady=true;const src=$('map');if(src){const cl=src.cloneNode(true);const st=cl.querySelector('#mstn');if(st)st.remove();const gr=cl.querySelector('#mgrid');if(gr){gr.removeAttribute('id');gr.setAttribute('stroke','#0e3a48');}
  const land=cl.querySelector('.land');if(land)land.setAttribute('fill','#1d4a5c');$('dmap').innerHTML=cl.innerHTML;}
 const g=document.createElementNS('http://www.w3.org/2000/svg','g');g.id='dpts';$('dmap').appendChild(g);
 try{$('dqth').value=localStorage.getItem('dqth')||'';$('dalarm').value=localStorage.getItem('dalarm')||'';}catch(e){}
 $('dqth').onchange=()=>{try{localStorage.setItem('dqth',$('dqth').value.toUpperCase());}catch(e){}dDraw();};
 $('dalarm').onchange=()=>{try{localStorage.setItem('dalarm',$('dalarm').value);}catch(e){}};
 $('dsave').onclick=()=>{const t=DLOG.map(r=>r.join('\t')).join('\n');const a=document.createElement('a');a.href=URL.createObjectURL(new Blob([t],{type:'text/plain'}));a.download='resonik-decodes-'+new Date().toISOString().slice(0,10)+'.txt';a.click();};
 $('dclr').onclick=()=>{DLOG.length=0;$('dtab').tBodies[0].innerHTML='';dDraw();};}
function dBeep(){try{const c=new (window.AudioContext||window.webkitAudioContext)(),o=c.createOscillator(),g=c.createGain();o.frequency.value=880;o.connect(g);g.connect(c.destination);g.gain.value=0.2;o.start();o.stop(c.currentTime+0.4);}catch(e){}}
function dAlarm(call,ctry){const a=($('dalarm').value||'').trim().toUpperCase();if(!a)return false;return a.split(/[ ,]+/).some(x=>x&&((call&&call.toUpperCase().startsWith(x))||(ctry&&ctry.toUpperCase()===x)));}
function dDecodes(d){dInit();$('dmode').textContent=d.m+' · '+d.r.length+' decoded'+(d.k?' · '+d.k+' kHz audio':'');const utc=new Date().toISOString().substr(11,8);let ring=false;const tb=$('dtab').tBodies[0];
 d.r.forEach(r=>{const p=parseMsg(r[2]),c=country(p.call);const row=[utc,d.m,r[0],p.call||'',c?c[1]:'',r[2]];DLOG.unshift(row);if(DLOG.length>500)DLOG.pop();
  const al=dAlarm(p.call,c&&c[1]);if(al)ring=true;const tr=document.createElement('tr');if(al)tr.style.background='#5b1d2b';
  tr.innerHTML='<td>'+utc+'</td><td>'+d.m+'</td><td>'+r[0]+'</td><td><b>'+(p.call||'')+'</b></td><td>'+(c?FLAG(c[2])+' '+c[1]:'')+'</td><td>'+r[2].replace(/</g,'&lt;')+'</td>';
  if(p.grid)tr.dataset.g=p.grid;tb.insertBefore(tr,tb.firstChild);});
 while(tb.rows.length>200)tb.deleteRow(-1);if(ring)dBeep();dDraw();}
function dDraw(){const g=$('dpts');if(!g)return;const q=grid2ll(($('dqth').value||'').toUpperCase());let h='',seen={};
 const P=([la,lo])=>[(lo+180)*2,(90-la)*2];
 [...$('dtab').tBodies[0].rows].slice(0,60).forEach(tr=>{const gr=tr.dataset.g;if(!gr||seen[gr])return;seen[gr]=1;const ll=grid2ll(gr);if(!ll)return;const [x,y]=P(ll);
  if(q){const [qx,qy]=P(q);h+='<line x1="'+qx+'" y1="'+qy+'" x2="'+x+'" y2="'+y+'" stroke="#108AB1" stroke-width="1" opacity=".7"/>';}
  h+='<circle cx="'+x+'" cy="'+y+'" r="4" fill="#06D7A0"><title>'+tr.cells[3].textContent+' '+gr+'</title></circle>';});
 if(q){const [qx,qy]=P(q);h+='<circle cx="'+qx+'" cy="'+qy+'" r="5" fill="#F04770"/>';}g.innerHTML=h;}
function dText(d){dInit();$('dmode').textContent=d.m+(d.m==='CW'?(d.c?' · '+d.c+' WPM':''):(d.c?' · '+d.c+' Hz':' · no signal'))+(d.k?' · '+d.k+' kHz audio':'');$('dtxt').textContent=d.t;
 const a=($('dalarm').value||'').trim().toUpperCase();if(a&&a.split(/[ ,]+/).some(x=>x&&d.t.toUpperCase().endsWith(x))&&!dText.rang){dText.rang=1;dBeep();setTimeout(()=>dText.rang=0,10000);}}
function dSpec(s){const c=$('dwf');if(!c)return;const x=c.getContext('2d'),w=c.width,h=c.height;const im=x.getImageData(0,0,w,h-1);x.putImageData(im,0,1);
 const n=s.v.length;for(let i=0;i<n;i++){const v=s.v[i];x.fillStyle='rgb('+Math.min(255,v*1.6|0)+','+Math.min(255,v*1.2|0)+','+(80+v*0.6|0)+')';x.fillRect(Math.floor(i*w/n),0,Math.ceil(w/n),1);}}
function dFax(rows){const c=$('dfax');c.style.display='';$('dfaxsave').style.display='';const x=c.getContext('2d'),w=c.width,h=c.height,lh=2;
 rows.forEach(r=>{const im=x.getImageData(0,lh,w,h-lh);x.putImageData(im,0,0);const n=r.length;for(let i=0;i<n;i++){const v=r[i];x.fillStyle='rgb('+v+','+v+','+v+')';x.fillRect(Math.floor(i*w/n),h-lh,Math.ceil(w/n),lh);}});}
function drawBeacons(){if(!document.querySelector('section[data-tab="bcn"].on'))return;
 {const fi=V&&V.v?V.v[15360]-2:-1,on=fi>=0&&!!BC[fi],e=$('bfolt'),now=Date.now();let t='Not following',hot=false;
  if(on){const u=Math.floor(now/1000),k=(Math.floor((u%180)/10)-fi+18)%18;   // k: which of its five bands, 5.. = silent
   t='Following '+BC[fi][0]+' ('+BC[fi][1]+') \u00b7 '+(k<5?'on '+(BF[k]/1000).toFixed(3)+' MHz now':'back in '+((18-k)*10-u%10)+' s, waiting on 14.100 MHz');}
  else if(bMsg&&now-bMsgT<8000){t=bMsg;hot=true;}
  else if(bWait&&now-bWait<2500)t='Starting\u2026';
  e.textContent=t;e.style.color=hot?'var(--accent)':'';
  $('bstop').disabled=!on;if(on&&document.activeElement!==$('bsel'))$('bsel').value=fi;}
 const utc=Math.floor(Date.now()/1000),left=10-utc%10;
 $('bclk').textContent=new Date(utc*1000).toISOString().substr(11,8)+' UTC';
 // Which band is the radio on? Its heard colours are shown
 if(P){const k=BF.findIndex(f=>Math.abs(P.f-f*1000)<=1000);if(k>=0)bSel=k;}
 const onAir={};BF.forEach((f,b)=>onAir[bOn(b,utc)]=b);
 $('bnds').querySelectorAll('button').forEach(x=>{const b=+x.dataset.b,i=bOn(b,utc);x.classList.toggle('on',b===bSel);
  x.querySelector('b').textContent=(BF[b]/1000).toFixed(2)+' MHz';x.querySelector('span').textContent=BC[i][0]+' \u00b7 '+left+' s';});
 $('mstn').querySelectorAll('.stn').forEach(g=>{const i=+g.dataset.i,dot=g.querySelector('.dot'),pl=g.querySelector('.pulse'),h=heard[bSel][i];
  const b=onAir[i];if(b!==undefined){pl.style.display='';pl.style.stroke=BCOL[b];}else pl.style.display='none';
  dot.style.fill=!h?'transparent':h.q>=10?'#06D7A0':h.q>=3?'#f5c542':'#5b6b72';dot.style.stroke=h?'#06171e':'#dce6ea';
  g.querySelector('title').textContent=BC[i][0]+' '+BC[i][1]+(b!==undefined?' \u2014 on air now, '+(BF[b]/1000).toFixed(2)+' MHz':'')+
   (h?'\nLast slot on '+(BF[bSel]/1000).toFixed(2)+' MHz: SNR '+h.q+' dB, '+h.r+' dB\u00b5V, '+new Date(h.t*1000).toISOString().substr(11,5)+' UTC':'');});}
setInterval(drawBeacons,1000);
function beaconReport(j){heard[j.b][j.i]={q:j.q,r:j.r,t:j.t};drawBeacons();}
// kHz; on FM a value under 200 is MHz (102.6), as the hint says
// Seek in SSB/CW: the frequency moves on the panel while it searches
function seekShow(f){$('pfq').innerHTML=fmtF(f,P?P.m:1);$('pfq').style.opacity=.6;note('Seeking\u2026');}
let noteT=0;function note(t){const n=$('note');n.textContent=t;n.style.display='block';clearTimeout(noteT);if(/eacon/.test(t))bSay(t);if(/^Decoder:/.test(t)){dpMsg=t.slice(9);dpMsgT=Date.now();dpDraw();}
 if(!/Seeking/.test(t)){$('pfq').style.opacity='';if(P)$('pfq').innerHTML=fmtF(P.f,P.m);noteT=setTimeout(()=>n.style.display='none',5000);}}
function tune(){const v=parseFloat(document.getElementById('f').value.replace(',','.'));
 if(v>0)send('F'+Math.round(P&&P.m===0&&v<200?v*1e6:v*1000)+'\n');}
document.getElementById('go').onclick=tune;
document.getElementById('f').addEventListener('keydown',e=>{if(e.key==='Enter')tune();});
// Audio: 8 kHz samples scheduled with a small buffer
let ac=null,next=0,listening=false,gain=null,gaps=0,gapMs=0,reserve=0.35,lastS=0;
// mu-law to 16-bit
const ulaw=new Int16Array(256);for(let i=0;i<256;i++){const u=~i&255,s=u&128,e=(u>>4)&7,m=u&15;
 let v=((m<<3)+0x84)<<e;v-=0x84;ulaw[i]=s?-v:v;}
document.getElementById('au').onclick=function(){
 listening=!listening;this.textContent=listening?'\u{1F507} Stop listening':'\u{1F50A} Listen (experimental)';
 if(listening&&!ac){ac=new(window.AudioContext||window.webkitAudioContext)();gain=ac.createGain();gain.gain.value=1;
  // Filters before the level: a low cut for hum and rumble, and a steep
  // high cut (two stages) for the ADC hiss above the voice band
  hp=ac.createBiquadFilter();hp.type='highpass';hp.frequency.value=180;
  lp1=ac.createBiquadFilter();lp1.type='lowpass';lp2=ac.createBiquadFilter();lp2.type='lowpass';
  hp.connect(lp1);lp1.connect(lp2);lp2.connect(gain);gain.connect(ac.destination);setHiss();}
 if(ac)ac.resume();send(listening?'!audio1':'!audio0');send('!mute'+(listening?document.getElementById('sil').value:'0'));next=0;};
document.getElementById('sil').onchange=function(){if(listening)send('!mute'+this.value);};
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
 // Boost at most 10 times: more only made the hiss louder
 if(!quiet&&peak>0.003)gain.gain.value=Math.min(10,Math.max(0.5,gain.gain.value*0.9+0.1*(0.5/peak)));
 const src=ac.createBufferSource();src.buffer=buf;src.connect(hp);
 const t=ac.currentTime;
 // Ran dry: the sound stopped until this packet (a gap you hear)
 // After a gap keep a bigger reserve (up to 1 s)
 if(next<t+0.02){if(next>0){gaps++;gapMs+=Math.round((t+reserve-next)*1000);reserve=Math.min(1,reserve+0.15);}next=t+reserve;}
 else if(next>t+2)next=t+reserve;
 src.start(next);next+=buf.duration;}
let hp=null,lp1=null,lp2=null;
function setHiss(){if(!lp1)return;const f=+document.getElementById('hf').value||3990;
 lp1.frequency.value=f;lp2.frequency.value=f;}
document.getElementById('hf').onchange=setHiss;
let quiet=false,emptyS=0;
function status(j){
 // No input on GPIO11 (V4B with headphones plugged in: the speaker line is
 // cut): play silence instead of the ADC noise turned up 40 times. Not
 // while the radio tests switching its amplifier off (silent for 2 s on
 // purpose, test 10c mistook that for headphones), and only after 3 s
 emptyS=(j.audio&&j.level<2&&j.spk!=='testing')?emptyS+1:0;
 quiet=emptyS>=3;
 if(quiet&&gain)gain.gain.value=0;
 if(quiet){ai.innerHTML='<b>No audio on GPIO11</b> (level '+j.level+') &middot; headphones plugged into the radio? On the V4B they cut the line the web audio comes from. '+
  'Unplug them; to keep the radio quieter use &ldquo;Radio speaker while listening&rdquo;.';return;}
 if(!j.audio){ai.innerHTML='Audio off'+(j.level<2?' &middot; <b>no audio on GPIO11 (V4B board needed)</b>':'');return;}
 ai.innerHTML='<b>Streaming</b> 8 kHz &middot; level '+j.level+' &middot; '+j.cpu+' MHz &middot; speaker '+
  ({on:'on',testing:'testing\u2026',off:'off',quiet:'quieter'}[j.spk]||j.spk)+'<br>'+
  'Radio: ADC failed '+j.adcErrors+', dropped '+j.late+', filled in '+j.caughtUp+', ring overrun '+j.overrun+
  ', read '+j.readUs+'/'+j.readMax+' &micro;s<br>'+
  'Network: busy '+j.waits+'<br>Browser: gaps '+gaps+' ('+gapMs+' ms)';}
connect();
</script></body></html>)HTML";


#endif // PAGE_CONTROL_H
