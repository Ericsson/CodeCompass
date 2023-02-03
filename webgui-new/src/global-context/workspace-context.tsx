import { createContext } from 'react';
import { WorkspaceInfo } from '@thrift-generated/workspace_types';

export const WorkspaceContext = createContext<WorkspaceInfo[]>([]);
