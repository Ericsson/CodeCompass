import { Button, Table, TableHead, TableBody, TableRow, TableCell } from '@mui/material';
import { ChevronRight, Code, ExpandMore } from '@mui/icons-material';
import { AstNodeInfo, BuildLog, FileInfo, Range } from '@thrift-generated';
import React, { useContext, useEffect, useState } from 'react';
import { getCppFileReferenceCount, getCppFileReferences, getCppFileReferenceTypes } from 'service/cpp-service';
import { TabName } from 'enums/tab-enum';
import { AppContext } from 'global-context/app-context';
import { RefIcon } from 'components/custom-icon/custom-icon';
import * as SC from './styled-components';
import { convertSelectionRangeToString } from 'utils/utils';
import { useRouter } from 'next/router';
import { RouterQueryType } from 'utils/types';
import { getBuildLog } from 'service/project-service';
import { useTranslation } from 'react-i18next';

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
  const { t } = useTranslation();
  const router = useRouter();
  const appCtx = useContext(AppContext);

  const buildLogMessages = {
    0: t('fileName.buildLogs.messageTypes.unknown'),
    1: t('fileName.buildLogs.messageTypes.error'),
    2: t('fileName.buildLogs.messageTypes.fatalError'),
    3: t('fileName.buildLogs.messageTypes.warning'),
    4: t('fileName.buildLogs.messageTypes.note'),
    5: t('fileName.buildLogs.messageTypes.codingRule'),
  };

  const [anchorEl, setAnchorEl] = useState<HTMLElement | null>(null);
  const [buildLogAnchorEl, setBuildLogAnchorEl] = useState<HTMLElement | null>(null);
  const [references, setReferences] = useState<React.ReactNode[]>([]);
  const [buildLog, setBuildLog] = useState<BuildLog[]>([]);

  const getParseStatusText = (status: number): string => {
    if (status === 2) {
      return t('fileName.parseStatus.partial');
    } else if (status === 3) {
      return t('fileName.parseStatus.full');
    } else {
      return t('fileName.parseStatus.noParse');
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

  const jumpToBuildLog = (range: Range) => {
    router.push({
      pathname: '/project',
      query: {
        ...router.query,
        projectFileId: info?.id as string,
        editorSelection: convertSelectionRangeToString(range),
        activeTab: TabName.CODE.toString(),
      } as RouterQueryType,
    });
    setBuildLogAnchorEl(null);
  };

  useEffect(() => {
    if (!buildLogAnchorEl) return;
    const init = async () => {
      const initBuildLog = await getBuildLog(info?.id as string);
      setBuildLog(initBuildLog);
    };
    init();
  }, [buildLogAnchorEl, info]);

  return (
    <SC.Container>
      {info ? (
        <>
          <SC.StyledDiv sx={{ display: 'flex', alignItems: 'center', gap: '1rem' }}>
            {!info.isDirectory && (
              <>
                <SC.ParseStatus
                  onClick={(e) => setBuildLogAnchorEl(e.currentTarget)}
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
                    cursor: 'pointer',
                  }}
                >
                  {getParseStatusText(parseStatus)}
                </SC.ParseStatus>
                <SC.BuildLogMenu
                  anchorEl={buildLogAnchorEl}
                  open={Boolean(buildLogAnchorEl)}
                  onClose={() => setBuildLogAnchorEl(null)}
                >
                  <SC.BuildLogHeader>{t('fileName.buildLogs.title')}</SC.BuildLogHeader>
                  {buildLog.length ? (
                    <Table>
                      <TableHead>
                        <TableRow>
                          <TableCell>{t('fileName.buildLogs.type')}</TableCell>
                          <TableCell>{t('fileName.buildLogs.message')}</TableCell>
                          <TableCell>{t('fileName.buildLogs.from')}</TableCell>
                          <TableCell>{t('fileName.buildLogs.to')}</TableCell>
                        </TableRow>
                      </TableHead>
                      <TableBody>
                        {buildLog.map((log, idx) => (
                          <SC.BuildLogTableRow key={idx} onClick={() => jumpToBuildLog(log.range as Range)}>
                            <TableCell>{buildLogMessages[log.messageType ?? '0']}</TableCell>
                            <TableCell>{log.message}</TableCell>
                            <TableCell>{`${log.range?.startpos?.line}:${log.range?.startpos?.column}`}</TableCell>
                            <TableCell>{`${log.range?.endpos?.line}:${log.range?.endpos?.column}`}</TableCell>
                          </SC.BuildLogTableRow>
                        ))}
                      </TableBody>
                    </Table>
                  ) : (
                    <div>{t('fileName.buildLogs.noBuildLogs')}</div>
                  )}
                </SC.BuildLogMenu>
              </>
            )}
            <div>{fileName}</div>
            <div>{'::'}</div>
            <div>{filePath}</div>
            {gitBlameEnabled && (
              <Button onClick={() => appCtx.setGitBlameInfo([])}>{t('fileName.hideGitBlame')}</Button>
            )}
          </SC.StyledDiv>
          {!info.isDirectory && !hideFileRefMenu && (
            <>
              <Button
                onClick={(e) => {
                  setAnchorEl(e.currentTarget);
                  renderFileReferences();
                }}
              >
                {t('fileName.fileRefs.title')}
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
        <div>{t('fileName.noFile')}</div>
      )}
    </SC.Container>
  );
};
