import { FileInfo, FileSearchResult, SearchResult, SearchType } from '@thrift-generated';

type TreeNode = {
  info: FileInfo;
  children?: TreeNode[];
};

type FileNodesType = {
  [key: string]: {
    expandedNodes: string[];
  };
};

type StoreOptions = {
  storedWorkspace?: string;
  storedTheme?: 'light' | 'dark';
  storedActiveAccordion?: string;
  storedActiveTab?: number;
  storedTreeViewOption?: boolean;
  storedSelectedFile?: string;
  storedFileInfo?: FileInfo;
  storedFileContent?: string;
  storedFileTree?: TreeNode;
  storedFiles?: FileInfo[];
  storedFolderPath?: string;
  storedSearchResults?: SearchResult | FileSearchResult;
  storedIsFileSearch?: boolean;
  storedSearchOption?: SearchType;
  storedSearchStart?: number;
  storedSearchSize?: number;
  storedSearchQuery?: string;
  storedSearchFileFilterQuery?: string;
  storedSearchDirFilterQuery?: string;
  storedExpandedFileTreeNodes?: string[];
  storedExpandedSearchPathNodes?: string[];
  storedExpandedSearchFileNodes?: FileNodesType;
};

type StoreOptionKey =
  | 'storedWorkspace'
  | 'storedTheme'
  | 'storedActiveAccordion'
  | 'storedActiveTab'
  | 'storedTreeViewOption'
  | 'storedSelectedFile'
  | 'storedFileInfo'
  | 'storedFileContent'
  | 'storedFileTree'
  | 'storedFiles'
  | 'storedFolderPath'
  | 'storedSearchResults'
  | 'storedIsFileSearch'
  | 'storedSearchOption'
  | 'storedSearchStart'
  | 'storedSearchSize'
  | 'storedSearchQuery'
  | 'storedSearchFileFilterQuery'
  | 'storedSearchDirFilterQuery'
  | 'storedExpandedFileTreeNodes'
  | 'storedExpandedSearchPathNodes'
  | 'storedExpandedSearchFileNodes';

type StoreOptionValue =
  | string
  | string[]
  | number
  | boolean
  | FileInfo
  | FileInfo[]
  | TreeNode
  | SearchResult
  | FileSearchResult
  | SearchType
  | FileNodesType;

type StoreObject = { [key in keyof StoreOptions]?: StoreOptionValue };

export const setStore = (options: StoreOptions) => {
  const ccStoreObj: StoreObject = JSON.parse(localStorage.getItem('cc_store') ?? '{}');
  for (const [key, value] of Object.entries(options)) {
    if (value !== undefined) {
      ccStoreObj[key as StoreOptionKey] = value;
    }
  }
  localStorage.setItem('cc_store', JSON.stringify(ccStoreObj));
};

export const removeStore = (keys: StoreOptionKey[]) => {
  const ccStore = localStorage.getItem('cc_store');
  if (!ccStore) {
    return;
  }
  const ccStoreObj: StoreObject = JSON.parse(ccStore);
  for (const key of keys) {
    delete ccStoreObj[key];
  }
  localStorage.setItem('cc_store', JSON.stringify(ccStoreObj));
};

export const getStore = (): StoreOptions => {
  const ccStore = localStorage.getItem('cc_store');
  if (!ccStore) {
    return {};
  }
  const ccStoreObj: StoreObject = JSON.parse(ccStore);
  return ccStoreObj as StoreOptions;
};
