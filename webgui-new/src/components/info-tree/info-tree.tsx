import { TreeView, TreeItem, treeItemClasses } from '@mui/lab';
import { alpha, Box, CircularProgress, styled } from '@mui/material';
import { Code } from '@mui/icons-material';
import { ExpandMore, ChevronRight } from '@mui/icons-material';
import { useContext, useEffect, useState } from 'react';
import {
  getCppReferenceTypes,
  getCppReferences,
  getCppProperties,
  getCppReferenceCount,
  getCppAstNodeInfo,
} from 'service/cpp-service';
import { AstNodeInfo, FileInfo, Range } from '@thrift-generated';
import { FileIcon, RefIcon } from 'components/custom-icon/custom-icon';
import { TabName } from 'enums/tab-enum';
import { AppContext } from 'global-context/app-context';
import { getFileInfo } from 'service/project-service';

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
  const appCtx = useContext(AppContext);

  const [astNodeInfo, setAstNodeInfo] = useState<AstNodeInfo | undefined>(undefined);
  const [props, setProps] = useState<Map<string, string>>(new Map());
  const [refTypes, setRefTypes] = useState<Map<string, number>>(new Map());
  const [refCounts, setRefCounts] = useState<Map<string, number>>(new Map());
  const [refs, setRefs] = useState<Map<string, AstNodeInfo[]>>(new Map());
  const [fileUsages, setFileUsages] = useState<Map<string, FileInfo[]>>(new Map());
  const [loadComplete, setLoadComplete] = useState<boolean>(false);

  useEffect(() => {
    if (!appCtx.languageNodeId) return;
    setLoadComplete(false);
    const init = async () => {
      const initAstNodeInfo = await getCppAstNodeInfo(appCtx.languageNodeId as string);
      if (!initAstNodeInfo) return;

      const initProps = await getCppProperties(initAstNodeInfo.id as string);
      const initRefTypes = await getCppReferenceTypes(initAstNodeInfo.id as string);
      const initRefCounts: typeof refCounts = new Map();
      const initRefs: typeof refs = new Map();
      const initFileUsages: typeof fileUsages = new Map();

      for (const [rType, rId] of initRefTypes) {
        const refCount = await getCppReferenceCount(initAstNodeInfo.id as string, rId);
        initRefCounts.set(rType, refCount);

        const refsForType = await getCppReferences(initAstNodeInfo.id as string, rId, initAstNodeInfo.tags ?? []);
        initRefs.set(rType, refsForType);

        if (rType === 'Caller' || rType === 'Usage') {
          const fileInfos: FileInfo[] = [];
          for (const fId of [...new Set(refsForType.map((aInfo) => aInfo.range?.file as string))]) {
            const fInfo = await getFileInfo(fId);
            fileInfos.push(fInfo as FileInfo);
          }
          initFileUsages.set(rType, [...new Set(fileInfos)]);
        } else {
          initFileUsages.set(rType, []);
        }
      }

      setAstNodeInfo(initAstNodeInfo);
      setProps(initProps);
      setRefTypes(initRefTypes);
      setRefCounts(initRefCounts);
      setRefs(initRefs);
      setFileUsages(initFileUsages);
    };
    init().then(() => setLoadComplete(true));
  }, [appCtx.languageNodeId]);

  const jumpToRef = async (astNodeInfo: AstNodeInfo) => {
    const fileId = astNodeInfo.range?.file as string;
    appCtx.setProjectFileId(fileId);
    appCtx.setEditorSelection(astNodeInfo.range?.range as Range);
    appCtx.setActiveTab(TabName.CODE);
  };

  return appCtx.languageNodeId && astNodeInfo ? (
    loadComplete ? (
      <OuterContainer>
        <StyledDiv
          onClick={() => jumpToRef(astNodeInfo)}
          sx={{
            fontWeight: 'bold',
            cursor: 'pointer',
            ':hover': {
              backgroundColor: (theme) => alpha(theme.backgroundColors?.secondary as string, 0.3),
            },
          }}
        >{`${astNodeInfo.symbolType}: ${astNodeInfo.astNodeValue}`}</StyledDiv>
        <StyledDiv>
          {Array.from(props.keys()).map((name, idx) => (
            <StyledDiv key={idx}>
              <StyledSpan sx={{ textDecoration: 'underline' }}>{name}:</StyledSpan> {props.get(name)}
            </StyledDiv>
          ))}
        </StyledDiv>
        <StyledTreeView
          defaultExpandIcon={<ChevronRight />}
          defaultEndIcon={<ChevronRight />}
          defaultCollapseIcon={<ExpandMore />}
          sx={{ width: 'max-content' }}
        >
          {Array.from(refTypes.keys())
            .filter((type) => refCounts.get(type) !== 0)
            .map((type, refTypeIdx) => (
              <StyledTreeItem
                nodeId={`${refTypeIdx}`}
                key={refTypeIdx}
                icon={<RefIcon refName={type} />}
                label={
                  <StyledDiv sx={{ fontSize: '0.85rem' }}>
                    {type} ({refCounts.get(type)})
                  </StyledDiv>
                }
              >
                {fileUsages.get(type)?.length
                  ? fileUsages.get(type)?.map((fileInfo) => (
                      <StyledTreeItem
                        nodeId={`${fileInfo.id}:${refTypeIdx}`}
                        key={fileInfo.id}
                        icon={<FileIcon fileName={fileInfo.name as string} />}
                        label={
                          <StyledDiv sx={{ fontSize: '0.85rem' }}>
                            {fileInfo.name} (
                            {refs.get(type)?.filter((aInfo) => aInfo.range?.file === fileInfo.id).length})
                          </StyledDiv>
                        }
                      >
                        {refs
                          .get(type)
                          ?.filter((aInfo) => aInfo.range?.file === fileInfo.id)
                          .map((aInfo) => (
                            <Label key={aInfo.id} onClick={() => jumpToRef(aInfo)}>
                              <Code sx={{ width: '20px', height: '20px' }} />
                              <StyledDiv>{`${aInfo.range?.range?.startpos?.line}:${aInfo.range?.range?.startpos?.column}: ${aInfo.astNodeValue}`}</StyledDiv>
                            </Label>
                          ))}
                      </StyledTreeItem>
                    ))
                  : refs.get(type)?.map((aInfo) => (
                      <Label key={aInfo.id} onClick={() => jumpToRef(aInfo)}>
                        <Code sx={{ width: '20px', height: '20px' }} />
                        <StyledDiv>{`${aInfo.range?.range?.startpos?.line}:${aInfo.range?.range?.startpos?.column}: ${aInfo.astNodeValue}`}</StyledDiv>
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
