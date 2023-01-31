import '../themes/globals.scss';
import { ThemeContext } from '../themes/theme-context';
import { lightTheme, darkTheme } from '../themes/theme';
import { CssBaseline, ThemeProvider } from '@mui/material';
import type { AppProps } from 'next/app';
import { useEffect, useState } from 'react';
import { WorkspaceInfo } from '../../build/cc/service/workspace';
import { getWorkspaces } from '../service/workspace-service';
import { WorkspaceContext } from '../global-context/workspace-context';

const App = ({ Component, pageProps }: AppProps): JSX.Element => {
  const [theme, setTheme] = useState('dark');
  const [workspaces, setWorkspaces] = useState<WorkspaceInfo[]>([]);

  useEffect(() => {
    const getWorkspaceData = async () => {
      const workspaceData = await getWorkspaces();
      setWorkspaces(workspaceData);
    };
    getWorkspaceData();
  }, []);

  return (
    <WorkspaceContext.Provider value={workspaces}>
      <ThemeContext.Provider value={{ theme, setTheme }}>
        <ThemeProvider theme={theme === 'dark' ? darkTheme : lightTheme}>
          <CssBaseline />

          <Component {...pageProps} />
        </ThemeProvider>
      </ThemeContext.Provider>
    </WorkspaceContext.Provider>
  );
};

export default App;
