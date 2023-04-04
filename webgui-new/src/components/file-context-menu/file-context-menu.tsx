import { Dispatch, SetStateAction, useContext } from 'react';
import Menu from '@mui/material/Menu';
import MenuItem from '@mui/material/MenuItem';
import { ConfigContext } from 'global-context/config-context';
import { TabName } from 'enums/tab-enum';
import { FileInfo } from '@thrift-generated';
import { LanguageContext } from 'global-context/language-context';
import { MetricsContext } from 'global-context/metrics-context';

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
  const configCtx = useContext(ConfigContext);
  const languageCtx = useContext(LanguageContext);
  const metricsCtx = useContext(MetricsContext);

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
          metricsCtx.setMetricsFileInfo(fileInfo);
          configCtx.setActiveTab(TabName.METRICS);
        }}
      >
        {'Metrics'}
      </MenuItem>
      <MenuItem
        onClick={() => {
          setContextMenu(null);
          languageCtx.setDiagramInfo(fileInfo);
          configCtx.setActiveTab(TabName.DIAGRAMS);
        }}
      >
        {'Diagrams'}
      </MenuItem>
    </Menu>
  );
};
