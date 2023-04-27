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
    target: process.env.NEXT_PUBLIC_BACKEND_URL,
    changeOrigin: true,
  });

  app.use('/', proxyHandler);
  app.get('*', (req, res) => nextHandler(req, res));

  await nextApp.prepare();

  app.listen(port, () => console.log(`> Ready on localhost:${port}`));
};

main();
