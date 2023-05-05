import { TextSnippet } from '@mui/icons-material';
import { ToggleButton, ToggleButtonGroup } from '@mui/material';
import { ThemeContext } from 'global-context/theme-context';
import React, { SyntheticEvent, useContext, useEffect, useState } from 'react';
import ReactDiffViewer from 'react-diff-viewer-continued';
import { getCommit, getCommitDiffAsString } from 'service/git-service';
import { formatDate } from 'utils/utils';
import { GitCommit } from '@thrift-generated';
import { AppContext } from 'global-context/app-context';
import { diffViewerTheme } from 'themes/theme';
import { blue, red } from '@mui/material/colors';
import { FileIcon } from 'components/custom-icon/custom-icon';
import * as SC from './styled-components';

export const GitDiff = () => {
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

  return commit && commit.repoId ? (
    <>
      <SC.CommitSummary>
        <div>{commit.message}</div>
        <div>
          <div>
            <span style={{ fontWeight: 'bold' }}>{`${commit.author?.name} (${commit.author?.email})`}</span>
            {`, Commited on ${formatDate(new Date((commit.time as unknown as number) * 1000))}`}
          </div>
          <div>
            <span>{`${commit.parentOids?.length} parent(s)`}</span>{' '}
            {commit.parentOids?.map((id) => (
              <SC.StyledSpan key={id} sx={{ color: blue[800] }}>
                {`#${id.substring(0, 8)} `}
              </SC.StyledSpan>
            ))}
            <span>{`commit `}</span>
            <SC.StyledSpan sx={{ color: red[800] }}>{`#${commit.oid?.substring(0, 8)}`}</SC.StyledSpan>
          </div>
        </div>
      </SC.CommitSummary>
      <SC.DiffViewOptions>
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
      </SC.DiffViewOptions>
      <SC.DiffViewContainer>
        <SC.StyledTreeView
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
            <SC.StyledTreeItem
              key={idx}
              nodeId={`${idx}`}
              label={<SC.FileNameContainer>{fileName}</SC.FileNameContainer>}
              icon={<FileIcon fileName={fileName} />}
            >
              {Array.from(oldValues.get(fileName)?.keys() ?? []).map((change, cIdx) => (
                <div key={cIdx}>
                  <SC.ChangeLine>{change}</SC.ChangeLine>
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
            </SC.StyledTreeItem>
          ))}
        </SC.StyledTreeView>
      </SC.DiffViewContainer>
    </>
  ) : (
    <SC.Placeholder>{'No commit selected or commit information could not be loaded.'}</SC.Placeholder>
  );
};
