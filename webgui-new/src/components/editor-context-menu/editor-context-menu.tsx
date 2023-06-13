import React, { Dispatch, SetStateAction, useContext, useEffect, useRef, useState } from 'react';
import { IconButton, Menu, MenuItem, Modal, Tooltip } from '@mui/material';
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
import * as SC from './styled-components';
import { toast } from 'react-toastify';

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
      workspaceId: appCtx.workspaceId,
      projectFileId: appCtx.projectFileId,
      editorSelection: selection,
    });

    navigator.clipboard.writeText(selectionLink);
    toast.success('Selection link copied to clipboard!');
    setContextMenu(null);
  };

  const showGitBlame = async () => {
    setContextMenu(null);
    const fileInfo = await getFileInfo(appCtx.projectFileId as string);
    const currentRepo = await getRepositoryByProjectPath(fileInfo?.path as string);
    const srcPath = appCtx.labels.get('src');
    const filePath = fileInfo?.path as string;
    const path = filePath.replace(new RegExp(`^${srcPath}`), '').slice(1);
    const blameInfo = await getBlameInfo(
      currentRepo?.repoId as string,
      currentRepo?.commitId as string,
      path as string,
      fileInfo?.id as string
    );
    appCtx.setGitBlameInfo(blameInfo);
    appCtx.setActiveTab(TabName.CODE);
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
                      appCtx.setDiagramType('ast');
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
                    appCtx.setDiagramType('ast');
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
        <MenuItem onClick={() => getSelectionLink()}>{'Get permalink to selection'}</MenuItem>
        <MenuItem onClick={() => showGitBlame()}>{'Git blame'}</MenuItem>
      </Menu>
      <Modal open={modalOpen} onClose={() => closeModal()} keepMounted>
        <SC.ModalBox>
          <SC.ModalContainer>
            <SC.ModalHeader>
              <SC.AstNodeInfoHeader>{`${astNodeInfo?.astNodeType}: ${astNodeInfo?.astNodeValue}`}</SC.AstNodeInfoHeader>
              <IconButton onClick={() => closeModal()}>
                <Close />
              </IconButton>
            </SC.ModalHeader>
            <SC.ModalContent>
              <div ref={docsContainerRef} />
              <div ref={astHTMLContainerRef} />
            </SC.ModalContent>
          </SC.ModalContainer>
        </SC.ModalBox>
      </Modal>
    </>
  ) : (
    <Menu
      open={contextMenu !== null}
      onClose={() => setContextMenu(null)}
      anchorReference={'anchorPosition'}
      anchorPosition={contextMenu !== null ? { top: contextMenu.mouseY, left: contextMenu.mouseX } : undefined}
    >
      <MenuItem onClick={() => getSelectionLink()}>{'Get permalink to selection'}</MenuItem>
      <MenuItem onClick={() => showGitBlame()}>{'Git blame'}</MenuItem>
    </Menu>
  );
};
