import { Dispatch, SetStateAction, useContext, useEffect, useRef, useState } from 'react';
import { Box, IconButton, Menu, MenuItem, Modal, Tooltip, styled } from '@mui/material';
import { Close } from '@mui/icons-material';
import { TabName } from 'enums/tab-enum';
import { getCppAstNodeInfo, getCppDocumentation, getCppReferenceTypes, getCppReferences } from 'service/cpp-service';
import { getAsHTMLForNode } from 'service/cpp-reparse-service';
import { AstNodeInfo, Range } from '@thrift-generated';
import { updateUrlWithParams } from 'utils/utils';
import { AppContext } from 'global-context/app-context';

const StyledDiv = styled('div')({});

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

  const astHTMLContainerRef = useRef<HTMLDivElement | null>(null);

  const [astNodeInfo, setAstNodeInfo] = useState<AstNodeInfo | undefined>(undefined);
  const [modalOpen, setModalOpen] = useState<boolean>(false);
  const [docs, setDocs] = useState<string | undefined>(undefined);
  const [selectionTooltipOpen, setSelectionTooltipOpen] = useState<boolean>(false);

  useEffect(() => {
    if (!appCtx.languageNodeId) return;
    const init = async () => {
      const initAstNodeInfo = await getCppAstNodeInfo(appCtx.languageNodeId);
      setAstNodeInfo(initAstNodeInfo);
    };
    init();
  }, [appCtx.languageNodeId]);

  const closeModal = () => {
    setDocs(undefined);
    setModalOpen(false);
    if (!astHTMLContainerRef.current) return;
    astHTMLContainerRef.current.innerHTML = '';
  };

  const getDocs = async () => {
    const initDocs = await getCppDocumentation(astNodeInfo?.id as string);
    setDocs(initDocs);
    setModalOpen(true);
    setContextMenu(null);
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
        <MenuItem
          onClick={() => {
            setContextMenu(null);
            appCtx.setDiagramGenId(astNodeInfo?.id as string);
            appCtx.setActiveTab(TabName.DIAGRAMS);
          }}
        >
          {'Diagrams'}
        </MenuItem>
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
      </Menu>
      <Modal open={modalOpen} onClose={() => closeModal()} keepMounted>
        <Box
          sx={{
            position: 'absolute',
            top: '50%',
            left: '50%',
            transform: 'translate(-50%, -50%)',
          }}
        >
          <ModalContainer>
            <StyledDiv sx={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', width: '100%' }}>
              <StyledDiv sx={{ fontWeight: 'bold' }}>
                {`${astNodeInfo?.astNodeType}: ${astNodeInfo?.astNodeValue}`}
              </StyledDiv>
              <IconButton onClick={() => closeModal()}>
                <Close />
              </IconButton>
            </StyledDiv>
            <StyledDiv sx={{ padding: '10px', width: '100%', overflow: 'scroll' }}>
              {docs !== undefined ? (docs === '' ? 'No documentation available for this node.' : docs) : ''}
              <div ref={astHTMLContainerRef}></div>
            </StyledDiv>
          </ModalContainer>
        </Box>
      </Modal>
    </>
  ) : (
    <></>
  );
};
