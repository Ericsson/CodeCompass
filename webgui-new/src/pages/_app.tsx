import { CssBaseline, ThemeProvider } from '@mui/material';
import '../styles/globals.scss';
import type { AppProps } from 'next/app';
import { appTheme } from '../themes/theme';

const App = ({ Component, pageProps }: AppProps): JSX.Element => {
  return (
    <ThemeProvider theme={appTheme}>
      <CssBaseline />
      <Component {...pageProps} />
    </ThemeProvider>
  );
};

export default App;
