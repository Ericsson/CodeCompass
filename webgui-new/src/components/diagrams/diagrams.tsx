import {
  Box,
  Button,
  FormControl,
  IconButton,
  InputLabel,
  MenuItem,
  Modal,
  Select,
  Tooltip,
  styled,
} from '@mui/material';
import { ZoomIn, ZoomOut } from '@mui/icons-material';
import { FileName } from 'components/file-name/file-name';
import { LanguageContext } from 'global-context/language-context';
import { useContext, useEffect, useRef, useState } from 'react';
import { getCppFileDiagram, getCppFileDiagramLegend } from 'service/cpp-service';
import { TransformWrapper, TransformComponent, ReactZoomPanPinchRef } from 'react-zoom-pan-pinch';

const StyledDiv = styled('div')({});

const GenerateOptionsContainer = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  gap: '1rem',
  padding: '10px',
  borderBottom: `1px solid ${theme.colors?.primary}`,
}));

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
  padding: '10px',
  overflow: 'scroll',
  width: 'calc(100vw - 280px)',
  height: 'calc(100vh - 78px - 48px - 49px - 77px - 50px)',
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
  const languageCtx = useContext(LanguageContext);

  const [legendModalOpen, setLegendModalOpen] = useState<boolean>(false);
  const [exportTooltipOpen, setExportTooltipOpen] = useState<boolean>(false);

  const diagramContainerRef = useRef<HTMLDivElement | null>(null);
  const diagramLegendContainerRef = useRef<HTMLDivElement | null>(null);
  const transformComponentRef = useRef<ReactZoomPanPinchRef | null>(null);

  useEffect(() => {
    if (!diagramContainerRef.current) return;
    diagramContainerRef.current.innerHTML = '';
  }, [languageCtx.diagramFileInfo]);

  const generateFileDiagram = async (fileId: string) => {
    if (!diagramContainerRef.current || !transformComponentRef.current) return;

    transformComponentRef.current.resetTransform();

    const currentDiagramId = languageCtx.fileDiagramTypes.get(languageCtx.currentFileDiagramType) as number;
    const fileDiagram = await getCppFileDiagram(fileId, currentDiagramId);

    const parser = new DOMParser();
    const parsedFileDiagram = parser.parseFromString(fileDiagram, 'text/xml');
    const fileDiagramSvg = parsedFileDiagram.getElementsByTagName('svg')[0];

    fileDiagramSvg.style.height = '100%';
    fileDiagramSvg.style.cursor = 'pointer';

    fileDiagramSvg.onclick = (e) => {
      const parentNode = (e.target as HTMLElement)?.parentElement;
      if ((parentNode?.className as unknown as SVGAnimatedString).baseVal !== 'node') return;
      generateFileDiagram(parentNode?.id as string);
    };

    diagramContainerRef.current.replaceChildren('');
    diagramContainerRef.current.appendChild(fileDiagramSvg);
  };

  const generateLegend = async () => {
    if (!diagramLegendContainerRef.current) return;

    const currentDiagramId = languageCtx.fileDiagramTypes.get(languageCtx.currentFileDiagramType) as number;

    const fileDiagramLegend = await getCppFileDiagramLegend(currentDiagramId);

    const parser = new DOMParser();
    const parsedFileDiagramLegend = parser.parseFromString(fileDiagramLegend, 'text/xml');
    const fileDiagramLegendSvg = parsedFileDiagramLegend.getElementsByTagName('svg')[0];

    fileDiagramLegendSvg.style.height = '100%';

    diagramLegendContainerRef.current.innerHTML = '';
    diagramLegendContainerRef.current.appendChild(fileDiagramLegendSvg);

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

  return languageCtx.diagramFileInfo ? (
    <div>
      <FileName
        fileName={languageCtx.diagramFileInfo ? (languageCtx.diagramFileInfo.name as string) : ''}
        filePath={languageCtx.diagramFileInfo ? (languageCtx.diagramFileInfo.path as string) : ''}
        parseStatus={languageCtx.diagramFileInfo ? (languageCtx.diagramFileInfo.parseStatus as number) : 4}
        info={languageCtx.diagramFileInfo ?? undefined}
      />
      {languageCtx.fileDiagramTypes.size !== 0 ? (
        <>
          <GenerateOptionsContainer>
            <FormControl>
              <InputLabel>{`${languageCtx.fileType} Diagrams`}</InputLabel>
              <Select
                value={languageCtx.currentFileDiagramType}
                label={`${languageCtx.fileType} Diagrams`}
                onChange={(e) => languageCtx.setCurrentFileDiagramType(e.target.value)}
              >
                {Object.keys(Object.fromEntries(languageCtx.fileDiagramTypes)).map((diagramType, idx) => (
                  <MenuItem key={idx} value={diagramType}>
                    {diagramType}
                  </MenuItem>
                ))}
              </Select>
            </FormControl>
            <Button
              onClick={() => generateFileDiagram(languageCtx.diagramFileInfo?.id as string)}
              sx={{ textTransform: 'none' }}
            >
              {`Generate ${languageCtx.fileType} diagram`}
            </Button>
          </GenerateOptionsContainer>
          <TransformWrapper initialScale={1} initialPositionX={200} initialPositionY={100} ref={transformComponentRef}>
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
                  <DiagramContainer ref={diagramContainerRef} />
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
      ) : (
        <StyledDiv sx={{ padding: '30px' }}>{'No diagrams available for this file/directory.'}</StyledDiv>
      )}
    </div>
  ) : (
    <StyledDiv sx={{ padding: '10px' }}>
      {'No file/directory selected. Right click on a file/directory in the file manager to generate Diagrams.'}
    </StyledDiv>
  );
};
