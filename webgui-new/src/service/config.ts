import getConfig from 'next/config';

const { publicRuntimeConfig } = getConfig();

export const config = {
  webserver_host: publicRuntimeConfig.WEBSERVER_HOST,
  webserver_port: publicRuntimeConfig.WEBSERVER_PORT,
};
