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
            appCtx.setMetricsGenId(fInfo?.id as string);
            appCtx.setActiveTab(TabName.METRICS);
          }}
        >
          {'Metrics'}
        </MenuItem>
      )}
      {fileInfo && !fileInfo.isDirectory && (
        <MenuItem
          onClick={async () => {
            setContextMenu(null);
            const currentRepo = await getRepositoryByProjectPath(fileInfo.path as string);
            const srcPath = appCtx.labels.get('src');
            const filePath = fileInfo.path as string;
            const path = filePath.replace(new RegExp(`^${srcPath}`), '').slice(1);
            const blameInfo = await getBlameInfo(
              currentRepo?.repoId as string,
              currentRepo?.commitId as string,
              path as string,
              fileInfo.id as string
            );
            appCtx.setGitBlameInfo(blameInfo);
            appCtx.setActiveTab(TabName.CODE);
          }}
        >
          {'Git blame'}
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
                    appCtx.setDiagramGenId(fileInfo.id as string);
                    appCtx.setDiagramTypeId(diagramTypes.get(type) as number);
                    appCtx.setDiagramType('file');
                    appCtx.setActiveTab(TabName.DIAGRAMS);
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
            <div>{'Diagrams'}</div>
            <ChevronRight />
          </MenuItem>
        </Tooltip>
      ) : (
        ''
      )}
    </Menu>
  );
};
