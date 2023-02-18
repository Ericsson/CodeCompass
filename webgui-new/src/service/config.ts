import getConfig from 'next/config';

const { publicRuntimeConfig } = getConfig();

export const config = {
  webserver_host: publicRuntimeConfig.WEBSERVER_HOST as string,
  webserver_port: publicRuntimeConfig.WEBSERVER_PORT as number,
  webserver_https: publicRuntimeConfig.WEBSERVER_HTTPS as boolean,
  webserver_path: publicRuntimeConfig.WEBSERVER_PATH as string,
};
