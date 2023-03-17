import { Button, FormControl, InputLabel, MenuItem, Select, styled } from '@mui/material';
import { FileName } from 'components/file-name/file-name';
import { ProjectContext } from 'global-context/project-context';
import { useContext, useEffect, useRef, useState } from 'react';
import { getCppFileDiagram, getCppFileDiagramTypes } from 'service/cpp-service';

const OuterContainer = styled('div')({
  overflow: 'hidden',
});

const InnerContainer = styled('div')({
  padding: '10px',
});

const CppOptionsContainer = styled('div')({
  display: 'flex',
  alignItems: 'center',
  gap: '1rem',
});

const DiagramContainer = styled('div')({
  marginTop: '30px',
  display: 'flex',
  justifyContent: 'center',
  alignItems: 'center',
});

export const Diagrams = (): JSX.Element => {
  const projectCtx = useContext(ProjectContext);

  const diagramContainerRef = useRef<HTMLDivElement | null>(null);

  const [cppFileDiagramTypes, setCppFileDiagramTypes] = useState<Map<string, number>>(new Map());
  const [currentCppFileDiagramType, setCurrentCppFileDiagramType] = useState<string>('');

  useEffect(() => {
    if (!projectCtx.fileInfo) return;

    const init = async () => {
      const fileDiagramTypesData = await getCppFileDiagramTypes(projectCtx.fileInfo?.id as string);
      setCppFileDiagramTypes(fileDiagramTypesData);
      setCurrentCppFileDiagramType(
        fileDiagramTypesData ? Object.keys(Object.fromEntries(fileDiagramTypesData))[0] : ''
      );
    };
    init();
  }, [projectCtx.fileInfo]);

  const generateCppFileDiagram = async () => {
    if (!diagramContainerRef.current) return;

    const currentDiagramId = cppFileDiagramTypes.get(currentCppFileDiagramType) as number;
    const cppFileDiagramRes = await getCppFileDiagram(projectCtx.fileInfo?.id as string, currentDiagramId);

    const parser = new DOMParser();
    const parsedContents = parser.parseFromString(cppFileDiagramRes, 'text/xml');
    const svgElement = parsedContents.getElementsByTagName('svg')[0];
    (svgElement.style.height = '500px'), (svgElement.style.width = '800px');

    diagramContainerRef.current.innerHTML = '';
    diagramContainerRef.current.appendChild(svgElement);
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
          {cppFileDiagramTypes.size !== 0 ? (
            <>
              <CppOptionsContainer>
                <FormControl>
                  <InputLabel>{'C++ Diagrams'}</InputLabel>
                  <Select
                    value={currentCppFileDiagramType}
                    label={'C++ Diagrams'}
                    onChange={(e) => setCurrentCppFileDiagramType(e.target.value)}
                  >
                    {Object.keys(Object.fromEntries(cppFileDiagramTypes)).map((diagramType, idx) => (
                      <MenuItem key={idx} value={diagramType}>
                        {diagramType}
                      </MenuItem>
                    ))}
                  </Select>
                </FormControl>
                <Button onClick={() => generateCppFileDiagram()} sx={{ textTransform: 'none' }}>
                  {'Generate C++ Diagram'}
                </Button>
              </CppOptionsContainer>
              <DiagramContainer ref={diagramContainerRef} />
            </>
          ) : (
            <div>{'No C++ diagrams available for this file.'}</div>
          )}
        </InnerContainer>
      ) : (
        ''
      )}
    </OuterContainer>
  );
};
