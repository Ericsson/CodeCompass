import { createTheme } from '@mui/material/styles';

declare module '@mui/material/styles' {
  export interface Theme {
    backgroundColors?: {
      primary?: string;
      secondary?: string;
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
      secondary?: React.CSSProperties['color'];
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
      secondary: '#6598D9',
    },
    text: {
      primary: '#24292F',
      error: '#C62336',
      warning: '#FF6E3D',
      success: '#0BB662',
    },
  },
  dark: {
    background: {
      primary: '#0D1117',
      secondary: '#88C7EC',
    },
    text: {
      primary: '#C9D1D9',
      error: '#D03547',
      warning: '#EC7A54',
      success: '#3EE993',
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
    secondary: colors.light.background.secondary,
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
    secondary: colors.dark.background.secondary,
  },
  colors: {
    primary: colors.dark.text.primary,
    error: colors.dark.text.error,
    warning: colors.dark.text.warning,
    success: colors.dark.text.success,
  },
});
