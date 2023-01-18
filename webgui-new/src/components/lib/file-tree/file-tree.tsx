import { TreeItem, TreeView } from '@mui/lab';
import { styled } from '@mui/material';

const StyledTreeView = styled(TreeView)(({ theme }) => ({
  color: theme.colors?.primary,
  backgroundColor: theme.backgroundColors?.primary,
}));

export const FileTree = (): JSX.Element => {
  return (
    <StyledTreeView>
      <TreeItem nodeId={'1'} label={'package.json'} />
      <TreeItem nodeId={'2'} label={'src'}>
        <TreeItem nodeId={'3'} label={'app.ts'} />
      </TreeItem>
    </StyledTreeView>
  );
};
