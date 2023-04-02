import { Dispatch, SetStateAction, useContext, useEffect, useRef, useState } from 'react';
import { ConfigContext } from 'global-context/config-context';
import { LanguageContext } from 'global-context/language-context';
import { Box, IconButton, Menu, MenuItem, Modal, Tooltip, styled } from '@mui/material';
import { Close } from '@mui/icons-material';
import { TabName } from 'enums/tab-enum';
import { getCppDocumentation } from 'service/cpp-service';
import { getAsHTMLForNode } from 'service/cpp-reparse-service';

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
  const configCtx = useContext(ConfigContext);
  const languageCtx = useContext(LanguageContext);

  const astHTMLContainerRef = useRef<HTMLDivElement | null>(null);

  const [modalOpen, setModalOpen] = useState<boolean>(false);
  const [docs, setDocs] = useState<string | undefined>(undefined);
  const [selectionTooltipOpen, setSelectionTooltipOpen] = useState<boolean>(false);

  useEffect(() => {
    if (!languageCtx.astNodeInfo) return;
  }, [languageCtx.astNodeInfo]);

  const closeModal = () => {
    setDocs(undefined);
    setModalOpen(false);
    if (!astHTMLContainerRef.current) return;
    astHTMLContainerRef.current.innerHTML = '';
  };

  const getDocs = async () => {
    const initDocs = await getCppDocumentation(languageCtx.astNodeInfo?.id as string);
    setDocs(initDocs);
    setModalOpen(true);
    setContextMenu(null);
  };

  const getAstHTML = async () => {
    const initAstHTML = await getAsHTMLForNode(languageCtx.astNodeInfo?.id as string);
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

  // TODO: store selection in URL
  const getSelectionLink = () => {
    const selectionLink = window.location.href;
    navigator.clipboard.writeText(selectionLink);
    setSelectionTooltipOpen(true);
    setTimeout(() => {
      setSelectionTooltipOpen(false);
    }, 2000);
  };

  return languageCtx.astNodeInfo ? (
    <>
      <Menu
        open={contextMenu !== null}
        onClose={() => setContextMenu(null)}
        anchorReference={'anchorPosition'}
        anchorPosition={contextMenu !== null ? { top: contextMenu.mouseY, left: contextMenu.mouseX } : undefined}
      >
        <MenuItem onClick={() => getDocs()}>{'Documentation'}</MenuItem>
        <MenuItem
          onClick={() => {
            setContextMenu(null);
            languageCtx.setDiagramInfo(languageCtx.astNodeInfo);
            configCtx.setActiveTab(TabName.DIAGRAMS);
          }}
        >
          {'Diagrams'}
        </MenuItem>
        <MenuItem onClick={() => getAstHTML()}>{'Show AST HTML'}</MenuItem>
        <MenuItem disabled onClick={() => getSelectionLink()}>
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
                {`${languageCtx.astNodeInfo?.astNodeType}: ${languageCtx.astNodeInfo?.astNodeValue}`}
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
