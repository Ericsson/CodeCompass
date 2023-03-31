import { Dispatch, SetStateAction, useContext, useEffect, useState } from 'react';
import { ConfigContext } from 'global-context/config-context';
import { LanguageContext } from 'global-context/language-context';
import { Box, Menu, MenuItem, Modal, styled } from '@mui/material';
import { TabName } from 'enums/tab-enum';
import { getCppDocumentation } from 'service/cpp-service';

const StyledDiv = styled('div')({});

const DocsContainer = styled('div')(({ theme }) => ({
  display: 'flex',
  flexDirection: 'column',
  alignItems: 'center',
  gap: '1rem',
  padding: '10px',
  width: '500px',
  height: '500px',
  overflow: 'scroll',
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

  const [docsModalOpen, setDocsModalOpen] = useState<boolean>(false);
  const [docs, setDocs] = useState<string>('');

  useEffect(() => {
    if (!languageCtx.astNodeInfo) return;
  }, [languageCtx.astNodeInfo]);

  const getDocs = async () => {
    const initDocs = await getCppDocumentation(languageCtx.astNodeInfo?.id as string);
    setDocs(initDocs);
    setDocsModalOpen(true);
    setContextMenu(null);
  };

  return (
    <>
      <Menu
        open={contextMenu !== null}
        onClose={() => setContextMenu(null)}
        anchorReference="anchorPosition"
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
      </Menu>
      <Modal open={docsModalOpen} onClose={() => setDocsModalOpen(false)}>
        <Box
          sx={{
            position: 'absolute',
            top: '50%',
            left: '50%',
            transform: 'translate(-50%, -50%)',
          }}
        >
          <DocsContainer>
            <StyledDiv
              sx={{ fontWeight: 'bold' }}
            >{`${languageCtx.astNodeInfo?.astNodeType}: ${languageCtx.astNodeInfo?.astNodeValue}`}</StyledDiv>
            <div>{docs ? docs : 'No documentation available for this node.'}</div>
          </DocsContainer>
        </Box>
      </Modal>
    </>
  );
};
