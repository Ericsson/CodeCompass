import React, { useState, useContext, useEffect, SyntheticEvent } from 'react';
import { CommitListFilteredResult, GitCommit, GitRepository, ReferenceTopObjectResult } from '@thrift-generated';
import {
  getBranchList,
  getCommitListFiltered,
  getReferenceTopObject,
  getRepositoryList,
  getTagList,
  isRepositoryAvailable,
} from 'service/git-service';
import { Tooltip, alpha } from '@mui/material';
import { ChevronRight, ExpandMore, MoreHoriz } from '@mui/icons-material';
import { formatDate } from 'utils/utils';
import { TabName } from 'enums/tab-enum';
import { AppContext } from 'global-context/app-context';
import { GitIcon } from 'components/custom-icon/custom-icon';
import * as SC from './styled-components';
import { useRouter } from 'next/router';
import { RouterQueryType } from 'utils/types';
import { useTranslation } from 'react-i18next';

export const RevisionControl = (): JSX.Element => {
  const { t } = useTranslation();
  const router = useRouter();
  const appCtx = useContext(AppContext);

  const DISPLAYED_COMMIT_CNT = 15;

  const [repos, setRepos] = useState<GitRepository[]>([]);
  const [branches, setBranches] = useState<Map<string, string[]>>(new Map());
  const [tags, setTags] = useState<Map<string, string[]>>(new Map());
  const [topCommits, setTopCommits] = useState<Map<string, ReferenceTopObjectResult>>(new Map());
  const [commits, setCommits] = useState<Map<string, CommitListFilteredResult>>(new Map());
  const [commitOffsets, setCommitOffsets] = useState<Map<string, number>>(new Map());
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

      if (appCtx.gitRepoId && appCtx.gitBranch) {
        const initBranches: typeof branches = new Map();
        const initCommitOffsets: typeof commitOffsets = new Map();

        const repoBranches = await getBranchList(appCtx.gitRepoId);
        for (const repoBranch of repoBranches) {
          initCommitOffsets.set(repoBranch, 0);
        }
        initBranches.set(appCtx.gitRepoId, repoBranches);
        initExpandedTreeNodes.push(`${appCtx.gitRepoId}-${appCtx.gitBranch}`);
        initExpandedTreeNodes.push(`${appCtx.gitRepoId}-branches`);

        const initTopCommits: typeof topCommits = new Map();
        const initCommits: typeof commits = new Map();

        const topCommit = (await getReferenceTopObject(appCtx.gitRepoId, appCtx.gitBranch)) as ReferenceTopObjectResult;
        initTopCommits.set(appCtx.gitBranch, topCommit);
        const commitList = (await getCommitListFiltered(
          appCtx.gitRepoId,
          topCommit.oid as string,
          DISPLAYED_COMMIT_CNT,
          0,
          ''
        )) as CommitListFilteredResult;
        initCommits.set(appCtx.gitBranch, commitList);

        setTopCommits(initTopCommits);
        setCommits(initCommits);

        setBranches(initBranches);
        setCommitOffsets(initCommitOffsets);
      }

      setRepos(initRepos);
      setExpandedTreeNodes(initExpandedTreeNodes);
    };
    init();
  }, [appCtx.workspaceId, appCtx.gitRepoId, appCtx.gitBranch]);

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
    router.push({
      pathname: '/project',
      query: {
        ...router.query,
        gitRepoId: repoId,
        gitBranch: branch,
        gitCommitId: commitId,
        activeTab: TabName.GIT_DIFF.toString(),
      } as RouterQueryType,
    });
  };

  const RenderedCommits = ({
    commitResults,
    repoId,
    branch,
  }: {
    commitResults: GitCommit[];
    repoId: string;
    branch: string;
  }) => {
    return (
      <>
        {commitResults.map((commit) => (
          <SC.Label
            key={commit.oid}
            onClick={() => getCommitDiff(repoId, branch, commit.oid as string)}
            sx={{
              backgroundColor: (theme) =>
                commit.oid === appCtx.gitCommitId ? alpha(theme.backgroundColors?.secondary as string, 0.3) : '',
            }}
          >
            <Tooltip
              title={
                <SC.StyledDiv sx={{ width: 'max-content' }}>
                  <div>{`#${commit.oid?.substring(0, 8)}`}</div>
                  <div>{commit.message}</div>
                  <div>{`${commit.author?.name} (${commit.author?.email})`}</div>
                  <div>
                    {t('revisionControl.commitedOn', {
                      date: formatDate(new Date((commit.time as unknown as number) * 1000)),
                    })}
                  </div>
                </SC.StyledDiv>
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
              <SC.StyledDiv sx={{ display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
                <GitIcon name={'commit'} />
              </SC.StyledDiv>
            </Tooltip>
            <SC.StyledDiv sx={{ display: 'flex', flexDirection: 'column' }}>
              <SC.StyledDiv>{`${commit.message} (${commit.time})`}</SC.StyledDiv>
              <SC.StyledDiv>{`${commit.author?.name} (${commit.author?.email})`}</SC.StyledDiv>
            </SC.StyledDiv>
          </SC.Label>
        ))}
      </>
    );
  };

  return repos.length ? (
    <SC.OuterContainer>
      <SC.StyledDiv sx={{ display: 'flex', alignItems: 'center', gap: '5px' }}>
        <GitIcon name={'repolist'} />
        <div>{t('revisionControl.repoList')}</div>
      </SC.StyledDiv>
      <SC.StyledTreeView
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
          <SC.StyledTreeItem
            nodeId={`${repo.id as string}`}
            key={repo.id as string}
            label={
              <SC.StyledDiv sx={{ fontSize: '0.85rem' }}>
                {t('revisionControl.repoTitle', { repoName: repo.name, repoPath: repo.path })}
              </SC.StyledDiv>
            }
            icon={<GitIcon name={'repository'} />}
          >
            <SC.StyledTreeItem
              nodeId={`${repo.id as string}-branches`}
              label={<SC.StyledDiv sx={{ fontSize: '0.85rem' }}>{t('revisionControl.branches')}</SC.StyledDiv>}
              icon={<GitIcon name={'branch'} />}
              onClick={() => loadBranches(repo.id as string)}
            >
              {branches.get(repo.id as string)?.length ? (
                branches.get(repo.id as string)?.map((branch) => (
                  <SC.StyledTreeItem
                    nodeId={`${repo.id as string}-${branch}`}
                    key={`${repo.id as string}-${branch}`}
                    label={
                      <SC.StyledDiv sx={{ fontSize: '0.85rem' }}>
                        {t('revisionControl.commitsInBranch', { branch })}
                      </SC.StyledDiv>
                    }
                    icon={<GitIcon name={'commit'} />}
                    onClick={() => loadInitialCommits(repo.id as string, branch)}
                  >
                    <RenderedCommits
                      commitResults={commits.get(branch) ? (commits.get(branch)?.result as GitCommit[]) : []}
                      repoId={repo.id as string}
                      branch={branch}
                    />
                    <SC.Label
                      hidden={!commits.get(branch)?.hasRemaining}
                      onClick={() => loadCommits(repo.id as string, branch)}
                    >
                      <MoreHoriz />
                      <SC.StyledDiv>{t('revisionControl.loadMore')}</SC.StyledDiv>
                    </SC.Label>
                  </SC.StyledTreeItem>
                ))
              ) : (
                <div>{t('revisionControl.loading')}</div>
              )}
            </SC.StyledTreeItem>
            <SC.StyledTreeItem
              nodeId={`${repo.id as string}-tags`}
              label={<SC.StyledDiv sx={{ fontSize: '0.85rem' }}>{t('revisionControl.tags')}</SC.StyledDiv>}
              icon={<GitIcon name={'tag'} />}
              onClick={() => loadTags(repo.id as string)}
            >
              {tags.get(repo.id as string)?.length ? (
                tags.get(repo.id as string)?.map((tag) => (
                  <SC.StyledTreeItem
                    nodeId={`${repo.id as string}-${tag}`}
                    key={`${repo.id as string}-${tag}`}
                    label={
                      <SC.StyledDiv sx={{ fontSize: '0.85rem' }}>
                        {t('revisionControl.commitsInTag', { tag })}
                      </SC.StyledDiv>
                    }
                    icon={<GitIcon name={'commit'} />}
                    onClick={() => loadInitialCommits(repo.id as string, tag)}
                  >
                    <RenderedCommits
                      commitResults={commits.get(tag) ? (commits.get(tag)?.result as GitCommit[]) : []}
                      repoId={tag}
                      branch={tag}
                    />
                    <SC.Label
                      hidden={!commits.get(tag)?.hasRemaining}
                      onClick={() => loadCommits(repo.id as string, tag)}
                    >
                      <MoreHoriz />
                      <SC.StyledDiv>{t('revisionControl.loadMore')}</SC.StyledDiv>
                    </SC.Label>
                  </SC.StyledTreeItem>
                ))
              ) : (
                <div>{t('revisionControl.loading')}</div>
              )}
            </SC.StyledTreeItem>
          </SC.StyledTreeItem>
        ))}
      </SC.StyledTreeView>
    </SC.OuterContainer>
  ) : (
    <SC.Placeholder>{t('revisionControl.noRepos')}</SC.Placeholder>
  );
};
