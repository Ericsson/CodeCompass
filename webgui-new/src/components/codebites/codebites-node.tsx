import { cpp } from '@codemirror/lang-cpp';
import { styled } from '@mui/material';
import { AstNodeInfo, FileInfo } from '@thrift-generated';
import { githubDark, githubLight } from '@uiw/codemirror-theme-github';
import ReactCodeMirror, { ReactCodeMirrorRef } from '@uiw/react-codemirror';
import { FileName } from 'components/file-name/file-name';
import { ThemeContext } from 'global-context/theme-context';
import { Dispatch, SetStateAction, useContext, useEffect, useRef, useState } from 'react';
import { getCppAstNodeInfoByPosition, getCppSourceText } from 'service/cpp-service';
import { getFileInfo } from 'service/project-service';

const OuterContainer = styled('div')(({ theme }) => ({
  display: 'flex',
  flexDirection: 'column',
  alignItems: 'center',
  justifyContent: 'center',
  overflow: 'scroll',
  width: 'max-content',
  height: 'max-content',
  border: `1px solid ${theme.colors?.primary}`,
}));

export const CodeBitesNode = ({
  astNodeInfo,
  setDisplayedAstNodes,
}: {
  astNodeInfo: AstNodeInfo;
  setDisplayedAstNodes: Dispatch<SetStateAction<AstNodeInfo[]>>;
}): JSX.Element => {
  const { theme } = useContext(ThemeContext);

  const [fileInfo, setFileInfo] = useState<FileInfo | undefined>(undefined);
  const [text, setText] = useState<string>('');

  const editorRef = useRef<ReactCodeMirrorRef | null>(null);

  useEffect(() => {
    if (!astNodeInfo) return;
    const init = async () => {
      const initFileInfo = await getFileInfo(astNodeInfo.range?.file as string);
      const initText = await getCppSourceText(astNodeInfo.id as string);
      setFileInfo(initFileInfo);
      setText(initText);
    };
    init();
  }, [astNodeInfo]);

  const handleClick = async () => {
    if (!editorRef.current) return;

    const view = editorRef.current.view;
    if (!view) return;

    const astNodeStartLine = astNodeInfo?.range?.range?.startpos?.line as number;
    const head = view.state.selection.main.head as number;
    const line = view.state.doc.lineAt(head);
    const column = view.state.selection.ranges[0].head - line.from;

    const newAstNodeInfo = await getCppAstNodeInfoByPosition(
      fileInfo?.id as string,
      line.number + astNodeStartLine - 1,
      column
    );
    if (!newAstNodeInfo) return;

    setDisplayedAstNodes((prevNodes) =>
      !prevNodes.map((aInfo) => aInfo.id).includes(newAstNodeInfo.id) ? [...prevNodes, newAstNodeInfo] : [...prevNodes]
    );
  };

  return (
    <OuterContainer>
      <FileName
        fileName={fileInfo ? (fileInfo.name as string) : ''}
        filePath={fileInfo ? (fileInfo.path as string) : ''}
        parseStatus={fileInfo ? (fileInfo.parseStatus as number) : 4}
        info={fileInfo ?? undefined}
        hideFileRefMenu
      />
      <ReactCodeMirror
        readOnly={true}
        extensions={[cpp()]}
        theme={theme === 'dark' ? githubDark : githubLight}
        style={{ fontSize: '0.8rem' }}
        value={text}
        height={'300px'}
        width={'300px'}
        onCreateEditor={(view, state) => (editorRef.current = { view, state })}
        onClick={() => handleClick()}
      />
    </OuterContainer>
  );
};
