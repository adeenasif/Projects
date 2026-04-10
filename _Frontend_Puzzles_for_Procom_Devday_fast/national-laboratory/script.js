/**
 * ============================================================
 * HAWKINS NATIONAL LABORATORY — Signal Recovery Terminal
 * script.js — Interactive Terminal Logic
 * ============================================================
 *
 *  ██╗  ██╗ █████╗ ██╗    ██╗██╗  ██╗██╗███╗  ██╗ ███████╗
 *  ██║  ██║██╔══██╗██║    ██║██║ ██╔╝██║████╗ ██║ ██╔════╝
 *  ███████║███████║██║ █╗ ██║█████╔╝ ██║██╔██╗██║ ███████╗
 *  ██╔══██║██╔══██║██║███╗██║██╔═██╗ ██║██║╚████║ ╚════██║
 *  ██║  ██║██║  ██║╚███╔███╔╝██║ ╚██╗██║██║ ╚███║ ███████║
 *  ╚═╝  ╚═╝╚═╝  ╚═╝ ╚══╝╚══╝ ╚═╝  ╚═╝╚═╝╚═╝  ╚═╝ ╚══════╝
 *
 *  [SYSTEM NOTE]: Some signals exist beyond what the
 *  interface displays. Developers often hide things where
 *  users rarely look.
 *
 *  [FRAGMENT_002]: The source is always worth reading.
 *  Most users never do.
 *
 * ============================================================
 */

'use strict';

/* ============================================================
   CONSTANTS & CONFIG
   ============================================================ */

/** Typing speed in ms per character for boot sequence */
const BOOT_CHAR_DELAY = 18;

/** Delay between log line groups in ms */
const LINE_GROUP_DELAY = 600;

/** Colors used for frequency / signal canvases */
const CANVAS_COLORS = {
  green  : '#00ff88',
  red    : '#ff2244',
  blue   : '#1a8cff',
  yellow : '#ffd700',
  dim    : '#1e1e38',
};

/* ============================================================
   DOM REFERENCES
   ============================================================ */
const terminalBody  = document.getElementById('terminalBody');
const cursorLine    = document.getElementById('cursorLine');
const termClock     = document.getElementById('termClock');
const uptimeEl      = document.getElementById('uptime');
const freqCanvas    = document.getElementById('freqCanvas');
const signalCanvas  = document.getElementById('signalCanvas');
const accessLogEl   = document.getElementById('accessLog');
const scanBarWrap   = document.getElementById('scanBarWrap');
const scanBarFill   = document.getElementById('scanBarFill');
const scanBarPct    = document.getElementById('scanBarPct');
const glitchOverlay = document.getElementById('glitchOverlay');
const modalBackdrop = document.getElementById('modalBackdrop');
const modalBody     = document.getElementById('modalBody');
const modalTitle    = document.getElementById('modalTitle');
const modalClose    = document.getElementById('modalClose');

const btnScan    = document.getElementById('btnScan');
const btnDecode  = document.getElementById('btnDecode');
const btnAnalyze = document.getElementById('btnAnalyze');

/* ============================================================
   UPTIME & CLOCK
   ============================================================ */
const startTime = Date.now();

function pad(n) { return String(n).padStart(2, '0'); }

function updateClock() {
  const now = new Date();
  termClock.textContent = `${pad(now.getHours())}:${pad(now.getMinutes())}:${pad(now.getSeconds())}`;

  const elapsed = Math.floor((Date.now() - startTime) / 1000);
  const h = Math.floor(elapsed / 3600);
  const m = Math.floor((elapsed % 3600) / 60);
  const s = elapsed % 60;
  uptimeEl.textContent = `${pad(h)}:${pad(m)}:${pad(s)}`;
}
setInterval(updateClock, 1000);
updateClock();

/* ============================================================
   TERMINAL LOGGING ENGINE
   ============================================================ */

/**
 * Append a single log line to the terminal body.
 * @param {string} text    — Display text (HTML allowed)
 * @param {string} type    — CSS class suffix: system|info|warn|error|success|hint|decode|blank|divider
 * @param {number} delay   — Animation delay before line appears (ms)
 */
function logLine(text, type = 'info', delay = 0) {
  const el = document.createElement('span');
  el.classList.add('log-line', `log-${type}`);
  el.style.animationDelay = `${delay}ms`;
  el.innerHTML = text;
  terminalBody.appendChild(el);
  // Auto-scroll
  setTimeout(() => {
    terminalBody.scrollTop = terminalBody.scrollHeight;
  }, delay + 50);
}

/**
 * Append multiple lines with staggered delays.
 * @param {Array<[string, string]>} lines — Array of [text, type]
 * @param {number} baseDelay
 * @param {number} stride — ms between each line
 * @returns {number} total duration in ms
 */
function logLines(lines, baseDelay = 0, stride = 90) {
  lines.forEach(([text, type], i) => {
    logLine(text, type, baseDelay + i * stride);
  });
  return baseDelay + lines.length * stride;
}

/** Separator line */
function logSep(delay = 0) {
  logLine('─'.repeat(60), 'divider', delay);
}

/** Blank spacing line */
function logBlank(delay = 0) {
  logLine('', 'blank', delay);
}

/* ============================================================
   SIMULATED TYPING in the input row
   ============================================================ */
function typeCommand(text, delay = 0, speed = BOOT_CHAR_DELAY) {
  let i = 0;
  setTimeout(() => {
    const interval = setInterval(() => {
      cursorLine.textContent = text.slice(0, i);
      i++;
      if (i > text.length) {
        clearInterval(interval);
        setTimeout(() => { cursorLine.textContent = ''; }, 800);
      }
    }, speed);
  }, delay);
}

/* ============================================================
   BOOT SEQUENCE — runs on page load
   ============================================================ */
function bootSequence() {
  const lines = [
    ['HAWKINS NATIONAL LABORATORY — SIGNAL RECOVERY TERMINAL v4.1', 'system'],
    ['DEPARTMENT OF ENERGY — CLASSIFIED NETWORK NODE: HAWKINS-04', 'system'],
    ['', 'blank'],
    ['Initializing secure connection...', 'info'],
    ['Loading encryption keys... <span style="color:var(--green)">OK</span>', 'info'],
    ['Authenticating operator credentials... <span style="color:var(--green)">GRANTED</span>', 'info'],
    ['Mounting remote filesystem... <span style="color:var(--yellow)">PARTIAL</span>', 'warn'],
    ['', 'blank'],
    ['─────── SYSTEM BOOT COMPLETE ───────', 'divider'],
    ['', 'blank'],
    ['ALERT: Signal trace for SUBJECT #011-MK has resumed.', 'warn'],
    ['Last transmission detected: <span style="color:var(--yellow)">72 HOURS AGO</span>', 'warn'],
    ['Transmission origin: SECTOR 7-G, SUBLEVEL 3 — coordinates CLASSIFIED', 'warn'],
    ['', 'blank'],
    ['Searching active memory banks...', 'info'],
    ['Fragment integrity: <span style="color:var(--red)">17% — SEVERELY DEGRADED</span>', 'error'],
    ['Encrypted signals remain hidden in the system.', 'info'],
    ['Recovery requires manual operator intervention.', 'info'],
    ['', 'blank'],
    ['// Some signals never render on the screen.', 'hint'],
    ['// The interface only shows what it was meant to.', 'hint'],
    ['', 'blank'],
    ['Use the command panel below to begin recovery operations.', 'system'],
    ['─'.repeat(60), 'divider'],
  ];

  logLines(lines, 200, 120);
  typeCommand('init_recovery --node=HAWKINS-04 --mode=deep', 300, 22);
}

/* ============================================================
   BUTTON ACTIONS
   ============================================================ */

/* ── SCAN SYSTEM ─────────────────────────────────────────────── */
function runScan() {
  if (btnScan.disabled) return;
  lockButtons();
  triggerGlitch();
  typeCommand('scan_system --full --deep', 100, 20);

  logLine('SCAN: fluctuating frequency 8.13 MHz ...', 'system', 300);
  logLine('Signal fluctuating — fragments scattered across system layers.','warn', 700);
  logLine('Scan deeper....','hint', 900);

  runScanBar(1000);
  addAccessEntry('SYSTEM SCAN', 'green', 'OK');
  setTimeout(unlockButtons, 1200);
}

function runScanBar(totalDuration) {
  scanBarWrap.classList.add('active');
  scanBarFill.style.width = '0%';
  scanBarPct.textContent = '0%';
  const steps = 60;
  const stepDur = totalDuration / steps;
  let step = 0;
  const iv = setInterval(() => {
    step++;
    const pct = Math.min(100, Math.round((step / steps) * 100));
    scanBarFill.style.width = pct + '%';
    scanBarPct.textContent = pct + '%';
    if (step >= steps) {
      clearInterval(iv);
      setTimeout(() => { scanBarWrap.classList.remove('active'); }, 800);
    }
  }, stepDur);
}

/* ── DECODE SIGNAL ───────────────────────────────────────────── */
function runDecode() {
  if (btnDecode.disabled) return;
  lockButtons();
  triggerGlitch();
  typeCommand('decode_signal --channel=057-ALPHA --force', 100, 20);

  logLine('DECODING: "the developer hid signals in unexpected places."', 'hint', 300);
  logLine('Partial fragment: source_......', 'decode', 700);

  addAccessEntry('DECODE ATTEMPT', 'yellow', 'PART');
  setTimeout(unlockButtons, 1200);
}

/* ── ANALYZE TRANSMISSION ────────────────────────────────────── */
const trace = 'VEhFX0dBVEVXQVlfSVNfR0VUVElOR19ORUFS';
function runAnalyze() {
  if (btnAnalyze.disabled) return;
  lockButtons();
  triggerGlitch();
  typeCommand('analyze_transmission --protocol=PHOENIX --verbose', 100, 20);

  logLine('Triangulating ...', 'system', 300);
  logLine('Origin: 44.4° N, 110.6° W — Yellowstone?', 'system', 700);
  logLine('Signal buried in deep web.', 'hint', 1100);
  logLine('Operators usually stop at the interface. Developers rarely do.', 'hint', 1600);
  // PLACEHOLDER: hidden fragment — "developer initial: J.C."
  addAccessEntry('TRANSMISSION ANALYSIS', 'yellow', 'PART');
  setTimeout(unlockButtons, 1500);
}

/* ============================================================
   BUTTON STATE MANAGEMENT
   ============================================================ */
function lockButtons() {
  [btnScan, btnDecode, btnAnalyze].forEach(b => {
    b.disabled = true;
    b.style.opacity = '0.4';
  });
}

function unlockButtons() {
  [btnScan, btnDecode, btnAnalyze].forEach(b => {
    b.disabled = false;
    b.style.opacity = '';
  });
}

/* ============================================================
   GLITCH EFFECT
   ============================================================ */
function triggerGlitch() {
  glitchOverlay.classList.remove('active');
  // Force reflow
  void glitchOverlay.offsetWidth;
  glitchOverlay.classList.add('active');
  setTimeout(() => glitchOverlay.classList.remove('active'), 700);
}

/* ============================================================
   ACCESS LOG — live append
   ============================================================ */
const now = new Date();
function fmtTime() {
  const n = new Date();
  return `${pad(n.getHours())}:${pad(n.getMinutes())}:${pad(n.getSeconds())}`;
}

function addAccessEntry(action, colorClass, status) {
  const li = document.createElement('li');
  li.innerHTML = `<span class="ts">${fmtTime()}</span> ${action} <span class="text-${colorClass}">${status}</span>`;
  accessLogEl.insertBefore(li, accessLogEl.firstChild);
  // Keep list at max 10 entries
  while (accessLogEl.children.length > 10) {
    accessLogEl.removeChild(accessLogEl.lastChild);
  }
}

/* ============================================================
   MODAL
   ============================================================ */
function openModal(title, content) {
  modalTitle.textContent = title;
  modalBody.textContent = content;
  modalBackdrop.setAttribute('aria-hidden', 'false');
  modalBackdrop.classList.add('open');
}

function closeModal() {
  modalBackdrop.classList.remove('open');
  modalBackdrop.setAttribute('aria-hidden', 'true');
}

modalClose.addEventListener('click', closeModal);
modalBackdrop.addEventListener('click', e => {
  if (e.target === modalBackdrop) closeModal();
});
document.addEventListener('keydown', e => {
  if (e.key === 'Escape') closeModal();
});

/* ============================================================
   CANVAS — FREQUENCY BARS
   ============================================================ */
(function drawFreqCanvas() {
  const ctx = freqCanvas.getContext('2d');
  const W = freqCanvas.width;
  const H = freqCanvas.height;
  const bars = 28;
  const barW = (W / bars) - 1;

  // Stable "base" heights per bar (random-ish but seeded)
  const base = Array.from({ length: bars }, (_, i) =>
    0.1 + 0.55 * Math.abs(Math.sin(i * 1.3 + 0.7))
  );

  function draw() {
    ctx.clearRect(0, 0, W, H);
    const t = Date.now() / 1000;

    for (let i = 0; i < bars; i++) {
      const noise = 0.12 * Math.sin(t * 3.1 + i * 0.7) + 0.07 * Math.sin(t * 5.7 + i * 1.3);
      const h = Math.max(2, (base[i] + noise) * H);
      const x = i * (barW + 1);
      const y = H - h;

      // Color based on height
      const ratio = h / H;
      if (ratio > 0.75) {
        ctx.fillStyle = CANVAS_COLORS.red;
      } else if (ratio > 0.45) {
        ctx.fillStyle = CANVAS_COLORS.yellow;
      } else {
        ctx.fillStyle = CANVAS_COLORS.green;
      }

      ctx.globalAlpha = 0.85;
      ctx.fillRect(x, y, barW, h);

      // Glow cap
      ctx.globalAlpha = 0.3;
      ctx.fillRect(x, y - 2, barW, 3);
    }
    ctx.globalAlpha = 1;
    requestAnimationFrame(draw);
  }
  draw();
})();

/* ============================================================
   CANVAS — SIGNAL WAVEFORM (oscilloscope style)
   ============================================================ */
(function drawSignalCanvas() {
  const ctx = signalCanvas.getContext('2d');
  const W = signalCanvas.width;
  const H = signalCanvas.height;
  const MID = H / 2;

  function draw() {
    ctx.clearRect(0, 0, W, H);

    // Grid lines
    ctx.strokeStyle = CANVAS_COLORS.dim;
    ctx.lineWidth = 0.5;
    ctx.globalAlpha = 0.5;
    for (let y = 0; y < H; y += 20) {
      ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(W, y); ctx.stroke();
    }
    for (let x = 0; x < W; x += 30) {
      ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, H); ctx.stroke();
    }
    ctx.globalAlpha = 1;

    // Main waveform
    const t = Date.now() / 1000;
    ctx.beginPath();
    ctx.strokeStyle = CANVAS_COLORS.green;
    ctx.lineWidth = 1.5;
    ctx.shadowColor = CANVAS_COLORS.green;
    ctx.shadowBlur = 6;

    for (let x = 0; x < W; x++) {
      const phase = (x / W) * Math.PI * 6 - t * 3.5;
      const amp = 0.38 * MID;
      // Composite of two frequencies + noise
      const y = MID
        + amp * Math.sin(phase)
        + amp * 0.3 * Math.sin(phase * 2.3 + 1.2)
        + amp * 0.1 * (Math.random() - 0.5)
        // Glitch spike
        + (Math.random() > 0.997 ? (Math.random() - 0.5) * MID * 1.2 : 0);

      if (x === 0) ctx.moveTo(x, y);
      else         ctx.lineTo(x, y);
    }
    ctx.stroke();
    ctx.shadowBlur = 0;

    // Secondary faint waveform (interference)
    ctx.beginPath();
    ctx.strokeStyle = CANVAS_COLORS.red;
    ctx.lineWidth = 0.8;
    ctx.globalAlpha = 0.3;
    for (let x = 0; x < W; x++) {
      const phase = (x / W) * Math.PI * 8 + t * 2.1;
      const y = MID + 0.18 * MID * Math.sin(phase);
      if (x === 0) ctx.moveTo(x, y);
      else         ctx.lineTo(x, y);
    }
    ctx.stroke();
    ctx.globalAlpha = 1;

    requestAnimationFrame(draw);
  }
  draw();
})();

/* ============================================================
   PERIODIC AMBIENT LOG LINES
   (appear organically during idle sessions)
   ============================================================ */
const ambientLines = [
  ['// Signal ping received — origin: UNKNOWN', 'hint'],
  ['[NODE] Memory sweep cycle complete.', 'system'],
  ['[WARN] Dimensional pressure spike at sublevel 3.', 'warn'],
  ['// Encrypted fragments remain hidden in the system...', 'hint'],
  ['[NODE] Heartbeat: HAWKINS-04 ↔ HAWKINS-MAIN — OK', 'system'],
  ['[WARN] Carrier signal fluctuation detected: Ch.057', 'warn'],
  ['// Only those who inspect the system carefully will find the fragments.', 'hint'],
  ['[SYS] Garbage collection: 3 orphaned processes cleared.', 'system'],
  ['[WARN] Temporal echo detected — gate proximity warning.', 'warn'],
  ['[NODE] Auto-save state: fragment_cache_7g.bin', 'system'],
  ['// Button events triggering deeper system routines.', 'hint'],
  ['// Not everything the system knows is rendered to the screen.', 'hint']
];

let ambientIdx = 0;
setInterval(() => {
  if (document.hidden) return;
  const [text, type] = ambientLines[ambientIdx % ambientLines.length];
  logLine(text, type);
  ambientIdx++;
}, 7000);

/* ============================================================
   EVENT LISTENERS
   ============================================================ */
btnScan.addEventListener('click', runScan);
btnDecode.addEventListener('click', runDecode);
btnAnalyze.addEventListener('click', runAnalyze);

/* ============================================================
   EASTER EGG — Konami Code
   ============================================================ */
const KONAMI = [
  'ArrowUp','ArrowUp','ArrowDown','ArrowDown',
  'ArrowLeft','ArrowRight','ArrowLeft','ArrowRight',
  'b','a'
];
let konamiPos = 0;

document.addEventListener('keydown', e => {
  if (e.key === KONAMI[konamiPos]) {
    konamiPos++;
    if (konamiPos === KONAMI.length) {
      konamiPos = 0;
      triggerGlitch();
      logBlank();
      logSep();
      logLines([
        ['⚠ UNAUTHORIZED ACCESS SEQUENCE DETECTED ⚠', 'error'],
        ['Operator code: KONAMI-SEQUENCE-ACCEPTED', 'success'],
        ['', 'blank'],
        ['[FRAGMENT_003] — UNLOCKED', 'decode'],
        ['Subject designation: ELEVEN', 'decode'],
        ['Ability classification: TELEKINETIC / PSYCHIC', 'decode'],
        ['Last known state: DIMENSIONAL TRANSIT', 'decode'],
        ['', 'blank'],
        ['// You found a fragment. There are more.', 'hint'],
        ['// The source hides what the screen cannot hold.', 'hint'],
      ], 0, 90);
      logSep(1100);
    }
  } else {
    konamiPos = e.key === KONAMI[0] ? 1 : 0;
  }
});

/* ============================================================
   INIT — Run boot sequence on DOM ready
   ============================================================ */
bootSequence();

/*
 * ============================================================
 *  [FRAGMENT_004] — END OF SCRIPT
 *
 *  You have now read all four layers:
 *    HTML  → [FRAGMENT_001]
 *    HTML comments → [FRAGMENT_PLACEHOLDER_ALPHA/BETA/GAMMA]
 *    JS    → [FRAGMENT_002], [FRAGMENT_004]
 *    Konami → [FRAGMENT_003] (unlocked interactively)
 *
 *  The developer was last seen at:
 *  LAT 39.9231° N  LON 86.8612° W — SUBLEVEL 3
 *
 *  Trust no one. Especially not the terminal.
 * ============================================================
 */
