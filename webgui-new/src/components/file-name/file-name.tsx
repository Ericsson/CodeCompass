import { TreeView, TreeItem, treeItemClasses } from '@mui/lab';
import { alpha, Button, Menu, styled } from '@mui/material';
import { ChevronRight, Code, ExpandMore } from '@mui/icons-material';
import { AstNodeInfo, FileInfo, Range } from '@thrift-generated';
import { useContext, useState } from 'react';
import { getCppFileReferenceCount, getCppFileReferences, getCppFileReferenceTypes } from 'service/cpp-service';
import { TabName } from 'enums/tab-enum';
import { AppContext } from 'global-context/app-context';
import { RefIcon } from 'components/custom-icon/custom-icon';

const Container = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  justifyContent: 'space-between',
  gap: '0.6rem',
  padding: '0 15px',
  minHeight: '49px',
  borderBottom: `1px solid ${theme.colors?.primary}`,
  fontSize: '0.85rem',
}));

const ParseStatus = styled('div')({
  borderRadius: '5px',
  padding: '5px',
});

const StyledDiv = styled('div')({});

const StyledMenu = styled(Menu)(({ theme }) => ({
  '.MuiMenu-list': {
    border: `1px solid ${theme.colors?.primary}`,
    width: '500px',
    maxHeight: '500px',
    overflow: 'scroll',
  },
}));

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

export const FileName = ({
  fileName,
  filePath,
  parseStatus,
  info,
}: {
  fileName: string;
  filePath: string;
  parseStatus: number;
  info: FileInfo | undefined;
}): JSX.Element => {
  const appCtx = useContext(AppContext);

  const [anchorEl, setAnchorEl] = useState<HTMLElement | null>(null);
  const [references, setReferences] = useState<JSX.Element[]>([]);

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

    const elements: JSX.Element[] = [];
    for (const [key, value] of fileRefTypes) {
      const refCountForType = await getCppFileReferenceCount(info?.id as string, value);
      const refsForType = await getCppFileReferences(info?.id as string, value);

      elements.push(
        <StyledTreeItem
          key={value}
          nodeId={`${value}`}
          label={
            <StyledDiv sx={{ fontSize: '0.85rem' }}>
              {key} ({refCountForType})
            </StyledDiv>
          }
          icon={<RefIcon refName={key} />}
        >
          {refsForType.map((info) => {
            return (
              <Label key={info.id} onClick={() => jumpToRef(info)}>
                <Code sx={{ width: '20px', height: '20px' }} />
                <StyledDiv>{info.astNodeValue}</StyledDiv>
              </Label>
            );
          })}
        </StyledTreeItem>
      );
    }

    setReferences(elements);
  };

  const jumpToRef = async (astNodeInfo: AstNodeInfo) => {
    const fileId = astNodeInfo.range?.file as string;
    appCtx.setProjectFileId(fileId);
    appCtx.setEditorSelection(astNodeInfo.range?.range as Range);
    appCtx.setActiveTab(TabName.CODE);
  };

  return (
    <Container>
      {info ? (
        <>
          <StyledDiv sx={{ display: 'flex', alignItems: 'center', gap: '1rem' }}>
            {!info.isDirectory && (
              <ParseStatus
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
              </ParseStatus>
            )}
            <div>{fileName}</div>
            <div>{'::'}</div>
            <div>{filePath}</div>
          </StyledDiv>
          {!info.isDirectory ? (
            <>
              <Button
                sx={{ textTransform: 'none' }}
                onClick={(e) => {
                  setAnchorEl(e.currentTarget);
                  renderFileReferences();
                }}
              >
                {'File references'}
              </Button>
              <StyledMenu anchorEl={anchorEl} open={Boolean(anchorEl)} onClose={() => setAnchorEl(null)}>
                <StyledTreeView
                  defaultExpandIcon={<ChevronRight />}
                  defaultEndIcon={<ChevronRight />}
                  defaultCollapseIcon={<ExpandMore />}
                  sx={{ width: 'max-content' }}
                >
                  {references}
                </StyledTreeView>
              </StyledMenu>
            </>
          ) : (
            ''
          )}
        </>
      ) : (
        <div>{'No file selected'}</div>
      )}
    </Container>
  );
};
