import { Button, FormControl, InputLabel, MenuItem, Select, styled } from '@mui/material';
import { FileName } from 'components/file-name/file-name';
import { ProjectContext } from 'global-context/project-context';
import { useContext, useEffect, useRef, useState } from 'react';
import { getCppFileDiagram, getCppFileDiagramLegend, getCppFileDiagramTypes } from 'service/cpp-service';

const OuterContainer = styled('div')({});

const InnerContainer = styled('div')({
  width: 'calc(100vw - 280px)',
  height: 'calc(100vh - 78px - 48px - 49px - 77px)',
});

const StyledDiv = styled('div')({});

const DiagramOptionsContainer = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  gap: '1rem',
  padding: '10px',
  borderBottom: `1px solid ${theme.colors?.primary}`,
}));

const DiagramLegendContainer = styled('div')(({ theme }) => ({}));

const DiagramContainer = styled('div')({
  marginLeft: '50px',
});

export const Diagrams = (): JSX.Element => {
  const projectCtx = useContext(ProjectContext);

  const diagramLegendContainerRef = useRef<HTMLDivElement | null>(null);
  const diagramContainerRef = useRef<HTMLDivElement | null>(null);

  const [fileDiagramTypes, setFileDiagramTypes] = useState<Map<string, number>>(new Map());
  const [currentFileDiagramType, setCurrentFileDiagramType] = useState<string>('');
  const [fileType, setFileType] = useState<string>('');

  useEffect(() => {
    if (!projectCtx.fileInfo) return;

    const init = async () => {
      const fType = projectCtx.fileInfo?.type as string;
      const fDiagramTypes =
        fType === 'CPP' ? await getCppFileDiagramTypes(projectCtx.fileInfo?.id as string) : new Map();
      setFileType(fType);
      setFileDiagramTypes(fDiagramTypes);
      setCurrentFileDiagramType(Object.keys(Object.fromEntries(fDiagramTypes))[0]);
    };
    init();
  }, [projectCtx.fileInfo]);

  const generateFileDiagram = async (fileId: string) => {
    if (!diagramContainerRef.current || !diagramLegendContainerRef.current) return;

    const currentDiagramId = fileDiagramTypes.get(currentFileDiagramType) as number;
    const fileDiagramLegend = await getCppFileDiagramLegend(currentDiagramId);
    const fileDiagram = await getCppFileDiagram(fileId, currentDiagramId);

    const parser = new DOMParser();

    const parsedFileDiagramLegend = parser.parseFromString(fileDiagramLegend, 'text/xml');
    const parsedFileDiagram = parser.parseFromString(fileDiagram, 'text/xml');

    const fileDiagramLegendSvg = parsedFileDiagramLegend.getElementsByTagName('svg')[0];
    const fileDiagramSvg = parsedFileDiagram.getElementsByTagName('svg')[0];

    fileDiagramLegendSvg.style.width = '300px';
    fileDiagramLegendSvg.style.height = '90%';

    fileDiagramSvg.style.width = '500px';
    fileDiagramSvg.style.height = '90%';
    fileDiagramSvg.style.cursor = 'pointer';

    fileDiagramSvg.onclick = (e) => {
      const parentNode = (e.target as HTMLElement)?.parentElement;
      if ((parentNode?.className as unknown as SVGAnimatedString).baseVal !== 'node') return;
      generateFileDiagram(parentNode?.id as string);
    };

    diagramLegendContainerRef.current.innerHTML = '';
    diagramContainerRef.current.innerHTML = '';
    diagramLegendContainerRef.current.appendChild(fileDiagramLegendSvg);
    diagramContainerRef.current.appendChild(fileDiagramSvg);
  };

  return (
    <OuterContainer>
      <FileName
        fileName={projectCtx.fileInfo ? (projectCtx.fileInfo.name as string) : ''}
        filePath={projectCtx.fileInfo ? (projectCtx.fileInfo.path as string) : ''}
        parseStatus={projectCtx.fileInfo ? (projectCtx.fileInfo.parseStatus as number) : 4}
        info={projectCtx.fileInfo ?? undefined}
      />
      {projectCtx.fileInfo ? (
        <InnerContainer>
          {fileDiagramTypes.size !== 0 ? (
            <>
              <DiagramOptionsContainer>
                <FormControl>
                  <InputLabel>{`${fileType} Diagrams`}</InputLabel>
                  <Select
                    value={currentFileDiagramType}
                    label={`${fileType} Diagrams`}
                    onChange={(e) => setCurrentFileDiagramType(e.target.value)}
                  >
                    {Object.keys(Object.fromEntries(fileDiagramTypes)).map((diagramType, idx) => (
                      <MenuItem key={idx} value={diagramType}>
                        {diagramType}
                      </MenuItem>
                    ))}
                  </Select>
                </FormControl>
                <Button
                  onClick={() => generateFileDiagram(projectCtx.fileInfo?.id as string)}
                  sx={{ textTransform: 'none' }}
                >
                  {`Generate ${fileType} diagram`}
                </Button>
              </DiagramOptionsContainer>
              <StyledDiv sx={{ display: 'flex', height: '100%' }}>
                <DiagramLegendContainer ref={diagramLegendContainerRef} />
                <DiagramContainer ref={diagramContainerRef} />
              </StyledDiv>
            </>
          ) : (
            <StyledDiv sx={{ padding: '30px' }}>{'No diagrams available for this file.'}</StyledDiv>
          )}
        </InnerContainer>
      ) : (
        ''
      )}
    </OuterContainer>
  );
};
