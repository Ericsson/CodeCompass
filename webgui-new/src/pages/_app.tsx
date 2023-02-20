import '../themes/globals.scss';
import { ThemeContext } from '../themes/theme-context';
import { lightTheme, darkTheme } from '../themes/theme';
import { CssBaseline, ThemeProvider } from '@mui/material';
import type { AppProps } from 'next/app';
import { useEffect, useState } from 'react';
import { getWorkspaces } from '../service/workspace-service';
import { WorkspaceContext } from '../global-context/workspace-context';
import { WorkspaceInfo } from '@thrift-generated/index';
import { ProjectContext } from '../global-context/project-context';
import { FileInfo } from '@thrift-generated/index';

const App = ({ Component, pageProps }: AppProps): JSX.Element => {
  const [theme, setTheme] = useState('light');
  const [workspaces, setWorkspaces] = useState<WorkspaceInfo[]>([]);

  const [currentWorkspace, setCurrentWorkspace] = useState<string>('');
  const [rootFiles, setRootFiles] = useState<FileInfo[]>([]);
  const [files, setFiles] = useState<FileInfo[]>([]);
  const [fileContent, setFileContent] = useState<string>('');
  const [fileInfo, setFileInfo] = useState<FileInfo | undefined>(undefined);
  const [selectedFile, setSelectedFile] = useState<string>('');
  const [folderPath, setFolderPath] = useState<string>('');
  const [expandedFileTreeNodes, setExpandedFileTreeNodes] = useState<string[]>([]);
  const [projectLoadComplete, setProjectLoadComplete] = useState<boolean>(false);

  useEffect(() => {
    const getWorkspaceData = async () => {
      const workspaceData = await getWorkspaces();
      setWorkspaces(workspaceData);
      setCurrentWorkspace(workspaceData[0].id as string);

      const storedCurrentWorkspace = localStorage.getItem('currentWorkspace');
      if (storedCurrentWorkspace) {
        setCurrentWorkspace(storedCurrentWorkspace);
      }

      const storedTheme = localStorage.getItem('theme');
      if (storedTheme) {
        setTheme(storedTheme);
      }
    };
    getWorkspaceData();
  }, []);

  const projectContext = {
    currentWorkspace,
    setCurrentWorkspace,
    rootFiles,
    setRootFiles,
    files,
    setFiles,
    fileContent,
    setFileContent,
    fileInfo,
    setFileInfo,
    selectedFile,
    setSelectedFile,
    folderPath,
    setFolderPath,
    expandedFileTreeNodes,
    setExpandedFileTreeNodes,
    projectLoadComplete,
    setProjectLoadComplete,
  };

  return (
    <WorkspaceContext.Provider value={workspaces}>
      <ProjectContext.Provider value={projectContext}>
        <ThemeContext.Provider value={{ theme, setTheme }}>
          <ThemeProvider theme={theme === 'dark' ? darkTheme : lightTheme}>
            <CssBaseline />
            <Component {...pageProps} />
          </ThemeProvider>
        </ThemeContext.Provider>
      </ProjectContext.Provider>
    </WorkspaceContext.Provider>
  );
};

export default App;
