import { Box, Button, IconButton, Modal, Tooltip, styled } from '@mui/material';
import { ZoomIn, ZoomOut } from '@mui/icons-material';
import { FileName } from 'components/file-name/file-name';
import React, { useContext, useEffect, useRef, useState, MouseEvent } from 'react';
import {
  getCppAstNodeInfo,
  getCppDiagram,
  getCppDiagramLegend,
  getCppFileDiagram,
  getCppFileDiagramLegend,
  getCppReferenceTypes,
  getCppReferences,
} from 'service/cpp-service';
import { TransformWrapper, TransformComponent, ReactZoomPanPinchRef } from 'react-zoom-pan-pinch';
import { FileInfo, AstNodeInfo } from '@thrift-generated';
import { AppContext } from 'global-context/app-context';
import { getFileInfo } from 'service/project-service';
import { CodeBites } from 'components/codebites/codebites';

const DiagramLegendContainer = styled('div')({
  display: 'flex',
  alignItems: 'center',
  justifyContent: 'center',
  padding: '10px',
});

const DiagramContainer = styled('div')({
  display: 'flex',
  alignItems: 'center',
  justifyContent: 'center',
  overflow: 'scroll',
  width: 'calc(100vw - 280px)',
  height: 'calc(100vh - 78px - 48px - 49px - 50px)',
});

const DiagramOptionsContainer = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  gap: '1rem',
  padding: '10px',
  height: '50px',
  borderTop: `1px solid ${theme.colors?.primary}`,
}));

const AstNodeInfoHeader = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  padding: '0 15px',
  minHeight: '49px',
  borderBottom: `1px solid ${theme.colors?.primary}`,
  fontSize: '0.85rem',
}));

const TransformContainer = styled('div')({
  position: 'relative',
});

const ZoomOptions = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  gap: '1rem',
  position: 'absolute',
  right: 20,
  bottom: 20,
  zIndex: 10,
  backgroundColor: theme.backgroundColors?.primary,
  border: `1px solid ${theme.colors?.primary}`,
  borderRadius: '15px',
}));

const ModalBox = styled(Box)({
  position: 'absolute',
  top: '50%',
  left: '50%',
  transform: 'translate(-50%, -50%)',
});

const Placeholder = styled('div')({
  padding: '10px',
});

export const Diagrams = () => {
  const appCtx = useContext(AppContext);

  const [legendModalOpen, setLegendModalOpen] = useState<boolean>(false);
  const [exportTooltipOpen, setExportTooltipOpen] = useState<boolean>(false);
  const [diagramInfo, setDiagramInfo] = useState<FileInfo | AstNodeInfo | undefined>(undefined);
  const [codeBitesDisplayed, setCodeBitesDisplayed] = useState<boolean>(false);

  const diagramContainerRef = useRef<HTMLDivElement | null>(null);
  const diagramLegendContainerRef = useRef<HTMLDivElement | null>(null);
  const transformComponentRef = useRef<ReactZoomPanPinchRef | null>(null);

  useEffect(() => {
    if (appCtx.diagramGenId === '' || appCtx.diagramTypeId === -1) return;
    const init = async () => {
      const initDiagramInfo =
        (await getFileInfo(appCtx.diagramGenId)) ?? (await getCppAstNodeInfo(appCtx.diagramGenId));
      if (!initDiagramInfo) return;

      if (appCtx.diagramTypeId === 999) {
        const refTypes = await getCppReferenceTypes(initDiagramInfo.id as string);
        if (refTypes.get('Definition') === undefined) return;

        const astNodeDef = (
          await getCppReferences(initDiagramInfo.id as string, refTypes.get('Definition') as number, [])
        )[0];

        setDiagramInfo(astNodeDef);
        setCodeBitesDisplayed(true);
        return;
      }

      const diagram =
        initDiagramInfo instanceof FileInfo
          ? await getCppFileDiagram(appCtx.diagramGenId, appCtx.diagramTypeId)
          : initDiagramInfo instanceof AstNodeInfo
          ? await getCppDiagram(appCtx.diagramGenId, appCtx.diagramTypeId)
          : '';

      const parser = new DOMParser();
      const parsedDiagram = parser.parseFromString(diagram, 'text/xml');
      const diagramSvg = parsedDiagram.getElementsByTagName('svg')[0];

      diagramSvg.style.height = '100%';
      diagramSvg.style.cursor = 'pointer';

      transformComponentRef?.current?.resetTransform();
      diagramContainerRef?.current?.replaceChildren(diagramSvg);

      setDiagramInfo(initDiagramInfo);
    };
    init();
  }, [appCtx.diagramGenId, appCtx.diagramTypeId]);

  const generateDiagram = async (e: MouseEvent) => {
    const parentNode = (e.target as HTMLElement)?.parentElement;
    if ((parentNode?.className as unknown as SVGAnimatedString).baseVal !== 'node') return;

    const diagramGenId = parentNode?.id as string;

    const initDiagramInfo = (await getFileInfo(diagramGenId)) ?? (await getCppAstNodeInfo(diagramGenId));
    if (!initDiagramInfo) return;

    if (initDiagramInfo instanceof FileInfo) {
      appCtx.setProjectFileId(diagramGenId);
    } else if (initDiagramInfo instanceof AstNodeInfo) {
      const astNodeInfo = await getCppAstNodeInfo(diagramGenId);
      appCtx.setProjectFileId(astNodeInfo?.range?.file as string);
      appCtx.setEditorSelection(astNodeInfo?.range?.range);
      appCtx.setLanguageNodeId(diagramGenId);
    }

    appCtx.setDiagramGenId(diagramGenId);
  };

  const generateLegend = async () => {
    if (!diagramLegendContainerRef.current) return;

    const diagramLegend =
      diagramInfo instanceof FileInfo
        ? await getCppFileDiagramLegend(appCtx.diagramTypeId)
        : diagramInfo instanceof AstNodeInfo
        ? await getCppDiagramLegend(appCtx.diagramTypeId)
        : '';

    const parser = new DOMParser();
    const parsedDiagramLegend = parser.parseFromString(diagramLegend, 'text/xml');
    const diagramLegendSvg = parsedDiagramLegend.getElementsByTagName('svg')[0];

    diagramLegendSvg.style.height = '100%';
    diagramLegendContainerRef.current.replaceChildren(diagramLegendSvg);

    setLegendModalOpen(true);
  };

  const exportDiagramSVG = () => {
    const svgString = diagramContainerRef.current?.querySelector('svg')?.outerHTML as string;
    navigator.clipboard.writeText(svgString);
    setExportTooltipOpen(true);
    setTimeout(() => {
      setExportTooltipOpen(false);
    }, 2000);
  };

  const downloadSVG = () => {
    const svgElement = diagramContainerRef.current?.querySelector('svg') as SVGSVGElement;
    if (!svgElement) return;

    const svgCode = new XMLSerializer().serializeToString(svgElement);
    const blob = new Blob([svgCode], { type: 'image/svg+xml' });
    const url = URL.createObjectURL(blob);

    const link = document.createElement('a');
    link.setAttribute('download', 'file-diagram.svg');
    link.setAttribute('href', url);
    link.click();
  };

  return appCtx.diagramGenId ? (
    <>
      {diagramInfo instanceof FileInfo ? (
        <FileName
          fileName={diagramInfo ? (diagramInfo.name as string) : ''}
          filePath={diagramInfo ? (diagramInfo.path as string) : ''}
          parseStatus={diagramInfo ? (diagramInfo.parseStatus as number) : 4}
          info={diagramInfo ?? undefined}
        />
      ) : diagramInfo instanceof AstNodeInfo ? (
        <AstNodeInfoHeader>{`${diagramInfo.symbolType}: ${diagramInfo.astNodeValue}`}</AstNodeInfoHeader>
      ) : (
        <></>
      )}
      <>
        {codeBitesDisplayed ? (
          <CodeBites astNodeInfo={diagramInfo as AstNodeInfo} />
        ) : (
          <>
            <TransformWrapper ref={transformComponentRef}>
              {({ zoomIn, zoomOut, resetTransform }) => (
                <TransformContainer>
                  <ZoomOptions>
                    <IconButton onClick={() => zoomIn()}>
                      <ZoomIn />
                    </IconButton>
                    <Button onClick={() => resetTransform()}>{'Reset'}</Button>
                    <IconButton onClick={() => zoomOut()}>
                      <ZoomOut />
                    </IconButton>
                  </ZoomOptions>
                  <TransformComponent>
                    <DiagramContainer ref={diagramContainerRef} onClick={(e) => generateDiagram(e)} />
                  </TransformComponent>
                </TransformContainer>
              )}
            </TransformWrapper>

            <DiagramOptionsContainer>
              <Button onClick={() => generateLegend()}>{'Legend'}</Button>
              <Tooltip
                PopperProps={{
                  disablePortal: true,
                }}
                onClose={() => setExportTooltipOpen(false)}
                open={exportTooltipOpen}
                disableFocusListener
                disableHoverListener
                disableTouchListener
                title={'Copied to clipboard'}
                arrow
              >
                <Button onClick={() => exportDiagramSVG()}>{'Export SVG'}</Button>
              </Tooltip>
              <Button onClick={() => downloadSVG()}>{'Download image'}</Button>
              <Modal open={legendModalOpen} onClose={() => setLegendModalOpen(false)} keepMounted>
                <ModalBox>
                  <DiagramLegendContainer ref={diagramLegendContainerRef} />
                </ModalBox>
              </Modal>
            </DiagramOptionsContainer>
          </>
        )}
      </>
    </>
  ) : (
    <Placeholder>
      {
        'No file/directory/node selected. Right click on a file/directory in the file manager or a node in the editor to generate diagrams.'
      }
    </Placeholder>
  );
};
