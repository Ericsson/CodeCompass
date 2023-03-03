import thrift from 'thrift';
import { GitService } from '@thrift-generated/index';
import { config } from './config';

let client: GitService.Client | undefined;
export const createGitClient = (workspace: string) => {
  const connection = thrift.createXHRConnection(config.webserver_host, config.webserver_port, {
    transport: thrift.TBufferedTransport,
    protocol: thrift.TJSONProtocol,
    https: config.webserver_https,
    path: `${config.webserver_path}/${workspace}/GitService`,
  });
  client = thrift.createXHRClient(GitService, connection);
  return client;
};

export const getRepositories = async () => {
  if (!client) {
    return [];
  }
  const repositories = await client.getRepositoryList();
  return repositories;
};

export const getBranchList = async (repoId: string) => {
  if (!client) {
    return [];
  }
  const branches = await client.getBranchList(repoId);
  return branches;
};
