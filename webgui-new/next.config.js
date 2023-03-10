require('dotenv').config();

/** @type {import('next').NextConfig} */
const nextConfig = {
  reactStrictMode: true,
  modularizeImports: {
    '@mui/icons-material': {
      transform: '@mui/icons-material/{{member}}',
    },
  },
  publicRuntimeConfig: {
    DEV_ENV: process.env.DEV_ENV || false,
    WEBSERVER_HOST: process.env.WEBSERVER_HOST || 'localhost',
    WEBSERVER_PORT: process.env.WEBSERVER_PORT || 8080,
    WEBSERVER_HTTPS: process.env.WEBSERVER_HTTPS || false,
    WEBSERVER_PATH: process.env.WEBSERVER_PATH || '',
  },
  trailingSlash: true,
  ...(process.env.NODE_ENV === 'production' && {
    assetPrefix: '/new',
    basePath: '/new',
  }),
};

module.exports = nextConfig;
