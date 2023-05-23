import { AstNodeInfo } from '@thrift-generated';
import { CodeBitesNode } from './codebites-node';
import React, { useCallback, useEffect, useState } from 'react';
import ReactFlow, {
  Controls,
  Background,
  addEdge,
  FitViewOptions,
  applyNodeChanges,
  applyEdgeChanges,
  Node,
  NodeTypes,
  Edge,
  DefaultEdgeOptions,
  OnNodesChange,
  OnEdgesChange,
  OnConnect,
} from 'reactflow';
import 'reactflow/dist/style.css';
import * as SC from './styled-components';

type CodeBitesElement = {
  astNodeInfo: AstNodeInfo;
  rowNum: number;
};

const fitViewOptions: FitViewOptions = {
  padding: 0.2,
};

const defaultEdgeOptions: DefaultEdgeOptions = {
  animated: true,
};

const nodeTypes: NodeTypes = {
  codeBitesNode: CodeBitesNode,
};

export const CodeBites = ({ astNodeInfo }: { astNodeInfo: AstNodeInfo }): JSX.Element => {
  const [elements, setElements] = useState<CodeBitesElement[]>([{ astNodeInfo, rowNum: 1 }]);
  const [nodes, setNodes] = useState<Node[]>([]);
  const [edges, setEdges] = useState<Edge[]>([]);

  useEffect(() => {
    setNodes(
      elements.map((elem, idx) => {
        return {
          id: `${idx + 1}`,
          type: 'codeBitesNode',
          position: { x: idx + 1 > 2 ? (idx - 1) * 550 : 0, y: idx + 1 > 1 ? elem.rowNum * 500 : 0 },
          data: { astNodeInfo: elem.astNodeInfo, elements, setElements, rowNum: idx + 1 },
        };
      })
    );
    setEdges(
      elements.slice(1, elements.length).map((elem, idx) => {
        return {
          id: `e${elem.rowNum ?? 1}-${idx + 2}`,
          source: `${elem.rowNum}`,
          target: `${idx + 2}`,
          label: `${elem.astNodeInfo.astNodeValue?.split('\n')[0]}`,
        };
      })
    );
  }, [elements]);

  const onNodesChange: OnNodesChange = useCallback(
    (changes) => setNodes((nds) => applyNodeChanges(changes, nds)),
    [setNodes]
  );
  const onEdgesChange: OnEdgesChange = useCallback(
    (changes) => setEdges((eds) => applyEdgeChanges(changes, eds)),
    [setEdges]
  );
  const onConnect: OnConnect = useCallback((connection) => setEdges((eds) => addEdge(connection, eds)), [setEdges]);

  return (
    <SC.DiagramOuterContainer>
      <ReactFlow
        nodes={nodes}
        edges={edges}
        onNodesChange={onNodesChange}
        onEdgesChange={onEdgesChange}
        onConnect={onConnect}
        fitView
        fitViewOptions={fitViewOptions}
        defaultEdgeOptions={defaultEdgeOptions}
        nodeTypes={nodeTypes}
      >
        <Background />
        <Controls />
      </ReactFlow>
    </SC.DiagramOuterContainer>
  );
};
