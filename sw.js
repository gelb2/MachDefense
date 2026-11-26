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

const VERSION='dc8948feca28b8e5',CACHE='machdefense-assets-'+VERSION;
const MANIFEST={"version": "dc8948feca28b8e5", "cheat": false, "files": [{"url": "app.js", "size": 11803, "sha256": "bffc70e08b6604ef7b55e1778d168e77e51ef13f9979cc0de85fad474eedd888"}, {"url": "audio.js", "size": 1159, "sha256": "76cc213fb5a87b9d3646a0ea4fcbeb4d795049087de73a13d3b0e64b741e93ab"}, {"url": "game.data", "size": 73236616, "sha256": "0a8ffbd6f6ff231d6e23e3d2bc4924c0521bfb5f533f2695b4db49c80e9d6bb6", "parts": [{"url": "chunks/627c77924fecbdf9b7d7955db34c757f3153775fcbce38dd6e0d834ff4b54faf.bin", "size": 4194304, "sha256": "627c77924fecbdf9b7d7955db34c757f3153775fcbce38dd6e0d834ff4b54faf"}, {"url": "chunks/5261975aa88a99dc86934b5f82e73eea3ccb51a3816e514612046930bf92a154.bin", "size": 580438, "sha256": "5261975aa88a99dc86934b5f82e73eea3ccb51a3816e514612046930bf92a154"}, {"url": "chunks/9e63e349bc4e68373519a743257b505b13aa7c0f06e1ce29f5998d48d6ed8812.bin", "size": 4194304, "sha256": "9e63e349bc4e68373519a743257b505b13aa7c0f06e1ce29f5998d48d6ed8812"}, {"url": "chunks/d7ec72ba18ae743cf3cb41a06fee924353eb6c2c65b15bb6234f6b1e2e9ffde3.bin", "size": 2806714, "sha256": "d7ec72ba18ae743cf3cb41a06fee924353eb6c2c65b15bb6234f6b1e2e9ffde3"}, {"url": "chunks/941cfbc69c2e7a9f692549f7692f774e3aa79f9cacce41afceb9c15c2d225ece.bin", "size": 2359398, "sha256": "941cfbc69c2e7a9f692549f7692f774e3aa79f9cacce41afceb9c15c2d225ece"}, {"url": "chunks/a3ef5fc2a4de19c505e1d025a2e071f84c1a128aecf3cf935e29b7a6ea742e15.bin", "size": 957430, "sha256": "a3ef5fc2a4de19c505e1d025a2e071f84c1a128aecf3cf935e29b7a6ea742e15"}, {"url": "chunks/1f839806939291fca20e323f1afdaab7365d9abdccf2eec46ebc2c2d312e6f8e.bin", "size": 4194304, "sha256": "1f839806939291fca20e323f1afdaab7365d9abdccf2eec46ebc2c2d312e6f8e"}, {"url": "chunks/275afd1fce3acd8410fd5cacc029426c1a6baeb101c06f96bbcb9c303a54efcb.bin", "size": 1074946, "sha256": "275afd1fce3acd8410fd5cacc029426c1a6baeb101c06f96bbcb9c303a54efcb"}, {"url": "chunks/b827946869b0b3970accac84f40c96a91fee92908f6288fef1235c5dcbdafd37.bin", "size": 4194304, "sha256": "b827946869b0b3970accac84f40c96a91fee92908f6288fef1235c5dcbdafd37"}, {"url": "chunks/722e124c1e4ea7b1c4b0fad854373e9557707a8cb7b3001a600cb5cd3165eee6.bin", "size": 1372327, "sha256": "722e124c1e4ea7b1c4b0fad854373e9557707a8cb7b3001a600cb5cd3165eee6"}, {"url": "chunks/99c99da9db6f6f5f56180068b1d657f4d9603ba7eac1b336d5d6d6bc28065ca6.bin", "size": 35227, "sha256": "99c99da9db6f6f5f56180068b1d657f4d9603ba7eac1b336d5d6d6bc28065ca6"}, {"url": "chunks/f8f83015169205955976914b96ec94a61f00d6193c50b52bfb9e8c66f81cfb6e.bin", "size": 193859, "sha256": "f8f83015169205955976914b96ec94a61f00d6193c50b52bfb9e8c66f81cfb6e"}, {"url": "chunks/de23ff976315128cda3587a615a3608690e60d78779dfb0b047b1400c25e08d5.bin", "size": 65293, "sha256": "de23ff976315128cda3587a615a3608690e60d78779dfb0b047b1400c25e08d5"}, {"url": "chunks/4273c487b8cce9aaa491bbc6794b4f0d22bf952fae286a73807ea7492f5b57fc.bin", "size": 12411, "sha256": "4273c487b8cce9aaa491bbc6794b4f0d22bf952fae286a73807ea7492f5b57fc"}, {"url": "chunks/330213d489c4865e0b2da39f0e67303a323ca60edda8a0ce894a3b9fea60ca35.bin", "size": 131734, "sha256": "330213d489c4865e0b2da39f0e67303a323ca60edda8a0ce894a3b9fea60ca35"}, {"url": "chunks/296285cbb174507fb78d9cf58a5eca9d2bebfeb98f31a7b4377cefac3714b73f.bin", "size": 11971, "sha256": "296285cbb174507fb78d9cf58a5eca9d2bebfeb98f31a7b4377cefac3714b73f"}, {"url": "chunks/8c0bab55dbf5f17f71102af8b0b009ab18861895fee6e4437eb1cf8f667e2693.bin", "size": 161648, "sha256": "8c0bab55dbf5f17f71102af8b0b009ab18861895fee6e4437eb1cf8f667e2693"}, {"url": "chunks/86dd69da92d3fa2ac94d01475d6db5fb0bf47a22dc63dc82da7e63bc08dbec50.bin", "size": 58402, "sha256": "86dd69da92d3fa2ac94d01475d6db5fb0bf47a22dc63dc82da7e63bc08dbec50"}, {"url": "chunks/d14309e3334c339be196006c5358012329e9863a57bc74a9d10aa3f4131e5a82.bin", "size": 87393, "sha256": "d14309e3334c339be196006c5358012329e9863a57bc74a9d10aa3f4131e5a82"}, {"url": "chunks/c21beaae737fb83cf8aa14ffc0fa735689c93d76e01140ceaa0662fe66cdfbf0.bin", "size": 241684, "sha256": "c21beaae737fb83cf8aa14ffc0fa735689c93d76e01140ceaa0662fe66cdfbf0"}, {"url": "chunks/306c9dc40f1d9cc64760f5378605064f84177d12bb73f9a9a96bbaecae556a18.bin", "size": 2224817, "sha256": "306c9dc40f1d9cc64760f5378605064f84177d12bb73f9a9a96bbaecae556a18"}, {"url": "chunks/0e116bcc380f6e88234374b721861fbea3c77d1203d0f53738a22ae4dcf86647.bin", "size": 4139608, "sha256": "0e116bcc380f6e88234374b721861fbea3c77d1203d0f53738a22ae4dcf86647"}, {"url": "chunks/81f523a541b21393f0065e407b1c367bedc42c3392ba9a2362ad1d1cd2293740.bin", "size": 3888269, "sha256": "81f523a541b21393f0065e407b1c367bedc42c3392ba9a2362ad1d1cd2293740"}, {"url": "chunks/2cb846fbd7c36f88dde0de35a965d606409e44a0a28174f93a4f9096bc5c15ce.bin", "size": 4121209, "sha256": "2cb846fbd7c36f88dde0de35a965d606409e44a0a28174f93a4f9096bc5c15ce"}, {"url": "chunks/0aabf80c59ebf56c1147f8ecbf2b021030588f2c4626009194f8dde2ca601f70.bin", "size": 296283, "sha256": "0aabf80c59ebf56c1147f8ecbf2b021030588f2c4626009194f8dde2ca601f70"}, {"url": "chunks/805f62bc08866a1782f850ca7a466c18df3f783bb41f862fedb20abcec4741ca.bin", "size": 456066, "sha256": "805f62bc08866a1782f850ca7a466c18df3f783bb41f862fedb20abcec4741ca"}, {"url": "chunks/4ba55b7b07020a96ec13655cce24ca6f9c4f5d19cc9a63760c1c5e45b2f3afa9.bin", "size": 111833, "sha256": "4ba55b7b07020a96ec13655cce24ca6f9c4f5d19cc9a63760c1c5e45b2f3afa9"}, {"url": "chunks/c859fb0dff75c508267339c5ed8b58b426211b7374d38063b7bfea28468f1d7a.bin", "size": 270575, "sha256": "c859fb0dff75c508267339c5ed8b58b426211b7374d38063b7bfea28468f1d7a"}, {"url": "chunks/0ae8e0c8c036cb13e53d576fd64e87caaa48fc4149e7fdc11b4860f166764d15.bin", "size": 229709, "sha256": "0ae8e0c8c036cb13e53d576fd64e87caaa48fc4149e7fdc11b4860f166764d15"}, {"url": "chunks/1c61d77b982f17a059353ec0ee50a8aa0fb25c3510a3e85a53cf987cb661ad82.bin", "size": 98685, "sha256": "1c61d77b982f17a059353ec0ee50a8aa0fb25c3510a3e85a53cf987cb661ad82"}, {"url": "chunks/8f2723b6d01f5de5af12502303160b100a10dfbcbe0019125b7cd6246baa8c50.bin", "size": 157018, "sha256": "8f2723b6d01f5de5af12502303160b100a10dfbcbe0019125b7cd6246baa8c50"}, {"url": "chunks/9f044d8637ebbbd60ff9325df6cebec047293e9e5db894e1306e031bf282fdf0.bin", "size": 486661, "sha256": "9f044d8637ebbbd60ff9325df6cebec047293e9e5db894e1306e031bf282fdf0"}, {"url": "chunks/6457484dbd2a6674e381e364909fa417ab946789ffde9ed2c2e8636697b1f12b.bin", "size": 147952, "sha256": "6457484dbd2a6674e381e364909fa417ab946789ffde9ed2c2e8636697b1f12b"}, {"url": "chunks/fa7c1c23616f658e8204a1713141e28e8a648c6ca13ccaa9b95df1721ddc1bd7.bin", "size": 216590, "sha256": "fa7c1c23616f658e8204a1713141e28e8a648c6ca13ccaa9b95df1721ddc1bd7"}, {"url": "chunks/e962f136af1bbfc860914fa3482d5f1a0430ef06443f5cfac2ca9f490467d70e.bin", "size": 134100, "sha256": "e962f136af1bbfc860914fa3482d5f1a0430ef06443f5cfac2ca9f490467d70e"}, {"url": "chunks/f50c8e18db0103b20c50973e7e7904e36d8665bcc94ed5337375eac0293de2db.bin", "size": 44456, "sha256": "f50c8e18db0103b20c50973e7e7904e36d8665bcc94ed5337375eac0293de2db"}, {"url": "chunks/381eab3fad8eab4e1b4ad35413585131f47bac59997809b0a048b984546f489a.bin", "size": 210229, "sha256": "381eab3fad8eab4e1b4ad35413585131f47bac59997809b0a048b984546f489a"}, {"url": "chunks/efc492a952232d1f43fa71d185417bc5f970ba19e7cb8e485d00e7703b7f3117.bin", "size": 137142, "sha256": "efc492a952232d1f43fa71d185417bc5f970ba19e7cb8e485d00e7703b7f3117"}, {"url": "chunks/f0bcab384a3cf1ec6b598473d243dcbf02accd5e3348fc2aeceee96ecaaf7f85.bin", "size": 64254, "sha256": "f0bcab384a3cf1ec6b598473d243dcbf02accd5e3348fc2aeceee96ecaaf7f85"}, {"url": "chunks/6229f06e3e49765fa4d7e52f40b3aded762e01a894e934ffa12aca0c57a9df2a.bin", "size": 1553583, "sha256": "6229f06e3e49765fa4d7e52f40b3aded762e01a894e934ffa12aca0c57a9df2a"}, {"url": "chunks/9373da5b2eb77269cd822ad8e8add39aa99a15d28a4f41d1df95d00f5c8887ac.bin", "size": 367261, "sha256": "9373da5b2eb77269cd822ad8e8add39aa99a15d28a4f41d1df95d00f5c8887ac"}, {"url": "chunks/fa1277e97e86c0132701ac133587d23199d0701b85fa13c5f77f5bb8d1845420.bin", "size": 1028874, "sha256": "fa1277e97e86c0132701ac133587d23199d0701b85fa13c5f77f5bb8d1845420"}, {"url": "chunks/f893c42af76f755c93004e384f6ebad36ab224ca00ba13a099b0f87cbe168df7.bin", "size": 141409, "sha256": "f893c42af76f755c93004e384f6ebad36ab224ca00ba13a099b0f87cbe168df7"}, {"url": "chunks/03c16990badd3ec64db6dd8a1d944987cdfba7434d94394e972a25c4aa7c9a81.bin", "size": 154487, "sha256": "03c16990badd3ec64db6dd8a1d944987cdfba7434d94394e972a25c4aa7c9a81"}, {"url": "chunks/2eee16e7a8405bce24e36308bfa1ffa6ebba189b54866bbfb8fec88b67ff4d14.bin", "size": 752414, "sha256": "2eee16e7a8405bce24e36308bfa1ffa6ebba189b54866bbfb8fec88b67ff4d14"}, {"url": "chunks/be31c36b3ab6d1e47211cf72080e4730a7c6bd4e570e368f04b53111e2ccab9e.bin", "size": 344767, "sha256": "be31c36b3ab6d1e47211cf72080e4730a7c6bd4e570e368f04b53111e2ccab9e"}, {"url": "chunks/4cf1cdb4f1a8ae252606ee4dd7345a1f6978ead57754151a3c118a1b29ea7290.bin", "size": 80954, "sha256": "4cf1cdb4f1a8ae252606ee4dd7345a1f6978ead57754151a3c118a1b29ea7290"}, {"url": "chunks/5e3008c5be4e3496e51405f22cf6833d9a9ffbe3532c88766229584caefe0ce8.bin", "size": 92079, "sha256": "5e3008c5be4e3496e51405f22cf6833d9a9ffbe3532c88766229584caefe0ce8"}, {"url": "chunks/72de4599302833d96aba199bc7d0e2612da012257e8bdde3bd6162d586a68491.bin", "size": 79019, "sha256": "72de4599302833d96aba199bc7d0e2612da012257e8bdde3bd6162d586a68491"}, {"url": "chunks/954fd38cde8c1e27d7e8e7e43937648517cd7dfa1b795cb50825ed0a3eef4317.bin", "size": 88595, "sha256": "954fd38cde8c1e27d7e8e7e43937648517cd7dfa1b795cb50825ed0a3eef4317"}, {"url": "chunks/794587c323b01a0df423cff222a14ac10eec7e335b6b32dd8f71f3b3e6708823.bin", "size": 98897, "sha256": "794587c323b01a0df423cff222a14ac10eec7e335b6b32dd8f71f3b3e6708823"}, {"url": "chunks/1d13f2785fa23a2795f2e3b4ff411f983271372d1d224caba2eb87b69ca5a637.bin", "size": 117541, "sha256": "1d13f2785fa23a2795f2e3b4ff411f983271372d1d224caba2eb87b69ca5a637"}, {"url": "chunks/a5b94782d8de96bb0a89e7517f06132faed7f00ba467b9ca0c7e982109470946.bin", "size": 95230, "sha256": "a5b94782d8de96bb0a89e7517f06132faed7f00ba467b9ca0c7e982109470946"}, {"url": "chunks/50d8c7f218682ec8011a2b3c1e50e087586744a4cde8e960c28021b58bdca939.bin", "size": 20123, "sha256": "50d8c7f218682ec8011a2b3c1e50e087586744a4cde8e960c28021b58bdca939"}, {"url": "chunks/58c509425cee0e2809efaa74eb1fa535b2bd5ae1da2cbe054f499c2719c83117.bin", "size": 20732, "sha256": "58c509425cee0e2809efaa74eb1fa535b2bd5ae1da2cbe054f499c2719c83117"}, {"url": "chunks/caea24d881b1d1735219c5edee75ee4ae9bf55e62183e6b5ccb48f8f2b4cc83a.bin", "size": 16471, "sha256": "caea24d881b1d1735219c5edee75ee4ae9bf55e62183e6b5ccb48f8f2b4cc83a"}, {"url": "chunks/1bfd887ff11521e1ff9fba5109e36d0248761f889e3250ab61fa642b02cee0d0.bin", "size": 16220, "sha256": "1bfd887ff11521e1ff9fba5109e36d0248761f889e3250ab61fa642b02cee0d0"}, {"url": "chunks/736c55ab0b74f660c8bbd52174fc850645c8e4af5a184e21465eb48437fc5bf4.bin", "size": 16546, "sha256": "736c55ab0b74f660c8bbd52174fc850645c8e4af5a184e21465eb48437fc5bf4"}, {"url": "chunks/deb1c76dd4fa8170db1731c06ab6b030eef4ae708ac73be13cffd2c519d02c9b.bin", "size": 17740, "sha256": "deb1c76dd4fa8170db1731c06ab6b030eef4ae708ac73be13cffd2c519d02c9b"}, {"url": "chunks/a7990738256c9f04fae77ec1219a878e05cdcc86aa2bb6ec7e87ebbd9a2bf4c1.bin", "size": 17168, "sha256": "a7990738256c9f04fae77ec1219a878e05cdcc86aa2bb6ec7e87ebbd9a2bf4c1"}, {"url": "chunks/9ad5e06841747bee0955286f8f306149f4c367029756c624fbe158007987d205.bin", "size": 18952, "sha256": "9ad5e06841747bee0955286f8f306149f4c367029756c624fbe158007987d205"}, {"url": "chunks/8b40e57b6782af55f0750eca0f9a691c1783766b09ce3978f3d6aaa5a5498ab8.bin", "size": 3277458, "sha256": "8b40e57b6782af55f0750eca0f9a691c1783766b09ce3978f3d6aaa5a5498ab8"}, {"url": "chunks/ff985242d7dafae0c2319c5036b646fca9827a0db23653487d6413681c3c39cd.bin", "size": 233585, "sha256": "ff985242d7dafae0c2319c5036b646fca9827a0db23653487d6413681c3c39cd"}, {"url": "chunks/4bed6ad437a810480c8d1c60a85c3f0c8e52a847b39c8477d5c6d71849b79ab1.bin", "size": 85233, "sha256": "4bed6ad437a810480c8d1c60a85c3f0c8e52a847b39c8477d5c6d71849b79ab1"}, {"url": "chunks/73a82e0175d5be0781ee951bb08ea6edc69407c191f9f5943da2eb744c3e0add.bin", "size": 77696, "sha256": "73a82e0175d5be0781ee951bb08ea6edc69407c191f9f5943da2eb744c3e0add"}, {"url": "chunks/86918ee44177e7bb818eb2f0b19b3225008a0a5f3bd95ee63c9d30d71b4f18bd.bin", "size": 80899, "sha256": "86918ee44177e7bb818eb2f0b19b3225008a0a5f3bd95ee63c9d30d71b4f18bd"}, {"url": "chunks/11cc4a8aff7cfa18c2d7001fe4ce2c176842955294b334532a48b0a751339f2a.bin", "size": 86719, "sha256": "11cc4a8aff7cfa18c2d7001fe4ce2c176842955294b334532a48b0a751339f2a"}, {"url": "chunks/b4113e34ab9d93d3f92a203b31cd7820d65984781ab973aece4f3be66fce1817.bin", "size": 64097, "sha256": "b4113e34ab9d93d3f92a203b31cd7820d65984781ab973aece4f3be66fce1817"}, {"url": "chunks/e424ab9a4c9ed99c0d431019eff788654e278155f4cc4726bcf88b912ed51d89.bin", "size": 15134, "sha256": "e424ab9a4c9ed99c0d431019eff788654e278155f4cc4726bcf88b912ed51d89"}, {"url": "chunks/d20be8e65ee49608b6d53d7303088421e5377408208830ff6668be4bc7d038b0.bin", "size": 15655, "sha256": "d20be8e65ee49608b6d53d7303088421e5377408208830ff6668be4bc7d038b0"}, {"url": "chunks/8c25432662678b766b213080dfc39731b9e387e3fff838e5b75f0ea941af1e01.bin", "size": 12237, "sha256": "8c25432662678b766b213080dfc39731b9e387e3fff838e5b75f0ea941af1e01"}, {"url": "chunks/92b17040b94873ba0db3dd056a29d6862d1edc661cc9c38086c3f805d06e6536.bin", "size": 13742, "sha256": "92b17040b94873ba0db3dd056a29d6862d1edc661cc9c38086c3f805d06e6536"}, {"url": "chunks/2cb6c94f83c10838892bae243da2ac16693988e056fa14d3bd8a749574d2a051.bin", "size": 14605, "sha256": "2cb6c94f83c10838892bae243da2ac16693988e056fa14d3bd8a749574d2a051"}, {"url": "chunks/70f256c85c6e55680ca5388b6027246ebcee4f45ad3f708b99e26a099bac1717.bin", "size": 2763868, "sha256": "70f256c85c6e55680ca5388b6027246ebcee4f45ad3f708b99e26a099bac1717"}, {"url": "chunks/9d94fb44545cb679c6220bb1ca78c40f73f2d1d3953e78fd2c6c0ba3a75f6196.bin", "size": 4194304, "sha256": "9d94fb44545cb679c6220bb1ca78c40f73f2d1d3953e78fd2c6c0ba3a75f6196"}, {"url": "chunks/8a014104682ce5a37cb569a185d13f4cf68d05992772ddab11c7f84e24825bb4.bin", "size": 361990, "sha256": "8a014104682ce5a37cb569a185d13f4cf68d05992772ddab11c7f84e24825bb4"}, {"url": "chunks/b2c9bc274d85a1d35d44ad83a47b5ad93b46148809701ef4875b399313166024.bin", "size": 793360, "sha256": "b2c9bc274d85a1d35d44ad83a47b5ad93b46148809701ef4875b399313166024"}, {"url": "chunks/fb84fd4ae46dbb7c0c810b99394661cac99dbd7003a1b0c5a132a739045dbe38.bin", "size": 804382, "sha256": "fb84fd4ae46dbb7c0c810b99394661cac99dbd7003a1b0c5a132a739045dbe38"}, {"url": "chunks/c9989309dec5478fe8fa1b93ae400d1655977480e1b0f9d6eaf91181d3916ad2.bin", "size": 887182, "sha256": "c9989309dec5478fe8fa1b93ae400d1655977480e1b0f9d6eaf91181d3916ad2"}, {"url": "chunks/5526c0ca45f4e9b7610f1f8549722a9158975de119dca7ba2270f59db4d4fff6.bin", "size": 2759061, "sha256": "5526c0ca45f4e9b7610f1f8549722a9158975de119dca7ba2270f59db4d4fff6"}, {"url": "chunks/023cd945a94e8f33e35b683a1d3fc4f8bfe81c51c16bfd2ccc9da617c8372ef1.bin", "size": 2780556, "sha256": "023cd945a94e8f33e35b683a1d3fc4f8bfe81c51c16bfd2ccc9da617c8372ef1"}, {"url": "chunks/b5c4414efe1537e51f3f9db897368e195cc21c0e7037a977c8d3c71336181e62.bin", "size": 3169666, "sha256": "b5c4414efe1537e51f3f9db897368e195cc21c0e7037a977c8d3c71336181e62"}, {"url": "chunks/17e8f51b59d04c218f907fd998a7b60fad2c7ea53a15828382903a5427cf0bc4.bin", "size": 15662, "sha256": "17e8f51b59d04c218f907fd998a7b60fad2c7ea53a15828382903a5427cf0bc4"}, {"url": "chunks/b4464c13632afadf3dbd01a86fa6f67494456412dd29582516b3a0ada6580859.bin", "size": 14661, "sha256": "b4464c13632afadf3dbd01a86fa6f67494456412dd29582516b3a0ada6580859"}, {"url": "chunks/46684d92f5c389e5bf6596a7923d35c5dc4cf13a408181160a84aa6f7e8371cf.bin", "size": 16539, "sha256": "46684d92f5c389e5bf6596a7923d35c5dc4cf13a408181160a84aa6f7e8371cf"}, {"url": "chunks/462f0d4699996daeee601bb63a5b43f9c4e8c4cc85e8be1b26d966ecb277c96b.bin", "size": 68021, "sha256": "462f0d4699996daeee601bb63a5b43f9c4e8c4cc85e8be1b26d966ecb277c96b"}, {"url": "chunks/d18563b1250692585d45ae612e5aed512b43fe036df53934d55b43e7dc33e010.bin", "size": 13479, "sha256": "d18563b1250692585d45ae612e5aed512b43fe036df53934d55b43e7dc33e010"}, {"url": "chunks/0f59068257ab29bb0a3d721071d2f26a3642a82c7ad8ac977125a7c1d897e4a9.bin", "size": 13885, "sha256": "0f59068257ab29bb0a3d721071d2f26a3642a82c7ad8ac977125a7c1d897e4a9"}, {"url": "chunks/fceee5e8a60fa42dabf4a9a95520216451643196b5685b1a346270cef147f0ea.bin", "size": 171449, "sha256": "fceee5e8a60fa42dabf4a9a95520216451643196b5685b1a346270cef147f0ea"}, {"url": "chunks/244b29a1c1cbcfdec964c3c0d08dcddcff96b4e1c86f21d3e99687127a7f60a6.bin", "size": 166538, "sha256": "244b29a1c1cbcfdec964c3c0d08dcddcff96b4e1c86f21d3e99687127a7f60a6"}, {"url": "chunks/9cf738c5c22035628519bde126f1ee4d5537253af8fa7911378d22a465462b6d.bin", "size": 190057, "sha256": "9cf738c5c22035628519bde126f1ee4d5537253af8fa7911378d22a465462b6d"}, {"url": "chunks/b5321a0c1eb4bb1f39192d3b24e9afcd4429a4ff8661a9ecd48ad26969ea545d.bin", "size": 88843, "sha256": "b5321a0c1eb4bb1f39192d3b24e9afcd4429a4ff8661a9ecd48ad26969ea545d"}, {"url": "chunks/3b8613c3c8a7c871122b6d5cf4ffeb2940c8546d4dc0f19e8b1e4ad92674acf8.bin", "size": 15146, "sha256": "3b8613c3c8a7c871122b6d5cf4ffeb2940c8546d4dc0f19e8b1e4ad92674acf8"}, {"url": "chunks/338498b7ca94a7269971c73de98dc751df6d54066087d1ad8d258309f7026594.bin", "size": 42581, "sha256": "338498b7ca94a7269971c73de98dc751df6d54066087d1ad8d258309f7026594"}, {"url": "chunks/3d0bb86a9db6c2c3f7f667cc7db319dd42d05d6b24b1ce927edc49309d0e7bbd.bin", "size": 7829, "sha256": "3d0bb86a9db6c2c3f7f667cc7db319dd42d05d6b24b1ce927edc49309d0e7bbd"}, {"url": "chunks/c8a5d4dab7734be9f8f53fe6a547d9bb0c7d009e8255563fe4849cdc44a09df6.bin", "size": 7986, "sha256": "c8a5d4dab7734be9f8f53fe6a547d9bb0c7d009e8255563fe4849cdc44a09df6"}, {"url": "chunks/35ad06fd5d81e8672f7badcd305eee3573f3e3c4af784b6a93177cc58e676e51.bin", "size": 8231, "sha256": "35ad06fd5d81e8672f7badcd305eee3573f3e3c4af784b6a93177cc58e676e51"}, {"url": "chunks/56b3ad5c08694bfbda9a0996c5292bc1d6841b209c0c5f804de83a0d48af7260.bin", "size": 9591, "sha256": "56b3ad5c08694bfbda9a0996c5292bc1d6841b209c0c5f804de83a0d48af7260"}, {"url": "chunks/dab787526ce6c913075be1437ae08e1c6997d794b7936ea4b04b6b9fe7470d5c.bin", "size": 67064, "sha256": "dab787526ce6c913075be1437ae08e1c6997d794b7936ea4b04b6b9fe7470d5c"}, {"url": "chunks/6241c853bb91e861f800ce4317f2addd1cbe2fdcd2a3a71f5ecd8d14588da660.bin", "size": 144989, "sha256": "6241c853bb91e861f800ce4317f2addd1cbe2fdcd2a3a71f5ecd8d14588da660"}, {"url": "chunks/4cddda8fab958f4bcc5eb170ab86d5ec954d2cf5493679789288de72b6176b03.bin", "size": 7463, "sha256": "4cddda8fab958f4bcc5eb170ab86d5ec954d2cf5493679789288de72b6176b03"}, {"url": "chunks/00dc0ac42821a9327d52ca90fa96deb4573e5c02370e5b4ce85a645c4a9361e1.bin", "size": 8162, "sha256": "00dc0ac42821a9327d52ca90fa96deb4573e5c02370e5b4ce85a645c4a9361e1"}, {"url": "chunks/45310172b006018deb055a77753ea1da7d53c9fae3fcbfef0730061256d7ee51.bin", "size": 8399, "sha256": "45310172b006018deb055a77753ea1da7d53c9fae3fcbfef0730061256d7ee51"}, {"url": "chunks/69a3d0c075c8f1e64306fc6a13537fa3babdf72e99eaa056b8d8d0b60eeae738.bin", "size": 9143, "sha256": "69a3d0c075c8f1e64306fc6a13537fa3babdf72e99eaa056b8d8d0b60eeae738"}, {"url": "chunks/adc425bdb89a31eee19067868058d3ad88f0ab59058ea7ddbd47801bfd348b85.bin", "size": 10950, "sha256": "adc425bdb89a31eee19067868058d3ad88f0ab59058ea7ddbd47801bfd348b85"}, {"url": "chunks/7b4c8da55a48063696ce83afd4a1a5d03b29b6ecb3a0e47347768cd3cef98b19.bin", "size": 135910, "sha256": "7b4c8da55a48063696ce83afd4a1a5d03b29b6ecb3a0e47347768cd3cef98b19"}]}, {"url": "game.js", "size": 269502, "sha256": "ea5b19e458648e8ce680db9b865b9cf40a0a201e1b27c68627cf2649e9461a79"}, {"url": "game.wasm", "size": 1302967, "sha256": "32055404681e3be41d9d8be1c8deb88751e3d06e1e28d66a3d5075291b71aac8"}, {"url": "icon.png", "size": 1740173, "sha256": "fb089136c275acb1e9fb25c0066251870ffec0bb22d80eb2c776bceaa3517cbd"}, {"url": "index.html", "size": 2603, "sha256": "fbc8c59744f77fd792dc627dbdc233c507dbf9b08d484310054e62dd057259c2"}, {"url": "manifest.webmanifest", "size": 265, "sha256": "12869bff75f2eb5d6d3c250fc42b3cc7e9022696f0b4608ee6a85e2e051694b5"}, {"url": "social-preview.jpg", "size": 627410, "sha256": "15fddebb459273543a2ebad916f7fae99ab8bf2cd90bd242050b6c482a93f633"}, {"url": "storage.js", "size": 1864, "sha256": "f1bf0073367df291f09a6e721b5cf5629de3a06acfced595082857edb04cf38e"}, {"url": "style.css", "size": 1441, "sha256": "a2307a115f8a77ff27c86c57e8b2bae3ff1bc91f8d248f2cd7bb53dfef817b5a"}, {"url": "title.jpg", "size": 172705, "sha256": "50ce101b40757018130437f411e64dd8709234ebf649f7f8e9d555bb73cbf696"}]};
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
