import Link from 'next/link';
import { useContext } from 'react';
import { WorkspaceContext } from '../global-context/workspace-context';

const Home = () => {
  const workspaces = useContext(WorkspaceContext);

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
