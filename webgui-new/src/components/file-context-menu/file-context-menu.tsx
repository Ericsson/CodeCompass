import { Dispatch, SetStateAction, useContext } from 'react';
import Menu from '@mui/material/Menu';
import MenuItem from '@mui/material/MenuItem';
import { TabName } from 'enums/tab-enum';
import { FileInfo } from '@thrift-generated';
import { AppContext } from 'global-context/app-context';

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
}) => {
  const appCtx = useContext(AppContext);

  return (
    <Menu
      open={contextMenu !== null}
      onClose={() => setContextMenu(null)}
      anchorReference="anchorPosition"
      anchorPosition={contextMenu !== null ? { top: contextMenu.mouseY, left: contextMenu.mouseX } : undefined}
    >
      <MenuItem
        onClick={() => {
          setContextMenu(null);
          appCtx.setMetricsGenId(fileInfo.id as string);
          appCtx.setActiveTab(TabName.METRICS);
        }}
      >
        {'Metrics'}
      </MenuItem>
      <MenuItem
        onClick={() => {
          setContextMenu(null);
          appCtx.setDiagramGenId(fileInfo.id as string);
          appCtx.setActiveTab(TabName.DIAGRAMS);
        }}
      >
        {'Diagrams'}
      </MenuItem>
    </Menu>
  );
};
