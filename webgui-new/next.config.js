require('dotenv').config();

/** @type {import('next').NextConfig} */
const nextConfig = {
  reactStrictMode: true,
  modularizeImports: {
    '@mui/icons-material': {
      transform: '@mui/icons-material/{{member}}',
    },
  },
  trailingSlash: true,
  ...(process.env.NODE_ENV === 'production' && {
    assetPrefix: '/new',
    basePath: '/new',
  }),
};

module.exports = nextConfig;
