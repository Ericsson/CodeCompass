import React, { Dispatch, SetStateAction, useContext, useEffect, useState } from 'react';
import Menu from '@mui/material/Menu';
import MenuItem from '@mui/material/MenuItem';
import { TabName } from 'enums/tab-enum';
import { FileInfo } from '@thrift-generated';
import { AppContext } from 'global-context/app-context';
import { getCppFileDiagramTypes } from 'service/cpp-service';
import { Tooltip } from '@mui/material';
import { ChevronRight } from '@mui/icons-material';
import { getFileInfoByPath } from 'service/project-service';
import { getBlameInfo, getRepositoryByProjectPath } from 'service/git-service';
import { useRouter } from 'next/router';
import { RouterQueryType } from 'utils/types';
import { useTranslation } from 'react-i18next';

export const FileContextMenu = ({
  contextMenu,
  setContextMenu,
  fileInfo,
}: {
  contextMenu: {
    mouseX: number;
    mouseY: number;
  } | null;
  setContextMenu: Dispatch<
    SetStateAction<{
      mouseX: number;
      mouseY: number;
    } | null>
  >;
  fileInfo: FileInfo;
}): JSX.Element => {
  const { t } = useTranslation();
  const router = useRouter();
  const appCtx = useContext(AppContext);

  const [diagramTypes, setDiagramTypes] = useState<Map<string, number>>(new Map());

  useEffect(() => {
    if (!fileInfo) return;
    const init = async () => {
      const initDiagramTypes = await getCppFileDiagramTypes(fileInfo.id as string);
      setDiagramTypes(initDiagramTypes);
    };
    init();
  }, [fileInfo]);

  const getGitBlameInfo = async () => {
    setContextMenu(null);
    const currentRepo = await getRepositoryByProjectPath(fileInfo.path as string);
    const blameInfo = await getBlameInfo(
      currentRepo?.repoId as string,
      currentRepo?.commitId as string,
      currentRepo?.repoPath as string,
      fileInfo.id as string
    );
    return blameInfo;
  };

  return (
    <Menu
      open={contextMenu !== null}
      onClose={() => setContextMenu(null)}
      anchorReference="anchorPosition"
      anchorPosition={contextMenu !== null ? { top: contextMenu.mouseY, left: contextMenu.mouseX } : undefined}
    >
      {fileInfo && fileInfo.isDirectory && (
        <MenuItem
          onClick={async () => {
            setContextMenu(null);
            const sourceFolder = appCtx.labels.get('src') as string;
            const fInfo = fileInfo.path?.includes(sourceFolder) ? fileInfo : await getFileInfoByPath(sourceFolder);
            router.push({
              pathname: '/project',
              query: {
                ...router.query,
                metricsGenId: fInfo?.id as string,
                activeTab: TabName.METRICS.toString(),
              } as RouterQueryType,
            });
          }}
        >
          {t('fileContextMenu.metrics')}
        </MenuItem>
      )}
      {fileInfo && !fileInfo.isDirectory && (
        <MenuItem
          onClick={async () => {
            const blameInfo = await getGitBlameInfo();
            appCtx.setGitBlameInfo(blameInfo);
          }}
        >
          {t('fileContextMenu.gitBlame')}
        </MenuItem>
      )}
      {diagramTypes.size !== 0 ? (
        <Tooltip
          title={
            <>
              {Array.from(diagramTypes.keys()).map((type) => (
                <MenuItem
                  key={diagramTypes.get(type)}
                  onClick={() => {
                    setContextMenu(null);
                    router.push({
                      pathname: '/project',
                      query: {
                        ...router.query,
                        diagramGenId: fileInfo.id as string,
                        diagramTypeId: (diagramTypes.get(type) as number).toString(),
                        diagramType: 'file',
                        activeTab: TabName.DIAGRAMS.toString(),
                      } as RouterQueryType,
                    });
                  }}
                >
                  {type}
                </MenuItem>
              ))}
            </>
          }
          placement={'right-start'}
        >
          <MenuItem sx={{ display: 'flex', alignItems: 'center', gap: '1rem' }}>
            <div>{t('fileContextMenu.diagrams')}</div>
            <ChevronRight />
          </MenuItem>
        </Tooltip>
      ) : (
        ''
      )}
    </Menu>
  );
};
