# Download counter

`worker.js` is a Cloudflare Worker that counts firmware installs made from
resonik.mikroutik.com and adds the GitHub release download counts.

- Counted: each request for `firmware/<tag>/<variant>/ats-mini.ino.bin`, which the browser
  installer fetches once, after the user has picked the port and confirmed.
- `https://resonik.mikroutik.com/api/stats`: all numbers as JSON (the website shows them under the install buttons).
- `https://resonik.mikroutik.com/api/badge?type=web|github|total`: shields.io badges used in the README.

Setup in the Cloudflare dashboard (one time):

1. Storage & Databases → KV → Create namespace `resonik-counts`.
2. Workers & Pages → Create → Worker, name `resonik-counter`, Deploy, then Edit code:
   paste `worker.js`, Deploy.
3. Worker → Settings → Bindings → Add → KV namespace: variable `COUNTS`, namespace `resonik-counts`.
4. Worker → Settings → Domains & Routes → Add route (zone mikroutik.com), twice:
   `resonik.mikroutik.com/firmware/*` and `resonik.mikroutik.com/api/*`.
5. Optional: Settings → Variables and Secrets → Add secret `GITHUB_TOKEN`
   (a fine-grained token with no permissions is enough) to avoid GitHub's anonymous rate limit.

Or with wrangler: create the namespace, paste its id into `wrangler.toml`, `npx wrangler deploy`.

KV is not transactional: two installs in the same instant may count as one. Fine at this scale.
