import { ThemeProvider } from '@mui/material';
import React, { createContext, useEffect, useState } from 'react';
import { darkTheme, lightTheme } from 'themes/theme';
import { getStore, setStore } from 'utils/store';

/* eslint-disable no-unused-vars */
type ThemeContextType = {
  theme: 'light' | 'dark';
  setTheme: (_val: 'light' | 'dark') => void;
};

export const ThemeContext = createContext<ThemeContextType>({
  theme: 'light',
  setTheme: (_val) => {},
});
/* eslint-enable no-unused-vars */

export const ThemeContextController = ({ children }: { children: React.ReactNode }) => {
  const [theme, setTheme] = useState<'light' | 'dark' | undefined>(undefined);

  useEffect(() => {
    const { storedTheme } = getStore();
    setTheme(storedTheme ?? 'light');
  }, []);

  useEffect(() => {
    setStore({
      storedTheme: theme,
    });
  }, [theme]);

  return (
    <ThemeContext.Provider value={{ theme: theme as 'light' | 'dark', setTheme }}>
      <ThemeProvider theme={theme === 'light' ? lightTheme : darkTheme}>{children}</ThemeProvider>
    </ThemeContext.Provider>
  );
};
