import ReactCodeMirror, { ReactCodeMirrorRef } from '@uiw/react-codemirror';
import { AccordionLabel } from 'enums/accordion-enum';
import { ThemeContext } from 'global-context/theme-context';
import React, { useContext, useRef, useState, useEffect, MouseEvent } from 'react';
import { getCppAstNodeInfoByPosition } from 'service/cpp-service';
import { FileInfo, Range } from '@thrift-generated';
import { cpp } from '@codemirror/lang-cpp';
import { githubDark, githubLight } from '@uiw/codemirror-theme-github';
import { EditorContextMenu } from 'components/editor-context-menu/editor-context-menu';
import { FileName } from 'components/file-name/file-name';
import { AppContext } from 'global-context/app-context';
import { getFileContent, getFileInfo } from 'service/project-service';
import { gutter, GutterMarker } from '@codemirror/view';
import { formatDate } from 'utils/utils';
import { TabName } from 'enums/tab-enum';
import { getRepositoryByProjectPath } from 'service/git-service';

class GitBlameGutterMarker extends GutterMarker {
  number: string;
  commitMessage: string;
  commitDate: string;

  constructor(number: string, commitMessage: string, commitDate: string) {
    super();
    this.number = number;
    this.commitMessage = commitMessage;
    this.commitDate = commitDate;
  }

  eq(other: GitBlameGutterMarker) {
    return this.number === other.number;
  }

  toDOM() {
    const outerDiv = document.createElement('div');
    outerDiv.style.display = 'flex';
    outerDiv.style.alignItems = 'center';
    outerDiv.style.justifyContent = 'space-between';
    outerDiv.style.width = '400px';
    outerDiv.style.cursor = 'pointer';

    const commitDiv = document.createElement('div');
    commitDiv.style.display = 'flex';
    commitDiv.style.alignItems = 'center';
    commitDiv.style.justifyContent = 'space-between';
    commitDiv.style.flexGrow = '1';
    commitDiv.style.gap = '5px';

    const commitMessageDiv = document.createElement('div');
    commitMessageDiv.innerHTML = this.commitMessage;

    const commitDateDiv = document.createElement('div');
    commitDateDiv.innerHTML = this.commitDate !== '' ? `on ${this.commitDate}` : '';

    commitDiv.appendChild(commitMessageDiv);
    commitDiv.appendChild(commitDateDiv);

    const lineNumberDiv = document.createElement('div');
    lineNumberDiv.innerHTML = this.number;
    lineNumberDiv.style.padding = '0 10px';

    outerDiv.appendChild(commitDiv);
    outerDiv.appendChild(lineNumberDiv);

    return outerDiv;
  }
}

export const CodeMirrorEditor = () => {
  const appCtx = useContext(AppContext);
  const { theme } = useContext(ThemeContext);

  const editorRef = useRef<ReactCodeMirrorRef | null>(null);

  const [fileInfo, setFileInfo] = useState<FileInfo | undefined>(undefined);
  const [fileContent, setFileContent] = useState<string>('');
  const [contextMenu, setContextMenu] = useState<{
    mouseX: number;
    mouseY: number;
  } | null>(null);

  useEffect(() => {
    if (!appCtx.workspaceId) return;
    setFileInfo(undefined);
    setFileContent('');
  }, [appCtx.workspaceId]);

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
  }, [appCtx.editorSelection, fileContent]);

  const dispatchSelection = (range: Range) => {
    if (!range || !range.startpos || !range.endpos) return;

    const { line: startLine, column: startCol } = range.startpos;
    const { line: endLine, column: endCol } = range.endpos;

    const editor = editorRef.current?.view;
    if (editor) {
      try {
        const fromPos = editor.state.doc.line(startLine as number).from + (startCol as number) - 1;
        const codeSnippetEnd = editor.state.doc.line(endLine as number).from + (endCol as number) - 1;
        const startLineEnd = editor.state.doc.line(startLine as number).to;
        const toPos = Math.min(codeSnippetEnd, startLineEnd);

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
    if (!astNodeInfo) return;

    dispatchSelection(astNodeInfo?.range?.range as Range);
    appCtx.setEditorSelection(astNodeInfo?.range?.range);
    appCtx.setLanguageNodeId(astNodeInfo?.id as string);
    appCtx.setActiveAccordion(AccordionLabel.INFO_TREE);
  };

  const gitBlameGutter = gutter({
    class: 'customGutter',
    renderEmptyElements: false,
    lineMarker(view, line, others) {
      if (others.some((m) => m.toDOM)) return null;

      const number = view.state.doc.lineAt(line.from).number;
      const info = appCtx.gitBlameInfo.find((info) => info.finalStartLineNumber === number);

      if (!info || !info.finalCommitMessage) return new GitBlameGutterMarker(number.toString(), '', '');

      const trimmedMessage =
        info?.finalCommitMessage?.length > 27
          ? `${info.finalCommitMessage.split('').slice(0, 27).join('')}...`
          : info?.finalCommitMessage;
      const date = formatDate(new Date((info.finalSignature?.time as unknown as number) * 1000));

      return new GitBlameGutterMarker(number.toString(), trimmedMessage, date);
    },
    domEventHandlers: {
      click(view, line) {
        const info = appCtx.gitBlameInfo.find(
          (info) => info.finalStartLineNumber === view.state.doc.lineAt(line.from).number
        );
        if (!info) return true;
        getRepositoryByProjectPath(fileInfo?.path as string).then((result) => {
          appCtx.setGitRepoId(result?.repoId as string);
          appCtx.setGitCommitId(info.finalCommitId as string);
          appCtx.setActiveTab(TabName.GIT_DIFF);
        });
        return true;
      },
    },
  });

  return (
    <>
      <FileName
        fileName={fileInfo ? (fileInfo.name as string) : ''}
        filePath={fileInfo ? (fileInfo.path as string) : ''}
        parseStatus={fileInfo ? (fileInfo.parseStatus as number) : 4}
        info={fileInfo ?? undefined}
        gitBlameEnabled={appCtx.gitBlameInfo.length !== 0}
      />
      <ReactCodeMirror
        readOnly={true}
        extensions={appCtx.gitBlameInfo.length !== 0 ? [gitBlameGutter, cpp()] : [cpp()]}
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
        basicSetup={{
          lineNumbers: appCtx.gitBlameInfo.length === 0,
        }}
      />
      <EditorContextMenu contextMenu={contextMenu} setContextMenu={setContextMenu} />
    </>
  );
};
