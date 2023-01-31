import { createContext } from 'react';
import { WorkspaceInfo } from '../../build/cc/service/workspace';

export const WorkspaceContext = createContext<WorkspaceInfo[]>([]);
