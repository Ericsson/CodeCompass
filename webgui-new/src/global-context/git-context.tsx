import { createContext, useContext, useEffect, useState } from 'react';
import { createGitClient } from 'service/git-service';
import { ProjectContext } from './project-context';
import { getStore, setStore } from 'utils/store';

type GitContextType = {
  repoId: string;
  setRepoId: (_val: string) => void;
  commitId: string;
  setCommitId: (_val: string) => void;
  branch: string;
  setBranch: (_val: string) => void;
};

export const GitContext = createContext<GitContextType>({
  repoId: '',
  setRepoId: (_val) => {},
  commitId: '',
  setCommitId: (_val) => '',
  branch: '',
  setBranch: (_val) => {},
});

export const GitContextController = ({ children }: { children: JSX.Element | JSX.Element[] }): JSX.Element => {
  const projectCtx = useContext(ProjectContext);

  const [repoId, setRepoId] = useState<string | undefined>(undefined);
  const [commitId, setCommitId] = useState<string | undefined>(undefined);
  const [branch, setBranch] = useState<string | undefined>(undefined);

  useEffect(() => {
    if (!projectCtx.currentWorkspace) {
      return;
    }
    const init = async () => {
      createGitClient(projectCtx.currentWorkspace);

      const { storedGitRepoId, storedGitCommitId, storedGitBranch } = getStore();

      setRepoId(storedGitRepoId);
      setCommitId(storedGitCommitId);
      setBranch(storedGitBranch);
    };
    init();
  }, [projectCtx.currentWorkspace]);

  useEffect(() => {
    setStore({
      storedGitRepoId: repoId,
      storedGitCommitId: commitId,
      storedGitBranch: branch,
    });
  }, [repoId, commitId, branch]);

  const gitContext = {
    repoId: repoId as string,
    commitId: commitId as string,
    branch: branch as string,
    setRepoId,
    setCommitId,
    setBranch,
  };

  return <GitContext.Provider value={gitContext}>{children}</GitContext.Provider>;
};
