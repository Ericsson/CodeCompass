import { Dispatch, SetStateAction, useContext, useEffect, useRef, useState } from 'react';
import { Box, IconButton, Menu, MenuItem, Modal, Tooltip, styled } from '@mui/material';
import { ChevronRight, Close } from '@mui/icons-material';
import { TabName } from 'enums/tab-enum';
import {
  getCppAstNodeInfo,
  getCppDiagramTypes,
  getCppDocumentation,
  getCppReferenceTypes,
  getCppReferences,
} from 'service/cpp-service';
import { getAsHTMLForNode } from 'service/cpp-reparse-service';
import { AstNodeInfo, Range } from '@thrift-generated';
import { updateUrlWithParams } from 'utils/utils';
import { AppContext } from 'global-context/app-context';
import { getBlameInfo, getRepositoryByProjectPath } from 'service/git-service';
import { getFileInfo } from 'service/project-service';

const AstNodeInfoHeader = styled('div')({
  fontWeight: 'bold',
});

const ModalHeader = styled('div')({
  display: 'flex',
  alignItems: 'center',
  justifyContent: 'space-between',
  width: '100%',
});

const ModalContent = styled('div')({
  padding: '10px',
  width: '100%',
  overflow: 'scroll',
});

const ModalBox = styled(Box)({
  position: 'absolute',
  top: '50%',
  left: '50%',
  transform: 'translate(-50%, -50%)',
});

const ModalContainer = styled('div')(({ theme }) => ({
  display: 'flex',
  flexDirection: 'column',
  alignItems: 'center',
  gap: '1rem',
  padding: '10px',
  width: '80vw',
  height: '80vh',
  backgroundColor: theme.backgroundColors?.primary,
}));

export const EditorContextMenu = ({
  contextMenu,
  setContextMenu,
}: {
  contextMenu: {
    mouseX: number;
    mouseY: number;
  } | null;
  setContextMenu: Dispatch<
    SetStateAction<{
      mouseX: number;
      mouseY: number;
    } | null>
  >;
}): JSX.Element => {
  const appCtx = useContext(AppContext);

  const docsContainerRef = useRef<HTMLDivElement | null>(null);
  const astHTMLContainerRef = useRef<HTMLDivElement | null>(null);

  const [astNodeInfo, setAstNodeInfo] = useState<AstNodeInfo | undefined>(undefined);
  const [modalOpen, setModalOpen] = useState<boolean>(false);
  const [selectionTooltipOpen, setSelectionTooltipOpen] = useState<boolean>(false);
  const [diagramTypes, setDiagramTypes] = useState<Map<string, number>>(new Map());

  useEffect(() => {
    if (!appCtx.languageNodeId) return;
    const init = async () => {
      const initAstNodeInfo = await getCppAstNodeInfo(appCtx.languageNodeId);
      setAstNodeInfo(initAstNodeInfo);

      const initDiagramTypes = await getCppDiagramTypes(appCtx.languageNodeId);
      setDiagramTypes(initDiagramTypes);
    };
    init();
  }, [appCtx.languageNodeId]);

  const closeModal = () => {
    setModalOpen(false);
    if (!docsContainerRef.current || !astHTMLContainerRef.current) return;
    docsContainerRef.current.innerHTML = '';
    astHTMLContainerRef.current.innerHTML = '';
  };

  const getDocs = async () => {
    const initDocs = await getCppDocumentation(astNodeInfo?.id as string);
    const parser = new DOMParser();
    const parsedHTML = parser.parseFromString(initDocs, 'text/html');
    setModalOpen(true);
    setContextMenu(null);
    if (!docsContainerRef.current) return;
    if (initDocs !== '') {
      docsContainerRef.current.appendChild(parsedHTML.body);
    } else {
      const placeHolder = document.createElement('div');
      placeHolder.innerHTML = 'No documentation available for this node';
      docsContainerRef.current.appendChild(placeHolder);
    }
  };

  const getAstHTML = async () => {
    const initAstHTML = await getAsHTMLForNode(astNodeInfo?.id as string);
    const parser = new DOMParser();
    const parsedHTML = parser.parseFromString(initAstHTML, 'text/html');
    setModalOpen(true);
    setContextMenu(null);
    if (!astHTMLContainerRef.current) return;
    if (initAstHTML !== '') {
      astHTMLContainerRef.current.appendChild(parsedHTML.body);
    } else {
      const placeHolder = document.createElement('div');
      placeHolder.innerHTML = 'No AST HTML available for this node';
      astHTMLContainerRef.current.appendChild(placeHolder);
    }
  };

  const jumpToDef = async () => {
    if (!astNodeInfo) return;

    const refTypes = await getCppReferenceTypes(astNodeInfo.id as string);
    const defRefs = await getCppReferences(
      astNodeInfo.id as string,
      refTypes.get('Definition') as number,
      astNodeInfo.tags ?? []
    );
    const def = defRefs[0];
    if (!def) {
      setContextMenu(null);
      return;
    }

    const fileId = def.range?.file as string;
    appCtx.setProjectFileId(fileId);
    appCtx.setEditorSelection(def.range?.range as Range);
    appCtx.setActiveTab(TabName.CODE);
    setContextMenu(null);
  };

  const getSelectionLink = () => {
    const currentSelectionRange = appCtx.editorSelection;
    if (!currentSelectionRange || !currentSelectionRange.startpos || !currentSelectionRange.endpos) return;

    const { line: startLine, column: startCol } = currentSelectionRange.startpos;
    const { line: endLine, column: endCol } = currentSelectionRange.endpos;
    const selection = `${startLine}|${startCol}|${endLine}|${endCol}`;

    const selectionLink = updateUrlWithParams({
      wsId: appCtx.workspaceId,
      projFileId: appCtx.projectFileId,
      selection: selection,
    });

    navigator.clipboard.writeText(selectionLink);
    setSelectionTooltipOpen(true);
    setTimeout(() => {
      setSelectionTooltipOpen(false);
    }, 2000);
    setContextMenu(null);
  };

  return appCtx.languageNodeId ? (
    <>
      <Menu
        open={contextMenu !== null}
        onClose={() => setContextMenu(null)}
        anchorReference={'anchorPosition'}
        anchorPosition={contextMenu !== null ? { top: contextMenu.mouseY, left: contextMenu.mouseX } : undefined}
      >
        <MenuItem onClick={() => jumpToDef()}>{'Jump to definiton'}</MenuItem>
        <MenuItem onClick={() => getDocs()}>{'Documentation'}</MenuItem>
        {diagramTypes.size !== 0 && (
          <Tooltip
            title={
              <>
                {Array.from(diagramTypes.keys()).map((type) => (
                  <MenuItem
                    key={diagramTypes.get(type)}
                    onClick={() => {
                      setContextMenu(null);
                      appCtx.setDiagramGenId(astNodeInfo?.id as string);
                      appCtx.setDiagramTypeId(diagramTypes.get(type) as number);
                      appCtx.setActiveTab(TabName.DIAGRAMS);
                    }}
                  >
                    {type}
                  </MenuItem>
                ))}
                <MenuItem
                  onClick={() => {
                    setContextMenu(null);
                    appCtx.setDiagramGenId(astNodeInfo?.id as string);
                    appCtx.setDiagramTypeId(999);
                    appCtx.setActiveTab(TabName.DIAGRAMS);
                  }}
                >
                  {'CodeBites'}
                </MenuItem>
              </>
            }
            placement={'right-start'}
          >
            <MenuItem sx={{ display: 'flex', alignItems: 'center', gap: '1rem' }}>
              <div>{'Diagrams'}</div>
              <ChevronRight />
            </MenuItem>
          </Tooltip>
        )}
        <MenuItem onClick={() => getAstHTML()}>{'Show AST HTML'}</MenuItem>
        <MenuItem onClick={() => getSelectionLink()}>
          <Tooltip
            PopperProps={{
              disablePortal: true,
            }}
            onClose={() => setSelectionTooltipOpen(false)}
            open={selectionTooltipOpen}
            disableFocusListener
            disableHoverListener
            disableTouchListener
            title={'Copied to clipboard'}
            arrow
          >
            <div>{'Get permalink to selection'}</div>
          </Tooltip>
        </MenuItem>
        {appCtx.languageNodeId && (
          <MenuItem
            onClick={async () => {
              setContextMenu(null);
              const fileInfo = await getFileInfo(appCtx.projectFileId as string);
              const currentRepo = await getRepositoryByProjectPath(fileInfo?.path as string);
              const fileName = fileInfo?.path?.split('/').reverse()[0];
              const blameInfo = await getBlameInfo(
                currentRepo?.repoId as string,
                currentRepo?.commitId as string,
                fileName as string,
                fileInfo?.id as string
              );
              appCtx.setGitBlameInfo(blameInfo);
              appCtx.setActiveTab(TabName.CODE);
            }}
          >
            {'Git blame'}
          </MenuItem>
        )}
      </Menu>
      <Modal open={modalOpen} onClose={() => closeModal()} keepMounted>
        <ModalBox>
          <ModalContainer>
            <ModalHeader>
              <AstNodeInfoHeader>{`${astNodeInfo?.astNodeType}: ${astNodeInfo?.astNodeValue}`}</AstNodeInfoHeader>
              <IconButton onClick={() => closeModal()}>
                <Close />
              </IconButton>
            </ModalHeader>
            <ModalContent>
              <div ref={docsContainerRef} />
              <div ref={astHTMLContainerRef} />
            </ModalContent>
          </ModalContainer>
        </ModalBox>
      </Modal>
    </>
  ) : (
    <></>
  );
};
