/* Boxes service worker — makes the game fully playable offline.
 *
 * The game is a small static app shell (index.html + game.js + style.css plus
 * the PNG sprites), so precaching the shell is enough to run with no network.
 * Strategy: precache the shell on install; serve same-origin GETs
 * stale-while-revalidate (instant offline, refreshed in the background when
 * online); fall back to the app shell for offline navigations. Cross-origin
 * requests are left to the network.
 *
 * Bump CACHE when the shell changes so old caches are cleaned up on activate.
 */
const CACHE = 'boxes-v2';
const SHELL = [
  './',
  './index.html',
  './game.js',
  './style.css',
  './manifest.webmanifest',
  './icon-192.png',
  './icon-512.png',
  './sprites/box.png',
  './sprites/beast.png',
  './sprites/normal.png',
  './sprites/glass.png',
  './sprites/wink.png',
  './sprites/winsmile.png',
  './sprites/winlaugh.png',
  './sprites/winchuckle.png',
  './sprites/winbigsmile.png',
  './sprites/winrelieved.png',
  './sprites/wincool.png',
  './sprites/winsmoke.png',
];

self.addEventListener('install', (e) => {
  e.waitUntil(
    caches.open(CACHE).then((c) => c.addAll(SHELL)).then(() => self.skipWaiting())
  );
});

self.addEventListener('activate', (e) => {
  e.waitUntil(
    caches.keys()
      .then((keys) => Promise.all(keys.filter((k) => k !== CACHE).map((k) => caches.delete(k))))
      .then(() => self.clients.claim())
  );
});

self.addEventListener('fetch', (e) => {
  const req = e.request;
  if (req.method !== 'GET') return;
  const url = new URL(req.url);
  if (url.origin !== self.location.origin) return;   // let cross-origin hit the network

  e.respondWith((async () => {
    const cache = await caches.open(CACHE);
    const cached = await cache.match(req, { ignoreSearch: true });
    const network = fetch(req)
      .then((res) => { if (res && res.ok) cache.put(req, res.clone()); return res; })
      .catch(() => null);
    // stale-while-revalidate; fall back to the app shell for offline navigations
    return cached || (await network) || cache.match('./index.html');
  })());
});
