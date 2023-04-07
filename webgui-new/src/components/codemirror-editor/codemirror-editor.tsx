import ReactCodeMirror, { ReactCodeMirrorRef } from '@uiw/react-codemirror';
import { AccordionLabel } from 'enums/accordion-enum';
import { ThemeContext } from 'global-context/theme-context';
import { ConfigContext } from 'global-context/config-context';
import { LanguageContext } from 'global-context/language-context';
import { ProjectContext } from 'global-context/project-context';
import { useContext, useRef, useState, useEffect, MouseEvent } from 'react';
import { getCppAstNodeInfoByPosition } from 'service/cpp-service';
import { Range } from '@thrift-generated';
import { cpp } from '@codemirror/lang-cpp';
import { githubDark, githubLight } from '@uiw/codemirror-theme-github';
import { EditorContextMenu } from 'components/editor-context-menu/editor-context-menu';
import { FileName } from 'components/file-name/file-name';
import { styled } from '@mui/material';

const CodeMirrorContainer = styled('div')({
  display: 'flex',
});

export const CodeMirrorEditor = (): JSX.Element => {
  const { theme } = useContext(ThemeContext);
  const configCtx = useContext(ConfigContext);
  const projectCtx = useContext(ProjectContext);
  const languageCtx = useContext(LanguageContext);

  const editorRef = useRef<ReactCodeMirrorRef | null>(null);

  const [contextMenu, setContextMenu] = useState<{
    mouseX: number;
    mouseY: number;
  } | null>(null);

  useEffect(() => {
    if (!languageCtx.nodeSelectionRange) return;
    dispatchSelection(languageCtx.nodeSelectionRange);
  }, [languageCtx.nodeSelectionRange, editorRef.current?.view]);

  const dispatchSelection = (range: Range) => {
    if (!range || !range.startpos || !range.endpos) return;

    const { line: startLine, column: startCol } = range.startpos;
    const { line: endLine, column: endCol } = range.endpos;

    const editor = editorRef.current?.view;
    if (editor) {
      try {
        const fromPos = editor.state.doc.line(startLine as number).from + (startCol as number) - 1;
        const toPos = editor.state.doc.line(endLine as number).from + (endCol as number) - 1;

        editor.dispatch({
          selection: {
            anchor: fromPos,
            head: toPos,
          },
          scrollIntoView: true,
        });
      } catch {
        return;
      }
    }
  };

  const handleContextMenu = async (event: MouseEvent) => {
    event.preventDefault();
    setContextMenu(
      contextMenu === null
        ? {
            mouseX: event.clientX + 2,
            mouseY: event.clientY - 6,
          }
        : null
    );
    handleAstNodeSelect();
  };

  const handleAstNodeSelect = async () => {
    if (!editorRef.current) return;

    const view = editorRef.current.view;
    if (!view) return;

    const head = view.state.selection.main.head as number;
    const line = view.state.doc.lineAt(head);
    const column = view.state.selection.ranges[0].head - line.from;

    const astNodeInfo = await getCppAstNodeInfoByPosition(projectCtx.fileInfo?.id as string, line.number, column);

    dispatchSelection(astNodeInfo?.range?.range as Range);
    languageCtx.setNodeSelectionRange(astNodeInfo?.range?.range);
    languageCtx.setAstNodeInfo(astNodeInfo);
    if (!astNodeInfo) return;
    configCtx.setActiveAccordion(AccordionLabel.INFO_TREE);
  };

  return (
    <>
      <FileName
        fileName={projectCtx.fileInfo ? (projectCtx.fileInfo.name as string) : ''}
        filePath={projectCtx.fileInfo ? (projectCtx.fileInfo.path as string) : ''}
        parseStatus={projectCtx.fileInfo ? (projectCtx.fileInfo.parseStatus as number) : 4}
        info={projectCtx.fileInfo ?? undefined}
      />
      <CodeMirrorContainer>
        <ReactCodeMirror
          readOnly={true}
          extensions={[cpp()]}
          theme={theme === 'dark' ? githubDark : githubLight}
          style={{ fontSize: '0.8rem', flexGrow: '1' }}
          maxHeight={'calc(100vh - 78px - 48px - 49px)'}
          value={projectCtx.fileContent ?? ''}
          ref={editorRef}
          onCreateEditor={(view, state) => (editorRef.current = { view, state })}
          onClick={() => handleAstNodeSelect()}
          onContextMenu={(e) => handleContextMenu(e)}
        />
        <EditorContextMenu contextMenu={contextMenu} setContextMenu={setContextMenu} />
      </CodeMirrorContainer>
    </>
  );
};
