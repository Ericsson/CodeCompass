import { blue } from '@mui/material/colors';
import { alpha, createTheme } from '@mui/material/styles';
import { ReactDiffViewerStylesOverride } from 'react-diff-viewer-continued/lib/styles';
import React from 'react';

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
    iconColors?: {
      folder?: string;
      ref?: string;
      git?: string;
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
    iconColors?: {
      folder?: React.CSSProperties['color'];
      ref?: React.CSSProperties['color'];
      git?: React.CSSProperties['color'];
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
    icon: {
      folder: blue[900],
      ref: blue[900],
      git: blue[900],
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
    icon: {
      folder: blue[400],
      ref: blue[400],
      git: blue[400],
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
  iconColors: {
    folder: colors.light.icon.folder,
    ref: colors.light.icon.ref,
    git: colors.light.icon.git,
  },
  components: {
    MuiList: {
      styleOverrides: {
        root: {
          color: colors.light.text.primary,
          backgroundColor: colors.light.background.primary,
        },
      },
    },
    MuiTooltip: {
      styleOverrides: {
        tooltip: {
          color: colors.light.text.primary,
          backgroundColor: colors.light.background.primary,
          border: `1px solid ${colors.light.text.primary}`,
        },
      },
    },
    MuiButton: {
      styleOverrides: {
        root: {
          textTransform: 'none',
        },
      },
    },
    MuiCssBaseline: {
      styleOverrides: {
        body: {
          '&::-webkit-scrollbar, & *::-webkit-scrollbar': {
            width: '0.5rem',
            height: '0.5rem',
          },
          '&::-webkit-scrollbar-track, & *::-webkit-scrollbar-track': {
            backgroundColor: colors.light.background.primary,
          },
          '&::-webkit-scrollbar-thumb, & *::-webkit-scrollbar-thumb': {
            borderRadius: '1rem',
            backgroundColor: alpha(colors.light.background.secondary, 0.5),
          },
        },
      },
    },
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
  iconColors: {
    folder: colors.dark.icon.folder,
    ref: colors.dark.icon.ref,
    git: colors.dark.icon.git,
  },
  components: {
    MuiList: {
      styleOverrides: {
        root: {
          color: colors.dark.text.primary,
          backgroundColor: colors.dark.background.primary,
        },
      },
    },
    MuiTooltip: {
      styleOverrides: {
        tooltip: {
          color: colors.dark.text.primary,
          backgroundColor: colors.dark.background.primary,
          border: `1px solid ${colors.dark.text.primary}`,
        },
      },
    },
    MuiButton: {
      styleOverrides: {
        root: {
          textTransform: 'none',
        },
      },
    },
    MuiCssBaseline: {
      styleOverrides: {
        body: {
          '&::-webkit-scrollbar, & *::-webkit-scrollbar': {
            width: '0.5rem',
            height: '0.5rem',
          },
          '&::-webkit-scrollbar-track, & *::-webkit-scrollbar-track': {
            backgroundColor: colors.dark.background.primary,
          },
          '&::-webkit-scrollbar-thumb, & *::-webkit-scrollbar-thumb': {
            borderRadius: '1rem',
            backgroundColor: alpha(colors.dark.background.secondary, 0.5),
          },
        },
      },
    },
  },
});

export const diffViewerTheme: ReactDiffViewerStylesOverride = {
  variables: {
    light: {
      diffViewerBackground: colors.light.background.primary,
      diffViewerColor: colors.light.text.primary,
      gutterBackground: colors.light.background.primary,
      gutterColor: colors.light.text.primary,
      addedBackground: alpha(colors.light.text.success, 0.3),
      removedBackground: alpha(colors.light.text.error, 0.3),
    },
    dark: {
      diffViewerBackground: colors.dark.background.primary,
      diffViewerColor: colors.dark.text.primary,
      gutterBackground: colors.dark.background.primary,
      gutterColor: colors.dark.text.primary,
      addedBackground: alpha(colors.dark.text.success, 0.3),
      removedBackground: alpha(colors.dark.text.error, 0.3),
    },
  },
};
