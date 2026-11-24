// Shared by the three PWA packagers. This file runs inside the service worker.
function releaseIntegrity(scope, prefix, name, ownsCache = async () => true) {
  const url = relative => new URL(relative, scope).href;
  const routeKey = url('.client-editions');
  const manifests = url('.release-files');
  let routes;

  async function verified(response, file) {
    if (!response?.ok) return null;
    const data = await response.clone().arrayBuffer();
    if (data.byteLength !== file.bytes) return null;
    const digest = await crypto.subtle.digest('SHA-256', data);
    const hex = Array.from(new Uint8Array(digest), byte => byte.toString(16).padStart(2, '0')).join('');
    return hex === file.sha256 ? response : null;
  }

  async function requireFile(cache, file, alternatives = []) {
    const key = url(file.path);
    const cached = await verified(await cache.match(key), file);
    if (cached) return cached;
    for (const alternative of alternatives) {
      const reuse = await verified(await alternative.match(key), file);
      if (reuse) {
        await cache.put(key, reuse.clone());
        return reuse;
      }
    }
    const fresh = await verified(await fetch(key, {cache: 'reload'}), file);
    if (!fresh) throw Error('Release file mismatch: ' + file.path);
    await cache.put(key, fresh.clone());
    return fresh;
  }

  async function stage(cache, older) {
    let previous = older.at(-1);
    for (const key of older) {
      if (await (await caches.open(key)).match(routeKey)) previous = key;
    }
    const inherited = previous ? await readRoutes(await caches.open(previous)) : {};
    const clients = await self.clients.matchAll({type: 'window', includeUncontrolled: true});
    const assigned = Object.fromEntries(clients.filter(client => previous && client.url.startsWith(scope))
      .map(client => [client.id, inherited[client.id] ?? previous]));
    await cache.put(routeKey, new Response(JSON.stringify(assigned)));
    routes = Promise.resolve(assigned);
    const keep = new Set([previous, ...Object.values(assigned)]);
    for (const key of older) if (!keep.has(key)) await caches.delete(key);
  }

  async function readRoutes(cache) {
    return (await (await cache.match(routeKey))?.json()) ?? {};
  }

  async function edition(id) {
    if (!id) return name;
    routes ??= readRoutes(await caches.open(name));
    return (await routes)[id] ?? name;
  }

  async function recordFiles(cache, files) {
    await cache.put(manifests, new Response(JSON.stringify(files)));
  }

  async function fileList(cache) {
    return (await (await cache.match(manifests))?.json()) ?? null;
  }

  async function olderCaches() {
    const names = (await caches.keys()).filter(key => key.startsWith(prefix) && key !== name);
    const owned = await Promise.all(names.map(key => ownsCache(key)));
    return names.filter((key, index) => owned[index]);
  }

  async function activate() {
    if (!(await olderCaches()).length) await self.clients.claim();
  }

  return {verified, requireFile, stage, edition, recordFiles, fileList, olderCaches, activate};
}
