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
    WEBSERVER_HOST: process.env.WEBSERVER_HOST || 'localhost',
    WEBSERVER_PORT: process.env.WEBSERVER_PORT || 8080,
  },
};

module.exports = nextConfig;
