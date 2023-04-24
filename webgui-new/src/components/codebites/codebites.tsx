import { styled } from '@mui/material';
import { AstNodeInfo } from '@thrift-generated';
import { CodeBitesNode } from './codebites-node';
import { useState } from 'react';

const OuterContainer = styled('div')({
  padding: '10px',
  display: 'grid',
  gap: '55px',
  gridTemplateColumns: 'repeat(auto-fill, minmax(300px, 1fr))',
  overflow: 'scroll',
  width: 'calc(100vw - 280px)',
  height: 'calc(100vh - 78px - 48px - 49px)',
});

export const CodeBites = ({ astNodeInfo }: { astNodeInfo: AstNodeInfo }): JSX.Element => {
  const [displayedAstNodes, setDisplayedAstNodes] = useState<AstNodeInfo[]>([astNodeInfo]);

  return (
    <OuterContainer>
      {displayedAstNodes.map((aInfo, idx) => (
        <CodeBitesNode key={idx} astNodeInfo={aInfo} setDisplayedAstNodes={setDisplayedAstNodes} />
      ))}
    </OuterContainer>
  );
};
