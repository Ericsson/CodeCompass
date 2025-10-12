/* eslint-disable @typescript-eslint/no-var-requires */

require('dotenv').config();

/** @type {import('next').NextConfig} */
const nextConfig = {
  output: 'export',
  reactStrictMode: true,
  modularizeImports: {
    '@mui/icons-material': {
      transform: '@mui/icons-material/{{member}}',
    },
  },
  trailingSlash: true,
  ...(process.env.NODE_ENV === 'production' && {
    assetPrefix: `${process.env.PUBLIC_URL || ''}/new`,
    basePath: `${process.env.PUBLIC_URL || ''}/new`,
  }),
};

module.exports = nextConfig;
