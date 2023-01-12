import { CssBaseline, ThemeProvider } from '@mui/material';
import type { AppProps } from 'next/app';
import { appTheme } from '../themes/theme';
import { globalStyles } from '../themes/globals';
import { ProSidebarProvider } from 'react-pro-sidebar';

const App = ({ Component, pageProps }: AppProps): JSX.Element => {
  return (
    <ThemeProvider theme={appTheme}>
      <style jsx global>
        {globalStyles}
      </style>
      <CssBaseline />
      <ProSidebarProvider>
        <Component {...pageProps} />
      </ProSidebarProvider>
    </ThemeProvider>
  );
};

export default App;
