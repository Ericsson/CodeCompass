import getConfig from 'next/config';

const { publicRuntimeConfig } = getConfig();
const isDevEnvironment = publicRuntimeConfig.DEV_ENV as boolean;

type ConfigType = {
  webserver_host: string;
  webserver_port: number;
  webserver_https: boolean;
  webserver_path: string;
};

export let config: ConfigType | undefined = undefined;

export const createConfig = (props: ConfigType) => {
  if (!isDevEnvironment) {
    config = {
      webserver_host: props.webserver_host,
      webserver_port: props.webserver_port,
      webserver_https: props.webserver_https,
      webserver_path: props.webserver_path,
    };
  } else {
    config = {
      webserver_host: publicRuntimeConfig.WEBSERVER_HOST as string,
      webserver_port: publicRuntimeConfig.WEBSERVER_PORT as number,
      webserver_https: publicRuntimeConfig.WEBSERVER_HTTPS as boolean,
      webserver_path: publicRuntimeConfig.WEBSERVER_PATH as string,
    };
  }
};
