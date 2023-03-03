import { createContext, useContext, useEffect, useState } from 'react';
import { createGitClient, getBranchList, getRepositories } from 'service/git-service';
import { ProjectContext } from './project-context';

type GitContextType = {
  repository: string;
  setRepository: (_val: string) => void;
  branches: string[];
  setBranches: (_val: string[]) => void;
};

export const GitContext = createContext<GitContextType>({
  repository: '',
  setRepository: (_val) => {},
  branches: [],
  setBranches: (_val) => {},
});

export const GitContextController = ({ children }: { children: JSX.Element | JSX.Element[] }): JSX.Element => {
  const projectCtx = useContext(ProjectContext);
  const [repository, setRepository] = useState<string>('');
  const [branches, setBranches] = useState<string[]>([]);

  useEffect(() => {
    if (!projectCtx.currentWorkspace) {
      return;
    }
    const init = async () => {
      createGitClient(projectCtx.currentWorkspace);

      const repositories = await getRepositories();
      if (!repositories.length) {
        return;
      }

      const repoId = repositories[0].id as string;
      setRepository(repoId);

      const branches = await getBranchList(repoId);
      setBranches(branches);
    };
    init();
  }, [projectCtx.currentWorkspace]);

  const gitContext = {
    repository,
    setRepository,
    branches,
    setBranches,
  };

  return <GitContext.Provider value={gitContext}>{children}</GitContext.Provider>;
};
