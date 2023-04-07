import { createContext, useContext, useEffect, useState } from 'react';
import { FileInfo } from '@thrift-generated';
import { WorkspaceContext } from './workspace-context';
import { createProjectClient, getFileContent, getFileInfo, getRootFiles } from 'service/project-service';
import { getStore, setStore } from 'utils/store';
import { useRouter } from 'next/router';
import { RouterQueryType } from 'utils/types';

type ProjectContextType = {
  currentWorkspace: string;
  setCurrentWorkspace: (_val: string) => void;
  fileContent: string;
  setFileContent: (_val: string) => void;
  fileInfo: FileInfo | undefined;
  setFileInfo: (_val: FileInfo | undefined) => void;
  loadComplete: boolean;
  setLoadComplete: (_val: boolean) => void;
};

export const ProjectContext = createContext<ProjectContextType>({
  currentWorkspace: '',
  setCurrentWorkspace: (_val) => {},
  fileContent: '',
  setFileContent: (_val) => {},
  fileInfo: undefined,
  setFileInfo: (_val) => {},
  loadComplete: false,
  setLoadComplete: (_val) => {},
});

export const ProjectContextController = ({ children }: { children: JSX.Element | JSX.Element[] }): JSX.Element => {
  const router = useRouter();
  const routerQuery = router.query as RouterQueryType;

  const workspaces = useContext(WorkspaceContext);

  const [currentWorkspace, setCurrentWorkspace] = useState<string | undefined>(undefined);
  const [fileContent, setFileContent] = useState<string | undefined>(undefined);
  const [fileInfo, setFileInfo] = useState<FileInfo | undefined>(undefined);
  const [loadComplete, setLoadComplete] = useState<boolean>(false);

  useEffect(() => {
    if (!workspaces.length) {
      return;
    }
    const initWorkspace = async () => {
      const { storedWorkspace } = getStore();
      setCurrentWorkspace(storedWorkspace ?? (workspaces[0].id as string));
    };
    initWorkspace();
  }, [workspaces]);

  useEffect(() => {
    if (!routerQuery.wsId) return;
    setCurrentWorkspace(routerQuery.wsId);
  }, [routerQuery.wsId]);

  useEffect(() => {
    if (!currentWorkspace) return;

    setLoadComplete(false);

    const init = async () => {
      createProjectClient(currentWorkspace);

      // This is only temporary - init() has to await something so the searchCtx does not start rendering before the projectCtx. (This can cause problems.)
      // TODO: Find a better solution for this!
      const _ = await getRootFiles();

      const { storedFileContent, storedFileInfo } = getStore();

      setFileContent(storedFileContent ?? '');
      setFileInfo(storedFileInfo ?? undefined);

      if (routerQuery.projFileId) {
        try {
          const initFileInfo = await getFileInfo(routerQuery.projFileId);
          const fileContent = await getFileContent(routerQuery.projFileId);

          setFileContent(fileContent);
          setFileInfo(initFileInfo);
        } catch {}
      }

      setStore({
        storedWorkspace: currentWorkspace,
      });
    };
    init().then(() => setLoadComplete(true));
  }, [currentWorkspace, routerQuery.projFileId]);

  useEffect(() => {
    setStore({
      storedFileInfo: fileInfo,
      storedFileContent: fileContent,
    });
  }, [fileContent, fileInfo]);

  const projectContext = {
    currentWorkspace: currentWorkspace as string,
    fileContent: fileContent as string,
    fileInfo: fileInfo as FileInfo,
    loadComplete,
    setCurrentWorkspace,
    setFileContent,
    setFileInfo,
    setLoadComplete,
  };

  return <ProjectContext.Provider value={projectContext}>{children}</ProjectContext.Provider>;
};
