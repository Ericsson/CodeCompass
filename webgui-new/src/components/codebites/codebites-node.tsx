import { cpp } from '@codemirror/lang-cpp';
import { styled } from '@mui/material';
import { AstNodeInfo, FileInfo } from '@thrift-generated';
import { githubDark, githubLight } from '@uiw/codemirror-theme-github';
import ReactCodeMirror, { ReactCodeMirrorRef } from '@uiw/react-codemirror';
import { FileName } from 'components/file-name/file-name';
import { ThemeContext } from 'global-context/theme-context';
import { Dispatch, SetStateAction, useContext, useEffect, useRef, useState } from 'react';
import {
  getCppAstNodeInfoByPosition,
  getCppReferenceTypes,
  getCppReferences,
  getCppSourceText,
} from 'service/cpp-service';
import { getFileInfo } from 'service/project-service';
import { NodeProps, Handle, Position } from 'reactflow';

const OuterContainer = styled('div')(({ theme }) => ({
  display: 'flex',
  flexDirection: 'column',
  alignItems: 'center',
  justifyContent: 'center',
  width: '500px',
  height: '349px',
  border: `1px solid ${theme.colors?.primary}`,
}));

type CodeBitesElement = {
  astNodeInfo: AstNodeInfo;
  rowNum: number;
};

type DataProps = {
  astNodeInfo: AstNodeInfo;
  elements: CodeBitesElement[];
  setElements: Dispatch<SetStateAction<CodeBitesElement[]>>;
  rowNum: number;
};

export const CodeBitesNode = ({ data }: NodeProps<DataProps>): JSX.Element => {
  const { theme } = useContext(ThemeContext);

  const [fileInfo, setFileInfo] = useState<FileInfo | undefined>(undefined);
  const [text, setText] = useState<string>('');

  const editorRef = useRef<ReactCodeMirrorRef | null>(null);

  useEffect(() => {
    if (!data.astNodeInfo) return;
    const init = async () => {
      const initFileInfo = await getFileInfo(data.astNodeInfo.range?.file as string);
      const initText = await getCppSourceText(data.astNodeInfo.id as string);
      setFileInfo(initFileInfo);
      setText(initText);
    };
    init();
  }, [data.astNodeInfo]);

  const handleClick = async () => {
    if (!editorRef.current) return;

    const view = editorRef.current.view;
    if (!view) return;

    const astNodeStartLine = data.astNodeInfo?.range?.range?.startpos?.line as number;
    const head = view.state.selection.main.head as number;
    const line = view.state.doc.lineAt(head);
    const column = view.state.selection.ranges[0].head - line.from;

    const newAstNodeInfo = await getCppAstNodeInfoByPosition(
      fileInfo?.id as string,
      line.number + astNodeStartLine - 1,
      column
    );
    if (!newAstNodeInfo) return;

    const newAstNodeRefTypes = await getCppReferenceTypes(newAstNodeInfo.id as string);
    if (newAstNodeRefTypes.get('Definition') === undefined) return;

    const newAstNodeDef = (
      await getCppReferences(
        newAstNodeInfo.id as string,
        newAstNodeRefTypes.get('Definition') as number,
        newAstNodeInfo.tags ?? []
      )
    )[0];

    if (data.elements.map((elem) => elem.astNodeInfo.id).includes(newAstNodeDef.id)) return;
    data.setElements((prevNodes) => [...prevNodes, { astNodeInfo: newAstNodeDef, rowNum: data.rowNum }]);
  };

  return (
    <OuterContainer>
      <Handle type="target" position={Position.Top} />
      <Handle type="source" position={Position.Bottom} />
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
        width={'490px'}
        height={'290px'}
        onCreateEditor={(view, state) => (editorRef.current = { view, state })}
        onClick={() => handleClick()}
      />
    </OuterContainer>
  );
};
