type ConfigType = {
  webserver_host: string;
  webserver_port: number;
  webserver_https: boolean;
  webserver_path: string;
};

export let config: ConfigType | undefined = undefined;

export const createConfig = (props: ConfigType) => {
  config = {
    webserver_host: props.webserver_host,
    webserver_port: props.webserver_port,
    webserver_https: props.webserver_https,
    webserver_path: props.webserver_path,
  };
};
