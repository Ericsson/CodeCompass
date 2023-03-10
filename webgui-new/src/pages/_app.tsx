import 'themes/globals.scss';
import type { AppProps } from 'next/app';
import { CssBaseline } from '@mui/material';
import { WorkspaceContextController } from 'global-context/workspace-context';
import { ProjectContextController } from 'global-context/project-context';
import { ThemeContextController } from 'global-context/theme-context';
import { ConfigContextController } from 'global-context/config-context';

const App = ({ Component, pageProps }: AppProps): JSX.Element => {
  return (
    <ConfigContextController>
      <WorkspaceContextController>
        <ProjectContextController>
          <ThemeContextController>
            <CssBaseline />
            <Component {...pageProps} />
          </ThemeContextController>
        </ProjectContextController>
      </WorkspaceContextController>
    </ConfigContextController>
  );
};

export default App;
