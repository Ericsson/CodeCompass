import ReactCodeMirror, { ReactCodeMirrorRef } from '@uiw/react-codemirror';
import { AccordionLabel } from 'enums/accordion-enum';
import { ThemeContext } from 'global-context/theme-context';
import { useContext, useRef, useState, useEffect, MouseEvent } from 'react';
import { getCppAstNodeInfoByPosition } from 'service/cpp-service';
import { FileInfo, Range } from '@thrift-generated';
import { cpp } from '@codemirror/lang-cpp';
import { githubDark, githubLight } from '@uiw/codemirror-theme-github';
import { EditorContextMenu } from 'components/editor-context-menu/editor-context-menu';
import { FileName } from 'components/file-name/file-name';
import { AppContext } from 'global-context/app-context';
import { getFileContent, getFileInfo } from 'service/project-service';

export const CodeMirrorEditor = (): JSX.Element => {
  const { theme } = useContext(ThemeContext);
  const appCtx = useContext(AppContext);

  const editorRef = useRef<ReactCodeMirrorRef | null>(null);

  const [fileInfo, setFileInfo] = useState<FileInfo | undefined>(undefined);
  const [fileContent, setFileContent] = useState<string>('');

  const [contextMenu, setContextMenu] = useState<{
    mouseX: number;
    mouseY: number;
  } | null>(null);

  useEffect(() => {
    if (!appCtx.projectFileId) return;
    const init = async () => {
      const initFileInfo = await getFileInfo(appCtx.projectFileId);
      const initFileContent = await getFileContent(appCtx.projectFileId);
      setFileInfo(initFileInfo);
      setFileContent(initFileContent);
    };
    init();
  }, [appCtx.projectFileId]);

  useEffect(() => {
    if (!appCtx.editorSelection) return;
    dispatchSelection(appCtx.editorSelection);
  }, [appCtx.editorSelection, editorRef.current?.view]);

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

    const astNodeInfo = await getCppAstNodeInfoByPosition(fileInfo?.id as string, line.number, column);

    dispatchSelection(astNodeInfo?.range?.range as Range);
    appCtx.setEditorSelection(astNodeInfo?.range?.range);
    appCtx.setLanguageNodeId(astNodeInfo?.id as string);

    if (!astNodeInfo) return;
    appCtx.setActiveAccordion(AccordionLabel.INFO_TREE);
  };

  return (
    <>
      <FileName
        fileName={fileInfo ? (fileInfo.name as string) : ''}
        filePath={fileInfo ? (fileInfo.path as string) : ''}
        parseStatus={fileInfo ? (fileInfo.parseStatus as number) : 4}
        info={fileInfo ?? undefined}
      />
      <ReactCodeMirror
        readOnly={true}
        extensions={[cpp()]}
        theme={theme === 'dark' ? githubDark : githubLight}
        style={{ fontSize: '0.8rem' }}
        width={'100%'}
        height={'100%'}
        minWidth={'calc(1460px - 280px)'}
        maxWidth={'calc(100vw - 280px)'}
        maxHeight={'calc(100vh - 78px - 48px - 49px)'}
        value={fileContent ?? ''}
        ref={editorRef}
        onCreateEditor={(view, state) => (editorRef.current = { view, state })}
        onClick={() => handleAstNodeSelect()}
        onContextMenu={(e) => handleContextMenu(e)}
      />
      <EditorContextMenu contextMenu={contextMenu} setContextMenu={setContextMenu} />
    </>
  );
};
