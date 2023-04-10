import { TextSnippet } from '@mui/icons-material';
import { TreeView, TreeItem, treeItemClasses } from '@mui/lab';
import { ToggleButton, ToggleButtonGroup, alpha, styled } from '@mui/material';
import { ThemeContext } from 'global-context/theme-context';
import { SyntheticEvent, useContext, useEffect, useState } from 'react';
import ReactDiffViewer from 'react-diff-viewer-continued';
import { getCommit, getCommitDiffAsString } from 'service/git-service';
import { formatDate } from 'utils/utils';
import { GitCommit } from '@thrift-generated';
import { AppContext } from 'global-context/app-context';
import { diffViewerTheme } from 'themes/theme';

const CommitSummary = styled('div')(({ theme }) => ({
  display: 'flex',
  flexDirection: 'column',
  justifyContent: 'center',
  borderBottom: `1px solid ${theme.colors?.primary}`,
  padding: '5px',
  fontSize: '0.85rem',
  height: '65px',
  overflow: 'scroll',

  '& > div:nth-of-type(2)': {
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'space-between',
  },
}));

const FileNameContainer = styled('div')(({ theme }) => ({
  padding: '10px',
  fontSize: '0.85rem',
}));

const ChangeLine = styled('div')(({ theme }) => ({
  padding: '10px',
  color: theme.colors?.primary,
  backgroundColor: alpha(theme.backgroundColors?.secondary as string, 0.2),
}));

const DiffViewOptions = styled('div')({
  display: 'flex',
  alignItems: 'center',
  height: '60px',
  padding: '10px',
});

const DiffViewContainer = styled('div')({
  width: 'calc(100vw - 280px)',
  height: 'calc(100vh - 78px - 48px - 70px - 65px)',
  overflow: 'scroll',
});

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

export const GitDiff = (): JSX.Element => {
  const appCtx = useContext(AppContext);
  const themeCtx = useContext(ThemeContext);

  const [splitView, setSplitView] = useState<boolean>(false);
  const [commit, setCommit] = useState<GitCommit | undefined>(undefined);
  const [oldValues, setOldValues] = useState<Map<string, Map<string, string>>>(new Map());
  const [newValues, setNewValues] = useState<Map<string, Map<string, string>>>(new Map());
  const [expandedFiles, setExpandedFiles] = useState<string[]>([]);

  useEffect(() => {
    if (!appCtx.gitRepoId || !appCtx.gitCommitId) {
      setSplitView(false);
      setCommit(undefined);
      setOldValues(new Map());
      setNewValues(new Map());
      setExpandedFiles([]);
      return;
    }

    const init = async () => {
      const initOldValues: typeof oldValues = new Map();
      const initNewValues: typeof newValues = new Map();
      const initExpandedFiles: string[] = [];

      const initCommit = await getCommit(appCtx.gitRepoId, appCtx.gitCommitId);
      const initialDiff = await getCommitDiffAsString(appCtx.gitRepoId, appCtx.gitCommitId, true, 3);

      const lines = initialDiff.split(/\r?\n/);
      const diffLines = lines.filter((line) => line.startsWith('diff --git'));

      const gitDiffs: string[] = [];
      for (const diffLine of diffLines) {
        const diffFiles = diffLine
          .split(' ')
          .slice(2, 4)
          .map((fname) => fname.split('/')[1]);
        const fileName = diffFiles[0] === diffFiles[1] ? diffFiles[0] : `${diffFiles[0]} => ${diffFiles[1]}`;
        const diff = await getCommitDiffAsString(appCtx.gitRepoId, appCtx.gitCommitId, false, 3, [fileName]);
        gitDiffs.push(diff);
      }

      let fIdx = 0;
      for (const diff of gitDiffs) {
        const fileName = (
          diff
            .split('\n')
            .find((line) => line.startsWith('diff --git'))
            ?.split(' ')
            .slice(2, 4)
            .map((fname) => fname.split('/')[1]) as string[]
        )[0];

        const lines = diff.split(/\r?\n/).slice(4);
        const changeLines = lines.filter((line) => line.startsWith('@@'));
        const changesByLine = lines
          .join('\n')
          .split(/^@@.*$/m)
          .slice(1);

        const oldChanges: Map<string, string> = new Map();
        const newChanges: Map<string, string> = new Map();

        for (const changeLine of changeLines) {
          const oldLineValues: string[] = [];
          const newLineValues: string[] = [];

          let i = 0;
          for (const changeByLine of changesByLine[i].split('\n').slice(1)) {
            if (changeByLine.startsWith('-')) {
              oldLineValues.push(changeByLine.slice(1));
            } else if (changeByLine.startsWith('+')) {
              newLineValues.push(changeByLine.slice(1));
            } else {
              oldLineValues.push(changeByLine);
              newLineValues.push(changeByLine);
            }
          }

          oldChanges.set(changeLine, oldLineValues.join('\n'));
          newChanges.set(changeLine, newLineValues.join('\n'));
          ++i;
        }

        initOldValues.set(fileName, oldChanges);
        initNewValues.set(fileName, newChanges);
        initExpandedFiles.push(fIdx.toString());
        ++fIdx;
      }

      setCommit(initCommit);
      setOldValues(initOldValues);
      setNewValues(initNewValues);
      setExpandedFiles(initExpandedFiles);
    };
    init();
  }, [appCtx.gitRepoId, appCtx.gitCommitId]);

  const getLineOffset = (change: string): number => {
    const offset = parseInt(change.split(' ')[1].slice(1, change.split(' ')[1].indexOf(',')));
    return offset === 0 ? 1 : offset - 1;
  };

  return commit ? (
    <div>
      <CommitSummary>
        <div>{commit.message}</div>
        <div>
          <div>{`${commit.author?.name} (${commit.author?.email}), Commited on ${formatDate(
            new Date((commit.time as unknown as number) * 1000)
          )}`}</div>
          <div>{`${commit.parentOids?.length} parent(s) ${commit.parentOids?.map((id) =>
            id.substring(0, 8)
          )} commit ${commit.oid?.substring(0, 8)}`}</div>
        </div>
      </CommitSummary>
      <DiffViewOptions>
        <ToggleButtonGroup
          color={'primary'}
          value={splitView ? 'Side-by-side' : 'Inline'}
          exclusive
          onChange={(_e, newAlignment) => setSplitView(newAlignment === 'Side-by-side')}
        >
          <ToggleButton sx={{ textTransform: 'none' }} value={'Inline'}>
            {'Inline'}
          </ToggleButton>
          <ToggleButton sx={{ textTransform: 'none' }} value={'Side-by-side'}>
            {'Side-by-side'}
          </ToggleButton>
        </ToggleButtonGroup>
      </DiffViewOptions>
      <DiffViewContainer>
        <StyledTreeView
          defaultCollapseIcon={<TextSnippet />}
          defaultExpandIcon={<TextSnippet />}
          expanded={expandedFiles}
          onNodeSelect={(_e: SyntheticEvent<Element, Event>, nodeId: string) => {
            const index = expandedFiles.indexOf(nodeId) as number;
            const copyExpanded = [...expandedFiles];
            if (index === -1) {
              copyExpanded.push(nodeId);
            } else {
              copyExpanded.splice(index, 1);
            }
            setExpandedFiles(copyExpanded);
          }}
        >
          {Array.from(oldValues.keys()).map((fileName, idx) => (
            <StyledTreeItem key={idx} nodeId={`${idx}`} label={<FileNameContainer>{fileName}</FileNameContainer>}>
              {Array.from(oldValues.get(fileName)?.keys() ?? []).map((change, cIdx) => (
                <div key={cIdx}>
                  <ChangeLine>{change}</ChangeLine>
                  <ReactDiffViewer
                    styles={diffViewerTheme}
                    oldValue={oldValues.get(fileName)?.get(change)}
                    newValue={newValues.get(fileName)?.get(change)}
                    splitView={splitView}
                    useDarkTheme={themeCtx.theme === 'dark'}
                    linesOffset={getLineOffset(change)}
                  />
                </div>
              ))}
            </StyledTreeItem>
          ))}
        </StyledTreeView>
      </DiffViewContainer>
    </div>
  ) : (
    <div>{'No commit selected'}</div>
  );
};
