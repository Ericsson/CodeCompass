import { Menu, MenuItem } from '@mui/material';
import { FileInfo } from '@thrift-generated';
import { Dispatch, SetStateAction } from 'react';

export const FileContextMenu = ({
  contextMenu,
  setContextMenu,
  fileInfo,
}: {
  contextMenu: { mouseX: number; mouseY: number } | null;
  setContextMenu: Dispatch<SetStateAction<{ mouseX: number; mouseY: number } | null>>;
  fileInfo: FileInfo | undefined;
}) => {
  const handleClose = () => {
    setContextMenu(null);
  };

  return fileInfo ? (
    <Menu
      open={contextMenu !== null}
      onClose={handleClose}
      anchorReference='anchorPosition'
      anchorPosition={contextMenu !== null ? { top: contextMenu.mouseY, left: contextMenu.mouseX } : undefined}
    >
      {'context menu'}
    </Menu>
  ) : (
    <></>
  );
};
