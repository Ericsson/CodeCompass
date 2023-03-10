import 'themes/globals.scss';
import type { AppProps } from 'next/app';
import { CssBaseline } from '@mui/material';
import { WorkspaceContextController } from 'global-context/workspace-context';
import { ProjectContextController } from 'global-context/project-context';
import { ThemeContextController } from 'global-context/theme-context';
import { ConfigContextController } from 'global-context/config-context';
import { SearchContextController } from 'global-context/search-context';

const App = ({ Component, pageProps }: AppProps): JSX.Element => {
  return (
    <SearchContextController>
      <ProjectContextController>
        <WorkspaceContextController>
          <ConfigContextController>
            <ThemeContextController>
              <CssBaseline />
              <Component {...pageProps} />
            </ThemeContextController>
          </ConfigContextController>
        </WorkspaceContextController>
      </ProjectContextController>
    </SearchContextController>
  );
};

export default App;
