import { green, orange, red } from '@mui/material/colors';
import { createTheme } from '@mui/material/styles';

declare module '@mui/material/styles' {
  export interface Theme {
    backgroundColors?: {
      primary?: string;
    };
    colors?: {
      primary?: string;
      error?: string;
      warning?: string;
      success?: string;
    };
  }
  export interface ThemeOptions {
    backgroundColors?: {
      primary?: React.CSSProperties['color'];
    };
    colors?: {
      primary?: React.CSSProperties['color'];
      error?: React.CSSProperties['color'];
      warning?: React.CSSProperties['color'];
      success?: React.CSSProperties['color'];
    };
  }
}

const colors = {
  light: {
    background: {
      primary: '#FFFFFF',
    },
    text: {
      primary: '#24292F',
      error: '#C13333',
      warning: orange[800],
      success: '#43BA45',
    },
  },
  dark: {
    background: {
      primary: '#0D1117',
    },
    text: {
      primary: '#C9D1D9',
      error: '#BF0000',
      warning: orange[900],
      success: green[700],
    },
  },
};

export const lightTheme = createTheme({
  palette: {
    mode: 'light',
    background: {
      default: colors.light.background.primary,
    },
    text: {
      primary: colors.light.text.primary,
    },
  },
  backgroundColors: {
    primary: colors.light.background.primary,
  },
  colors: {
    primary: colors.light.text.primary,
    error: colors.light.text.error,
    warning: colors.light.text.warning,
    success: colors.light.text.success,
  },
});

export const darkTheme = createTheme({
  palette: {
    mode: 'dark',
    background: {
      default: colors.dark.background.primary,
    },
    text: {
      primary: colors.dark.text.primary,
    },
  },
  backgroundColors: {
    primary: colors.dark.background.primary,
  },
  colors: {
    primary: colors.dark.text.primary,
    error: colors.dark.text.error,
    warning: colors.dark.text.warning,
    success: colors.dark.text.success,
  },
});
