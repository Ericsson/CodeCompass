import { createContext } from 'react';
import { WorkspaceInfo } from '@thrift-generated/index';

export const WorkspaceContext = createContext<WorkspaceInfo[]>([]);
