import Link from 'next/link';
import { useEffect, useState } from 'react';
import { WorkspaceInfo } from '../../build/cc/service/workspace';
import { getWorkspaces } from '../service/workspace-service';

const Home = () => {
  const [workspaces, setWorkspaces] = useState<WorkspaceInfo[]>([]);

  useEffect(() => {
    const getWorkspaceData = async () => {
      const workspaceData = await getWorkspaces();
      setWorkspaces(workspaceData);
    };
    getWorkspaceData();
  }, []);

  return (
    <div>
      {workspaces.map((ws) => (
        <Link key={ws.id} href={`/project/${ws.id}`}>
          {ws.description}
        </Link>
      ))}
    </div>
  );
};

export default Home;
