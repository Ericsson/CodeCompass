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
import { getCppDiagram, getCppDiagramLegend, getCppFileDiagram, getCppFileDiagramLegend } from 'service/cpp-service';
import { TransformWrapper, TransformComponent, ReactZoomPanPinchRef } from 'react-zoom-pan-pinch';
import { FileInfo, AstNodeInfo } from '@thrift-generated';

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
  }, [languageCtx.diagramInfo]);

  const generateDiagram = async (id: string) => {
    if (!diagramContainerRef.current || !transformComponentRef.current) return;

    transformComponentRef.current.resetTransform();

    const currentDiagramId = languageCtx.diagramTypes.get(languageCtx.currentDiagramType) as number;
    const diagram =
      languageCtx.diagramInfo instanceof FileInfo
        ? await getCppFileDiagram(id, currentDiagramId)
        : languageCtx.diagramInfo instanceof AstNodeInfo
        ? await getCppDiagram(id, currentDiagramId)
        : '';

    const parser = new DOMParser();
    const parsedDiagram = parser.parseFromString(diagram, 'text/xml');
    const diagramSvg = parsedDiagram.getElementsByTagName('svg')[0];

    diagramSvg.style.height = '100%';
    diagramSvg.style.cursor = 'pointer';

    diagramSvg.onclick = (e) => {
      const parentNode = (e.target as HTMLElement)?.parentElement;
      if ((parentNode?.className as unknown as SVGAnimatedString).baseVal !== 'node') return;
      generateDiagram(parentNode?.id as string);
    };

    diagramContainerRef.current.replaceChildren('');
    diagramContainerRef.current.appendChild(diagramSvg);
  };

  const generateLegend = async () => {
    if (!diagramLegendContainerRef.current) return;

    const currentDiagramId = languageCtx.diagramTypes.get(languageCtx.currentDiagramType) as number;

    const diagramLegend =
      languageCtx.diagramInfo instanceof FileInfo
        ? await getCppFileDiagramLegend(currentDiagramId)
        : languageCtx.diagramInfo instanceof AstNodeInfo
        ? await getCppDiagramLegend(currentDiagramId)
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

  return languageCtx.diagramInfo ? (
    <div>
      {languageCtx.diagramInfo instanceof FileInfo ? (
        <FileName
          fileName={languageCtx.diagramInfo ? (languageCtx.diagramInfo.name as string) : ''}
          filePath={languageCtx.diagramInfo ? (languageCtx.diagramInfo.path as string) : ''}
          parseStatus={languageCtx.diagramInfo ? (languageCtx.diagramInfo.parseStatus as number) : 4}
          info={languageCtx.diagramInfo ?? undefined}
        />
      ) : languageCtx.diagramInfo instanceof AstNodeInfo ? (
        <StyledDiv
          sx={{
            display: 'flex',
            alignItems: 'center',
            padding: '0 15px',
            minHeight: '49px',
            borderBottom: (theme) => `1px solid ${theme.colors?.primary}`,
            fontSize: '0.85rem',
          }}
        >{`${languageCtx.diagramInfo.astNodeType}: ${languageCtx.diagramInfo.astNodeValue}`}</StyledDiv>
      ) : (
        <></>
      )}
      {languageCtx.diagramTypes.size !== 0 ? (
        <>
          <GenerateOptionsContainer>
            <FormControl>
              <InputLabel>{`${languageCtx.entityType} Diagrams`}</InputLabel>
              <Select
                value={languageCtx.currentDiagramType}
                label={`${languageCtx.entityType} Diagrams`}
                onChange={(e) => languageCtx.setCurrentDiagramType(e.target.value)}
              >
                {Object.keys(Object.fromEntries(languageCtx.diagramTypes)).map((diagramType, idx) => (
                  <MenuItem key={idx} value={diagramType}>
                    {diagramType}
                  </MenuItem>
                ))}
              </Select>
            </FormControl>
            <Button
              onClick={() => generateDiagram(languageCtx.diagramInfo?.id as string)}
              sx={{ textTransform: 'none' }}
            >
              {`Generate ${languageCtx.entityType} diagram`}
            </Button>
          </GenerateOptionsContainer>
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
        <StyledDiv sx={{ padding: '30px' }}>{'No diagrams available for this file/directory/node.'}</StyledDiv>
      )}
    </div>
  ) : (
    <StyledDiv sx={{ padding: '10px' }}>
      {
        'No file/directory/node selected. Right click on a file/directory in the file manager or a node in the editor to generate Diagrams.'
      }
    </StyledDiv>
  );
};
