import { TreeView, TreeItem, treeItemClasses } from '@mui/lab';
import { alpha, styled } from '@mui/material';
import { ExpandMore, ChevronRight } from '@mui/icons-material';
import { LanguageContext } from 'global-context/language-context';
import { useContext, useEffect, useState } from 'react';
import { getCppReferenceTypes, getCppReferences, getCppProperties } from 'service/cpp-service';
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
  cursor: 'pointer',
  ':hover': {
    backgroundColor: alpha(theme.backgroundColors?.secondary as string, 0.3),
  },
}));

export const InfoTree = (): JSX.Element => {
  const projectCtx = useContext(ProjectContext);
  const languageCtx = useContext(LanguageContext);

  const [referenceTypes, setReferenceTypes] = useState<Map<string, number>>(new Map());
  const [references, setReferences] = useState<Map<string, AstNodeInfo[]>>(new Map());
  const [properties, setProperties] = useState<Map<string, string>>(new Map());

  useEffect(() => {
    if (!languageCtx.astNodeInfo) return;

    const init = async () => {
      const refTypes = await getCppReferenceTypes(languageCtx.astNodeInfo?.id as string);
      setReferenceTypes(refTypes);

      const props = await getCppProperties(languageCtx.astNodeInfo?.id as string);
      setProperties(props);
    };
    init();
  }, [languageCtx.astNodeInfo]);

  const getRefs = async (refType: string, refId: number) => {
    const refsForType = await getCppReferences(languageCtx.astNodeInfo?.id as string, refId, []);
    setReferences((prevRefs) => {
      prevRefs.set(refType, refsForType);
      return prevRefs;
    });
  };

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
        {Array.from(referenceTypes.keys()).map((name, idx) => (
          <StyledTreeItem
            nodeId={`${idx}`}
            key={idx}
            label={
              <StyledDiv
                onClick={async () => await getRefs(name, referenceTypes.get(name) as number)}
                sx={{ fontSize: '0.85rem' }}
              >
                {name}
              </StyledDiv>
            }
          >
            {references.get(name)?.length ? (
              references.get(name)?.map((astNodeInfo) => (
                <Label onClick={() => jumpToRef(astNodeInfo)} sx={{ paddingLeft: '20px' }} key={astNodeInfo.id}>
                  {astNodeInfo.astNodeValue}
                </Label>
              ))
            ) : (
              <StyledDiv sx={{ paddingLeft: '20px' }}>{'No references'}</StyledDiv>
            )}
          </StyledTreeItem>
        ))}
      </StyledTreeView>
    </OuterContainer>
  ) : (
    <></>
  );
};
