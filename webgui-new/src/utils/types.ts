import { FileInfo } from '@thrift-generated';

export type RouterQueryType = {
  wsId?: string;
  projFileId?: string;
  selection?: string;
};

export type TreeNode = {
  info: FileInfo;
  children?: TreeNode[];
};

export type FileNode = {
  [key: string]: {
    expandedNodes: string[];
  };
};
