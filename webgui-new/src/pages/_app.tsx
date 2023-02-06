import '../themes/globals.scss';
import { ThemeContext } from '../themes/theme-context';
import { lightTheme, darkTheme } from '../themes/theme';
import { CssBaseline, ThemeProvider } from '@mui/material';
import type { AppProps } from 'next/app';
import { useEffect, useState } from 'react';
import { getWorkspaces } from '../service/workspace-service';
import { WorkspaceContext } from '../global-context/workspace-context';
import { WorkspaceInfo } from '@thrift-generated/workspace_types';
import { ProjectContext } from '../global-context/project-context';
import { FileInfo } from '@thrift-generated/project_types';

const App = ({ Component, pageProps }: AppProps): JSX.Element => {
  const [theme, setTheme] = useState('dark');
  const [workspaces, setWorkspaces] = useState<WorkspaceInfo[]>([]);
  const [fileContent, setFileContent] = useState<string | undefined>();
  const [fileInfo, setFileInfo] = useState<FileInfo | undefined>();

  useEffect(() => {
    const getWorkspaceData = async () => {
      const workspaceData = await getWorkspaces();
      setWorkspaces(workspaceData);
    };
    getWorkspaceData();
  }, []);

  return (
    <WorkspaceContext.Provider value={workspaces}>
      <ProjectContext.Provider value={{ fileContent, setFileContent, fileInfo, setFileInfo }}>
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
