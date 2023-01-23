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

const OuterContainer = styled('div')({
  display: 'grid',
  gridTemplateColumns: '1fr',
  gridTemplateRows: '76px 1fr',
  height: '100vh',
});

const InnerContainer = styled('div')({
  display: 'grid',
  gridTemplateColumns: '280px 1fr',
  gridTemplateRows: '1fr',
});

const CodeMirrorContainer = styled('div')({
  height: 'calc(100vh - 76px - 48px)',
});

const Page = () => {
  const { theme } = useContext(ThemeContext);

  return (
    <OuterContainer>
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
            style={{ height: 'calc(100vh - 76px - 48px)', overflowY: 'scroll', fontSize: '0.8rem' }}
          />
        </CodeMirrorContainer>
      </InnerContainer>
    </OuterContainer>
  );
};

export default Page;
