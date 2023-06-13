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
import dagre from 'dagre';
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

const dagreGraph = new dagre.graphlib.Graph();
dagreGraph.setDefaultEdgeLabel(() => ({}));

const nodeWidth = 560;
const nodeHeight = 360;

const getLayoutedElements = (nodes: Node[], edges: Edge[], direction = 'TB') => {
  const isHorizontal = direction === 'LR';
  dagreGraph.setGraph({ rankdir: direction });

  nodes.forEach((node: { id: string }) => {
    dagreGraph.setNode(node.id, { width: nodeWidth, height: nodeHeight });
  });

  edges.forEach((edge) => {
    dagreGraph.setEdge(edge.source, edge.target);
  });

  dagre.layout(dagreGraph);

  /**
   * https://reactflow.dev/docs/examples/layout/dagre/
   * According to this example in plain JavaScript, 'targetPosition' and 'sourcePosition'
   * should be given string values, which is incompatible with the original 'Position' type.
   * Since the layout only works correctly this way, this has to be bypassed with an 'any'.
   */
  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  nodes.forEach((node: any) => {
    const nodeWithPosition = dagreGraph.node(node.id);
    node.targetPosition = isHorizontal ? 'left' : 'top';
    node.sourcePosition = isHorizontal ? 'right' : 'bottom';
    node.position = {
      x: nodeWithPosition.x - nodeWidth / 2,
      y: nodeWithPosition.y - nodeHeight / 2,
    };

    return node;
  });

  return { nodes: nodes as Node[], edges: edges as Edge[] };
};

export const CodeBites = ({ astNodeInfo }: { astNodeInfo: AstNodeInfo }): JSX.Element => {
  const [elements, setElements] = useState<CodeBitesElement[]>([{ astNodeInfo, rowNum: 1 }]);
  const [nodes, setNodes] = useState<Node[]>([]);
  const [edges, setEdges] = useState<Edge[]>([]);

  useEffect(() => {
    const calculatedNodes = elements.map((elem, idx) => {
      return {
        id: `${idx + 1}`,
        type: 'codeBitesNode',
        data: { astNodeInfo: elem.astNodeInfo, elements, setElements, rowNum: idx + 1 },
      };
    }) as Node[];
    const calculatedEdges = elements.slice(1, elements.length).map((elem, idx) => {
      return {
        id: `e${elem.rowNum ?? 1}-${idx + 2}`,
        source: `${elem.rowNum}`,
        target: `${idx + 2}`,
        label: `${elem.astNodeInfo.astNodeValue?.split('\n')[0]}`,
      };
    }) as Edge[];
    const { nodes: layoutedNodes, edges: layoutedEdges } = getLayoutedElements(calculatedNodes, calculatedEdges);
    setNodes(layoutedNodes);
    setEdges(layoutedEdges);
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
        nodesDraggable={false}
        nodesConnectable={false}
      >
        <Background />
        <Controls />
      </ReactFlow>
    </SC.DiagramOuterContainer>
  );
};
