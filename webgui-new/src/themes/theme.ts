import { createTheme } from '@mui/material/styles';

declare module '@mui/material/styles' {
  export interface Theme {
    backgroundColors?: {
      primary?: string;
    };
    colors?: {
      primary?: string;
    };
  }
  export interface ThemeOptions {
    backgroundColors?: {
      primary?: React.CSSProperties['color'];
    };
    colors?: {
      primary?: React.CSSProperties['color'];
    };
  }
}

const colors = {
  background: {
    primary: '#000000',
  },
  text: {
    primary: '#ffffff',
  },
};

export const appTheme = createTheme({
  palette: {
    mode: 'dark',
    background: {
      default: '#000000',
    },
    text: {
      primary: '#ffffff',
    },
  },
  backgroundColors: {
    primary: colors.background.primary,
  },
  colors: {
    primary: colors.text.primary,
  },
});
