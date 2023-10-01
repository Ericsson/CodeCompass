import { ThemeProvider, useMediaQuery } from '@mui/material';
import React, { createContext, useEffect, useState } from 'react';
import { darkTheme, lightTheme } from 'themes/theme';
import { getStore, setStore } from 'utils/store';
import { ToastContainer } from 'react-toastify';
import 'react-toastify/dist/ReactToastify.css';

/* eslint-disable no-unused-vars */
/* eslint-disable @typescript-eslint/no-empty-function */
/* eslint-disable @typescript-eslint/no-unused-vars */
type ThemeContextType = {
  theme: 'light' | 'dark';
  setTheme: (_val: 'light' | 'dark') => void;
};

export const ThemeContext = createContext<ThemeContextType>({
  theme: 'light',
  setTheme: (_val) => {},
});
/* eslint-enable no-unused-vars */
/* eslint-enable @typescript-eslint/no-empty-function */
/* eslint-enable @typescript-eslint/no-unused-vars */

export const ThemeContextController = ({ children }: { children: React.ReactNode }): JSX.Element => {
  const [theme, setTheme] = useState<'light' | 'dark' | undefined>(undefined);
  const prefersDarkMode = useMediaQuery('(prefers-color-scheme: dark)');

  useEffect(() => {
    const { storedTheme } = getStore();
    setTheme(storedTheme ?? (prefersDarkMode ? 'dark' : 'light'));
  }, [prefersDarkMode]);

  useEffect(() => {
    setStore({
      storedTheme: theme,
    });
  }, [theme]);

  return (
    <ThemeContext.Provider value={{ theme: theme as 'light' | 'dark', setTheme }}>
      <ThemeProvider theme={theme === 'light' ? lightTheme : darkTheme}>
        <>{children}</>
        <ToastContainer theme={theme} position={'top-right'} />
      </ThemeProvider>
    </ThemeContext.Provider>
  );
};
