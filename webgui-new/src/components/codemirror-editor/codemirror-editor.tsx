import ReactCodeMirror, { Decoration, EditorView, ReactCodeMirrorRef } from '@uiw/react-codemirror';
import { AccordionLabel } from 'enums/accordion-enum';
import { ThemeContext } from 'global-context/theme-context';
import React, { useContext, useRef, useState, useEffect, MouseEvent } from 'react';
import { getCppAstNodeInfoByPosition, getCppReferenceTypes, getCppReferences } from 'service/cpp-service';
import { AstNodeInfo, FileInfo, Position, Range } from '@thrift-generated';
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
import { useTranslation } from 'react-i18next';
import { sendGAEvent } from 'utils/analytics';

type HighlightPosition = {
  startpos: {line: number, column: number}
  endpos: {line: number, column: number}
}

const HIGHLIGHT_FOR_DARK = 'rgba(187, 181, 255, 0.3)';
const HIGHLIGHT_FOR_LIGHT = '#f0d8a8';

export const CodeMirrorEditor = (): JSX.Element => {
  const { t } = useTranslation();
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
  const [highlightRanges, setHighlightRanges] = useState<HighlightPosition[]>([]);
  const [visitedLastAstNode, setVisitedLastAstNode] = useState<AstNodeInfo | null>(null);
  const [highlightColor, setHighlightColor] = useState(theme === 'dark' ?  HIGHLIGHT_FOR_DARK : HIGHLIGHT_FOR_LIGHT);

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

  useEffect(() => {
    setHighlightColor(theme === 'dark' ? HIGHLIGHT_FOR_DARK : HIGHLIGHT_FOR_LIGHT);
  }, [theme])

  useEffect(() => {
    if(!editorRef.current || !editorRef.current.view) return;
    setHighlightRanges([]);
  }, [appCtx.workspaceId, fileInfo, fileContent])

  const createHighlightDecoration = (view: EditorView, highlightPosition: HighlightPosition, highlightColor: string) => {
    if (!editorRef.current || !editorRef.current.state || !editorRef.current.view) return;
    
    const startPos = highlightPosition.startpos as Position;
    const endPos = highlightPosition.endpos as Position;

    const from =
      view.state.doc.line(startPos.line as number).from +
      (startPos.column as number) -
      1;
    const to =
      view.state.doc.line(endPos.line as number).from + (endPos.column as number) - 1;

    return Decoration.mark({
      attributes: { style: `background-color:${highlightColor}` },
    }).range(from, to);
  };
  
  const highlightExtension = () => {return EditorView.decorations.of((view) => {
    const decorations = highlightRanges.map((pos) => createHighlightDecoration(view, pos, highlightColor)) as never;
    return Decoration.set(decorations, true);
  })}

  const updateHighlights = async (astNode : AstNodeInfo) => {
    const refTypes = await getCppReferenceTypes(astNode.id as string)
    if(visitedLastAstNode?.id !== astNode.id){
      const allReferences = await getCppReferences(astNode.id as string, refTypes.get('Usage') as number, []);
      const referencesInFile = allReferences.filter(ref => ref.range?.file === fileInfo?.id);
      setHighlightRanges(referencesInFile.map(nodeInfo => {
        const startpos = nodeInfo?.range?.range?.startpos as { line: number, column: number };
        const endpos = nodeInfo?.range?.range?.endpos as { line: number, column: number };
        return {
          startpos: { line: startpos.line, column: startpos.column },
          endpos: { line: endpos.line, column: endpos.column }
        };
      }));
      setVisitedLastAstNode(astNode);

    }else{
      setHighlightRanges([]);
      setVisitedLastAstNode(null);
    }
  }

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
      sendGAEvent({
        event_action: 'click_on_word',
        event_category: appCtx.workspaceId,
        event_label: `${fileInfo?.name}: ${astNodeInfo.astNodeValue}`,
      });
      const nodeRange = astNodeInfo?.range?.range as Range;
      await updateHighlights(astNodeInfo);
      dispatchSelection(nodeRange);
      router.push({
        pathname: '/project',
        query: {
          ...router.query,
          editorSelection: convertSelectionRangeToString(nodeRange),
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
      sendGAEvent({
        event_action: 'click_on_word',
        event_category: appCtx.workspaceId,
        event_label: `${fileInfo?.name}: ${convertSelectionRangeToString(range)}`,
      });
      setHighlightRanges([]);
      setVisitedLastAstNode(null);
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

  const GitBlameLines = (): JSX.Element => {
    let previousLineCount = 1;
    const renderedLines = appCtx.gitBlameInfo.map((info, idx) => {
      const trimmedMessage =
        (info?.finalCommitMessage?.length as number) > 27
          ? `${info.finalCommitMessage?.split('').slice(0, 27).join('')}...`
          : info?.finalCommitMessage;
      const date = formatDate(new Date((info.finalSignature?.time as unknown as number) * 1000));

      previousLineCount = info.linesInHunk as number;
      const lineHeight = 17.9 * previousLineCount;

      return (
        <Tooltip
          key={idx}
          title={
            <div style={{ width: 'max-content' }}>
              <div>{`#${info.finalCommitId?.substring(0, 8)} `}</div>
              <div>{info.finalCommitMessage}</div>
              <div>{`${info.finalSignature?.name} (${info.finalSignature?.email})`}</div>
              <div>{t('gitBlame.commitedOn', { date })}</div>
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
              marginBottom: `${lineHeight}px`,
              borderTop: (theme) => `1px solid ${theme.colors?.primary}`,
              ':hover': {
                backgroundColor: (theme) => alpha(theme.backgroundColors?.secondary as string, 0.3),
              },
            }}
            onClick={() => getCommitInfo(info.finalCommitId as string)}
          >
            <div>{trimmedMessage}</div>
            <div>{t('gitBlame.onDate', { date })}</div>
          </SC.GitBlameLine>
        </Tooltip>
      );
    });

    return <>{renderedLines}</>;
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
          <GitBlameLines />
        </SC.GitBlameContainer>
        <ReactCodeMirror
          readOnly={true}
          extensions={[cpp(), highlightExtension()]}
          theme={theme === 'dark' ? githubDark : githubLight}
          basicSetup={{
            syntaxHighlighting: false,
            highlightSelectionMatches: false,
            highlightActiveLine: false
          }}
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
          onChange={(_, update) => {
            editorRef.current = { view: update.view, state: update.state };
          }}
          onClick={() => handleAstNodeSelect()}
          onContextMenu={(e) => handleContextMenu(e)}
        />
      </SC.OuterContainer>
      <EditorContextMenu contextMenu={contextMenu} setContextMenu={setContextMenu} />
    </>
  );
};
