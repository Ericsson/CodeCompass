import { cpp } from '@codemirror/lang-cpp';
import { AstNodeInfo, FileInfo } from '@thrift-generated';
import { githubDark, githubLight } from '@uiw/codemirror-theme-github';
import ReactCodeMirror, { ReactCodeMirrorRef } from '@uiw/react-codemirror';
import { ThemeContext } from 'global-context/theme-context';
import React, { Dispatch, SetStateAction, useContext, useEffect, useRef, useState } from 'react';
import {
  getCppAstNodeInfoByPosition,
  getCppReferenceTypes,
  getCppReferences,
  getCppSourceText,
} from 'service/cpp-service';
import { getFileInfo } from 'service/project-service';
import { NodeProps, Handle, Position, useReactFlow, Node, Edge, getOutgoers, getConnectedEdges } from 'reactflow';
import { gutter, GutterMarker } from '@codemirror/view';
import * as SC from './styled-components';
import { IconButton, Tooltip } from '@mui/material';
import { Close } from '@mui/icons-material';
import { AppContext } from 'global-context/app-context';
import dagre from 'dagre';
import { sendGAEvent } from 'utils/analytics';

type CodeBitesElement = {
  astNodeInfo: AstNodeInfo;
  rowNum: number;
};

type DataProps = {
  astNodeInfo: AstNodeInfo;
  elements: CodeBitesElement[];
  setElements: Dispatch<SetStateAction<CodeBitesElement[]>>;
  rowNum: number;
};

const dagreGraph = new dagre.graphlib.Graph();
dagreGraph.setDefaultEdgeLabel(() => ({}));

const nodeWidth = 660;
const nodeHeight = 460;

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

class CustomOffsetGutterMarker extends GutterMarker {
  number: string;

  constructor(number: string) {
    super();
    this.number = number;
  }

  eq(other: CustomOffsetGutterMarker) {
    return this.number === other.number;
  }

  toDOM() {
    const div = document.createElement('div');
    div.innerHTML = this.number;
    return div;
  }
}

export const CodeBitesNode = ({ data }: NodeProps<DataProps>): JSX.Element => {
  const { diagramGenId: initialNodeId } = useContext(AppContext);
  const appCtx = useContext(AppContext);
  const { theme } = useContext(ThemeContext);

  const [fileInfo, setFileInfo] = useState<FileInfo | undefined>(undefined);
  const [text, setText] = useState<string>('');

  const editorRef = useRef<ReactCodeMirrorRef | null>(null);

  const reactFlowInstance = useReactFlow();

  useEffect(() => {
    if (!data.astNodeInfo) return;
    const init = async () => {
      const initFileInfo = await getFileInfo(data.astNodeInfo.range?.file as string);
      const initText = await getCppSourceText(data.astNodeInfo.id as string);
      sendGAEvent({
        event_action: 'code_bites',
        event_category: appCtx.workspaceId,
        event_label: `${initFileInfo?.name}: ${initText}`,
      });
      setFileInfo(initFileInfo);
      setText(initText);
    };
    init();
  }, [appCtx.workspaceId, data.astNodeInfo]);

  const handleClick = async () => {
    if (!editorRef.current) return;

    const view = editorRef.current.view;
    if (!view) return;

    const astNodeStartLine = data.astNodeInfo?.range?.range?.startpos?.line as number;
    const head = view.state.selection.main.head as number;
    const line = view.state.doc.lineAt(head);
    const column = view.state.selection.ranges[0].head - line.from;

    const newAstNodeInfo = await getCppAstNodeInfoByPosition(
      fileInfo?.id as string,
      line.number + astNodeStartLine - 1,
      column
    );
    if (!newAstNodeInfo) return;

    const newAstNodeRefTypes = await getCppReferenceTypes(newAstNodeInfo.id as string);
    if (newAstNodeRefTypes.get('Definition') === undefined) return;

    const newAstNodeDef = (
      await getCppReferences(
        newAstNodeInfo.id as string,
        newAstNodeRefTypes.get('Definition') as number,
        newAstNodeInfo.tags ?? []
      )
    )[0];
    if (!newAstNodeDef) return;

    const nodes = reactFlowInstance.getNodes();
    const edges = reactFlowInstance.getEdges();

    const nodeAlreadyPresent = nodes.map((node) => node.id).includes(newAstNodeDef.id as string);
    if (nodeAlreadyPresent) return;

    const newNode: Node = {
      id: newAstNodeDef.id as string,
      type: 'codeBitesNode',
      data: { astNodeInfo: newAstNodeDef },
      position: { x: 100, y: 100 },
    };

    const newEdge: Edge = {
      id: `e${data.astNodeInfo.id}-${newAstNodeDef.id}`,
      source: data.astNodeInfo.id as string,
      target: newAstNodeDef.id as string,
      label: `${newAstNodeDef.astNodeValue?.split('\n')[0]}`,
    };

    const { nodes: layoutedNodes, edges: layoutedEdges } = getLayoutedElements(
      [...nodes, newNode],
      [...edges, newEdge]
    );
    reactFlowInstance.setNodes(layoutedNodes);
    reactFlowInstance.setEdges(layoutedEdges);
  };

  const handleDelete = () => {
    const nodes = reactFlowInstance.getNodes();
    const edges = reactFlowInstance.getEdges();
    const currentNode = reactFlowInstance.getNode(data.astNodeInfo.id as string) as Node;

    const relatedNodes = getOutgoers(currentNode, nodes, edges) as Node[];
    const relatedNodeIds = relatedNodes.map((node) => node.id);

    const relatedEdges = getConnectedEdges([...relatedNodes, currentNode], edges);
    const relatedEdgeIds = relatedEdges.map((edge) => edge.id);

    const updatedNodes = nodes.filter((node) => !relatedNodeIds.includes(node.id) && node.id !== currentNode?.id);
    const updatedEdges = edges.filter((edge) => !relatedEdgeIds.includes(edge.id));

    const { nodes: layoutedNodes, edges: layoutedEdges } = getLayoutedElements(updatedNodes, updatedEdges);

    reactFlowInstance.setNodes(layoutedNodes);
    reactFlowInstance.setEdges(layoutedEdges);
  };

  const customOffsetGutter = gutter({
    class: 'customGutter',
    renderEmptyElements: false,
    lineMarker(view, line, others) {
      if (others.some((m) => m.toDOM)) return null;

      const number = view.state.doc.lineAt(line.from).number + (data.astNodeInfo.range?.range?.startpos?.line ?? 0) - 1;

      return new CustomOffsetGutterMarker(number.toString());
    },
  });

  return (
    <SC.NodeOuterContainer>
      <Handle type="target" position={Position.Top} />
      <Handle type="source" position={Position.Bottom} />
      <SC.NodeHeader>
        <Tooltip title={fileInfo ? (fileInfo.path as string) : 'Unknown file path'} placement={'top'}>
          <SC.NodeTitle>{fileInfo ? (fileInfo.name as string) : 'Unknown file'}</SC.NodeTitle>
        </Tooltip>
        {data.astNodeInfo.id !== initialNodeId && (
          <IconButton onClick={() => handleDelete()}>
            <Close />
          </IconButton>
        )}
      </SC.NodeHeader>
      <ReactCodeMirror
        readOnly={true}
        extensions={[customOffsetGutter, cpp()]}
        theme={theme === 'dark' ? githubDark : githubLight}
        style={{ fontSize: '0.8rem' }}
        value={text}
        width={'490px'}
        height={'310px'}
        onCreateEditor={(view, state) => (editorRef.current = { view, state })}
        onClick={() => handleClick()}
        basicSetup={{
          lineNumbers: false,
        }}
      />
    </SC.NodeOuterContainer>
  );
};
