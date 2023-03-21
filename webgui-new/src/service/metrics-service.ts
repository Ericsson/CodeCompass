import thrift from 'thrift';
import { MetricsService, MetricsType } from '@thrift-generated';
import { config } from './config';

let client: MetricsService.Client | undefined;
export const createMetricsClient = (workspace: string) => {
  if (!config) return;
  const connection = thrift.createXHRConnection(config.webserver_host, config.webserver_port, {
    transport: thrift.TBufferedTransport,
    protocol: thrift.TJSONProtocol,
    https: config.webserver_https,
    path: `${config.webserver_path}/${workspace}/MetricsService`,
  });
  client = thrift.createXHRClient(MetricsService, connection);
  return client;
};

export const getMetricsTypeNames = async () => {
  if (!client) {
    return [];
  }
  try {
    return await client.getMetricsTypeNames();
  } catch {
    return [];
  }
};

export const getMetrics = async (fileId: string, fileTypeFilter: string[], metricsType: MetricsType) => {
  if (!client) {
    return '';
  }
  try {
    return await client.getMetrics(fileId, fileTypeFilter, metricsType);
  } catch {
    return '';
  }
};
