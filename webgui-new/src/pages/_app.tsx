import 'themes/globals.scss';
import type { AppProps } from 'next/app';
import { CssBaseline } from '@mui/material';
import { AppContextController } from 'global-context/app-context';
import { ThemeContextController } from 'global-context/theme-context';
import { I18nextProvider } from 'react-i18next';
import i18n from 'i18n/i18n';
import React from 'react';
import { CookieNotice } from 'components/cookie-notice/cookie-notice';

const App = ({ Component, pageProps }: AppProps): JSX.Element => {
  return (
    <I18nextProvider i18n={i18n}>
      <AppContextController>
        <ThemeContextController>
          <CssBaseline />
          <Component {...pageProps} />
          <CookieNotice />
        </ThemeContextController>
      </AppContextController>
    </I18nextProvider>
  );
};

export default App;
