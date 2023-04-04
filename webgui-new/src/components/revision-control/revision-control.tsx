import { GitContext } from 'global-context/git-context';
import { useState, useContext, useEffect } from 'react';
import { CommitListFilteredResult, GitCommit, GitRepository, ReferenceTopObjectResult } from '@thrift-generated';
import {
  getBranchList,
  getCommitListFiltered,
  getReferenceTopObject,
  getRepositoryList,
  getTagList,
} from 'service/git-service';
import { Tooltip, alpha, styled } from '@mui/material';
import { TreeView, TreeItem, treeItemClasses } from '@mui/lab';
import { ChevronRight, ExpandMore, Commit, MoreHoriz } from '@mui/icons-material';
import { formatDate } from 'utils/utils';

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
  const gitContext = useContext(GitContext);

  const DISPLAYED_COMMIT_CNT: number = 15;

  const [repos, setRepos] = useState<GitRepository[]>([]);
  const [branches, setBranches] = useState<Map<RepoId, Branches>>(new Map());
  const [tags, setTags] = useState<Map<RepoId, Tags>>(new Map());
  const [topCommits, setTopCommits] = useState<Map<Branch, ReferenceTopObjectResult>>(new Map());
  const [commits, setCommits] = useState<Map<Branch, CommitListFilteredResult>>(new Map());
  const [commitOffsets, setCommitOffsets] = useState<Map<Branch, number>>(new Map());

  useEffect(() => {
    const init = async () => {
      const initRepos = await getRepositoryList();
      const initBranches: typeof branches = new Map();
      const initTags: typeof tags = new Map();
      const initTopCommits: typeof topCommits = new Map();
      const initCommits: typeof commits = new Map();
      const initCommitOffsets: typeof commitOffsets = new Map();

      for (const repo of initRepos) {
        const repoBranches = await getBranchList(repo.id as string);
        const repoTags = await getTagList(repo.id as string);

        for (const branch of repoBranches) {
          const topCommit = (await getReferenceTopObject(repo.id as string, branch)) as ReferenceTopObjectResult;
          initTopCommits.set(branch, topCommit);

          const commitList = (await getCommitListFiltered(
            repo.id as string,
            topCommit.oid as string,
            DISPLAYED_COMMIT_CNT,
            0,
            ''
          )) as CommitListFilteredResult;

          initCommits.set(branch, commitList);
          initCommitOffsets.set(branch, 0);
        }

        for (const tag of repoTags) {
          const topCommit = (await getReferenceTopObject(repo.id as string, tag)) as ReferenceTopObjectResult;
          initTopCommits.set(tag, topCommit);

          const commitList = (await getCommitListFiltered(
            repo.id as string,
            topCommit.oid as string,
            DISPLAYED_COMMIT_CNT,
            0,
            ''
          )) as CommitListFilteredResult;

          initCommits.set(tag, commitList);
          initCommitOffsets.set(tag, 0);
        }

        initBranches.set(repo.id as string, repoBranches);
        initTags.set(repo.id as string, repoTags);
      }

      setRepos(initRepos);
      setBranches(initBranches);
      setTags(initTags);
      setTopCommits(initTopCommits);
      setCommits(initCommits);
      setCommitOffsets(initCommitOffsets);
    };
    init();
  }, []);

  const loadMoreCommits = async (repoId: string, branch: string) => {
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

  const RenderedCommits = ({ commitResults }: { commitResults: GitCommit[] }): JSX.Element => {
    return (
      <>
        {commitResults.map((commit) => (
          <Label key={commit.oid}>
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
              <Commit sx={{ width: '20px', height: '20px' }} />
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

  return (
    <OuterContainer>
      <div>{'List of repositories'}</div>
      <StyledTreeView
        defaultExpandIcon={<ChevronRight />}
        defaultEndIcon={<ChevronRight />}
        defaultCollapseIcon={<ExpandMore />}
        sx={{ width: 'max-content' }}
      >
        {repos.map((repo, idx) => (
          <StyledTreeItem
            nodeId={`${idx}`}
            key={idx}
            label={<StyledDiv sx={{ fontSize: '0.85rem' }}>{`Repository of ${repo.name} (${repo.path})`}</StyledDiv>}
          >
            <StyledTreeItem
              nodeId={`${idx}-branches`}
              label={<StyledDiv sx={{ fontSize: '0.85rem' }}>{'Branches'}</StyledDiv>}
            >
              {branches.get(repo.id as string)?.map((branch, idx) => (
                <StyledTreeItem
                  nodeId={`${idx}-${branch}`}
                  key={branch}
                  label={<StyledDiv sx={{ fontSize: '0.85rem' }}>{`Commits in ${branch}`}</StyledDiv>}
                >
                  <RenderedCommits
                    commitResults={commits.get(branch) ? (commits.get(branch)?.result as GitCommit[]) : []}
                  />
                  <Label
                    hidden={!commits.get(branch)?.hasRemaining}
                    onClick={() => loadMoreCommits(repo.id as string, branch)}
                  >
                    <MoreHoriz />
                    <StyledDiv>{'Load more'}</StyledDiv>
                  </Label>
                </StyledTreeItem>
              ))}
            </StyledTreeItem>
            <StyledTreeItem nodeId={`${idx}-tags`} label={<StyledDiv sx={{ fontSize: '0.85rem' }}>{'Tags'}</StyledDiv>}>
              {tags.get(repo.id as string)?.map((tag) => (
                <StyledTreeItem
                  nodeId={`${idx}-${tag}`}
                  key={tag}
                  label={<StyledDiv sx={{ fontSize: '0.85rem' }}>{`Commits in ${tag}`}</StyledDiv>}
                >
                  <RenderedCommits commitResults={commits.get(tag) ? (commits.get(tag)?.result as GitCommit[]) : []} />
                  <Label
                    hidden={!commits.get(tag)?.hasRemaining}
                    onClick={() => loadMoreCommits(repo.id as string, tag)}
                  >
                    <MoreHoriz />
                    <StyledDiv>{'Load more'}</StyledDiv>
                  </Label>
                </StyledTreeItem>
              ))}
            </StyledTreeItem>
          </StyledTreeItem>
        ))}
      </StyledTreeView>
    </OuterContainer>
  );
};
