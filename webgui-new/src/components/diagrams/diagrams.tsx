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
import { ProjectContext } from 'global-context/project-context';
import { useContext, useRef, useState } from 'react';
import { getCppFileDiagram, getCppFileDiagramLegend } from 'service/cpp-service';

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
  justifyContent: 'space-between',
  gap: '1rem',
  padding: '10px',
  height: '50px',
  borderTop: `1px solid ${theme.colors?.primary}`,
}));

const ZoomOptions = styled('div')({
  display: 'flex',
  alignItems: 'center',
  gap: '1rem',
});

export const Diagrams = (): JSX.Element => {
  const projectCtx = useContext(ProjectContext);
  const languageCtx = useContext(LanguageContext);

  const [legendModalOpen, setLegendModalOpen] = useState<boolean>(false);
  const [exportTooltipOpen, setExportTooltipOpen] = useState<boolean>(false);
  const [diagramSize, setDiagramSize] = useState<number>(1);

  const diagramContainerRef = useRef<HTMLDivElement | null>(null);
  const diagramLegendContainerRef = useRef<HTMLDivElement | null>(null);

  const generateFileDiagram = async (fileId: string, dir: boolean) => {
    if (!diagramContainerRef.current) return;

    setDiagramSize(1);

    const currentDiagramId = dir
      ? (languageCtx.dirDiagramTypes.get(languageCtx.currentDirDiagramType) as number)
      : (languageCtx.fileDiagramTypes.get(languageCtx.currentFileDiagramType) as number);

    const fileDiagram = await getCppFileDiagram(fileId, currentDiagramId);

    const parser = new DOMParser();
    const parsedFileDiagram = parser.parseFromString(fileDiagram, 'text/xml');
    const fileDiagramSvg = parsedFileDiagram.getElementsByTagName('svg')[0];

    fileDiagramSvg.style.height = '100%';
    fileDiagramSvg.style.cursor = 'pointer';

    fileDiagramSvg.onclick = (e) => {
      const parentNode = (e.target as HTMLElement)?.parentElement;
      if ((parentNode?.className as unknown as SVGAnimatedString).baseVal !== 'node') return;
      generateFileDiagram(parentNode?.id as string, dir);
    };

    diagramContainerRef.current.innerHTML = '';
    diagramContainerRef.current.appendChild(fileDiagramSvg);
  };

  const generateLegend = async (dir: boolean) => {
    if (!diagramLegendContainerRef.current) return;

    const currentDiagramId = dir
      ? (languageCtx.dirDiagramTypes.get(languageCtx.currentDirDiagramType) as number)
      : (languageCtx.fileDiagramTypes.get(languageCtx.currentFileDiagramType) as number);

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

  const scaleSVG = (option: 'zoomIn' | 'zoomOut' | 'reset') => {
    const svgElement = diagramContainerRef.current?.querySelector('svg') as SVGSVGElement;
    if (!svgElement) return;

    if (option === 'zoomIn') {
      svgElement.style.transform = `scale(${diagramSize + 0.1})`;
      setDiagramSize((prevSize) => prevSize + 0.1);
    } else if (option === 'zoomOut') {
      svgElement.style.transform = `scale(${diagramSize - 0.1})`;
      setDiagramSize((prevSize) => prevSize - 0.1);
    } else {
      svgElement.style.transform = 'scale(1)';
      setDiagramSize(1);
    }
  };

  return (
    <div>
      <FileName
        fileName={projectCtx.fileInfo ? (projectCtx.fileInfo.name as string) : ''}
        filePath={projectCtx.fileInfo ? (projectCtx.fileInfo.path as string) : ''}
        parseStatus={projectCtx.fileInfo ? (projectCtx.fileInfo.parseStatus as number) : 4}
        info={projectCtx.fileInfo ?? undefined}
      />
      {projectCtx.fileInfo ? (
        <>
          {languageCtx.fileDiagramTypes.size !== 0 || languageCtx.dirDiagramTypes.size !== 0 ? (
            <>
              <GenerateOptionsContainer>
                {languageCtx.fileDiagramTypes.size !== 0 ? (
                  <>
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
                      onClick={() => generateFileDiagram(projectCtx.fileInfo?.id as string, false)}
                      sx={{ textTransform: 'none' }}
                    >
                      {`Generate ${languageCtx.fileType} diagram`}
                    </Button>
                  </>
                ) : (
                  ''
                )}
                {languageCtx.dirDiagramTypes.size !== 0 ? (
                  <>
                    <FormControl>
                      <InputLabel>{`Dir Diagrams`}</InputLabel>
                      <Select
                        value={languageCtx.currentDirDiagramType}
                        label={`Dir Diagrams`}
                        onChange={(e) => languageCtx.setCurrentDirDiagramType(e.target.value)}
                      >
                        {Object.keys(Object.fromEntries(languageCtx.dirDiagramTypes)).map((diagramType, idx) => (
                          <MenuItem key={idx} value={diagramType}>
                            {diagramType}
                          </MenuItem>
                        ))}
                      </Select>
                    </FormControl>
                    <Button
                      onClick={() => generateFileDiagram(projectCtx.fileInfo?.parent as string, true)}
                      sx={{ textTransform: 'none' }}
                    >
                      {`Generate diagram for dir: ${projectCtx.fileInfo.path?.split('/').reverse()[1]}`}
                    </Button>
                  </>
                ) : (
                  ''
                )}
              </GenerateOptionsContainer>
              <DiagramContainer ref={diagramContainerRef} />
              <DiagramOptionsContainer>
                <ZoomOptions>
                  <IconButton onClick={() => scaleSVG('zoomIn')}>
                    <ZoomIn />
                  </IconButton>
                  <Button onClick={() => scaleSVG('reset')} sx={{ textTransform: 'none' }}>
                    {'Reset'}
                  </Button>
                  <IconButton onClick={() => scaleSVG('zoomOut')}>
                    <ZoomOut />
                  </IconButton>
                </ZoomOptions>
                <StyledDiv sx={{ display: 'flex', alignItems: 'center', gap: '1.5rem' }}>
                  <Button onClick={() => generateLegend(false)} sx={{ textTransform: 'none' }}>
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
                    title='Copied to clipboard'
                    arrow
                  >
                    <Button onClick={() => exportDiagramSVG()} sx={{ textTransform: 'none' }}>
                      {'Export SVG'}
                    </Button>
                  </Tooltip>
                  <Button onClick={() => downloadSVG()} sx={{ textTransform: 'none' }}>
                    {'Download image'}
                  </Button>
                </StyledDiv>
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
            <StyledDiv sx={{ padding: '30px' }}>{'No diagrams available for this file.'}</StyledDiv>
          )}
        </>
      ) : (
        ''
      )}
    </div>
  );
};
