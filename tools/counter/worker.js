// RESONIK download counter: a Cloudflare Worker in front of resonik.mikroutik.com
//
// Routes (Workers Routes on the mikroutik.com zone):
//   resonik.mikroutik.com/firmware/*   counts each firmware install from the website
//   resonik.mikroutik.com/api/*        serves the numbers
// KV namespace bound as COUNTS. Optional secret GITHUB_TOKEN raises the GitHub API limit.
//
// A browser install fetches firmware/<tag>/<variant>/ats-mini.ino.bin once, only after the
// user has picked a port and confirmed, so that request is counted as one install.
//
// GET /api/stats               {"web":{"total":N,"versions":{...}},"github":{...},"total":N}
// GET /api/badge?type=web      shields.io endpoint JSON (type = web, github or total)

const REPO = "ep3mhb/resonik-ats-mini";
const APP = /^\/firmware\/(v[\w.-]+)\/(ospi|qspi)\/ats-mini\.ino\.bin$/;

export default {
  async fetch(request, env, ctx) {
    const url = new URL(request.url);

    if (url.pathname.startsWith("/api/")) return api(url, env, ctx);

    const response = await fetch(request);
    const m = url.pathname.match(APP);
    if (m && request.method === "GET" && response.ok) {
      ctx.waitUntil(Promise.all([bump(env, "web:total"), bump(env, `web:${m[1]}:${m[2]}`)]));
    }
    return response;
  },
};

async function bump(env, key) {
  const n = parseInt((await env.COUNTS.get(key)) || "0", 10);
  await env.COUNTS.put(key, String(n + 1));
}

async function webStats(env) {
  const versions = {};
  let cursor;
  do {
    const page = await env.COUNTS.list({ prefix: "web:v", cursor });
    for (const k of page.keys) versions[k.name.slice(4)] = parseInt((await env.COUNTS.get(k.name)) || "0", 10);
    cursor = page.list_complete ? undefined : page.cursor;
  } while (cursor);
  return { total: parseInt((await env.COUNTS.get("web:total")) || "0", 10), versions };
}

// GitHub release asset downloads, cached for an hour
async function githubStats(env, ctx) {
  const cache = caches.default;
  const key = new Request("https://resonik-counter.internal/github-stats");
  const hit = await cache.match(key);
  if (hit) return hit.json();

  const headers = { "User-Agent": "resonik-counter", Accept: "application/vnd.github+json" };
  if (env.GITHUB_TOKEN) headers.Authorization = `Bearer ${env.GITHUB_TOKEN}`;
  const r = await fetch(`https://api.github.com/repos/${REPO}/releases?per_page=100`, { headers });
  // GitHub refuses now and then (60 requests an hour per address without a
  // token, shared by everything on Cloudflare's address): show the last good numbers
  if (!r.ok) return (await env.COUNTS.get("github:last", "json")) || { total: null, releases: {} };

  const releases = {};
  let total = 0;
  for (const rel of await r.json()) {
    // Count firmware downloads only (.zip and OTA .bin), not source archives
    const n = rel.assets.reduce((s, a) => s + a.download_count, 0);
    releases[rel.tag_name] = n;
    total += n;
  }
  const stats = { total, releases };
  ctx.waitUntil(env.COUNTS.put("github:last", JSON.stringify(stats)));
  ctx.waitUntil(cache.put(key, new Response(JSON.stringify(stats), {
    headers: { "Cache-Control": "max-age=3600" },
  })));
  return stats;
}

async function api(url, env, ctx) {
  const web = await webStats(env);
  const github = await githubStats(env, ctx);
  const total = web.total + (github.total || 0);
  const cors = { "Access-Control-Allow-Origin": "*", "Cache-Control": "max-age=300" };

  if (url.pathname === "/api/stats") {
    return Response.json({ web, github, total }, { headers: cors });
  }
  if (url.pathname === "/api/badge") {
    const type = url.searchParams.get("type") || "total";
    const badge = {
      web: ["web installs", web.total],
      github: ["GitHub downloads", github.total ?? "?"],
      total: ["downloads", total],
    }[type] || ["downloads", total];
    return Response.json(
      { schemaVersion: 1, label: badge[0], message: String(badge[1]), color: "108AB1" },
      { headers: cors },
    );
  }
  return new Response("Not found", { status: 404, headers: cors });
}
