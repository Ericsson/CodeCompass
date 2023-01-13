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
      primary: '#ffffff',
    },
    text: {
      primary: '#000000',
      error: red.A400,
      warning: orange.A400,
      success: green.A400,
    },
  },
  dark: {
    background: {
      primary: '#000000',
    },
    text: {
      primary: '#ffffff',
      error: red.A400,
      warning: orange.A400,
      success: green.A400,
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
