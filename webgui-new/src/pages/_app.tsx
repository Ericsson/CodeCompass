import 'themes/globals.scss';
import type { AppProps } from 'next/app';
import { CssBaseline } from '@mui/material';
import { AppContextController } from 'global-context/app-context';
import { ThemeContextController } from 'global-context/theme-context';

const App = ({ Component, pageProps }: AppProps): JSX.Element => {
  return (
    <AppContextController>
      <ThemeContextController>
        <CssBaseline />
        <Component {...pageProps} />
      </ThemeContextController>
    </AppContextController>
  );
};

export default App;
