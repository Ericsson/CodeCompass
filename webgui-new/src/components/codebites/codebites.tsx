import { AstNodeInfo } from '@thrift-generated';
import { CodeBitesNode } from './codebites-node';
import React from 'react';
import ReactFlow, {
  Controls,
  Background,
  FitViewOptions,
  Node,
  NodeTypes,
  Edge,
  DefaultEdgeOptions,
  ReactFlowProvider,
  useNodesState,
  useEdgesState,
} from 'reactflow';
import 'reactflow/dist/style.css';
import * as SC from './styled-components';

const fitViewOptions: FitViewOptions = {
  padding: 0.2,
};

const defaultEdgeOptions: DefaultEdgeOptions = {
  animated: true,
};

const nodeTypes: NodeTypes = {
  codeBitesNode: CodeBitesNode,
};

const CodeBitesReactFlow = ({ astNodeInfo }: { astNodeInfo: AstNodeInfo }): JSX.Element => {
  const [nodes, , onNodesChange] = useNodesState<Node>([
    { id: astNodeInfo.id, type: 'codeBitesNode', data: { astNodeInfo }, position: { x: 100, y: 100 } } as Node,
  ]);
  const [edges, , onEdgesChange] = useEdgesState<Edge>([]);

  return (
    <SC.DiagramOuterContainer>
      <ReactFlow
        nodes={nodes}
        edges={edges}
        onNodesChange={onNodesChange}
        onEdgesChange={onEdgesChange}
        fitView
        fitViewOptions={fitViewOptions}
        defaultEdgeOptions={defaultEdgeOptions}
        nodeTypes={nodeTypes}
        nodesDraggable={false}
        nodesConnectable={false}
      >
        <Background />
        <Controls />
      </ReactFlow>
    </SC.DiagramOuterContainer>
  );
};

export const CodeBites = ({ astNodeInfo }: { astNodeInfo: AstNodeInfo }): JSX.Element => {
  return (
    <ReactFlowProvider>
      <CodeBitesReactFlow astNodeInfo={astNodeInfo} />
    </ReactFlowProvider>
  );
};
