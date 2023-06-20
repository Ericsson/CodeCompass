import ReactCodeMirror, { ReactCodeMirrorRef } from '@uiw/react-codemirror';
import { AccordionLabel } from 'enums/accordion-enum';
import { ThemeContext } from 'global-context/theme-context';
import React, { useContext, useRef, useState, useEffect, MouseEvent } from 'react';
import { getCppAstNodeInfoByPosition } from 'service/cpp-service';
import { FileInfo, Position, Range } from '@thrift-generated';
import { cpp } from '@codemirror/lang-cpp';
import { githubDark, githubLight } from '@uiw/codemirror-theme-github';
import { EditorContextMenu } from 'components/editor-context-menu/editor-context-menu';
import { FileName } from 'components/file-name/file-name';
import { AppContext } from 'global-context/app-context';
import { getFileContent, getFileInfo } from 'service/project-service';
import { convertSelectionRangeToString, convertSelectionStringToRange, formatDate } from 'utils/utils';
import { TabName } from 'enums/tab-enum';
import { getRepositoryByProjectPath } from 'service/git-service';
import { useRouter } from 'next/router';
import { RouterQueryType } from 'utils/types';
import { Tooltip, alpha } from '@mui/material';
import * as SC from './styled-components';

export const CodeMirrorEditor = (): JSX.Element => {
  const router = useRouter();
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
    dispatchSelection(convertSelectionStringToRange(appCtx.editorSelection));
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

    const astNodeInfo =
      fileInfo?.type === 'Unknown'
        ? null
        : await getCppAstNodeInfoByPosition(fileInfo?.id as string, line.number, column);
    if (astNodeInfo) {
      dispatchSelection(astNodeInfo?.range?.range as Range);
      router.push({
        pathname: '/project',
        query: {
          ...router.query,
          editorSelection: convertSelectionRangeToString(astNodeInfo?.range?.range),
          languageNodeId: astNodeInfo?.id as string,
          activeAccordion: AccordionLabel.INFO_TREE,
        } as RouterQueryType,
      });
    } else {
      const range = new Range({
        startpos: new Position({
          line: line.number,
          column: line.text.indexOf(line.text.trimStart()[0]) + 1,
        }),
        endpos: new Position({
          line: line.number,
          column: line.length + 1,
        }),
      });
      dispatchSelection(range);
      router.push({
        pathname: '/project',
        query: {
          ...router.query,
          editorSelection: convertSelectionRangeToString(range),
          languageNodeId: '',
        } as RouterQueryType,
      });
    }
  };

  const getCommitInfo = async (finalCommitId: string) => {
    const repo = await getRepositoryByProjectPath(fileInfo?.path as string);
    router.push({
      pathname: '/project',
      query: {
        ...router.query,
        gitRepoId: repo?.repoId as string,
        gitCommitId: finalCommitId,
        activeTab: TabName.GIT_DIFF.toString(),
      } as RouterQueryType,
    });
  };

  return (
    <>
      <FileName
        fileName={fileInfo ? (fileInfo.name as string) : ''}
        filePath={fileInfo ? (fileInfo.path as string) : ''}
        parseStatus={fileInfo ? (fileInfo.parseStatus as number) : 4}
        info={fileInfo ?? undefined}
        gitBlameEnabled={appCtx.gitBlameInfo.length !== 0}
        hideFileRefMenu={fileInfo?.type === 'Unknown'}
      />
      <SC.OuterContainer>
        <SC.GitBlameContainer
          id={'gitBlameContainer'}
          sx={{ display: appCtx.gitBlameInfo.length ? 'flex' : 'none' }}
          onScroll={(e) => {
            const editor = document.querySelector('.cm-scroller');
            if (!editor) return;
            const scroll = e.currentTarget.scrollTop;
            editor.scrollTop = scroll;
          }}
        >
          {appCtx.gitBlameInfo.map((_, idx) => {
            const info = appCtx.gitBlameInfo.find((info) => info.finalStartLineNumber === idx + 1);
            if (!info || !info.finalCommitMessage) return <SC.GitBlameLine key={idx}>&nbsp;</SC.GitBlameLine>;

            const trimmedMessage =
              info?.finalCommitMessage?.length > 27
                ? `${info.finalCommitMessage?.split('').slice(0, 27).join('')}...`
                : info?.finalCommitMessage;
            const date = formatDate(new Date((info.finalSignature?.time as unknown as number) * 1000));

            return (
              <Tooltip
                key={idx}
                title={
                  <div style={{ width: 'max-content' }}>
                    <div>{`#${info.finalCommitId?.substring(0, 8)} `}</div>
                    <div>{info.finalCommitMessage}</div>
                    <div>{`${info.finalSignature?.name} (${info.finalSignature?.email})`}</div>
                    <div>{`Commited on ${date}`}</div>
                  </div>
                }
                placement={'top-start'}
                componentsProps={{
                  tooltip: {
                    sx: {
                      fontSize: '0.85rem',
                      padding: '10px',
                      width: '400px',
                      height: 'auto',
                      overflow: 'scroll',
                    },
                  },
                }}
              >
                <SC.GitBlameLine
                  sx={{
                    cursor: 'pointer',
                    borderTop: (theme) => `1px solid ${theme.colors?.primary}`,
                    ':hover': {
                      backgroundColor: (theme) => alpha(theme.backgroundColors?.secondary as string, 0.3),
                    },
                  }}
                  onClick={() => getCommitInfo(info.finalCommitId as string)}
                >
                  <div>{trimmedMessage}</div>
                  <div>{`on ${date}`}</div>
                </SC.GitBlameLine>
              </Tooltip>
            );
          })}
        </SC.GitBlameContainer>
        <ReactCodeMirror
          readOnly={true}
          extensions={[cpp()]}
          theme={theme === 'dark' ? githubDark : githubLight}
          style={{ fontSize: '0.8rem' }}
          width={appCtx.gitBlameInfo.length !== 0 ? 'calc(100vw - 280px - 400px)' : 'calc(100vw - 280px)'}
          height={'100%'}
          minWidth={'calc(1460px - 280px)'}
          maxWidth={'calc(100vw - 280px)'}
          maxHeight={'calc(100vh - 78px - 48px - 49px)'}
          value={fileContent ?? ''}
          ref={editorRef}
          onCreateEditor={(view, state) => {
            editorRef.current = { view, state };

            const cmScroller = document.querySelector('.cm-scroller') as HTMLDivElement;
            const gitBlameContainer = document.querySelector('#gitBlameContainer') as HTMLDivElement;

            cmScroller.addEventListener('scroll', () => {
              gitBlameContainer.scrollTop = cmScroller.scrollTop;
            });
          }}
          onClick={() => handleAstNodeSelect()}
          onContextMenu={(e) => handleContextMenu(e)}
        />
      </SC.OuterContainer>
      <EditorContextMenu contextMenu={contextMenu} setContextMenu={setContextMenu} />
    </>
  );
};
