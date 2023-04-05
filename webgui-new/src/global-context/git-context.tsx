import { createContext, useContext, useEffect, useState } from 'react';
import { createGitClient } from 'service/git-service';
import { ProjectContext } from './project-context';
import { GitCommit } from '@thrift-generated';

type GitContextType = {
  commit: GitCommit | undefined;
  setCommit: (_val: GitCommit | undefined) => void;
  diffs: string[];
  setDiffs: (_val: string[]) => void;
};

export const GitContext = createContext<GitContextType>({
  commit: undefined,
  setCommit: (_val) => {},
  diffs: [],
  setDiffs: (_val) => {},
});

export const GitContextController = ({ children }: { children: JSX.Element | JSX.Element[] }): JSX.Element => {
  const projectCtx = useContext(ProjectContext);

  const [commit, setCommit] = useState<GitCommit | undefined>(undefined);
  const [diffs, setDiffs] = useState<string[]>([]);

  useEffect(() => {
    if (!projectCtx.currentWorkspace) {
      return;
    }
    const init = async () => {
      createGitClient(projectCtx.currentWorkspace);
    };
    init();
  }, [projectCtx.currentWorkspace]);

  const gitContext = {
    commit,
    setCommit,
    diffs,
    setDiffs,
  };

  return <GitContext.Provider value={gitContext}>{children}</GitContext.Provider>;
};
