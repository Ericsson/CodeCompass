import { CssBaseline, ThemeProvider } from '@mui/material';
import type { AppProps } from 'next/app';
import { lightTheme, darkTheme } from '../themes/theme';
import { globalStyles } from '../themes/globals';
import { ProSidebarProvider } from 'react-pro-sidebar';
import { useState } from 'react';
import { ThemeContext } from '../themes/theme-context';

const App = ({ Component, pageProps }: AppProps): JSX.Element => {
  const [theme, setTheme] = useState('dark');

  return (
    <ThemeContext.Provider value={{ theme, setTheme }}>
      <ThemeProvider theme={theme === 'dark' ? darkTheme : lightTheme}>
        <style jsx global>
          {globalStyles}
        </style>
        <CssBaseline />
        <ProSidebarProvider>
          <Component {...pageProps} />
        </ProSidebarProvider>
      </ThemeProvider>
    </ThemeContext.Provider>
  );
};

export default App;
