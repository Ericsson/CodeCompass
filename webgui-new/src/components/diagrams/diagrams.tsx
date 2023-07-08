import { Button, IconButton, Modal, Tooltip } from '@mui/material';
import { ZoomIn, ZoomOut } from '@mui/icons-material';
import { FileName } from 'components/file-name/file-name';
import React, { useContext, useEffect, useRef, useState, MouseEvent } from 'react';
import {
  getCppAstNodeInfo,
  getCppDiagram,
  getCppDiagramLegend,
  getCppFileDiagram,
  getCppFileDiagramLegend,
  getCppReferenceTypes,
  getCppReferences,
} from 'service/cpp-service';
import { TransformWrapper, TransformComponent, ReactZoomPanPinchRef } from 'react-zoom-pan-pinch';
import { FileInfo, AstNodeInfo } from '@thrift-generated';
import { AppContext } from 'global-context/app-context';
import { getFileInfo } from 'service/project-service';
import { CodeBites } from 'components/codebites/codebites';
import * as SC from './styled-components';
import { convertSelectionRangeToString } from 'utils/utils';
import { useRouter } from 'next/router';
import { RouterQueryType } from 'utils/types';

export const Diagrams = (): JSX.Element => {
  const router = useRouter();
  const appCtx = useContext(AppContext);

  const [legendModalOpen, setLegendModalOpen] = useState<boolean>(false);
  const [exportTooltipOpen, setExportTooltipOpen] = useState<boolean>(false);
  const [diagramInfo, setDiagramInfo] = useState<FileInfo | AstNodeInfo | undefined>(undefined);
  const [codeBitesDisplayed, setCodeBitesDisplayed] = useState<boolean>(false);

  const diagramContainerRef = useRef<HTMLDivElement | null>(null);
  const diagramLegendContainerRef = useRef<HTMLDivElement | null>(null);
  const transformComponentRef = useRef<ReactZoomPanPinchRef | null>(null);

  useEffect(() => {
    if (appCtx.diagramGenId === '' || appCtx.diagramTypeId === '' || !appCtx.diagramType) return;
    const init = async () => {
      const initDiagramInfo =
        appCtx.diagramType === 'file'
          ? await getFileInfo(appCtx.diagramGenId)
          : appCtx.diagramType === 'ast'
          ? await getCppAstNodeInfo(appCtx.diagramGenId)
          : undefined;

      if (!initDiagramInfo) return;

      if (parseInt(appCtx.diagramTypeId) === 999) {
        const refTypes = await getCppReferenceTypes(initDiagramInfo.id as string);
        if (refTypes.get('Definition') === undefined) return;

        const astNodeDef = (
          await getCppReferences(initDiagramInfo.id as string, refTypes.get('Definition') as number, [])
        )[0];

        setDiagramInfo(astNodeDef);
        setCodeBitesDisplayed(true);
        return;
      }

      const diagram =
        initDiagramInfo instanceof FileInfo
          ? await getCppFileDiagram(appCtx.diagramGenId, parseInt(appCtx.diagramTypeId))
          : initDiagramInfo instanceof AstNodeInfo
          ? await getCppDiagram(appCtx.diagramGenId, parseInt(appCtx.diagramTypeId))
          : '';

      const parser = new DOMParser();
      const parsedDiagram = parser.parseFromString(diagram, 'text/xml');
      const diagramSvg = parsedDiagram.getElementsByTagName('svg')[0];

      diagramSvg.style.height = '100%';
      diagramSvg.style.cursor = 'pointer';

      transformComponentRef?.current?.resetTransform();
      diagramContainerRef?.current?.replaceChildren(diagramSvg);

      setDiagramInfo(initDiagramInfo);
    };
    init();
  }, [appCtx.diagramGenId, appCtx.diagramTypeId, appCtx.diagramType]);

  const generateDiagram = async (e: MouseEvent) => {
    const parentNode = (e.target as HTMLElement)?.parentElement;
    if ((parentNode?.className as unknown as SVGAnimatedString).baseVal !== 'node') return;

    const diagramGenId = parentNode?.id as string;

    const initDiagramInfo =
      appCtx.diagramType === 'file'
        ? await getFileInfo(appCtx.diagramGenId)
        : appCtx.diagramType === 'ast'
        ? await getCppAstNodeInfo(appCtx.diagramGenId)
        : undefined;

    if (!initDiagramInfo) return;

    if (initDiagramInfo instanceof FileInfo) {
      router.push({
        pathname: '/project',
        query: {
          ...router.query,
          projectFileId: diagramGenId,
        } as RouterQueryType,
      });
    } else if (initDiagramInfo instanceof AstNodeInfo) {
      const astNodeInfo = await getCppAstNodeInfo(diagramGenId);

      router.push({
        pathname: '/project',
        query: {
          ...router.query,
          projectFileId: astNodeInfo?.range?.file as string,
          editorSelection: convertSelectionRangeToString(astNodeInfo?.range?.range),
          languageNodeId: diagramGenId,
        } as RouterQueryType,
      });
    }

    router.push({
      pathname: '/project',
      query: {
        ...router.query,
        diagramGenId: diagramGenId,
      } as RouterQueryType,
    });
  };

  const generateLegend = async () => {
    if (!diagramLegendContainerRef.current) return;

    const diagramLegend =
      diagramInfo instanceof FileInfo
        ? await getCppFileDiagramLegend(parseInt(appCtx.diagramTypeId))
        : diagramInfo instanceof AstNodeInfo
        ? await getCppDiagramLegend(parseInt(appCtx.diagramTypeId))
        : '';

    const parser = new DOMParser();
    const parsedDiagramLegend = parser.parseFromString(diagramLegend, 'text/xml');
    const diagramLegendSvg = parsedDiagramLegend.getElementsByTagName('svg')[0];

    diagramLegendSvg.style.height = '100%';
    diagramLegendContainerRef.current.replaceChildren(diagramLegendSvg);

    setLegendModalOpen(true);
  };

  const exportDiagramSVG = () => {
    const svgString = diagramContainerRef.current?.querySelector('svg')?.outerHTML as string;
    navigator.clipboard.writeText(svgString);
    setExportTooltipOpen(true);
    setTimeout(() => {
      setExportTooltipOpen(false);
    }, 2000);
  };

  const downloadSVG = () => {
    const svgElement = diagramContainerRef.current?.querySelector('svg') as SVGSVGElement;
    if (!svgElement) return;

    const svgCode = new XMLSerializer().serializeToString(svgElement);
    const blob = new Blob([svgCode], { type: 'image/svg+xml' });
    const url = URL.createObjectURL(blob);

    const link = document.createElement('a');
    link.setAttribute('download', 'file-diagram.svg');
    link.setAttribute('href', url);
    link.click();
  };

  return appCtx.diagramGenId ? (
    <>
      {diagramInfo instanceof FileInfo ? (
        <FileName
          fileName={diagramInfo ? (diagramInfo.name as string) : ''}
          filePath={diagramInfo ? (diagramInfo.path as string) : ''}
          parseStatus={diagramInfo ? (diagramInfo.parseStatus as number) : 4}
          info={diagramInfo ?? undefined}
        />
      ) : diagramInfo instanceof AstNodeInfo ? (
        <SC.AstNodeInfoHeader>{`${diagramInfo.symbolType}: ${diagramInfo.astNodeValue}`}</SC.AstNodeInfoHeader>
      ) : (
        <></>
      )}
      <>
        {codeBitesDisplayed ? (
          <CodeBites astNodeInfo={diagramInfo as AstNodeInfo} />
        ) : (
          <>
            <TransformWrapper ref={transformComponentRef}>
              {({ zoomIn, zoomOut, resetTransform }) => (
                <SC.TransformContainer>
                  <SC.ZoomOptions>
                    <IconButton onClick={() => zoomIn()}>
                      <ZoomIn />
                    </IconButton>
                    <Button onClick={() => resetTransform()}>{'Reset'}</Button>
                    <IconButton onClick={() => zoomOut()}>
                      <ZoomOut />
                    </IconButton>
                  </SC.ZoomOptions>
                  <TransformComponent>
                    <SC.DiagramContainer ref={diagramContainerRef} onClick={(e) => generateDiagram(e)} />
                  </TransformComponent>
                </SC.TransformContainer>
              )}
            </TransformWrapper>

            <SC.DiagramOptionsContainer>
              <Button onClick={() => generateLegend()}>{'Legend'}</Button>
              <Tooltip
                PopperProps={{
                  disablePortal: true,
                }}
                onClose={() => setExportTooltipOpen(false)}
                open={exportTooltipOpen}
                disableFocusListener
                disableHoverListener
                disableTouchListener
                title={'Copied to clipboard'}
                arrow
              >
                <Button onClick={() => exportDiagramSVG()}>{'Export SVG'}</Button>
              </Tooltip>
              <Button onClick={() => downloadSVG()}>{'Download image'}</Button>
              <Modal open={legendModalOpen} onClose={() => setLegendModalOpen(false)} keepMounted>
                <SC.ModalBox>
                  <SC.DiagramLegendContainer ref={diagramLegendContainerRef} />
                </SC.ModalBox>
              </Modal>
            </SC.DiagramOptionsContainer>
          </>
        )}
      </>
    </>
  ) : (
    <SC.Placeholder>
      {
        'No file/directory/node selected. Right click on a file/directory in the file manager or a node in the editor to generate diagrams.'
      }
    </SC.Placeholder>
  );
};
