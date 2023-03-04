import 'themes/globals.scss';
import type { AppProps } from 'next/app';
import { CssBaseline } from '@mui/material';
import { WorkspaceContextController } from 'global-context/workspace-context';
import { ProjectContextController } from 'global-context/project-context';
import { ThemeContextController } from 'global-context/theme-context';
import { OtherContextController } from 'global-context/other-context';

const App = ({ Component, pageProps }: AppProps): JSX.Element => {
  return (
    <OtherContextController>
      <WorkspaceContextController>
        <ProjectContextController>
          <ThemeContextController>
            <CssBaseline />
            <Component {...pageProps} />
          </ThemeContextController>
        </ProjectContextController>
      </WorkspaceContextController>
    </OtherContextController>
  );
};

export default App;
