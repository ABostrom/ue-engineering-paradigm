#!/usr/bin/env node
/**
 * check-citations.mjs
 *
 * Every engine path the skill cites (Engine/Source/..., Engine/Plugins/..., Engine/Config/...)
 * must exist in a real engine tree. This keeps the skill's claims honest and doubles as the
 * upgrade check when re-targeting a new engine version: anything that moved fails at once.
 *
 * It checks that the cited FILE exists. It does not prove the claim made about it; when a
 * citation fails, re-read the claim against the new source, do not just fix the path.
 *
 * Usage:
 *   UE_ENGINE_ROOT=/path/to/UE node scripts/check-citations.mjs
 *   node scripts/check-citations.mjs --engine-root /path/to/UE [subdir-under-skills]
 *
 * UE_ENGINE_ROOT / --engine-root is the directory that CONTAINS Engine/ (an installed build with
 * source, or a source checkout).
 *
 * Exit codes: 0 all citations exist; 1 at least one missing; 2 engine root not given or not found.
 */

import { readdir, readFile, stat } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import path from 'node:path';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const repoRoot = path.resolve(__dirname, '..');
const skillsRoot = path.join(repoRoot, 'skills');

const args = process.argv.slice(2);
let engineRoot = process.env.UE_ENGINE_ROOT?.trim() || '';
let subdir = '';
for (let i = 0; i < args.length; i++) {
  if (args[i] === '--engine-root') engineRoot = args[++i] ?? '';
  else subdir = args[i];
}

// Engine/Source|Plugins|Config/... ending in a source or config extension.
const CITATION_RE =
  /Engine[\\/](?:Source|Plugins|Config)[\\/][A-Za-z0-9_.\-\\/]+?\.(?:h|hpp|cpp|inl|cs|ini|usf|ush)\b/g;

async function exists(p, kind) {
  try {
    const s = await stat(p);
    return kind === 'dir' ? s.isDirectory() : s.isFile();
  } catch {
    return false;
  }
}

async function findFiles(dir) {
  const out = [];
  for (const e of await readdir(dir, { withFileTypes: true })) {
    const full = path.join(dir, e.name);
    if (e.isDirectory()) out.push(...(await findFiles(full)));
    else if (/\.(md|h|cpp)$/i.test(e.name)) out.push(full);
  }
  return out;
}

async function main() {
  if (!engineRoot) {
    console.error('[check-citations] No engine root. Set UE_ENGINE_ROOT or pass --engine-root <dir containing Engine/>.');
    process.exit(2);
  }
  if (!(await exists(path.join(engineRoot, 'Engine'), 'dir'))) {
    console.error(`[check-citations] Not an engine root (no Engine/ inside): ${engineRoot}`);
    process.exit(2);
  }

  const scanRoot = subdir ? path.join(skillsRoot, subdir) : skillsRoot;
  const files = await findFiles(scanRoot);

  const cache = new Map();
  const failures = [];
  let checked = 0;

  for (const file of files) {
    const text = await readFile(file, 'utf8');
    const cited = [...new Set((text.match(CITATION_RE) || []).map((m) => m.replace(/\\/g, '/')))];
    const missing = [];
    for (const c of cited) {
      checked++;
      if (!cache.has(c)) cache.set(c, await exists(path.join(engineRoot, ...c.split('/')), 'file'));
      if (!cache.get(c)) missing.push(c);
    }
    if (missing.length) failures.push({ file: path.relative(repoRoot, file), missing });
  }

  console.log(`[check-citations] Files scanned: ${files.length}`);
  console.log(`[check-citations] Citations checked: ${checked} (${cache.size} unique)`);

  if (!failures.length) {
    console.log('[check-citations] OK - every cited engine path exists.');
    return;
  }
  for (const f of failures) {
    console.log(`  ${f.file}`);
    for (const m of f.missing) console.log(`    - ${m}`);
  }
  process.exit(1);
}

main().catch((err) => {
  console.error('[check-citations] Failed:', err);
  process.exit(1);
});
