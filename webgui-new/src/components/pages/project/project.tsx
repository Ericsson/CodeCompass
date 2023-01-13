import ReactCodeMirror from '@uiw/react-codemirror';
import { Header } from '../../lib/header/header';
import { SidebarMenu } from '../../lib/sidebar-menu/sidebar-menu';
import { styled } from '@mui/material';
import { cpp } from '@codemirror/lang-cpp';
import { useContext } from 'react';
import { ThemeContext } from '../../../themes/theme-context';
import { tokyoNight } from '@uiw/codemirror-theme-tokyo-night';
import { tokyoNightDay } from '@uiw/codemirror-theme-tokyo-night-day';

const MainContainer = styled('div')({
  display: 'flex',
});

const placeholder = `
#include <stdio.h>
#include <stdlib.h>

char* readStr() {
  char* str;

  size_t len = 0;
  size_t size = 10;
  char ch;

  str = realloc(NULL, sizeof(*str) * size);

  while (((ch = fgetc(stdin)) != EOF) && (ch != '\n')) {
    str[len++] = ch;

    if (len == size) {
      size += 16;
      str = realloc(str, sizeof(*str) * size);
    }
  }

  str[len++] = '\0';
  str = realloc(str, sizeof(*str) * len);

  return str;
}

int main() {
  printf("Input: ");
  char* str = readStr();
  
  printf("Output: %s\n", str);
  free(str);

  return 0;
}
`;

export const Project = (): JSX.Element => {
  const { theme } = useContext(ThemeContext);

  return (
    <>
      <Header />
      <MainContainer>
        <SidebarMenu />
        <ReactCodeMirror
          readOnly={true}
          extensions={[cpp()]}
          theme={theme === 'dark' ? tokyoNight : tokyoNightDay}
          style={{ flexGrow: '1' }}
          value={placeholder}
          width={'100%'}
          height={'100%'}
        />
      </MainContainer>
    </>
  );
};
