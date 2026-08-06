/*
 * Boxes - a web port of the 2002 Windows game by Michael Schmitz.
 *
 * Ported faithfully from the original C++ (World.cpp, Beast.cpp, View.cpp,
 * Procedure.cpp). Cage the squirrels by pushing lines of boxes around them
 * with the arrow keys. Beasts move on a timer, so the game is real-time.
 */
'use strict';

// ---- Constants -----------------------------------------------------------
const TILE = 20;
const COLS = 30, ROWS = 20;          // full window in tiles (incl. border)
const BOARD_W = COLS - 2;            // 28 playable columns
const BOARD_H = ROWS - 2;            // 18 playable rows
const BAR_H = 20;                    // top / bottom info bar height (px)
const PLAY_Y = BAR_H;                // play field starts below the top bar

const ERROR = 0, EMPTY = 1, BOX = 2; // board tile values (out-of-bounds = ERROR)

// state
const INTRO = 0, GAME = 1, ARCADE = 2;

// player faces
const FACE_NORMAL = 0, FACE_COOL = 1, FACE_WINK = 2;
// win faces
const WIN_SMILE = 0, WIN_LAUGH = 1, WIN_CHUCKLE = 2, WIN_BIGSMILE = 3,
      WIN_RELIEVED = 4, WIN_COOL = 5, WIN_SMOKER = 6;

const WINK_INTERVAL = 35000, WINK_DURATION = 350;

const HS_KEY = 'boxes_highscore';

// ---- Sprites -------------------------------------------------------------
const SPRITE_FILES = {
  box: 'box', beast: 'beast',
  normal: 'normal', glass: 'glass', wink: 'wink',
  winsmile: 'winsmile', winlaugh: 'winlaugh', winchuckle: 'winchuckle',
  winbigsmile: 'winbigsmile', winrelieved: 'winrelieved',
  wincool: 'wincool', winsmoke: 'winsmoke',
};
const FACE_SPRITE = ['normal', 'glass', 'wink'];
const WIN_SPRITE = ['winsmile', 'winlaugh', 'winchuckle', 'winbigsmile',
                    'winrelieved', 'wincool', 'winsmoke'];
const sprites = {};

function loadSprites() {
  const names = Object.keys(SPRITE_FILES);
  return Promise.all(names.map(name => new Promise(res => {
    const img = new Image();
    img.onload = img.onerror = () => res();
    img.src = 'sprites/' + SPRITE_FILES[name] + '.png';
    sprites[name] = img;
  })));
}

// ---- World ---------------------------------------------------------------
const W = {
  state: INTRO,
  paused: false, won: false, lost: false,
  level: 1,
  seconds: 0, totalSeconds: 0,
  iBeasts: 4, iBeastDelay: 45, iBoxDensity: 25,
  faceIndex: FACE_NORMAL, winFaceIndex: WIN_SMILE,
  isNewHighscore: false,
  board: new Int8Array(BOARD_W * BOARD_H),
  player: { x: 0, y: 0 },
  beasts: [],
  highscore: { level: 1, seconds: 0 },
};

function loadHighscore() {
  try {
    const raw = localStorage.getItem(HS_KEY);
    if (raw) {
      const h = JSON.parse(raw);
      if (typeof h.level === 'number' && typeof h.seconds === 'number')
        W.highscore = h;
    }
  } catch (e) { /* ignore */ }
}
function saveHighscore() {
  try { localStorage.setItem(HS_KEY, JSON.stringify(W.highscore)); }
  catch (e) { /* ignore */ }
}

// board helpers
function boardGet(x, y) {
  if (x < 0 || x >= BOARD_W || y < 0 || y >= BOARD_H) return ERROR;
  return W.board[x + BOARD_W * y];
}
function boardSet(x, y, v) { W.board[x + BOARD_W * y] = v; }
function boardClear() { W.board.fill(EMPTY); }

function randInt(n) { return Math.floor(Math.random() * n); }

function populateBoxes(density) {
  const nBoxes = Math.floor(BOARD_W * BOARD_H * density);
  for (let i = 0; i < nBoxes; i++) {
    while (true) {
      const x = randInt(BOARD_W), y = randInt(BOARD_H);
      if (boardGet(x, y) === EMPTY) { boardSet(x, y, BOX); break; }
    }
  }
}
function randomSpace() {
  while (true) {
    const x = randInt(BOARD_W), y = randInt(BOARD_H);
    if (boardGet(x, y) === EMPTY) return { x, y };
  }
}

function isBeast(x, y, exclude) {
  for (const b of W.beasts) {
    if (b === exclude) continue;
    if (b.x === x && b.y === y) return true;
  }
  return false;
}

function outOfBounds(x, y) {
  return x < 0 || y < 0 || x > BOARD_W - 1 || y > BOARD_H - 1;
}
function illegalPosition(x, y) {
  return outOfBounds(x, y) || boardGet(x, y) === BOX;
}

// ---- Board / life setup --------------------------------------------------
function newBoard() {
  W.won = false; W.lost = false; W.isNewHighscore = false;
  W.seconds = 0;

  boardClear();
  populateBoxes(W.iBoxDensity / 100);

  const p = randomSpace();
  W.player.x = p.x; W.player.y = p.y;

  W.beasts = [];
  for (let i = 0; i < W.iBeasts; i++) {
    let s;
    do { s = randomSpace(); }
    while (isBeast(s.x, s.y) || (W.player.x === s.x && W.player.y === s.y));
    W.beasts.push({
      x: s.x, y: s.y, type: 1 /* REGULAR */,
      xPre: 0, yPre: 0, cxTrace: 0, cyTrace: 0, iTrace: 0,
    });
  }

  if (beastsContained()) doWin();
}

function newLevel() {
  W.iBeasts     = 3 + (W.level - 1);
  W.iBeastDelay = 60 - 2 * (W.level - 1);
  W.iBoxDensity = 35 - Math.trunc(0.5 * (W.level - 1));
}

function newGame() {
  W.state = GAME;
  W.level = 1;
  W.totalSeconds = 0;
  newLevel();
  newLife();
}

function newLife() {
  W.faceIndex = (randInt(50) === 0) ? FACE_COOL : FACE_NORMAL;
  newBoard();
}

function newArcade(beastDelay, beasts, density) {
  W.state = ARCADE;
  W.totalSeconds = 0;
  W.iBeastDelay = beastDelay;
  W.iBeasts = beasts;
  W.iBoxDensity = density;
  W.faceIndex = (randInt(20) === 0) ? FACE_COOL : FACE_NORMAL;
  newBoard();
}

// ---- Win / lose ----------------------------------------------------------
function beastsContained() {
  for (const b of W.beasts) {
    for (let cx = -1; cx <= 1; cx++) {
      for (let cy = -1; cy <= 1; cy++) {
        if (cx === 0 && cy === 0) continue;
        const tile = boardGet(b.x + cx, b.y + cy);
        if (tile !== BOX && tile !== ERROR && !isBeast(b.x + cx, b.y + cy))
          return false;
      }
    }
  }
  return true;
}

function evaluateHighscore() {
  let better = false;
  if (W.level > W.highscore.level) better = true;
  else if (W.level === W.highscore.level && W.totalSeconds < W.highscore.seconds)
    better = true;
  if (better) {
    W.highscore.level = W.level;
    W.highscore.seconds = W.totalSeconds;
    saveHighscore();
    return true;
  }
  return false;
}

function doWin() {
  W.totalSeconds += W.seconds;

  if (W.faceIndex === FACE_NORMAL) {
    if (W.seconds < 60) W.winFaceIndex = WIN_LAUGH;
    else if (W.seconds < 120) W.winFaceIndex = WIN_CHUCKLE;
    else if (W.seconds < 240) W.winFaceIndex = WIN_BIGSMILE;
    else if (W.seconds > 480) W.winFaceIndex = WIN_RELIEVED;
    else W.winFaceIndex = WIN_SMILE;
  } else if (W.faceIndex === FACE_COOL) {
    W.winFaceIndex = (W.seconds < 60 + W.level * 20) ? WIN_SMOKER : WIN_COOL;
  }

  W.won = true;
  W.level++;                                 // matches original ordering
  if (W.state === GAME) W.isNewHighscore = evaluateHighscore();
}

function doLoose() {
  W.totalSeconds += W.seconds;
  W.lost = true;
  if (W.state === GAME) W.isNewHighscore = evaluateHighscore();
}

// ---- Player movement -----------------------------------------------------
// Returns true if the player attempted a move (mirrors CWorld::KeyDown).
function worldKeyDown(dx, dy) {
  if (W.won || W.lost) return false;

  const xOld = W.player.x, yOld = W.player.y;
  W.player.x += dx; W.player.y += dy;
  let bIllegal = false;

  if (boardGet(W.player.x, W.player.y) === BOX) {
    // Push a line of boxes: find the first empty space beyond them.
    const cx = dx, cy = dy;
    let x = W.player.x, y = W.player.y;
    while (boardGet(x, y) === BOX) {
      x += cx; y += cy;
      if (x < 0 || y < 0 || x >= BOARD_W || y >= BOARD_H) { bIllegal = true; break; }
    }
    if (!bIllegal && !isBeast(x, y)) {
      boardSet(x, y, BOX);
      boardSet(W.player.x, W.player.y, EMPTY);
    } else {
      bIllegal = true;
    }
  }

  if (illegalPosition(W.player.x, W.player.y) || bIllegal) {
    W.player.x = xOld; W.player.y = yOld;
  }

  if (beastsContained()) doWin();
  else if (isBeast(W.player.x, W.player.y)) doLoose();

  return true;
}

// ---- Beast AI (faithful port of CBeast::RunAI) ---------------------------
function beastAI(b) {
  const xP = W.player.x, yP = W.player.y;
  let cx = 0, cy = 0;

  switch (b.type) {
    case 1: // REGULAR
      if (b.x < xP) cx = 1; else if (b.x > xP) cx = -1;
      if (b.y < yP) cy = 1; else if (b.y > yP) cy = -1;
      if (boardGet(b.x + cx, b.y + cy) === BOX) {
        cx = randInt(3) - 1;
        cy = randInt(3) - 1;
      }
      break;
    case 2: // RANDOM
      for (let i = 0; i < 5; i++) {
        cx = randInt(3) - 1;
        cy = randInt(3) - 1;
        if (boardGet(b.x + cx, b.y + cy) !== BOX) break;
      }
      break;
    case 3: // TRACER
      if (b.x < xP) cx = 1; else if (b.x > xP) cx = -1;
      if (b.y < yP) cy = 1; else if (b.y > yP) cy = -1;
      if (b.iTrace === 0) {
        if (boardGet(b.x + cx, b.y + cy) === BOX) {
          b.yPre = b.y; b.xPre = b.x; b.iTrace = 20;
          if (cx !== 0) {
            if (yP > b.y) b.cyTrace = 1;
            else if (yP < b.y) b.cyTrace = -1;
            else b.cyTrace = (b.y < BOARD_H / 2) ? 1 : -1;
            b.cxTrace = 0;
          } else {
            if (xP > b.x) b.cxTrace = 1;
            else if (xP < b.x) b.cxTrace = -1;
            else b.cxTrace = (b.x < BOARD_W / 2) ? 1 : -1;
            b.cyTrace = 0;
          }
        }
      }
      if (b.iTrace > 0) {
        if (b.cyTrace !== 0) {
          cy = b.cyTrace;
          while (boardGet(b.x + cx, b.y + cy) === BOX) {
            if (b.x < xP) cx -= 1; else if (b.x > xP) cx += 1;
            else { b.iTrace = 0; break; }
            if (cx > 1) { cx = 1; break; }
            if (cx < -1) { cx = -1; break; }
          }
          if (xP - (b.x + cx) < xP - b.xPre) b.iTrace = 0;
        }
        if (b.cxTrace !== 0) {
          cx = b.cxTrace;
          while (boardGet(b.x + cx, b.y + cy) === BOX) {
            if (b.y < yP) cy -= 1; else if (b.y > yP) cy += 1;
            else { b.iTrace = 0; break; }
            if (cy > 1) { cy = 1; break; }
            if (cy < -1) { cy = -1; break; }
          }
          if (yP - (b.y + cy) < yP - b.yPre) b.iTrace = 0;
        }
      }
      break;
  }

  b.x += cx; b.y += cy;
}

function runBeastAI() {
  for (const b of W.beasts) {
    const xOld = b.x, yOld = b.y;
    beastAI(b);
    if (illegalPosition(b.x, b.y) || isBeast(b.x, b.y, b)) {
      b.x = xOld; b.y = yOld;
    }
  }
  if (isBeast(W.player.x, W.player.y)) doLoose();
}

// ---- Rendering -----------------------------------------------------------
const canvas = document.getElementById('screen');
const ctx = canvas.getContext('2d');
ctx.imageSmoothingEnabled = false;
ctx.textBaseline = 'middle';
ctx.font = '11px "Tahoma","MS Sans Serif","Segoe UI",sans-serif';

function drawSprite(name, tx, ty) {
  const img = sprites[name];
  if (img && img.width) ctx.drawImage(img, tx * TILE, PLAY_Y + ty * TILE, TILE, TILE);
}

function render() {
  // white wash
  ctx.fillStyle = '#fff';
  ctx.fillRect(0, 0, canvas.width, canvas.height);

  // outline / border of boxes around the play field
  for (let tx = 0; tx < COLS; tx++) {
    drawSprite('box', tx, 0);
    drawSprite('box', tx, ROWS - 1);
  }
  for (let ty = 0; ty < ROWS; ty++) {
    drawSprite('box', 0, ty);
    drawSprite('box', COLS - 1, ty);
  }

  if (W.state === INTRO) {
    ctx.fillStyle = '#000';
    ctx.textAlign = 'center';
    const cx = canvas.width / 2;
    ctx.fillText('Boxes', cx, PLAY_Y + 400 / 2 - 8);
    ctx.fillText('©2002 Michael Schmitz', cx, PLAY_Y + 400 / 2 + 8);
  } else {
    // boxes
    for (let x = 0; x < BOARD_W; x++)
      for (let y = 0; y < BOARD_H; y++)
        if (boardGet(x, y) === BOX) drawSprite('box', x + 1, y + 1);

    // beasts
    for (const b of W.beasts) drawSprite('beast', b.x + 1, b.y + 1);

    // player (hidden when lost - the beast sits on top)
    if (!W.lost) {
      let name;
      if (W.won) name = WIN_SPRITE[W.winFaceIndex];
      else name = FACE_SPRITE[W.faceIndex];
      drawSprite(name, W.player.x + 1, W.player.y + 1);
    }
  }

  renderBars();
}

function fmtStats(withLevel) {
  let s = ' Beasts: ' + W.iBeasts +
          '        Beast Delay: ' + W.iBeastDelay +
          '        Box Density: ' + W.iBoxDensity + '/100';
  if (withLevel) s += '        Level: ' + W.level;
  s += '        Time: ' + W.seconds + 's';
  return s;
}

function renderBars() {
  // clear bars
  ctx.fillStyle = '#fff';
  ctx.fillRect(0, 0, canvas.width, BAR_H);
  ctx.fillRect(0, canvas.height - BAR_H, canvas.width, BAR_H);
  ctx.fillStyle = '#000';

  const midTop = BAR_H / 2;
  const midBot = canvas.height - BAR_H / 2;
  const cx = canvas.width / 2;

  if (W.state === GAME) {
    // top: highscore
    ctx.textAlign = 'center';
    ctx.fillText('Highscore:  Level ' + W.highscore.level +
                 ' in ' + W.highscore.seconds + ' seconds', cx, midTop);

    // bottom
    if (W.won) {
      ctx.textAlign = 'center';
      ctx.fillText(W.isNewHighscore
        ? 'New Highscore!  Press ENTER to continue.'
        : 'Level ' + W.level + ' reached in ' + W.seconds +
          ' seconds.  Press ENTER to continue.', cx, midBot);
    } else if (W.lost) {
      ctx.textAlign = 'center';
      ctx.fillText(W.isNewHighscore
        ? 'New Highscore!  Press ENTER to retry level.'
        : 'Died on level ' + W.level + ' in ' + W.seconds +
          ' seconds.  Press ENTER to retry level.', cx, midBot);
    } else {
      ctx.textAlign = 'left';
      ctx.fillText(fmtStats(true), 2, midBot);
    }
  } else if (W.state === ARCADE) {
    ctx.textAlign = 'left';
    ctx.fillText(fmtStats(false).replace(/ {8}/g, '         '), 2, midBot);
  }
}

// ---- Timers --------------------------------------------------------------
let beastTimer = null, secondTimer = null, winkTimer = null, unwinkTimer = null;

function beastInterval() {
  return Math.max(16, Math.floor(W.iBeastDelay / 60 * 1000));
}

function stopTimers() {
  clearInterval(beastTimer); beastTimer = null;
  clearInterval(secondTimer); secondTimer = null;
}

function startBeastTimer() {
  clearInterval(beastTimer);
  beastTimer = setInterval(() => {
    if (!W.paused && !W.lost && !W.won) { runBeastAI(); render(); }
    else { clearInterval(beastTimer); beastTimer = null; }
  }, beastInterval());
}

function startSecondTimer() {
  clearInterval(secondTimer);
  secondTimer = setInterval(() => {
    if (!W.paused && !W.lost && !W.won) { W.seconds++; renderBars(); }
    else { clearInterval(secondTimer); secondTimer = null; }
  }, 1000);
}

function resetWinkTimer() {
  clearTimeout(winkTimer);
  winkTimer = setTimeout(() => {
    if (!W.paused && !W.lost && !W.won && W.state !== INTRO &&
        W.faceIndex === FACE_NORMAL) {
      W.faceIndex = FACE_WINK;
      render();
      clearTimeout(unwinkTimer);
      unwinkTimer = setTimeout(() => {
        W.faceIndex = FACE_NORMAL;
        render();
      }, WINK_DURATION);
    }
  }, WINK_INTERVAL);
}

// ---- Controller (mirrors CProcedure) -------------------------------------
function startNewGame() {
  newGame();
  W.paused = true;
  stopTimers();
  render();
  resetWinkTimer();
  canvas.focus();
}

function startNewArcade(delay, beasts, density) {
  newArcade(delay, beasts, density);
  W.paused = true;
  stopTimers();
  render();
  resetWinkTimer();
  canvas.focus();
}

function handleArrow(dx, dy) {
  if (worldKeyDown(dx, dy)) {
    if (W.paused) {
      W.seconds++;
      runBeastAI();
      W.paused = false;
      if (!W.won && !W.lost) { startBeastTimer(); startSecondTimer(); }
    }
    resetWinkTimer();
    render();
    return true;
  } else if ((W.won || W.lost) && W.state === GAME) {
    return false; // ENTER handled separately
  }
  return false;
}

function handleEnter() {
  if ((W.won || W.lost) && W.state === GAME) {
    if (W.won) newLevel();
    if (W.lost) newLife();
    else newBoard();

    W.paused = true;
    stopTimers();
    render();
    resetWinkTimer();
  }
}

function pauseGame() {
  if (W.state === INTRO) return;
  W.paused = true;
  stopTimers();
  render();
}

// ---- Input ---------------------------------------------------------------
const ARROWS = {
  ArrowLeft: [-1, 0], ArrowRight: [1, 0], ArrowUp: [0, -1], ArrowDown: [0, 1],
};

document.addEventListener('keydown', (e) => {
  // menu accelerators
  if (e.ctrlKey) {
    const k = e.key.toLowerCase();
    if (k === 'n') { e.preventDefault(); startNewGame(); return; }
    if (k === 'a') { e.preventDefault(); openSettings(); return; }
    if (k === 'p') { e.preventDefault(); pauseGame(); return; }
    if (k === 'q') { e.preventDefault(); exitToIntro(); return; }
    return;
  }
  if (anyDialogOpen()) return;

  if (e.key in ARROWS) {
    e.preventDefault();
    const [dx, dy] = ARROWS[e.key];
    handleArrow(dx, dy);
  } else if (e.key === 'Enter') {
    e.preventDefault();
    handleEnter();
  } else if (e.key === '?') {
    openDialog('dlg-instructions');
  } else if (e.key === '!') {
    window.open('https://www.schmitztech.com/', '_blank');
  }
});

// touch / click-to-move (swipe) for mobile
let touchStart = null;
canvas.addEventListener('touchstart', (e) => {
  const t = e.changedTouches[0];
  touchStart = { x: t.clientX, y: t.clientY };
}, { passive: true });
canvas.addEventListener('touchend', (e) => {
  if (!touchStart) return;
  const t = e.changedTouches[0];
  const dx = t.clientX - touchStart.x, dy = t.clientY - touchStart.y;
  touchStart = null;
  if (Math.abs(dx) < 12 && Math.abs(dy) < 12) {
    if (W.won || W.lost) handleEnter();
    return;
  }
  if (Math.abs(dx) > Math.abs(dy)) handleArrow(dx > 0 ? 1 : -1, 0);
  else handleArrow(0, dy > 0 ? 1 : -1);
}, { passive: true });

// ---- Menus & dialogs -----------------------------------------------------
function closeMenus() {
  document.querySelectorAll('.menu.open').forEach(m => m.classList.remove('open'));
}
document.querySelectorAll('.menu-title').forEach(t => {
  t.addEventListener('click', (e) => {
    e.stopPropagation();
    const menu = t.parentElement;
    const wasOpen = menu.classList.contains('open');
    closeMenus();
    if (!wasOpen) menu.classList.add('open');
  });
});
document.addEventListener('click', closeMenus);

document.querySelectorAll('.menu-item').forEach(item => {
  item.addEventListener('click', () => {
    closeMenus();
    switch (item.dataset.cmd) {
      case 'newgame': startNewGame(); break;
      case 'newarcade': openSettings(); break;
      case 'pause': pauseGame(); break;
      case 'exit': exitToIntro(); break;
      case 'instructions': openDialog('dlg-instructions'); break;
      case 'schmitztech': window.open('https://www.schmitztech.com/', '_blank'); break;
      case 'about': openDialog('dlg-about'); break;
    }
  });
});

function openDialog(id) { document.getElementById(id).classList.add('open'); }
function closeDialog(el) { el.classList.remove('open'); }
function anyDialogOpen() {
  return !!document.querySelector('.dialog-overlay.open');
}
document.querySelectorAll('.dlg-ok').forEach(btn => {
  btn.addEventListener('click', () => closeDialog(btn.closest('.dialog-overlay')));
});
document.querySelectorAll('.dialog-overlay').forEach(ov => {
  ov.addEventListener('click', (e) => { if (e.target === ov) closeDialog(ov); });
});

function openSettings() { openDialog('dlg-settings'); }
document.querySelector('.settings-cancel').addEventListener('click', () => {
  closeDialog(document.getElementById('dlg-settings'));
});
document.querySelector('.settings-ok').addEventListener('click', () => {
  const delay = parseInt(document.getElementById('set-delay').value, 10) || 0;
  const beasts = parseInt(document.getElementById('set-beasts').value, 10) || 0;
  const density = parseInt(document.getElementById('set-density').value, 10) || 0;
  closeDialog(document.getElementById('dlg-settings'));
  startNewArcade(delay, beasts, density);
});

function exitToIntro() {
  stopTimers();
  clearTimeout(winkTimer); clearTimeout(unwinkTimer);
  W.state = INTRO;
  W.paused = false; W.won = false; W.lost = false;
  render();
}

// ---- Boot ----------------------------------------------------------------
loadHighscore();
loadSprites().then(() => {
  render();
  canvas.focus();
});
