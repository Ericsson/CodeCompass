import { Box, Button, IconButton, Modal, Tooltip, styled } from '@mui/material';
import { ZoomIn, ZoomOut } from '@mui/icons-material';
import { FileName } from 'components/file-name/file-name';
import { useContext, useEffect, useRef, useState, MouseEvent } from 'react';
import {
  getCppAstNodeInfo,
  getCppDiagram,
  getCppDiagramLegend,
  getCppFileDiagram,
  getCppFileDiagramLegend,
} from 'service/cpp-service';
import { TransformWrapper, TransformComponent, ReactZoomPanPinchRef } from 'react-zoom-pan-pinch';
import { FileInfo, AstNodeInfo } from '@thrift-generated';
import { AppContext } from 'global-context/app-context';
import { getFileInfo } from 'service/project-service';

const StyledDiv = styled('div')({});

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

export const Diagrams = (): JSX.Element => {
  const appCtx = useContext(AppContext);

  const [legendModalOpen, setLegendModalOpen] = useState<boolean>(false);
  const [exportTooltipOpen, setExportTooltipOpen] = useState<boolean>(false);
  const [diagramInfo, setDiagramInfo] = useState<FileInfo | AstNodeInfo | undefined>(undefined);

  const diagramContainerRef = useRef<HTMLDivElement | null>(null);
  const diagramLegendContainerRef = useRef<HTMLDivElement | null>(null);
  const transformComponentRef = useRef<ReactZoomPanPinchRef | null>(null);

  useEffect(() => {
    if (appCtx.diagramGenId === '' || appCtx.diagramTypeId === -1) return;
    const init = async () => {
      const initDiagramInfo =
        (await getFileInfo(appCtx.diagramGenId)) ?? (await getCppAstNodeInfo(appCtx.diagramGenId));
      if (!initDiagramInfo) return;

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

  const generateDiagram = (e: MouseEvent) => {
    const parentNode = (e.target as HTMLElement)?.parentElement;
    if ((parentNode?.className as unknown as SVGAnimatedString).baseVal !== 'node') return;
    appCtx.setDiagramGenId(parentNode?.id as string);
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

    diagramLegendContainerRef.current.innerHTML = '';
    diagramLegendContainerRef.current.appendChild(diagramLegendSvg);

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
    <div>
      {diagramInfo instanceof FileInfo ? (
        <FileName
          fileName={diagramInfo ? (diagramInfo.name as string) : ''}
          filePath={diagramInfo ? (diagramInfo.path as string) : ''}
          parseStatus={diagramInfo ? (diagramInfo.parseStatus as number) : 4}
          info={diagramInfo ?? undefined}
        />
      ) : diagramInfo instanceof AstNodeInfo ? (
        <StyledDiv
          sx={{
            display: 'flex',
            alignItems: 'center',
            padding: '0 15px',
            minHeight: '49px',
            borderBottom: (theme) => `1px solid ${theme.colors?.primary}`,
            fontSize: '0.85rem',
          }}
        >{`${diagramInfo.astNodeType}: ${diagramInfo.astNodeValue}`}</StyledDiv>
      ) : (
        <></>
      )}
      <>
        <TransformWrapper ref={transformComponentRef}>
          {({ zoomIn, zoomOut, resetTransform, ...rest }) => (
            <StyledDiv sx={{ position: 'relative' }}>
              <ZoomOptions>
                <IconButton onClick={() => zoomIn()}>
                  <ZoomIn />
                </IconButton>
                <Button onClick={() => resetTransform()} sx={{ textTransform: 'none' }}>
                  {'Reset'}
                </Button>
                <IconButton onClick={() => zoomOut()}>
                  <ZoomOut />
                </IconButton>
              </ZoomOptions>
              <TransformComponent>
                <DiagramContainer ref={diagramContainerRef} onClick={(e) => generateDiagram(e)} />
              </TransformComponent>
            </StyledDiv>
          )}
        </TransformWrapper>
        <DiagramOptionsContainer>
          <Button onClick={() => generateLegend()} sx={{ textTransform: 'none' }}>
            {'Legend'}
          </Button>
          <Tooltip
            PopperProps={{
              disablePortal: true,
            }}
            onClose={() => setExportTooltipOpen(false)}
            open={exportTooltipOpen}
            disableFocusListener
            disableHoverListener
            disableTouchListener
            title="Copied to clipboard"
            arrow
          >
            <Button onClick={() => exportDiagramSVG()} sx={{ textTransform: 'none' }}>
              {'Export SVG'}
            </Button>
          </Tooltip>
          <Button onClick={() => downloadSVG()} sx={{ textTransform: 'none' }}>
            {'Download image'}
          </Button>
          <Modal open={legendModalOpen} onClose={() => setLegendModalOpen(false)} keepMounted>
            <Box
              sx={{
                position: 'absolute',
                top: '50%',
                left: '50%',
                transform: 'translate(-50%, -50%)',
              }}
            >
              <DiagramLegendContainer ref={diagramLegendContainerRef} />
            </Box>
          </Modal>
        </DiagramOptionsContainer>
      </>
    </div>
  ) : (
    <StyledDiv sx={{ padding: '10px' }}>
      {
        'No file/directory/node selected. Right click on a file/directory in the file manager or a node in the editor to generate Diagrams.'
      }
    </StyledDiv>
  );
};
