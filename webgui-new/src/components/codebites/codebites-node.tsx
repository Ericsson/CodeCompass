import { cpp } from '@codemirror/lang-cpp';
import { AstNodeInfo, FileInfo } from '@thrift-generated';
import { githubDark, githubLight } from '@uiw/codemirror-theme-github';
import ReactCodeMirror, { ReactCodeMirrorRef } from '@uiw/react-codemirror';
import { ThemeContext } from 'global-context/theme-context';
import React, { Dispatch, SetStateAction, useContext, useEffect, useRef, useState } from 'react';
import {
  getCppAstNodeInfoByPosition,
  getCppReferenceTypes,
  getCppReferences,
  getCppSourceText,
} from 'service/cpp-service';
import { getFileInfo } from 'service/project-service';
import { NodeProps, Handle, Position } from 'reactflow';
import { gutter, GutterMarker } from '@codemirror/view';
import * as SC from './styled-components';

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

class CustomOffsetGutterMarker extends GutterMarker {
  number: string;

  constructor(number: string) {
    super();
    this.number = number;
  }

  eq(other: CustomOffsetGutterMarker) {
    return this.number === other.number;
  }

  toDOM() {
    const div = document.createElement('div');
    div.innerHTML = this.number;
    return div;
  }
}

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
    if (!newAstNodeDef) return;

    if (data.elements.map((elem) => elem.astNodeInfo.id).includes(newAstNodeDef.id)) return;
    data.setElements((prevNodes) => [...prevNodes, { astNodeInfo: newAstNodeDef, rowNum: data.rowNum }]);
  };

  const customOffsetGutter = gutter({
    class: 'customGutter',
    renderEmptyElements: false,
    lineMarker(view, line, others) {
      if (others.some((m) => m.toDOM)) return null;

      const number = view.state.doc.lineAt(line.from).number + (data.astNodeInfo.range?.range?.startpos?.line ?? 0) - 1;

      return new CustomOffsetGutterMarker(number.toString());
    },
  });

  return (
    <SC.NodeOuterContainer>
      <Handle type="target" position={Position.Top} />
      <Handle type="source" position={Position.Bottom} />
      <SC.NodeHeader>
        {fileInfo ? (fileInfo.name as string) : ''} :: {fileInfo ? (fileInfo.path as string) : ''}
      </SC.NodeHeader>
      <ReactCodeMirror
        readOnly={true}
        extensions={[customOffsetGutter, cpp()]}
        theme={theme === 'dark' ? githubDark : githubLight}
        style={{ fontSize: '0.8rem' }}
        value={text}
        width={'490px'}
        height={'310px'}
        onCreateEditor={(view, state) => (editorRef.current = { view, state })}
        onClick={() => handleClick()}
        basicSetup={{
          lineNumbers: false,
        }}
      />
    </SC.NodeOuterContainer>
  );
};
