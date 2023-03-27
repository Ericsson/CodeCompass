import { createContext, useEffect, useState } from 'react';
import { WorkspaceInfo } from '@thrift-generated';
import { createWorkspaceClient, getWorkspaces } from 'service/workspace-service';

export const WorkspaceContext = createContext<WorkspaceInfo[]>([]);

export const WorkspaceContextController = ({ children }: { children: JSX.Element | JSX.Element[] }): JSX.Element => {
  const [workspaces, setWorkspaces] = useState<WorkspaceInfo[]>([]);

  useEffect(() => {
    const init = async () => {
      createWorkspaceClient();
      const workspaceData = await getWorkspaces();
      setWorkspaces(workspaceData);
    };
    init();
  }, []);

  return <WorkspaceContext.Provider value={workspaces}>{children}</WorkspaceContext.Provider>;
};
