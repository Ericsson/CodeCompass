import Link from 'next/link';
import { useContext } from 'react';
import { WorkspaceContext } from '../global-context/workspace-context';

const Home = () => {
  const workspaces = useContext(WorkspaceContext);

  return (
    <ul>
      {workspaces.map((ws) => (
        <li key={ws.id}>
          <Link href={`/project/${ws.id}`}>{ws.description}</Link>
        </li>
      ))}
    </ul>
  );
};

export default Home;
