import { TreeView, TreeItem, treeItemClasses } from '@mui/lab';
import { alpha, styled } from '@mui/material';
import { ExpandMore, ChevronRight, Cancel } from '@mui/icons-material';
import { LanguageContext } from 'global-context/language-context';
import { useContext, useEffect, useState } from 'react';
import { getCppReferenceTypes, getCppReferences } from 'service/cpp-service';
import { AstNodeInfo } from '@thrift-generated';

const StyledDiv = styled('div')({});

const OuterContainer = styled('div')({
  padding: '10px',
  fontSize: '0.85rem',
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
  const languageCtx = useContext(LanguageContext);

  const [referenceTypes, setReferenceTypes] = useState<Map<string, number>>(new Map());
  const [references, setReferences] = useState<Map<string, AstNodeInfo[]>>(new Map());

  useEffect(() => {
    if (!languageCtx.astNodeInfo) return;

    const init = async () => {
      const refTypes = await getCppReferenceTypes(languageCtx.astNodeInfo?.id as string);
      setReferenceTypes(refTypes);

      const refs: Map<string, AstNodeInfo[]> = new Map();
      for (const [key, value] of refTypes) {
        const refsForType = await getCppReferences(languageCtx.astNodeInfo?.id as string, value, []);
        refs.set(key, refsForType);
      }
      setReferences(refs);
    };
    init();
  }, [languageCtx.astNodeInfo]);

  return languageCtx.astNodeInfo ? (
    <OuterContainer>
      <StyledDiv>{`${languageCtx.astNodeInfo.symbolType}: ${languageCtx.astNodeInfo.astNodeValue}`}</StyledDiv>
      <StyledTreeView
        defaultExpandIcon={<ChevronRight />}
        defaultCollapseIcon={<ExpandMore />}
        sx={{ width: 'fit-content' }}
      >
        {Array.from(referenceTypes.keys()).map((name, idx) =>
          references.get(name)?.length ? (
            <StyledTreeItem
              nodeId={`${idx}`}
              key={idx}
              label={<StyledDiv sx={{ fontSize: '0.85rem' }}>{name}</StyledDiv>}
            >
              {references.get(name)?.map((astNodeInfo) => (
                <Label key={astNodeInfo.id}>{astNodeInfo.astNodeValue}</Label>
              ))}
            </StyledTreeItem>
          ) : (
            <></>
          )
        )}
      </StyledTreeView>
    </OuterContainer>
  ) : (
    <></>
  );
};
