import { ProjectContext } from 'global-context/project-context';
import { useRouter } from 'next/router';
import { useContext } from 'react';
import { WorkspaceContext } from '../global-context/workspace-context';

const Home = () => {
  const router = useRouter();
  const workspaces = useContext(WorkspaceContext);
  const projectCtx = useContext(ProjectContext);

  const loadWorkspace = (workspace: string) => {
    localStorage.removeItem('currentFiles');
    localStorage.removeItem('currentPath');
    localStorage.removeItem('currentFileContent');
    localStorage.removeItem('currentFileInfo');
    localStorage.removeItem('currentSelectedFile');
    projectCtx.setCurrentWorkspace(workspace);
    router.push('/project');
  };

  return (
    <div>
      {workspaces.map((ws) => (
        <div key={ws.id} onClick={() => loadWorkspace(ws.id as string)}>
          {ws.id}
        </div>
      ))}
    </div>
  );
};

export default Home;
