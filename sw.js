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

const VERSION='93e8b92a9d94ea74',CACHE='machdefense-assets-'+VERSION;
const MANIFEST={"version": "93e8b92a9d94ea74", "cheat": false, "files": [{"url": "app.js", "size": 11803, "sha256": "bffc70e08b6604ef7b55e1778d168e77e51ef13f9979cc0de85fad474eedd888"}, {"url": "audio.js", "size": 1159, "sha256": "76cc213fb5a87b9d3646a0ea4fcbeb4d795049087de73a13d3b0e64b741e93ab"}, {"url": "game.data", "size": 73231574, "sha256": "a21fad80cb57f69fad87d4fc7d51efe41b4250e96472371eb7746fe0109c642a", "parts": [{"url": "chunks/627c77924fecbdf9b7d7955db34c757f3153775fcbce38dd6e0d834ff4b54faf.bin", "size": 4194304, "sha256": "627c77924fecbdf9b7d7955db34c757f3153775fcbce38dd6e0d834ff4b54faf"}, {"url": "chunks/5261975aa88a99dc86934b5f82e73eea3ccb51a3816e514612046930bf92a154.bin", "size": 580438, "sha256": "5261975aa88a99dc86934b5f82e73eea3ccb51a3816e514612046930bf92a154"}, {"url": "chunks/9e63e349bc4e68373519a743257b505b13aa7c0f06e1ce29f5998d48d6ed8812.bin", "size": 4194304, "sha256": "9e63e349bc4e68373519a743257b505b13aa7c0f06e1ce29f5998d48d6ed8812"}, {"url": "chunks/d7ec72ba18ae743cf3cb41a06fee924353eb6c2c65b15bb6234f6b1e2e9ffde3.bin", "size": 2806714, "sha256": "d7ec72ba18ae743cf3cb41a06fee924353eb6c2c65b15bb6234f6b1e2e9ffde3"}, {"url": "chunks/941cfbc69c2e7a9f692549f7692f774e3aa79f9cacce41afceb9c15c2d225ece.bin", "size": 2359398, "sha256": "941cfbc69c2e7a9f692549f7692f774e3aa79f9cacce41afceb9c15c2d225ece"}, {"url": "chunks/a3ef5fc2a4de19c505e1d025a2e071f84c1a128aecf3cf935e29b7a6ea742e15.bin", "size": 957430, "sha256": "a3ef5fc2a4de19c505e1d025a2e071f84c1a128aecf3cf935e29b7a6ea742e15"}, {"url": "chunks/1f839806939291fca20e323f1afdaab7365d9abdccf2eec46ebc2c2d312e6f8e.bin", "size": 4194304, "sha256": "1f839806939291fca20e323f1afdaab7365d9abdccf2eec46ebc2c2d312e6f8e"}, {"url": "chunks/275afd1fce3acd8410fd5cacc029426c1a6baeb101c06f96bbcb9c303a54efcb.bin", "size": 1074946, "sha256": "275afd1fce3acd8410fd5cacc029426c1a6baeb101c06f96bbcb9c303a54efcb"}, {"url": "chunks/b827946869b0b3970accac84f40c96a91fee92908f6288fef1235c5dcbdafd37.bin", "size": 4194304, "sha256": "b827946869b0b3970accac84f40c96a91fee92908f6288fef1235c5dcbdafd37"}, {"url": "chunks/722e124c1e4ea7b1c4b0fad854373e9557707a8cb7b3001a600cb5cd3165eee6.bin", "size": 1372327, "sha256": "722e124c1e4ea7b1c4b0fad854373e9557707a8cb7b3001a600cb5cd3165eee6"}, {"url": "chunks/99c99da9db6f6f5f56180068b1d657f4d9603ba7eac1b336d5d6d6bc28065ca6.bin", "size": 35227, "sha256": "99c99da9db6f6f5f56180068b1d657f4d9603ba7eac1b336d5d6d6bc28065ca6"}, {"url": "chunks/f8f83015169205955976914b96ec94a61f00d6193c50b52bfb9e8c66f81cfb6e.bin", "size": 193859, "sha256": "f8f83015169205955976914b96ec94a61f00d6193c50b52bfb9e8c66f81cfb6e"}, {"url": "chunks/de23ff976315128cda3587a615a3608690e60d78779dfb0b047b1400c25e08d5.bin", "size": 65293, "sha256": "de23ff976315128cda3587a615a3608690e60d78779dfb0b047b1400c25e08d5"}, {"url": "chunks/4273c487b8cce9aaa491bbc6794b4f0d22bf952fae286a73807ea7492f5b57fc.bin", "size": 12411, "sha256": "4273c487b8cce9aaa491bbc6794b4f0d22bf952fae286a73807ea7492f5b57fc"}, {"url": "chunks/330213d489c4865e0b2da39f0e67303a323ca60edda8a0ce894a3b9fea60ca35.bin", "size": 131734, "sha256": "330213d489c4865e0b2da39f0e67303a323ca60edda8a0ce894a3b9fea60ca35"}, {"url": "chunks/296285cbb174507fb78d9cf58a5eca9d2bebfeb98f31a7b4377cefac3714b73f.bin", "size": 11971, "sha256": "296285cbb174507fb78d9cf58a5eca9d2bebfeb98f31a7b4377cefac3714b73f"}, {"url": "chunks/8c0bab55dbf5f17f71102af8b0b009ab18861895fee6e4437eb1cf8f667e2693.bin", "size": 161648, "sha256": "8c0bab55dbf5f17f71102af8b0b009ab18861895fee6e4437eb1cf8f667e2693"}, {"url": "chunks/86dd69da92d3fa2ac94d01475d6db5fb0bf47a22dc63dc82da7e63bc08dbec50.bin", "size": 58402, "sha256": "86dd69da92d3fa2ac94d01475d6db5fb0bf47a22dc63dc82da7e63bc08dbec50"}, {"url": "chunks/d14309e3334c339be196006c5358012329e9863a57bc74a9d10aa3f4131e5a82.bin", "size": 87393, "sha256": "d14309e3334c339be196006c5358012329e9863a57bc74a9d10aa3f4131e5a82"}, {"url": "chunks/c21beaae737fb83cf8aa14ffc0fa735689c93d76e01140ceaa0662fe66cdfbf0.bin", "size": 241684, "sha256": "c21beaae737fb83cf8aa14ffc0fa735689c93d76e01140ceaa0662fe66cdfbf0"}, {"url": "chunks/306c9dc40f1d9cc64760f5378605064f84177d12bb73f9a9a96bbaecae556a18.bin", "size": 2224817, "sha256": "306c9dc40f1d9cc64760f5378605064f84177d12bb73f9a9a96bbaecae556a18"}, {"url": "chunks/0e116bcc380f6e88234374b721861fbea3c77d1203d0f53738a22ae4dcf86647.bin", "size": 4139608, "sha256": "0e116bcc380f6e88234374b721861fbea3c77d1203d0f53738a22ae4dcf86647"}, {"url": "chunks/81f523a541b21393f0065e407b1c367bedc42c3392ba9a2362ad1d1cd2293740.bin", "size": 3888269, "sha256": "81f523a541b21393f0065e407b1c367bedc42c3392ba9a2362ad1d1cd2293740"}, {"url": "chunks/2cb846fbd7c36f88dde0de35a965d606409e44a0a28174f93a4f9096bc5c15ce.bin", "size": 4121209, "sha256": "2cb846fbd7c36f88dde0de35a965d606409e44a0a28174f93a4f9096bc5c15ce"}, {"url": "chunks/0aabf80c59ebf56c1147f8ecbf2b021030588f2c4626009194f8dde2ca601f70.bin", "size": 296283, "sha256": "0aabf80c59ebf56c1147f8ecbf2b021030588f2c4626009194f8dde2ca601f70"}, {"url": "chunks/805f62bc08866a1782f850ca7a466c18df3f783bb41f862fedb20abcec4741ca.bin", "size": 456066, "sha256": "805f62bc08866a1782f850ca7a466c18df3f783bb41f862fedb20abcec4741ca"}, {"url": "chunks/4ba55b7b07020a96ec13655cce24ca6f9c4f5d19cc9a63760c1c5e45b2f3afa9.bin", "size": 111833, "sha256": "4ba55b7b07020a96ec13655cce24ca6f9c4f5d19cc9a63760c1c5e45b2f3afa9"}, {"url": "chunks/c859fb0dff75c508267339c5ed8b58b426211b7374d38063b7bfea28468f1d7a.bin", "size": 270575, "sha256": "c859fb0dff75c508267339c5ed8b58b426211b7374d38063b7bfea28468f1d7a"}, {"url": "chunks/0ae8e0c8c036cb13e53d576fd64e87caaa48fc4149e7fdc11b4860f166764d15.bin", "size": 229709, "sha256": "0ae8e0c8c036cb13e53d576fd64e87caaa48fc4149e7fdc11b4860f166764d15"}, {"url": "chunks/1c61d77b982f17a059353ec0ee50a8aa0fb25c3510a3e85a53cf987cb661ad82.bin", "size": 98685, "sha256": "1c61d77b982f17a059353ec0ee50a8aa0fb25c3510a3e85a53cf987cb661ad82"}, {"url": "chunks/9ec8e868ad0e4c8d37b247bda55d5b397599016a0e321e02ac0870f51eab1131.bin", "size": 129827, "sha256": "9ec8e868ad0e4c8d37b247bda55d5b397599016a0e321e02ac0870f51eab1131"}, {"url": "chunks/3e080d78ed70d2f2066be3cc653a828081293cc3561ef013fecbf5022c7907d3.bin", "size": 512461, "sha256": "3e080d78ed70d2f2066be3cc653a828081293cc3561ef013fecbf5022c7907d3"}, {"url": "chunks/111a6b0445193f808caea840fc6d57311b98f831109f9800baa8018e796feeca.bin", "size": 147625, "sha256": "111a6b0445193f808caea840fc6d57311b98f831109f9800baa8018e796feeca"}, {"url": "chunks/5970da79d531432561e9ea61fb3c3e926cb5fe3abfde6066cd333933f7a85955.bin", "size": 212565, "sha256": "5970da79d531432561e9ea61fb3c3e926cb5fe3abfde6066cd333933f7a85955"}, {"url": "chunks/1bf8f83336b75c8f42f3f704dea9a13910975c1d2b74a3dc61e3e9e3e83c2670.bin", "size": 127286, "sha256": "1bf8f83336b75c8f42f3f704dea9a13910975c1d2b74a3dc61e3e9e3e83c2670"}, {"url": "chunks/1906f1b0874dae9640bd64a98aee6c2def8d293063cb356ad3e48b9ff0c23f26.bin", "size": 54787, "sha256": "1906f1b0874dae9640bd64a98aee6c2def8d293063cb356ad3e48b9ff0c23f26"}, {"url": "chunks/5d08d5f4c5df1f40de4191479ea39e87dcc5c254a91b6042c8f8b991f767b34b.bin", "size": 187180, "sha256": "5d08d5f4c5df1f40de4191479ea39e87dcc5c254a91b6042c8f8b991f767b34b"}, {"url": "chunks/f7c9fc499a464cb397015e7073f631a11e08e620aec82e136b1487f96aada44f.bin", "size": 155775, "sha256": "f7c9fc499a464cb397015e7073f631a11e08e620aec82e136b1487f96aada44f"}, {"url": "chunks/f48a80cffbb3c4e621d6f61fc617ee633ce17567b3e0b174d7db8b0903f03ce3.bin", "size": 53716, "sha256": "f48a80cffbb3c4e621d6f61fc617ee633ce17567b3e0b174d7db8b0903f03ce3"}, {"url": "chunks/fe3bb06b86026b66f9cfc0d93b7538b443da83c0ca24300ec32010e674bb4820.bin", "size": 1565527, "sha256": "fe3bb06b86026b66f9cfc0d93b7538b443da83c0ca24300ec32010e674bb4820"}, {"url": "chunks/514a2d506d66731a9a30921c876e375a8d6ea331fe1ed4d47248fc3f2516f643.bin", "size": 372163, "sha256": "514a2d506d66731a9a30921c876e375a8d6ea331fe1ed4d47248fc3f2516f643"}, {"url": "chunks/d3a790a49c72ca034154185f34be8412fa3d9d4d0391e55d12fe3b28bd981ca2.bin", "size": 1023752, "sha256": "d3a790a49c72ca034154185f34be8412fa3d9d4d0391e55d12fe3b28bd981ca2"}, {"url": "chunks/1d2d64694c6e95d600232ac199f819c891772e78a6f9f16dba1b97bbb0ad029b.bin", "size": 130598, "sha256": "1d2d64694c6e95d600232ac199f819c891772e78a6f9f16dba1b97bbb0ad029b"}, {"url": "chunks/3b5b913a228e12eac8546aaffbb42bc4b2f70d9530e472fca32f7a9abae982bf.bin", "size": 155853, "sha256": "3b5b913a228e12eac8546aaffbb42bc4b2f70d9530e472fca32f7a9abae982bf"}, {"url": "chunks/394d5892ab3b26ad993b4f6eef16bf22af98c9b00a080923f33763d412ca34a0.bin", "size": 759771, "sha256": "394d5892ab3b26ad993b4f6eef16bf22af98c9b00a080923f33763d412ca34a0"}, {"url": "chunks/a00a4328f91e7dcafacde911a3f526876539d7055f3d9e58afaf4c63edcdc12a.bin", "size": 345815, "sha256": "a00a4328f91e7dcafacde911a3f526876539d7055f3d9e58afaf4c63edcdc12a"}, {"url": "chunks/0b371da5e21f0f1a991258524c14b42d016c9303360d18f85a99721a63c1084d.bin", "size": 76607, "sha256": "0b371da5e21f0f1a991258524c14b42d016c9303360d18f85a99721a63c1084d"}, {"url": "chunks/98d6656bca0c5e64bd6ca6ba75d24fcfdddfc3bf43ae0dbd0dd78200320ee6a1.bin", "size": 93968, "sha256": "98d6656bca0c5e64bd6ca6ba75d24fcfdddfc3bf43ae0dbd0dd78200320ee6a1"}, {"url": "chunks/87c12fd276af4a49df46baba3b63bbcdd8c885e3082eac4f6ca7efbc87810aae.bin", "size": 76483, "sha256": "87c12fd276af4a49df46baba3b63bbcdd8c885e3082eac4f6ca7efbc87810aae"}, {"url": "chunks/7871ed582f7c202d7311f3e7f7d21476953747fe4441ef14a1836bf6c2595c34.bin", "size": 87021, "sha256": "7871ed582f7c202d7311f3e7f7d21476953747fe4441ef14a1836bf6c2595c34"}, {"url": "chunks/b195cc2ddb59d738f3c448447bdb2892361747ad534f086e020885ab93380161.bin", "size": 102873, "sha256": "b195cc2ddb59d738f3c448447bdb2892361747ad534f086e020885ab93380161"}, {"url": "chunks/ccccd19dd19c8ff044fe62eea642879f119cf595266a37a68d6dcd8d0eb275cf.bin", "size": 82304, "sha256": "ccccd19dd19c8ff044fe62eea642879f119cf595266a37a68d6dcd8d0eb275cf"}, {"url": "chunks/36826db9d60ca2f18ff4f55b9e552441af04f5df2269be29d858d805be710dc7.bin", "size": 133527, "sha256": "36826db9d60ca2f18ff4f55b9e552441af04f5df2269be29d858d805be710dc7"}, {"url": "chunks/f8e2a27eb754d125c2c2633636d383e496397a03e2b3d11180e5da350acc32df.bin", "size": 19540, "sha256": "f8e2a27eb754d125c2c2633636d383e496397a03e2b3d11180e5da350acc32df"}, {"url": "chunks/ca8874e2983855b2ab11f470a8f3295c48d29141e5b1557bd3521c432201af80.bin", "size": 21051, "sha256": "ca8874e2983855b2ab11f470a8f3295c48d29141e5b1557bd3521c432201af80"}, {"url": "chunks/8d72c25e435d4e24693029d8621e40006f1e2b24c84d2068ac48c5182e601f3b.bin", "size": 16607, "sha256": "8d72c25e435d4e24693029d8621e40006f1e2b24c84d2068ac48c5182e601f3b"}, {"url": "chunks/968bd35c9693cc10568e9113ceb7614712925d108da6f9ac1c8d2fafd8319fe8.bin", "size": 16595, "sha256": "968bd35c9693cc10568e9113ceb7614712925d108da6f9ac1c8d2fafd8319fe8"}, {"url": "chunks/f97f0f71311031514079dc5d0acd404941f85d652a7ef963c4cb83c5c01e269d.bin", "size": 16428, "sha256": "f97f0f71311031514079dc5d0acd404941f85d652a7ef963c4cb83c5c01e269d"}, {"url": "chunks/b022523847aec461f53edf633b9a7e5d44c1d104fe667413406b40e3dac5b975.bin", "size": 17505, "sha256": "b022523847aec461f53edf633b9a7e5d44c1d104fe667413406b40e3dac5b975"}, {"url": "chunks/492523a5f25edbf464899431c7a26b82d5729b285b58d94d0d0cbb4a864bcc15.bin", "size": 17350, "sha256": "492523a5f25edbf464899431c7a26b82d5729b285b58d94d0d0cbb4a864bcc15"}, {"url": "chunks/b0b1bf0d4079832766abc09839eb427b03d0b96b7f0649bdbf1fc414973c1e7d.bin", "size": 19616, "sha256": "b0b1bf0d4079832766abc09839eb427b03d0b96b7f0649bdbf1fc414973c1e7d"}, {"url": "chunks/19bfc05d7864200d9c64e781fc09bc8716382e1337fc054f5a25ef9aaeaff56e.bin", "size": 3273555, "sha256": "19bfc05d7864200d9c64e781fc09bc8716382e1337fc054f5a25ef9aaeaff56e"}, {"url": "chunks/86c1fefbb06308554dc2eb057c8284496385cdbadb7c69df033d22660f7eaf8c.bin", "size": 230757, "sha256": "86c1fefbb06308554dc2eb057c8284496385cdbadb7c69df033d22660f7eaf8c"}, {"url": "chunks/c0de470981b13046dc476ba07904f4053d089e7519fa25c6c0bdbe79a4bb0e1f.bin", "size": 89145, "sha256": "c0de470981b13046dc476ba07904f4053d089e7519fa25c6c0bdbe79a4bb0e1f"}, {"url": "chunks/747bf8d7575534eb98f6c7689ef24232c86e90f16765554454be108a47594715.bin", "size": 71953, "sha256": "747bf8d7575534eb98f6c7689ef24232c86e90f16765554454be108a47594715"}, {"url": "chunks/76c0e5d3b00bde947931f475e1120277f823ad0757a39833f27caf3fd9772969.bin", "size": 85422, "sha256": "76c0e5d3b00bde947931f475e1120277f823ad0757a39833f27caf3fd9772969"}, {"url": "chunks/a03cb53b2002fc0ead9ac34eea6cda108be1ef11d7f557215dc565542a111694.bin", "size": 83401, "sha256": "a03cb53b2002fc0ead9ac34eea6cda108be1ef11d7f557215dc565542a111694"}, {"url": "chunks/5e1ae4a68bdf4055c81f7b8caad81764e627384cda46282e8ee539176f3fe81f.bin", "size": 71098, "sha256": "5e1ae4a68bdf4055c81f7b8caad81764e627384cda46282e8ee539176f3fe81f"}, {"url": "chunks/ff9a2ce2e60479d687f2e0520ead5955d81267d6ebdb8d34928a71bce4838a23.bin", "size": 14845, "sha256": "ff9a2ce2e60479d687f2e0520ead5955d81267d6ebdb8d34928a71bce4838a23"}, {"url": "chunks/c09b04c80d5c2e91cf5b0a125b9a91279f829c61a2ac80b47e03b3e3674ae420.bin", "size": 15356, "sha256": "c09b04c80d5c2e91cf5b0a125b9a91279f829c61a2ac80b47e03b3e3674ae420"}, {"url": "chunks/bc2b0f065e47476277deefdc78b141cb75e7987e46118f8895a29858b46a0bc4.bin", "size": 12543, "sha256": "bc2b0f065e47476277deefdc78b141cb75e7987e46118f8895a29858b46a0bc4"}, {"url": "chunks/ac7521f27452f92a0cff192ebcc1090e1b876608913c0e5674523bc60565ee8b.bin", "size": 13903, "sha256": "ac7521f27452f92a0cff192ebcc1090e1b876608913c0e5674523bc60565ee8b"}, {"url": "chunks/8d18be40c6903909eea7fcbfc4b7c35002335053992864d8ba9be3b6ec5787cf.bin", "size": 14520, "sha256": "8d18be40c6903909eea7fcbfc4b7c35002335053992864d8ba9be3b6ec5787cf"}, {"url": "chunks/eaad3f14a9526b7e19b1ef0b7fa06739ccd63a9ae628ecd0dfaf4a81191a10f1.bin", "size": 2523275, "sha256": "eaad3f14a9526b7e19b1ef0b7fa06739ccd63a9ae628ecd0dfaf4a81191a10f1"}, {"url": "chunks/6d20d38e57a436ba9492fd0d655c35a7c7630bf1632b1f28a13ab989331a2a9f.bin", "size": 4194304, "sha256": "6d20d38e57a436ba9492fd0d655c35a7c7630bf1632b1f28a13ab989331a2a9f"}, {"url": "chunks/1ed995ee3a8b53dfecac5c1f1b642027a12380d9381a9d3142179526638f41fc.bin", "size": 549765, "sha256": "1ed995ee3a8b53dfecac5c1f1b642027a12380d9381a9d3142179526638f41fc"}, {"url": "chunks/f27b6a5f5212fad777f70a4e87987d8f4ba6e2704322516226f8e0ec2d97fc75.bin", "size": 808827, "sha256": "f27b6a5f5212fad777f70a4e87987d8f4ba6e2704322516226f8e0ec2d97fc75"}, {"url": "chunks/ddc5a1d14c29fe136b4ed4631eab96d0ad454a21bff9d59f1f42321546c0623a.bin", "size": 812786, "sha256": "ddc5a1d14c29fe136b4ed4631eab96d0ad454a21bff9d59f1f42321546c0623a"}, {"url": "chunks/f0887e90aeef1a5f9962cf4555b21d495e789473f1eb0dfbb3486d7b0add3e13.bin", "size": 775360, "sha256": "f0887e90aeef1a5f9962cf4555b21d495e789473f1eb0dfbb3486d7b0add3e13"}, {"url": "chunks/090e6f180ad54b62df1e96e53ab4e505238447c31b8325e358a74fb8040cc11e.bin", "size": 2596268, "sha256": "090e6f180ad54b62df1e96e53ab4e505238447c31b8325e358a74fb8040cc11e"}, {"url": "chunks/e8252f15f7bb2e099041b85672531d36bd5a52d78d96f319429f815a8388c90a.bin", "size": 2858861, "sha256": "e8252f15f7bb2e099041b85672531d36bd5a52d78d96f319429f815a8388c90a"}, {"url": "chunks/1961cf62ce1c8975eceb471142b74e8b9c668fa696c19da96b9ddaea6aa4a698.bin", "size": 3395022, "sha256": "1961cf62ce1c8975eceb471142b74e8b9c668fa696c19da96b9ddaea6aa4a698"}, {"url": "chunks/8394c8cd4b1433e615448993484e697504bc2e71af7408bc94f805efe466ebf6.bin", "size": 14981, "sha256": "8394c8cd4b1433e615448993484e697504bc2e71af7408bc94f805efe466ebf6"}, {"url": "chunks/f514cbe5a990d1b32595b98d138f0ffb8ea2bef9835978ae1f5d7307b0523d0b.bin", "size": 15473, "sha256": "f514cbe5a990d1b32595b98d138f0ffb8ea2bef9835978ae1f5d7307b0523d0b"}, {"url": "chunks/a47f053ea8d05b50859cdd93ff713a28776e2dab5b7c21aa8a3743d4b851e850.bin", "size": 16519, "sha256": "a47f053ea8d05b50859cdd93ff713a28776e2dab5b7c21aa8a3743d4b851e850"}, {"url": "chunks/10d129bcb283a72604ea96005c374d934ac95485e24327f0ac5987424d0ffab3.bin", "size": 67864, "sha256": "10d129bcb283a72604ea96005c374d934ac95485e24327f0ac5987424d0ffab3"}, {"url": "chunks/5c549767f204822425701d7986d2a008887f5102a306ae837a8ccf9573fcc4ad.bin", "size": 13791, "sha256": "5c549767f204822425701d7986d2a008887f5102a306ae837a8ccf9573fcc4ad"}, {"url": "chunks/61753dd5b3bd5d88a4337a8b60ca778649acb34cfdadbfbf183f423207da9d08.bin", "size": 13803, "sha256": "61753dd5b3bd5d88a4337a8b60ca778649acb34cfdadbfbf183f423207da9d08"}, {"url": "chunks/3e19505d523ff08907f66302dd77b09c29311dc8eeb9b65ba1b5b9a2ba0a4c1c.bin", "size": 159806, "sha256": "3e19505d523ff08907f66302dd77b09c29311dc8eeb9b65ba1b5b9a2ba0a4c1c"}, {"url": "chunks/16fa3b22af2082f7a982c8b34ea8822a28a03f4561f5019c87921e9170df13e2.bin", "size": 166123, "sha256": "16fa3b22af2082f7a982c8b34ea8822a28a03f4561f5019c87921e9170df13e2"}, {"url": "chunks/9881bfb6eb2c67b95dad10592722728d23342d12a6397c437b53548b9ded715d.bin", "size": 190076, "sha256": "9881bfb6eb2c67b95dad10592722728d23342d12a6397c437b53548b9ded715d"}, {"url": "chunks/da2888f66bfa2235798ca4adce1ebfae63038b6f75aab22674dfb5b2f8332ea8.bin", "size": 100323, "sha256": "da2888f66bfa2235798ca4adce1ebfae63038b6f75aab22674dfb5b2f8332ea8"}, {"url": "chunks/a92af0833a3dfc6e04fed041da0936952664aca2487a079f55ccf58e5ee53584.bin", "size": 15428, "sha256": "a92af0833a3dfc6e04fed041da0936952664aca2487a079f55ccf58e5ee53584"}, {"url": "chunks/5c0a018bc156b115e541998278b792c79d14b8b6e307116d0cba49a9552ec817.bin", "size": 42997, "sha256": "5c0a018bc156b115e541998278b792c79d14b8b6e307116d0cba49a9552ec817"}, {"url": "chunks/77ef5629cd1dd0e29b0d302105b519aa248c0cc1ac454c539e6cc08110f2407a.bin", "size": 7691, "sha256": "77ef5629cd1dd0e29b0d302105b519aa248c0cc1ac454c539e6cc08110f2407a"}, {"url": "chunks/113337056c3fb7a6a5ffe9917f07ba792a1b574232dd9be250554bbfb44708c7.bin", "size": 7986, "sha256": "113337056c3fb7a6a5ffe9917f07ba792a1b574232dd9be250554bbfb44708c7"}, {"url": "chunks/bb2fa62ce3b5d6470905b3580915c379cd911a6beab08c1d393e5b6008454f76.bin", "size": 8230, "sha256": "bb2fa62ce3b5d6470905b3580915c379cd911a6beab08c1d393e5b6008454f76"}, {"url": "chunks/a3a9a93db4e2bc6b16a133846c728df08be267c9985acb047964f12189541f50.bin", "size": 9485, "sha256": "a3a9a93db4e2bc6b16a133846c728df08be267c9985acb047964f12189541f50"}, {"url": "chunks/ec18652c12425a4af5be13bd0a1424c7d77dca6f2689111116116792d126d1e1.bin", "size": 52173, "sha256": "ec18652c12425a4af5be13bd0a1424c7d77dca6f2689111116116792d126d1e1"}, {"url": "chunks/e656fa97dbe8d4d0bb161116d60dfb7ee8b25ea98b50cf300e0d332afd4bed01.bin", "size": 159582, "sha256": "e656fa97dbe8d4d0bb161116d60dfb7ee8b25ea98b50cf300e0d332afd4bed01"}, {"url": "chunks/9d85dcec1900101312ebab4b20838287aad1876b5bd99efa396a5371dc512571.bin", "size": 7875, "sha256": "9d85dcec1900101312ebab4b20838287aad1876b5bd99efa396a5371dc512571"}, {"url": "chunks/0175e591187e9b96c7f8439cdefbe718a60ff900ecfb567336a83259b1fee985.bin", "size": 8146, "sha256": "0175e591187e9b96c7f8439cdefbe718a60ff900ecfb567336a83259b1fee985"}, {"url": "chunks/e649b0e37dca4e082942eba63b7542f98888693731758bd8a1ca12c57350f70d.bin", "size": 8395, "sha256": "e649b0e37dca4e082942eba63b7542f98888693731758bd8a1ca12c57350f70d"}, {"url": "chunks/97a400187eb42aa6fcfbab13ada65f930c0c6f5bc7e70405d4415f76a42b30f6.bin", "size": 9052, "sha256": "97a400187eb42aa6fcfbab13ada65f930c0c6f5bc7e70405d4415f76a42b30f6"}, {"url": "chunks/10ece6045e5222ed1e1e9f94669a1d083d53b36df2f69e7252c89cbe0f2adb8b.bin", "size": 10850, "sha256": "10ece6045e5222ed1e1e9f94669a1d083d53b36df2f69e7252c89cbe0f2adb8b"}, {"url": "chunks/8150642062f857ff4ccf75e091b3a911d4c21892a51e7399601d5580231c2ad1.bin", "size": 136638, "sha256": "8150642062f857ff4ccf75e091b3a911d4c21892a51e7399601d5580231c2ad1"}]}, {"url": "game.js", "size": 269538, "sha256": "a7f627076bd3eb3e9fbc8bf5c239e60a73342d8503adda8198c5806ca62252e6"}, {"url": "game.wasm", "size": 1315406, "sha256": "cef4c1480feb4373202aa64c6bb840be352251481775b289630c12d9ea3c948e"}, {"url": "icon.png", "size": 1740173, "sha256": "fb089136c275acb1e9fb25c0066251870ffec0bb22d80eb2c776bceaa3517cbd"}, {"url": "index.html", "size": 2603, "sha256": "fbc8c59744f77fd792dc627dbdc233c507dbf9b08d484310054e62dd057259c2"}, {"url": "manifest.webmanifest", "size": 265, "sha256": "12869bff75f2eb5d6d3c250fc42b3cc7e9022696f0b4608ee6a85e2e051694b5"}, {"url": "social-preview.jpg", "size": 627410, "sha256": "15fddebb459273543a2ebad916f7fae99ab8bf2cd90bd242050b6c482a93f633"}, {"url": "storage.js", "size": 1864, "sha256": "f1bf0073367df291f09a6e721b5cf5629de3a06acfced595082857edb04cf38e"}, {"url": "style.css", "size": 1441, "sha256": "a2307a115f8a77ff27c86c57e8b2bae3ff1bc91f8d248f2cd7bb53dfef817b5a"}, {"url": "title.jpg", "size": 172705, "sha256": "50ce101b40757018130437f411e64dd8709234ebf649f7f8e9d555bb73cbf696"}]};
const BASE=new URL('./',self.location.href),BLOBS='machdefense-chunks-v1';
const urlFor=name=>new URL(name,BASE).href;
// The first published worker rewrites every cached file during preparation.
// Pin its manifest so this worker can serve its verified cache without writing it.
const LEGACY_CACHE='machdefense-assets-f9f26e94f1d4e3d6';
const LEGACY_MANIFEST_SHA256='60f4870d3efe8d8b6de31643f2d08ce845172bca6daf79f556415da947e0744a';
const release=releaseIntegrity(BASE.href,'machdefense-assets-',CACHE,async key=>
 Boolean(await (await caches.open(key)).match(urlFor('asset-manifest.json'))));
const scopedClients=async()=> (await self.clients.matchAll({includeUncontrolled:true,type:'window'})).filter(client=>client.url.startsWith(BASE.href));
const digest=async bytes=>Array.from(new Uint8Array(await crypto.subtle.digest('SHA-256',bytes)),b=>b.toString(16).padStart(2,'0')).join('');
let preparation=null, promotion=null, fallback;
async function checked(response,file){
 if(!response?.ok)return null;
 const bytes=await response.arrayBuffer();
 return bytes.byteLength===file.size&&await digest(bytes)===file.sha256?bytes:null;
}
async function download(file){
 const response=await fetch(urlFor(file.url),{cache:'no-store'}),bytes=await checked(response,file);
 if(!bytes)throw Error('Resource checksum failed: '+file.url);
 return bytes;
}
function responseFor(bytes,name){
 const type=name.endsWith('.js')?'text/javascript':name.endsWith('.html')?'text/html':name.endsWith('.css')?'text/css':name.endsWith('.wasm')?'application/wasm':name.endsWith('.json')?'application/json':name.endsWith('.webmanifest')?'application/manifest+json':'application/octet-stream';
 return new Response(bytes,{headers:{'Content-Type':type,'X-MachDefense-Version':VERSION}});
}
async function prepareResources(){
 if(preparation)return preparation;
 preparation=prepare().finally(()=>{preparation=null;});return preparation;
}
async function prepare(){
 if(MANIFEST.version!==VERSION)throw Error('Mixed resource versions');
 const cache=await caches.open(CACHE),blobs=await caches.open(BLOBS);
 const old=[];
 for(const key of await release.olderCaches())old.push(await caches.open(key));
 let done=0,downloaded=0,reused=0;const total=MANIFEST.files.reduce((n,f)=>n+f.size,0);
 for(const file of MANIFEST.files){
  let bytes=null;
  if(file.parts){
   const output=new Uint8Array(file.size);let offset=0;
   for(const part of file.parts){
    let piece=await checked(await blobs.match(urlFor(part.url)),part);
    if(piece)reused+=piece.byteLength;
    else{piece=await download(part);downloaded+=piece.byteLength;await blobs.put(urlFor(part.url),responseFor(piece,part.url));}
    output.set(new Uint8Array(piece),offset);offset+=piece.byteLength;
    for(const client of await scopedClients())client.postMessage({kind:'download',version:VERSION,done:done+offset,total,downloaded,reused});
   }
   if(offset!==file.size||await digest(output)!==file.sha256)throw Error('Resource assembly failed');
   bytes=output;
   // Store only the chunks. A second full data copy doubles the offline quota.
   await cache.delete(urlFor(file.url));
  }else{
   const local=await checked(await cache.match(urlFor(file.url)),file);
   bytes=local;
   if(!bytes)for(const source of old){bytes=await checked(await source.match(urlFor(file.url)),file);if(bytes)break;}
   if(bytes)reused+=file.size;
   else{bytes=await download(file);downloaded+=bytes.byteLength;}
   if(!local)await cache.put(urlFor(file.url),responseFor(bytes,file.url));
  }
  done+=file.size;
  for(const client of await scopedClients())client.postMessage({kind:'download',version:VERSION,done,total,downloaded,reused});
 }
 // Only complete, verified releases receive this marker. Progress lives in
 // IndexedDB and is never touched by this worker.
 const metadata={'asset-manifest.json':MANIFEST,'build.json':{version:VERSION,cheat:MANIFEST.cheat}};
 for(const [name,value] of Object.entries(metadata)){
  const text=JSON.stringify(value);
  if(await (await cache.match(urlFor(name)))?.text()!==text)await cache.put(urlFor(name),responseFor(text,name));
 }
 return {version:VERSION,downloaded,reused};
}
async function assemble(file){
 const blobs=await caches.open(BLOBS),output=new Uint8Array(file.size);let offset=0;
 for(const part of file.parts){
  const bytes=await checked(await blobs.match(urlFor(part.url)),part);
  if(!bytes||offset+bytes.byteLength>output.byteLength)return null;
  output.set(new Uint8Array(bytes),offset);
  offset+=bytes.byteLength;
 }
 return offset===file.size&&await digest(output)===file.sha256?output:null;
}
async function legacyManifest(){
 if(MANIFEST.cheat||!await caches.has(LEGACY_CACHE))return null;
 const cache=await caches.open(LEGACY_CACHE);
 const response=await cache.match(urlFor('asset-manifest.json'));
 if(!response)return null;
 const bytes=await response.arrayBuffer();
 if(await digest(bytes)!==LEGACY_MANIFEST_SHA256)return null;
 return JSON.parse(new TextDecoder().decode(bytes));
}
async function verifyLegacy(manifest){
 const cache=await caches.open(LEGACY_CACHE);
 for(const file of manifest.files){
  const bytes=file.parts?await assemble(file):await checked(await cache.match(urlFor(file.url)),file);
  if(!bytes)throw Error('Cached release file failed verification: '+file.url);
 }
}
async function usingFallback(){
 if(fallback!==undefined)return fallback;
 const published=await caches.has(CACHE)&&
  await (await caches.open(CACHE)).match(urlFor('.client-editions'));
 fallback=!published&&Boolean(await legacyManifest());
 return fallback;
}
function promoteRelease(){
 if(promotion)return promotion;
 promotion=(async()=>{
  const older=await release.olderCaches();
  try{
   const result=await prepareResources();
   await release.stage(await caches.open(CACHE),older);
   await retainChunks();
   fallback=false;
   return result;
  }catch(error){
   await caches.delete(CACHE);
   await retainChunks().catch(()=>{});
   if(older.at(-1)!==LEGACY_CACHE)throw error;
   const manifest=await legacyManifest();
   if(!manifest)throw error;
   await verifyLegacy(manifest);
   fallback=true;
   return {version:manifest.version,downloaded:0,reused:manifest.files.reduce((n,file)=>n+file.size,0)};
  }
 })().finally(()=>{promotion=null;});
 return promotion;
}
self.addEventListener('install',event=>event.waitUntil((async()=>{
 await promoteRelease();
 await self.skipWaiting();
})()));
self.addEventListener('message',event=>{
 const reply=value=>event.ports[0]?.postMessage(value);
 if(event.data?.kind==='prepare')event.waitUntil((async()=>{
  if(await usingFallback()){reply({ok:true,...await promoteRelease()});return;}
  const edition=await release.edition(event.source?.id);
  if(edition===CACHE){reply({ok:true,...await prepareResources()});return;}
  const previous=await (await (await caches.open(edition)).match(urlFor('build.json')))?.json();
  if(!previous)throw Error('Reopen the app to load a verified release.');
  reply({ok:true,version:previous.version});
 })().catch(error=>reply({ok:false,error:error.message})));
 if(event.data?.kind==='activate')event.waitUntil((async()=>{
  const result=await usingFallback()?await promoteRelease():await prepareResources();
  reply({ok:true,version:result.version});await self.skipWaiting();
 })().catch(error=>reply({ok:false,error:error.message})));
});
async function retainChunks(){
 const versions=await release.olderCaches();
 if((await caches.keys()).includes(CACHE))versions.push(CACHE);
 const needed=new Set();
 for(const key of versions){
  const cache=await caches.open(key),response=await cache.match(urlFor('asset-manifest.json'));
  if(!response)continue;const manifest=await response.json();
  for(const file of manifest.files)for(const part of file.parts||[])needed.add(urlFor(part.url));
 }
 const blobs=await caches.open(BLOBS);for(const request of await blobs.keys())if(request.url.startsWith(urlFor('chunks/'))&&!needed.has(request.url))await blobs.delete(request);
}
// A reopened Safari app can start without a controller while an older cache
// remains. Take control only after install has staged the verified release.
self.addEventListener('activate',event=>event.waitUntil(self.clients.claim()));
self.addEventListener('fetch',event=>{
 const url=new URL(event.request.url);if(url.origin!==BASE.origin||!url.pathname.startsWith(BASE.pathname)||event.request.method!=='GET')return;
 // SW updates must always reach the server. Never cache the worker script.
 if(url.pathname===new URL('sw.js',BASE).pathname)return;
 const navigation=event.request.mode==='navigate';
 const name=navigation?'index.html':decodeURIComponent(url.pathname.slice(BASE.pathname.length));
 event.respondWith((async()=>{
  const edition=await usingFallback()?LEGACY_CACHE:navigation?CACHE:await release.edition(event.clientId);
  const cache=await caches.open(edition);
  const manifest=edition===CACHE?MANIFEST:edition===LEGACY_CACHE?await legacyManifest():
   await (await cache.match(urlFor('asset-manifest.json')))?.json();
  if(edition===LEGACY_CACHE&&!manifest)throw Error('Cached release manifest failed verification');
  let response=await cache.match(urlFor(name));
  const file=manifest?.files.find(file=>file.url===name);
  if(!manifest&&response)return response;
  if(!file&&!['build.json','asset-manifest.json'].includes(name))throw Error('Unlisted release file');

  if(file?.parts){
   let bytes=await assemble(file);if(!bytes&&edition===CACHE){await prepareResources();bytes=await assemble(file);}
   if(!bytes)throw Error('Resource assembly unavailable');return responseFor(bytes,name);
  }
  if(file){const bytes=await checked(response,file);if(bytes)return responseFor(bytes,name);}
  else if(manifest)return responseFor(JSON.stringify(name==='build.json'?{version:manifest.version,cheat:manifest.cheat}:manifest),name);
  if(edition!==CACHE)throw Error('Reopen the app to load a verified release.');
  await prepareResources();response=await cache.match(urlFor(name));
  if(!response)throw Error('Resource unavailable: '+name);return response;
 })().catch(async error=>{const message=name+': '+String(error);for(const client of await self.clients.matchAll())client.postMessage({kind:'resource-error',message});return new Response(message,{status:503});}));
});
