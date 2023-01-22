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
import { placeholder } from '../../utils/placeholder';

const InnerContainer = styled('div')({
  display: 'grid',
  gridTemplate: `
    'sidebar codemirror' 1fr
    / 0.2fr 1fr
  `,
});

const CodeMirrorContainer = styled('div')({
  gridArea: 'codemirror',
  minWidth: 'calc(1440px - 280px)',
});

const Page = () => {
  const { theme } = useContext(ThemeContext);

  return (
    <>
      <Header />
      <InnerContainer>
        <AccordionMenu />
        <CodeMirrorContainer>
          <FileName
            fileName={'webserver.cpp'}
            filePath={'/projects/CodeCompass/webserver/src/webserver.cpp'}
            parseStatus={'Fully parsed'}
          />
          <ReactCodeMirror
            readOnly={true}
            extensions={[cpp()]}
            theme={theme === 'dark' ? tokyoNight : tokyoNightDay}
            value={placeholder}
            style={{ height: '86vh', overflowY: 'scroll', fontSize: '0.8rem' }}
          />
        </CodeMirrorContainer>
      </InnerContainer>
    </>
  );
};

export default Page;
