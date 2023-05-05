import { alpha, Box, CircularProgress } from '@mui/material';
import { Code } from '@mui/icons-material';
import { ExpandMore, ChevronRight } from '@mui/icons-material';
import React, { SyntheticEvent, useContext, useEffect, useState } from 'react';
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
import * as SC from './styled-components';

export const InfoTree = () => {
  const appCtx = useContext(AppContext);

  const [astNodeInfo, setAstNodeInfo] = useState<AstNodeInfo | undefined>(undefined);
  const [properties, setProperties] = useState<Map<string, string>>(new Map());
  const [refTypes, setRefTypes] = useState<Map<string, number>>(new Map());
  const [refCounts, setRefCounts] = useState<Map<string, number>>(new Map());
  const [refs, setRefs] = useState<Map<string, AstNodeInfo[]>>(new Map());
  const [fileUsages, setFileUsages] = useState<Map<string, FileInfo[]>>(new Map());
  const [loadComplete, setLoadComplete] = useState<boolean>(false);
  const [expandedTreeNodes, setExpandedTreeNodes] = useState<string[]>([]);

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
      setProperties(initProps);
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
      <SC.OuterContainer>
        <SC.StyledDiv sx={{ display: 'flex', alignItems: 'center', gap: '5px', marginBottom: '5px' }}>
          <RefIcon refName={astNodeInfo.symbolType as string} />
          <SC.StyledDiv
            onClick={() => jumpToRef(astNodeInfo)}
            sx={{
              fontWeight: 'bold',
              cursor: 'pointer',
              ':hover': {
                backgroundColor: (theme) => alpha(theme.backgroundColors?.secondary as string, 0.3),
              },
            }}
          >{`${astNodeInfo.symbolType}: ${astNodeInfo.astNodeValue}`}</SC.StyledDiv>
        </SC.StyledDiv>
        <SC.StyledDiv>
          {Array.from(properties.keys()).map((name, idx) => (
            <SC.StyledDiv key={idx} sx={{ display: 'flex', alignItems: 'center', gap: '5px' }}>
              <RefIcon refName={name} />
              <SC.StyledDiv>
                <SC.StyledSpan sx={{ textDecoration: 'underline' }}>{name}:</SC.StyledSpan> {properties.get(name)}
              </SC.StyledDiv>
            </SC.StyledDiv>
          ))}
        </SC.StyledDiv>
        <SC.StyledTreeView
          defaultExpandIcon={<ChevronRight />}
          defaultEndIcon={<ChevronRight />}
          defaultCollapseIcon={<ExpandMore />}
          sx={{ width: 'max-content', marginTop: '5px' }}
          expanded={expandedTreeNodes}
          onNodeSelect={(_e: SyntheticEvent<Element, Event>, nodeId: string) => {
            const index = expandedTreeNodes.indexOf(nodeId) as number;
            const copyExpanded = [...expandedTreeNodes];
            if (index === -1) {
              copyExpanded.push(nodeId);
            } else {
              copyExpanded.splice(index, 1);
            }
            setExpandedTreeNodes(copyExpanded);
          }}
        >
          {Array.from(refTypes.keys())
            .filter((type) => refCounts.get(type) !== 0)
            .map((type, refTypeIdx) => (
              <SC.StyledTreeItem
                nodeId={`${refTypeIdx}`}
                key={refTypeIdx}
                icon={<RefIcon refName={type} />}
                label={
                  <SC.StyledDiv sx={{ fontSize: '0.85rem' }}>
                    {type} ({refCounts.get(type)})
                  </SC.StyledDiv>
                }
              >
                {fileUsages.get(type)?.length
                  ? fileUsages.get(type)?.map((fileInfo) => (
                      <SC.StyledTreeItem
                        nodeId={`${fileInfo.id}:${refTypeIdx}`}
                        key={fileInfo.id}
                        icon={<FileIcon fileName={fileInfo.name as string} />}
                        label={
                          <SC.StyledDiv sx={{ fontSize: '0.85rem' }}>
                            {fileInfo.name} (
                            {refs.get(type)?.filter((aInfo) => aInfo.range?.file === fileInfo.id).length})
                          </SC.StyledDiv>
                        }
                      >
                        {refs
                          .get(type)
                          ?.filter((aInfo) => aInfo.range?.file === fileInfo.id)
                          .map((aInfo) => (
                            <SC.Label key={aInfo.id} onClick={() => jumpToRef(aInfo)}>
                              <Code sx={{ width: '20px', height: '20px' }} />
                              <SC.StyledDiv>{`${aInfo.range?.range?.startpos?.line}:${aInfo.range?.range?.startpos?.column}: ${aInfo.astNodeValue}`}</SC.StyledDiv>
                            </SC.Label>
                          ))}
                      </SC.StyledTreeItem>
                    ))
                  : refs.get(type)?.map((aInfo) => (
                      <SC.Label key={aInfo.id} onClick={() => jumpToRef(aInfo)}>
                        <Code sx={{ width: '20px', height: '20px' }} />
                        <SC.StyledDiv>{`${aInfo.range?.range?.startpos?.line}:${aInfo.range?.range?.startpos?.column}: ${aInfo.astNodeValue}`}</SC.StyledDiv>
                      </SC.Label>
                    ))}
              </SC.StyledTreeItem>
            ))}
        </SC.StyledTreeView>
      </SC.OuterContainer>
    ) : (
      <Box sx={{ display: 'flex', justifyContent: 'center', alignItems: 'center', marginTop: '100px' }}>
        <CircularProgress />
      </Box>
    )
  ) : (
    <SC.StyledDiv sx={{ display: 'flex', justifyContent: 'center', alignItems: 'center', marginTop: '10px' }}>
      {'No node selected'}
    </SC.StyledDiv>
  );
};
