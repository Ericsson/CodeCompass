import thrift from 'thrift';
import { GitDiffOptions, GitService } from '@thrift-generated';
import { config } from './config';

let client: GitService.Client | undefined;
export const createGitClient = (workspace: string) => {
  if (!config) return;
  const connection = thrift.createXHRConnection(config.webserver_host, config.webserver_port, {
    transport: thrift.TBufferedTransport,
    protocol: thrift.TJSONProtocol,
    https: config.webserver_https,
    path: `${config.webserver_path}/${workspace}/GitService`,
  });
  client = thrift.createXHRClient(GitService, connection);
  return client;
};

export const isRepositoryAvailable = async () => {
  if (!client) {
    return false;
  }
  try {
    return await client.isRepositoryAvailable();
  } catch {
    return false;
  }
};

export const getRepositoryList = async () => {
  if (!client) {
    return [];
  }
  try {
    return await client.getRepositoryList();
  } catch {
    return [];
  }
};

export const getRepositoryByProjectPath = async (path: string) => {
  if (!client) {
    return;
  }
  try {
    return await client.getRepositoryByProjectPath(path);
  } catch {
    return;
  }
};

export const getReferenceTopObject = async (repoId: string, branchName: string) => {
  if (!client) {
    return;
  }
  try {
    return await client.getReferenceTopObject(repoId, branchName);
  } catch {
    return;
  }
};

export const getReferenceList = async (repoId: string) => {
  if (!client) {
    return [];
  }
  try {
    return await client.getReferenceList(repoId);
  } catch {
    return [];
  }
};

export const getBranchList = async (repoId: string) => {
  if (!client) {
    return [];
  }
  try {
    return await client.getBranchList(repoId);
  } catch {
    return [];
  }
};

export const getTagList = async (repoId: string) => {
  if (!client) {
    return [];
  }
  try {
    return await client.getTagList(repoId);
  } catch {
    return [];
  }
};

export const getTag = async (repoId: string, hexId: string) => {
  if (!client) {
    return;
  }
  try {
    return await client.getTag(repoId, hexId);
  } catch {
    return;
  }
};

export const getBlameInfo = async (repoId: string, hexId: string, path: string, localModsFileId: string) => {
  if (!client) {
    return [];
  }
  try {
    return await client.getBlameInfo(repoId, hexId, path, localModsFileId);
  } catch {
    return [];
  }
};

export const getBlobContent = async (repoId: string, hexId: string) => {
  if (!client) {
    return '';
  }
  try {
    return await client.getBlobContent(repoId, hexId);
  } catch {
    return '';
  }
};

export const getBlobOidByPath = async (repoId: string, hexId: string, path: string) => {
  if (!client) {
    return '';
  }
  try {
    return await client.getBlobOidByPath(repoId, hexId, path);
  } catch {
    return '';
  }
};

export const getCommit = async (repoId: string, hexId: string) => {
  if (!client) {
    return;
  }
  try {
    return await client.getCommit(repoId, hexId);
  } catch {
    return;
  }
};

export const getCommitDiffAsString = async (
  repoId: string,
  hexId: string,
  isCompact: boolean,
  contextLines?: number,
  pathspec?: string[],
  fromCommit?: string
) => {
  if (!client) {
    return '';
  }
  try {
    return await client.getCommitDiffAsString(
      repoId,
      hexId,
      new GitDiffOptions({ contextLines, pathspec, fromCommit }),
      isCompact
    );
  } catch {
    return '';
  }
};

export const getCommitListFiltered = async (
  repoId: string,
  hexId: string,
  count: number,
  offset: number,
  filter: string
) => {
  if (!client) {
    return;
  }
  try {
    return await client.getCommitListFiltered(repoId, hexId, count, offset, filter);
  } catch {
    return;
  }
};
