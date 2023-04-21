import { useState, useContext, useEffect, SyntheticEvent } from 'react';
import { CommitListFilteredResult, GitCommit, GitRepository, ReferenceTopObjectResult } from '@thrift-generated';
import {
  getBranchList,
  getCommitListFiltered,
  getReferenceTopObject,
  getRepositoryList,
  getTagList,
  isRepositoryAvailable,
} from 'service/git-service';
import { Tooltip, alpha, styled } from '@mui/material';
import { TreeView, TreeItem, treeItemClasses } from '@mui/lab';
import { ChevronRight, ExpandMore, Commit, MoreHoriz } from '@mui/icons-material';
import { formatDate } from 'utils/utils';
import { TabName } from 'enums/tab-enum';
import { AppContext } from 'global-context/app-context';
import { getStore } from 'utils/store';
import { GitIcon } from 'components/custom-icon/custom-icon';

type RepoId = string;
type Branch = string;
type Branches = string[];
type Tags = string[];
type Commit = string;

const StyledDiv = styled('div')({});

const OuterContainer = styled('div')({
  padding: '10px',
  fontSize: '0.85rem',
  width: 'max-content',
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

export const RevisionControl = (): JSX.Element => {
  const appCtx = useContext(AppContext);

  const DISPLAYED_COMMIT_CNT: number = 15;

  const [repos, setRepos] = useState<GitRepository[]>([]);
  const [branches, setBranches] = useState<Map<RepoId, Branches>>(new Map());
  const [tags, setTags] = useState<Map<RepoId, Tags>>(new Map());
  const [topCommits, setTopCommits] = useState<Map<Branch, ReferenceTopObjectResult>>(new Map());
  const [commits, setCommits] = useState<Map<Branch, CommitListFilteredResult>>(new Map());
  const [commitOffsets, setCommitOffsets] = useState<Map<Branch, number>>(new Map());

  const [expandedTreeNodes, setExpandedTreeNodes] = useState<string[]>([]);

  useEffect(() => {
    if (!appCtx.workspaceId) return;

    const init = async () => {
      const isGitRepo = await isRepositoryAvailable();
      if (!isGitRepo) return;

      const initRepos = await getRepositoryList();
      const initExpandedTreeNodes: typeof expandedTreeNodes = [];

      for (const repo of initRepos) {
        initExpandedTreeNodes.push(repo.id as string);
      }

      const { storedGitRepoId, storedGitBranch } = getStore();

      if (storedGitRepoId && storedGitBranch) {
        const initBranches: typeof branches = new Map();
        const initCommitOffsets: typeof commitOffsets = new Map();

        const repoBranches = await getBranchList(storedGitRepoId);
        for (const repoBranch of repoBranches) {
          initCommitOffsets.set(repoBranch, 0);
        }
        initBranches.set(storedGitRepoId, repoBranches);
        initExpandedTreeNodes.push(`${storedGitRepoId}-${storedGitBranch}`);
        initExpandedTreeNodes.push(`${storedGitRepoId}-branches`);

        const initTopCommits: typeof topCommits = new Map();
        const initCommits: typeof commits = new Map();

        const topCommit = (await getReferenceTopObject(storedGitRepoId, storedGitBranch)) as ReferenceTopObjectResult;
        initTopCommits.set(storedGitBranch, topCommit);
        const commitList = (await getCommitListFiltered(
          storedGitRepoId,
          topCommit.oid as string,
          DISPLAYED_COMMIT_CNT,
          0,
          ''
        )) as CommitListFilteredResult;
        initCommits.set(storedGitBranch, commitList);

        setTopCommits(initTopCommits);
        setCommits(initCommits);

        setBranches(initBranches);
        setCommitOffsets(initCommitOffsets);
      }

      setRepos(initRepos);
      setExpandedTreeNodes(initExpandedTreeNodes);
    };
    init();
  }, [appCtx.workspaceId]);

  const loadBranches = async (repoId: string) => {
    if (branches.get(repoId)) return;

    const initBranches: typeof branches = new Map();
    const initCommitOffsets: typeof commitOffsets = new Map(commitOffsets);

    const repoBranches = await getBranchList(repoId);
    for (const repoBranch of repoBranches) {
      initCommitOffsets.set(repoBranch, 0);
    }
    initBranches.set(repoId, repoBranches);

    setBranches(initBranches);
    setCommitOffsets(initCommitOffsets);
  };

  const loadTags = async (repoId: string) => {
    if (tags.get(repoId)) return;

    const initTags: typeof tags = new Map();
    const initCommitOffsets: typeof commitOffsets = new Map(commitOffsets);

    const repoTags = await getTagList(repoId);
    for (const repoTag of repoTags) {
      initCommitOffsets.set(repoTag, 0);
    }
    initTags.set(repoId, repoTags);

    setTags(initTags);
    setCommitOffsets(initCommitOffsets);
  };

  const loadInitialCommits = async (repoId: string, branch: string) => {
    if (commits.get(branch)) return;

    const initTopCommits: typeof topCommits = new Map(topCommits);
    const initCommits: typeof commits = new Map(commits);

    const topCommit = (await getReferenceTopObject(repoId, branch)) as ReferenceTopObjectResult;
    initTopCommits.set(branch, topCommit);
    const commitList = (await getCommitListFiltered(
      repoId,
      topCommit.oid as string,
      DISPLAYED_COMMIT_CNT,
      0,
      ''
    )) as CommitListFilteredResult;
    initCommits.set(branch, commitList);

    setTopCommits(initTopCommits);
    setCommits(initCommits);
  };

  const loadCommits = async (repoId: string, branch: string) => {
    const currentOffset = commitOffsets.get(branch) as number;
    const topCommitId = topCommits.get(branch)?.oid as string;
    const moreCommits = (await getCommitListFiltered(
      repoId,
      topCommitId,
      DISPLAYED_COMMIT_CNT,
      currentOffset + 1 + DISPLAYED_COMMIT_CNT,
      ''
    )) as CommitListFilteredResult;

    const newCommitOffsets: typeof commitOffsets = new Map(commitOffsets);
    newCommitOffsets.set(branch, currentOffset + DISPLAYED_COMMIT_CNT);

    const newCommits: typeof commits = new Map(commits);
    const currentResults: GitCommit[] = commits.get(branch)?.result as GitCommit[];
    const newResults: GitCommit[] = moreCommits.result as GitCommit[];
    newCommits.set(
      branch,
      new CommitListFilteredResult({
        ...commits.get(branch),
        ...moreCommits,
        result: currentResults.concat(newResults),
      })
    );

    setCommits(newCommits);
    setCommitOffsets(newCommitOffsets);
  };

  const getCommitDiff = async (repoId: string, branch: string, commitId: string) => {
    appCtx.setGitRepoId(repoId);
    appCtx.setGitBranch(branch);
    appCtx.setGitCommitId(commitId);
    appCtx.setActiveTab(TabName.GIT_DIFF);
  };

  const RenderedCommits = ({
    commitResults,
    repoId,
    branch,
  }: {
    commitResults: GitCommit[];
    repoId: string;
    branch: string;
  }): JSX.Element => {
    return (
      <>
        {commitResults.map((commit) => (
          <Label
            key={commit.oid}
            onClick={() => getCommitDiff(repoId, branch, commit.oid as string)}
            sx={{
              backgroundColor: (theme) =>
                commit.oid === appCtx.gitCommitId ? alpha(theme.backgroundColors?.secondary as string, 0.3) : '',
            }}
          >
            <Tooltip
              title={
                <StyledDiv sx={{ width: 'max-content' }}>
                  <div>{`#${commit.oid?.substring(0, 8)}`}</div>
                  <div>{commit.message}</div>
                  <div>{`${commit.author?.name} (${commit.author?.email})`}</div>
                  <div>{`Commited on ${formatDate(new Date((commit.time as unknown as number) * 1000))}`}</div>
                </StyledDiv>
              }
              placement={'top-start'}
              componentsProps={{
                tooltip: {
                  sx: {
                    fontSize: '0.85rem',
                    padding: '10px',
                    width: '400px',
                    height: 'auto',
                    overflow: 'scroll',
                  },
                },
              }}
            >
              <GitIcon name={'commit'} />
            </Tooltip>
            <StyledDiv sx={{ display: 'flex', flexDirection: 'column' }}>
              <StyledDiv>{`${commit.message} (${commit.time})`}</StyledDiv>
              <StyledDiv>{`${commit.author?.name} (${commit.author?.email})`}</StyledDiv>
            </StyledDiv>
          </Label>
        ))}
      </>
    );
  };

  return repos.length ? (
    <OuterContainer>
      <div>{'List of repositories'}</div>
      <StyledTreeView
        defaultExpandIcon={<ChevronRight />}
        defaultEndIcon={<ChevronRight />}
        defaultCollapseIcon={<ExpandMore />}
        sx={{ width: 'max-content' }}
        expanded={expandedTreeNodes}
        onNodeSelect={(_e: SyntheticEvent<Element, Event>, nodeId: string) => {
          const index = expandedTreeNodes.indexOf(nodeId) as number;
          const copyExpanded = [...expandedTreeNodes];
          if (index === -1) {
            copyExpanded.push(nodeId);
          } else {
            copyExpanded.splice(index, 1);
          }
          setExpandedTreeNodes(copyExpanded);
        }}
      >
        {repos.map((repo) => (
          <StyledTreeItem
            nodeId={`${repo.id as string}`}
            key={repo.id as string}
            label={<StyledDiv sx={{ fontSize: '0.85rem' }}>{`Repository of ${repo.name} (${repo.path})`}</StyledDiv>}
            icon={<GitIcon name={'repository'} outlined={expandedTreeNodes.includes(`${repo.id as string}`)} />}
          >
            <StyledTreeItem
              nodeId={`${repo.id as string}-branches`}
              label={<StyledDiv sx={{ fontSize: '0.85rem' }}>{'Branches'}</StyledDiv>}
              icon={<GitIcon name={'branch'} outlined={expandedTreeNodes.includes(`${repo.id as string}-branches`)} />}
              onClick={() => loadBranches(repo.id as string)}
            >
              {branches.get(repo.id as string)?.length ? (
                branches.get(repo.id as string)?.map((branch) => (
                  <StyledTreeItem
                    nodeId={`${repo.id as string}-${branch}`}
                    key={`${repo.id as string}-${branch}`}
                    label={<StyledDiv sx={{ fontSize: '0.85rem' }}>{`Commits in ${branch}`}</StyledDiv>}
                    icon={
                      <GitIcon
                        name={'commit'}
                        outlined={expandedTreeNodes.includes(`${repo.id as string}-${branch}`)}
                      />
                    }
                    onClick={() => loadInitialCommits(repo.id as string, branch)}
                  >
                    <RenderedCommits
                      commitResults={commits.get(branch) ? (commits.get(branch)?.result as GitCommit[]) : []}
                      repoId={repo.id as string}
                      branch={branch}
                    />
                    <Label
                      hidden={!commits.get(branch)?.hasRemaining}
                      onClick={() => loadCommits(repo.id as string, branch)}
                    >
                      <MoreHoriz />
                      <StyledDiv>{'Load more'}</StyledDiv>
                    </Label>
                  </StyledTreeItem>
                ))
              ) : (
                <div>{'Loading'}</div>
              )}
            </StyledTreeItem>
            <StyledTreeItem
              nodeId={`${repo.id as string}-tags`}
              label={<StyledDiv sx={{ fontSize: '0.85rem' }}>{'Tags'}</StyledDiv>}
              icon={<GitIcon name={'tag'} outlined={expandedTreeNodes.includes(`${repo.id as string}-tags`)} />}
              onClick={() => loadTags(repo.id as string)}
            >
              {tags.get(repo.id as string)?.length ? (
                tags.get(repo.id as string)?.map((tag) => (
                  <StyledTreeItem
                    nodeId={`${repo.id as string}-${tag}`}
                    key={`${repo.id as string}-${tag}`}
                    label={<StyledDiv sx={{ fontSize: '0.85rem' }}>{`Commits in ${tag}`}</StyledDiv>}
                    icon={
                      <GitIcon name={'commit'} outlined={expandedTreeNodes.includes(`${repo.id as string}-${tag}`)} />
                    }
                    onClick={() => loadInitialCommits(repo.id as string, tag)}
                  >
                    <RenderedCommits
                      commitResults={commits.get(tag) ? (commits.get(tag)?.result as GitCommit[]) : []}
                      repoId={tag}
                      branch={tag}
                    />
                    <Label hidden={!commits.get(tag)?.hasRemaining} onClick={() => loadCommits(repo.id as string, tag)}>
                      <MoreHoriz />
                      <StyledDiv>{'Load more'}</StyledDiv>
                    </Label>
                  </StyledTreeItem>
                ))
              ) : (
                <div>{'Loading'}</div>
              )}
            </StyledTreeItem>
          </StyledTreeItem>
        ))}
      </StyledTreeView>
    </OuterContainer>
  ) : (
    <StyledDiv>{'No repositories available.'}</StyledDiv>
  );
};
