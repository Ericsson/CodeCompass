import { TreeView, TreeItem, treeItemClasses } from '@mui/lab';
import { alpha, Box, CircularProgress, styled } from '@mui/material';
import { Code } from '@mui/icons-material';
import { ExpandMore, ChevronRight } from '@mui/icons-material';
import { LanguageContext } from 'global-context/language-context';
import { useContext, useEffect, useState } from 'react';
import { getCppReferenceTypes, getCppReferences, getCppProperties, getCppReferenceCount } from 'service/cpp-service';
import { AstNodeInfo, FileInfo, Range } from '@thrift-generated';
import { ProjectContext } from 'global-context/project-context';
import { getParents, getFileContent, getFileInfo } from 'service/project-service';

const StyledDiv = styled('div')({});
const StyledSpan = styled('span')({});

const OuterContainer = styled('div')({
  padding: '10px',
  fontSize: '0.85rem',
  width: 'max-content',
});

const StyledTreeView = styled(TreeView)(({ theme }) => ({
  color: theme.colors?.primary,
  backgroundColor: theme.backgroundColors?.primary,
  padding: '5px',
  fontSize: '0.85rem',
}));

const StyledTreeItem = styled(TreeItem)(({ theme }) => ({
  [`& .${treeItemClasses.group}`]: {
    marginLeft: '10px',
    borderLeft: `1px dashed ${alpha(theme.palette.text.primary, 0.4)}`,
  },
}));

const Label = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  gap: '0.5rem',
  marginLeft: '5px',
  paddingLeft: '20px',
  cursor: 'pointer',
  ':hover': {
    backgroundColor: alpha(theme.backgroundColors?.secondary as string, 0.3),
  },
}));

export const InfoTree = (): JSX.Element => {
  const projectCtx = useContext(ProjectContext);
  const languageCtx = useContext(LanguageContext);

  const [referenceTypes, setReferenceTypes] = useState<Map<string, number>>(new Map());
  const [referenceCounts, setReferenceCounts] = useState<Map<string, number>>(new Map());
  const [references, setReferences] = useState<Map<string, AstNodeInfo[]>>(new Map());
  const [properties, setProperties] = useState<Map<string, string>>(new Map());
  const [loadComplete, setLoadComplete] = useState<boolean>(false);

  useEffect(() => {
    if (!languageCtx.astNodeInfo) return;
    setLoadComplete(false);

    const init = async () => {
      const astNodeId = languageCtx.astNodeInfo?.id as string;
      const refTypes = await getCppReferenceTypes(astNodeId);
      setReferenceTypes(refTypes);

      const refCounts: typeof referenceCounts = new Map();
      const refs: typeof references = new Map();
      for (const [key, value] of refTypes) {
        const refCount = await getCppReferenceCount(astNodeId, value);
        refCounts.set(key, refCount);

        const refsForType = await getCppReferences(astNodeId, value, []);
        refs.set(key, refsForType);
      }
      setReferenceCounts(refCounts);
      setReferences(refs);

      const props = await getCppProperties(astNodeId);
      setProperties(props);
    };
    init().then(() => setLoadComplete(true));
  }, [languageCtx.astNodeInfo]);

  const jumpToRef = async (astNodeInfo: AstNodeInfo) => {
    const fileId = astNodeInfo.range?.file as string;
    const fileInfo = (await getFileInfo(fileId)) as FileInfo;
    const parents = await getParents(fileInfo.path as string);
    const fileContent = await getFileContent(fileId);
    projectCtx.setFileContent(fileContent);
    projectCtx.setFileInfo(fileInfo);
    projectCtx.setSelectedFile(fileId);
    projectCtx.setExpandedFileTreeNodes(parents);
    languageCtx.setNodeSelectionRange(astNodeInfo.range?.range as Range);
  };

  return languageCtx.astNodeInfo ? (
    loadComplete ? (
      <OuterContainer>
        <StyledDiv
          sx={{ fontWeight: 'bold' }}
        >{`${languageCtx.astNodeInfo.symbolType}: ${languageCtx.astNodeInfo.astNodeValue}`}</StyledDiv>
        <StyledDiv>
          {Array.from(properties.keys()).map((name, idx) => (
            <StyledDiv key={idx}>
              <StyledSpan sx={{ textDecoration: 'underline' }}>{name}:</StyledSpan> {properties.get(name)}
            </StyledDiv>
          ))}
        </StyledDiv>
        <StyledTreeView
          defaultExpandIcon={<ChevronRight />}
          defaultEndIcon={<ChevronRight />}
          defaultCollapseIcon={<ExpandMore />}
          sx={{ width: 'max-content' }}
        >
          {Array.from(referenceTypes.keys())
            .filter((type) => referenceCounts.get(type) !== 0)
            .map((type, idx) => (
              <StyledTreeItem
                nodeId={`${idx}`}
                key={idx}
                label={
                  <StyledDiv sx={{ fontSize: '0.85rem' }}>
                    {type} ({referenceCounts.get(type)})
                  </StyledDiv>
                }
              >
                {references.get(type)?.map((ref) => (
                  <Label key={ref.id} onClick={() => jumpToRef(ref)}>
                    <Code sx={{ width: '20px', height: '20px' }} />
                    <StyledDiv>{ref.astNodeValue}</StyledDiv>
                  </Label>
                ))}
              </StyledTreeItem>
            ))}
        </StyledTreeView>
      </OuterContainer>
    ) : (
      <Box sx={{ display: 'flex', justifyContent: 'center', alignItems: 'center', marginTop: '100px' }}>
        <CircularProgress />
      </Box>
    )
  ) : (
    <StyledDiv sx={{ display: 'flex', justifyContent: 'center', alignItems: 'center', marginTop: '10px' }}>
      {'No node selected'}
    </StyledDiv>
  );
};
