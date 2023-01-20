import { cpp } from '@codemirror/lang-cpp';
import { ThemeContext } from '../../themes/theme-context';
import styled from '@emotion/styled';
import { tokyoNight } from '@uiw/codemirror-theme-tokyo-night';
import { tokyoNightDay } from '@uiw/codemirror-theme-tokyo-night-day';
import ReactCodeMirror from '@uiw/react-codemirror';
import { useContext } from 'react';
import { FileName } from '../../components/file-name/file-name';
import { Header } from '../../components/header/header';
import { AccordionMenu } from '../../components/accordion-menu/accordion-menu';

const OuterContainer = styled('div')({
  display: 'flex',
  flexDirection: 'column',
});

const InnerContainer = styled('div')({
  display: 'flex',
});

const CodeMirrorContainer = styled('div')({
  display: 'flex',
  flexDirection: 'column',
  width: '100%',
  height: '100%',
});

const placeholder = `#include <stdio.h>
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

const Page = () => {
  const { theme } = useContext(ThemeContext);

  return (
    <OuterContainer>
      <Header />
      <InnerContainer>
        <AccordionMenu />
        <CodeMirrorContainer>
          <FileName fileName={'main.c'} filePath={'/projects/c/main.c'} parseStatus={'Fully parsed'} />
          <ReactCodeMirror
            readOnly={true}
            extensions={[cpp()]}
            theme={theme === 'dark' ? tokyoNight : tokyoNightDay}
            style={{ flexGrow: '1' }}
            value={placeholder}
          />
        </CodeMirrorContainer>
      </InnerContainer>
    </OuterContainer>
  );
};

export default Page;
