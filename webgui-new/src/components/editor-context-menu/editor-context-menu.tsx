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
import { convertSelectionRangeToString } from 'utils/utils';
import { AppContext } from 'global-context/app-context';
import { getBlameInfo, getRepositoryByProjectPath } from 'service/git-service';
import { getFileInfo } from 'service/project-service';
import * as SC from './styled-components';
import { toast } from 'react-toastify';
import { useRouter } from 'next/router';
import { RouterQueryType } from 'utils/types';
import { useTranslation } from 'react-i18next';
import { diagramTypeArray } from 'enums/entity-types';

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
  const { t } = useTranslation();
  const router = useRouter();
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
      placeHolder.innerHTML = t('editorContextMenu.noDocs');
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
      placeHolder.innerHTML = t('editorContextMenu.noAST');
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

    router.push({
      pathname: '/project',
      query: {
        ...router.query,
        projectFileId: fileId,
        editorSelection: convertSelectionRangeToString(def.range?.range as Range),
        activeTab: TabName.CODE.toString(),
      } as RouterQueryType,
    });

    setContextMenu(null);
  };

  const getSelectionLink = () => {
    navigator.clipboard.writeText(window.location.href);
    toast.success(t('editorContextMenu.copyNotification'));
    setContextMenu(null);
  };

  const getGitBlameInfo = async () => {
    setContextMenu(null);
    const fileInfo = await getFileInfo(appCtx.projectFileId as string);
    const currentRepo = await getRepositoryByProjectPath(fileInfo?.path as string);
    const blameInfo = await getBlameInfo(
      currentRepo?.repoId as string,
      currentRepo?.commitId as string,
      currentRepo?.repoPath as string,
      fileInfo?.id as string
    );
    return blameInfo;
  };

  return appCtx.languageNodeId ? (
    <>
      <Menu
        open={contextMenu !== null}
        onClose={() => setContextMenu(null)}
        anchorReference={'anchorPosition'}
        anchorPosition={contextMenu !== null ? { top: contextMenu.mouseY, left: contextMenu.mouseX } : undefined}
      >
        <MenuItem onClick={() => jumpToDef()}>{t('editorContextMenu.jumpToDef')}</MenuItem>
        <MenuItem onClick={() => getDocs()}>{t('editorContextMenu.docs')}</MenuItem>
        {diagramTypes.size !== 0 && (
          <Tooltip
            title={
              <>
                {Array.from(diagramTypes.keys()).map((type) => (
                  <MenuItem
                    key={diagramTypes.get(type)}
                    onClick={() => {
                      setContextMenu(null);
                      router.push({
                        pathname: '/project',
                        query: {
                          ...router.query,
                          diagramGenId: astNodeInfo?.id as string,
                          diagramTypeId: (diagramTypes.get(type) as number).toString(),
                          diagramType: 'ast',
                          activeTab: TabName.DIAGRAMS.toString(),
                        } as RouterQueryType,
                      });
                    }}
                  >
                    {diagramTypeArray[diagramTypes.get(type) as number]}
                  </MenuItem>
                ))}
                <MenuItem
                  onClick={() => {
                    setContextMenu(null);
                    router.push({
                      pathname: '/project',
                      query: {
                        ...router.query,
                        diagramGenId: astNodeInfo?.id as string,
                        diagramTypeId: '999',
                        diagramType: 'ast',
                        activeTab: TabName.DIAGRAMS.toString(),
                      } as RouterQueryType,
                    });
                  }}
                >
                  {t('editorContextMenu.codeBites')}
                </MenuItem>
              </>
            }
            placement={'right-start'}
          >
            <MenuItem sx={{ display: 'flex', alignItems: 'center', gap: '1rem' }}>
              <div>{t('editorContextMenu.diagrams')}</div>
              <ChevronRight />
            </MenuItem>
          </Tooltip>
        )}
        <MenuItem onClick={() => getAstHTML()}>{t('editorContextMenu.ast')}</MenuItem>
        <MenuItem onClick={() => getSelectionLink()}>{t('editorContextMenu.selection')}</MenuItem>
        <MenuItem
          onClick={async () => {
            const blameInfo = await getGitBlameInfo();
            appCtx.setGitBlameInfo(blameInfo);
          }}
        >
          {t('editorContextMenu.gitBlame')}
        </MenuItem>
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
      <MenuItem onClick={() => getSelectionLink()}>{t('editorContextMenu.selection')}</MenuItem>
      <MenuItem
        onClick={async () => {
          const blameInfo = await getGitBlameInfo();
          appCtx.setGitBlameInfo(blameInfo);
        }}
      >
        {t('editorContextMenu.gitBlame')}
      </MenuItem>
    </Menu>
  );
};
