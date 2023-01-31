import { createContext } from 'react';
import { WorkspaceInfo } from '../../build/workspace/cc/service/workspace';

export const WorkspaceContext = createContext<WorkspaceInfo[]>([]);
