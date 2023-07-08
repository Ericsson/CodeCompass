import { Button } from '@mui/material';
import { ChevronRight, Code, ExpandMore } from '@mui/icons-material';
import { AstNodeInfo, FileInfo, Range } from '@thrift-generated';
import React, { useContext, useState } from 'react';
import { getCppFileReferenceCount, getCppFileReferences, getCppFileReferenceTypes } from 'service/cpp-service';
import { TabName } from 'enums/tab-enum';
import { AppContext } from 'global-context/app-context';
import { RefIcon } from 'components/custom-icon/custom-icon';
import * as SC from './styled-components';
import { convertSelectionRangeToString } from 'utils/utils';
import { useRouter } from 'next/router';
import { RouterQueryType } from 'utils/types';

export const FileName = ({
  fileName,
  filePath,
  parseStatus,
  info,
  hideFileRefMenu,
  gitBlameEnabled,
}: {
  fileName: string;
  filePath: string;
  parseStatus: number;
  info: FileInfo | undefined;
  hideFileRefMenu?: boolean;
  gitBlameEnabled?: boolean;
}): JSX.Element => {
  const router = useRouter();
  const appCtx = useContext(AppContext);

  const [anchorEl, setAnchorEl] = useState<HTMLElement | null>(null);
  const [references, setReferences] = useState<React.ReactNode[]>([]);

  const getParseStatusText = (status: number): string => {
    if (status === 2) {
      return 'Partially parsed';
    } else if (status === 3) {
      return 'Fully parsed';
    } else {
      return 'Not parsed';
    }
  };

  const renderFileReferences = async () => {
    const fileRefTypes = await getCppFileReferenceTypes(info?.id as string);

    const elements: React.ReactNode[] = [];
    for (const [key, value] of fileRefTypes) {
      const refCountForType = await getCppFileReferenceCount(info?.id as string, value);
      const refsForType = await getCppFileReferences(info?.id as string, value);

      elements.push(
        <SC.StyledTreeItem
          key={value}
          nodeId={`${value}`}
          label={
            <SC.StyledDiv sx={{ fontSize: '0.85rem' }}>
              {key} ({refCountForType})
            </SC.StyledDiv>
          }
          icon={<RefIcon refName={key} />}
        >
          {refsForType.map((info) => {
            return (
              <SC.Label key={info.id} onClick={() => jumpToRef(info)}>
                <Code sx={{ width: '20px', height: '20px' }} />
                <SC.StyledDiv>{info.astNodeValue}</SC.StyledDiv>
              </SC.Label>
            );
          })}
        </SC.StyledTreeItem>
      );
    }

    setReferences(elements);
  };

  const jumpToRef = async (astNodeInfo: AstNodeInfo) => {
    const fileId = astNodeInfo.range?.file as string;
    router.push({
      pathname: '/project',
      query: {
        ...router.query,
        projectFileId: fileId,
        editorSelection: convertSelectionRangeToString(astNodeInfo.range?.range as Range),
        activeTab: TabName.CODE.toString(),
      } as RouterQueryType,
    });
  };

  return (
    <SC.Container>
      {info ? (
        <>
          <SC.StyledDiv sx={{ display: 'flex', alignItems: 'center', gap: '1rem' }}>
            {!info.isDirectory && (
              <SC.ParseStatus
                sx={{
                  color: (theme) =>
                    parseStatus === 2
                      ? theme.backgroundColors?.primary
                      : parseStatus === 3
                      ? theme.backgroundColors?.primary
                      : '#FFFFFF',
                  backgroundColor: (theme) =>
                    parseStatus === 2
                      ? theme.colors?.warning
                      : parseStatus === 3
                      ? theme.colors?.success
                      : theme.colors?.error,
                  border: (theme) =>
                    `1px solid ${
                      parseStatus === 2
                        ? theme.colors?.warning
                        : parseStatus === 3
                        ? theme.colors?.success
                        : theme.colors?.error
                    }`,
                }}
              >
                {getParseStatusText(parseStatus)}
              </SC.ParseStatus>
            )}
            <div>{fileName}</div>
            <div>{'::'}</div>
            <div>{filePath}</div>
            {gitBlameEnabled && <Button onClick={() => appCtx.setGitBlameInfo([])}>{'Hide Git blame'}</Button>}
          </SC.StyledDiv>
          {!info.isDirectory && !hideFileRefMenu && (
            <>
              <Button
                onClick={(e) => {
                  setAnchorEl(e.currentTarget);
                  renderFileReferences();
                }}
              >
                {'File references'}
              </Button>
              <SC.StyledMenu anchorEl={anchorEl} open={Boolean(anchorEl)} onClose={() => setAnchorEl(null)}>
                <SC.StyledTreeView
                  defaultExpandIcon={<ChevronRight />}
                  defaultEndIcon={<ChevronRight />}
                  defaultCollapseIcon={<ExpandMore />}
                  sx={{ width: 'max-content' }}
                >
                  {references}
                </SC.StyledTreeView>
              </SC.StyledMenu>
            </>
          )}
        </>
      ) : (
        <div>{'No file selected'}</div>
      )}
    </SC.Container>
  );
};
