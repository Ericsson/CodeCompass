const { replaceInFileSync } = require('replace-in-file');

const PLACEHOLDER = '/__URL_BASE_PATH__';
const NEW_BASE = process.env.APP_BASE_PATH || '/new';

console.log(`[patch-basepath] ${PLACEHOLDER} -> ${NEW_BASE}`);

function escapeRegExp(s) {
  return s.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
}

try {
  const results = replaceInFileSync({
    files: ['out/**/*.html', 'out/**/*.js', 'out/**/*.json'],
    from: new RegExp(escapeRegExp(PLACEHOLDER), 'g'),
    to: () => NEW_BASE,
  });
  console.log('[patch-basepath] modified files:', results.length);
} catch (err) {
  console.error('[patch-basepath] error:', err);
  process.exit(1);
}