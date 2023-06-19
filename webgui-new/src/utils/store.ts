import { FileInfo, FileSearchResult, SearchResult, SearchType } from '@thrift-generated';
import { FileNode, SearchProps, TreeNode } from './types';

type StoreOptions = {
  storedTheme?: 'light' | 'dark';
  storedSearchProps?: SearchProps;
  storedSearchType?: SearchType;
  storedSearchLanguage?: string;
  storedSearchOtherLanguage?: string;
  storedSelectedSearchTypeOptions?: string[];
  storedSelectedSearchResult?: string;
  storedExpandedFileTreeNodes?: string[];
  storedExpandedSearchPathNodes?: string[];
  storedExpandedSearchFileNodes?: FileNode;
};

type StoreOptionKey =
  | 'storedTheme'
  | 'storedSearchProps'
  | 'storedSearchType'
  | 'storedSearchLanguage'
  | 'storedSearchOtherLanguage'
  | 'storedSelectedSearchTypeOptions'
  | 'storedSelectedSearchResult'
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
  | SearchProps
  | FileNode;

// eslint-disable-next-line no-unused-vars
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
