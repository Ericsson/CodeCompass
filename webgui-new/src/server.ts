import dotenv from 'dotenv';
import express from 'express';
import next from 'next';
import { createProxyMiddleware } from 'http-proxy-middleware';

dotenv.config({ path: '.env.local' });
dotenv.config();

const main = async () => {
  const app = express();
  const port = parseInt(process.env.DEVSERVER_PORT as string);
  const nextApp = next({ dev: process.env.NODE_ENV !== 'production' });

  const nextHandler = nextApp.getRequestHandler();

  const proxyHandler = createProxyMiddleware({
    target: process.env.BACKEND_URL,
    changeOrigin: true,
    pathFilter: ['**/*Service', '**/ga.txt'],
    pathRewrite: {
      '^/[^/]+/(.+Service)$': '/$1',
      '^/ga.txt' : '/ga.txt'
    },
  });

  let API_PREFIX = process.env.API_PREFIX || "/";

  if (!API_PREFIX.startsWith("/")) {
    API_PREFIX = `/${API_PREFIX}`;
  }

  if (/^https?:\/\//.test(API_PREFIX)) {
    console.warn(
      `[WARN] API_PREFIX appears to be a full URL (${API_PREFIX}). Expected a path like "/api". Using "/" instead.`
    );
    API_PREFIX = "/";
  }

  app.use(API_PREFIX, proxyHandler);
  app.get('*', (req, res) => nextHandler(req, res));

  await nextApp.prepare();

  app.listen(port, () => console.log(`> Ready on localhost:${port}`));
};

main();
