import 'themes/globals.scss';
import type { AppProps } from 'next/app';
import { CssBaseline } from '@mui/material';
import { WorkspaceContextController } from 'global-context/workspace-context';
import { ProjectContextController } from 'global-context/project-context';
import { GitContextController } from 'global-context/git-context';
import { ThemeContextController } from 'global-context/theme-context';
import { ConfigContextController } from 'global-context/config-context';
import { SearchContextController } from 'global-context/search-context';
import { MetricsContextController } from 'global-context/metrics-context';
import { LanguageContextController } from 'global-context/language-context';

const App = ({ Component, pageProps }: AppProps): JSX.Element => {
  return (
    <WorkspaceContextController>
      <ProjectContextController>
        <SearchContextController>
          <LanguageContextController>
            <MetricsContextController>
              <GitContextController>
                <ConfigContextController>
                  <ThemeContextController>
                    <CssBaseline />
                    <Component {...pageProps} />
                  </ThemeContextController>
                </ConfigContextController>
              </GitContextController>
            </MetricsContextController>
          </LanguageContextController>
        </SearchContextController>
      </ProjectContextController>
    </WorkspaceContextController>
  );
};

export default App;
