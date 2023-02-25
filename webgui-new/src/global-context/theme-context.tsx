import { ThemeProvider } from '@mui/material';
import { createContext, useEffect, useState } from 'react';
import { darkTheme, lightTheme } from 'themes/theme';

type ThemeContextType = {
  theme: string;
  setTheme: (_val: string) => void;
};

export const ThemeContext = createContext<ThemeContextType>({
  theme: 'light',
  setTheme: (_val: string) => {},
});

export const ThemeContextController = ({ children }: { children: JSX.Element | JSX.Element[] }): JSX.Element => {
  const [theme, setTheme] = useState('light');

  useEffect(() => {
    const init = async () => {
      const storedTheme = localStorage.getItem('theme');
      setTheme(storedTheme ?? 'light');
    };
    init();
  }, []);

  return (
    <ThemeContext.Provider value={{ theme, setTheme }}>
      <ThemeProvider theme={theme === 'light' ? lightTheme : darkTheme}>{children}</ThemeProvider>
    </ThemeContext.Provider>
  );
};
