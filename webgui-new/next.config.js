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
    DEV_ENV: process.env.NODE_ENV !== 'production',
    BACKEND_URL: process.env.BACKEND_URL || 'http://localhost:8080',
  },
  trailingSlash: true,
  ...(process.env.NODE_ENV === 'production' && {
    assetPrefix: '/new',
    basePath: '/new',
  }),
};

module.exports = nextConfig;
