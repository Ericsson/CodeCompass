import { cpp } from '@codemirror/lang-cpp';
import { ThemeContext } from '../../themes/theme-context';
import { tokyoNight } from '@uiw/codemirror-theme-tokyo-night';
import { tokyoNightDay } from '@uiw/codemirror-theme-tokyo-night-day';
import ReactCodeMirror from '@uiw/react-codemirror';
import { useContext } from 'react';
import { FileName } from '../../components/file-name/file-name';
import { Header } from '../../components/header/header';
import { AccordionMenu } from '../../components/accordion-menu/accordion-menu';
import { styled } from '@mui/material';
import { ProjectContext } from '../../global-context/project-context';

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

const CodeMirrorContainer = styled('div')(({ theme }) => ({
  height: 'calc(100vh - 78px - 49px)',
  '& > div:nth-of-type(2)': {
    height: 'calc(100vh - 78px - 49px)',
    overflowY: 'scroll',
    fontSize: '0.8rem',
    '&::-webkit-scrollbar': {
      width: '4px',
      borderLeft: `1px solid ${theme.colors?.primary}`,
    },
    '&::-webkit-scrollbar-track': {
      boxShadow: `inset 0 0 5px ${theme.backgroundColors?.primary}`,
      borderRadius: '10px',
    },
    '&::-webkit-scrollbar-thumb': {
      background: theme.colors?.primary,
      borderRadius: '5px',
    },
  },
}));

const Page = () => {
  const { theme } = useContext(ThemeContext);
  const { fileContent, fileInfo } = useContext(ProjectContext);

  return (
    <OuterContainer>
      <Header />
      <InnerContainer>
        <AccordionMenu />
        <CodeMirrorContainer>
          <FileName
            fileName={fileInfo ? (fileInfo.name as string) : ''}
            filePath={fileInfo ? (fileInfo.path as string) : ''}
            parseStatus={fileInfo ? (fileInfo.parseStatus as number) : 4}
          />
          <ReactCodeMirror
            readOnly={true}
            extensions={[cpp()]}
            theme={theme === 'dark' ? tokyoNight : tokyoNightDay}
            value={fileContent ?? ''}
          />
        </CodeMirrorContainer>
      </InnerContainer>
    </OuterContainer>
  );
};

export default Page;
