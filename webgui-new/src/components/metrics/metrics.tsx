import {
  Breadcrumbs,
  Button,
  Checkbox,
  FormControl,
  InputLabel,
  ListItemText,
  MenuItem,
  OutlinedInput,
  Select,
  styled,
  Tooltip,
  Typography,
} from '@mui/material';
import { FileName } from 'components/file-name/file-name';
import { Dispatch, SetStateAction, useContext, useEffect, useMemo, useState } from 'react';
import { getMetrics, getMetricsTypeNames } from 'service/metrics-service';
import { FileInfo, MetricsType, MetricsTypeName } from '@thrift-generated';
import { Treemap } from 'recharts';
import { AppContext } from 'global-context/app-context';
import { getFileInfo, getFileInfoByPath } from 'service/project-service';

type RespType = {
  [key: string]: {
    [key: string]: string | RespType;
  };
};

type DataItem = {
  name: string;
  size: number;
  children?: DataItem[];
};

type CustomTreeNodeProps = {
  root?: CustomTreeNodeProps;
  index?: number;
  x?: number;
  y?: number;
  width?: number;
  height?: number;
  depth?: number;
  size?: number;
  name?: string;
  children?: CustomTreeNodeProps[];
};

const StyledDiv = styled('div')({});

const StyledRect = styled('rect')({});

const MetricsOptionsContainer = styled('div')({
  display: 'flex',
  alignItems: 'center',
  gap: '1rem',
  padding: '10px',
});

const MetricsContainer = styled('div')({
  overflow: 'scroll',
  padding: '10px',
});

const CustomizedContent = (
  props: CustomTreeNodeProps & {
    setData: Dispatch<SetStateAction<DataItem[] | undefined>>;
    fileInfo: FileInfo;
    setFileInfo: Dispatch<SetStateAction<FileInfo | undefined>>;
  }
) => {
  const { x, y, width, height, depth, size, name, children } = props as {
    x: number;
    y: number;
    width: number;
    height: number;
    depth: number;
    size: number;
    name: string;
    children: CustomTreeNodeProps[];
  };

  const getDarkenedRGBCode = (value: number): string => {
    const normalizedValue = Math.min(Math.max(value, 0), 255);
    const red = Math.max(Math.round((1 - normalizedValue / 255) * 80), 10);
    const green = Math.max(Math.round((1 - normalizedValue / 255) * 120), 10);
    const blue = Math.max(Math.round((1 - normalizedValue / 255) * 255), 80);
    const rgbCode = `rgb(${red}, ${green}, ${blue})`;
    return rgbCode;
  };

  const updateTreemap = async () => {
    if (!children) return;

    const currentPath = props.fileInfo.path as string;
    const newPath = `${currentPath}/${name}`;

    const newFileInfo = await getFileInfoByPath(newPath);
    const newData = children.map((child) =>
      Object.fromEntries([
        ['name', child.name],
        ['size', child.size],
        ['children', child.children],
      ])
    );

    props.setFileInfo(newFileInfo);
    props.setData(newData);
  };

  return depth === 1 ? (
    <Tooltip title={size}>
      <g onClick={() => updateTreemap()}>
        <StyledRect
          x={x}
          y={y}
          width={width}
          height={height}
          sx={{
            fill: getDarkenedRGBCode(size),
            stroke: '#FFF',
            ':hover': {
              fill: props.children ? '#9597E4' : getDarkenedRGBCode(size),
            },
            cursor: props.children ? 'pointer' : '',
          }}
        />
        <text x={x + width / 2} y={y + height / 2} textAnchor="middle" fill="#fff" fontSize={14} strokeWidth={0.1}>
          {name}
        </text>
      </g>
    </Tooltip>
  ) : (
    <></>
  );
};

const fileTypeOptions = ['Unknown', 'Dir', 'Binary', 'CPP'];

const sumSizes = (node: DataItem): number => {
  let sum = 0;
  if (!node.children) {
    sum += node.size ?? 0;
  } else {
    node.children.forEach((child) => (sum += sumSizes(child)));
  }
  return sum;
};

const findChildren = (node: DataItem): DataItem[] | undefined => {
  if (node.children && node.children.length > 1) {
    return node.children;
  } else if (node.children && node.children.length === 1) {
    return findChildren(node.children[0]);
  } else {
    return undefined;
  }
};

const convertResObject = (res: RespType): DataItem => {
  const keys = Object.keys(res);
  const name = keys[0];
  const obj = res[name];
  const children = [];

  for (let key in obj) {
    const value = obj[key];

    if (typeof value === 'object') {
      const child = convertResObject({ [key]: value } as RespType);
      child.name = key;
      child.size = sumSizes(child);
      children.push(child);
    } else {
      children.push({ name: key, size: Number(value) });
    }
  }

  return { name, size: 0, children };
};

export const Metrics = (): JSX.Element => {
  const appCtx = useContext(AppContext);

  const [fileInfo, setFileInfo] = useState<FileInfo | undefined>(undefined);
  const [data, setData] = useState<DataItem[] | undefined>(undefined);
  const [selectedFileTypeOptions, setSelectedFileTypeOptions] = useState<string[]>(fileTypeOptions);
  const [sizeDimension, setSizeDimension] = useState<MetricsTypeName | undefined>(undefined);
  const [metricsTypeNames, setMetricsTypeNames] = useState<MetricsTypeName[]>([]);

  useEffect(() => {
    if (!appCtx.metricsGenId) return;
    const init = async () => {
      const initMetricsTypeNames = await getMetricsTypeNames();
      const initFileInfo = await getFileInfo(appCtx.metricsGenId);

      const metricsRes = await getMetrics(
        initFileInfo?.id as string,
        fileTypeOptions,
        initMetricsTypeNames[0].type as MetricsType
      );
      const convertedObject = convertResObject(JSON.parse(metricsRes));

      setMetricsTypeNames(initMetricsTypeNames);
      setSizeDimension(initMetricsTypeNames[0]);
      setFileInfo(initFileInfo);
      setData(findChildren(convertedObject));
    };
    init();
  }, [appCtx.metricsGenId]);

  const generateMetrics = async (filePath?: string) => {
    const fInfo = filePath ? await getFileInfoByPath(filePath) : fileInfo;
    const metricsRes = await getMetrics(
      fInfo?.id as string,
      selectedFileTypeOptions,
      sizeDimension?.type as MetricsType
    );
    const convertedObject = convertResObject(JSON.parse(metricsRes));
    if (filePath) setFileInfo(fInfo);
    setData(findChildren(convertedObject));
  };

  const renderTreeMap: JSX.Element = useMemo(() => {
    return data && fileInfo ? (
      <Treemap
        width={900}
        height={900}
        data={data}
        dataKey="size"
        stroke="#fff"
        fill="#8884d8"
        content={<CustomizedContent setData={setData} fileInfo={fileInfo as FileInfo} setFileInfo={setFileInfo} />}
      />
    ) : (
      <></>
    );
  }, [data, fileInfo]);

  return appCtx.metricsGenId && fileInfo && sizeDimension ? (
    <>
      <FileName
        fileName={fileInfo ? (fileInfo.name as string) : ''}
        filePath={fileInfo ? (fileInfo.path as string) : ''}
        parseStatus={fileInfo ? (fileInfo.parseStatus as number) : 4}
        info={fileInfo ?? undefined}
      />
      <StyledDiv
        sx={{
          width: 'calc(100vw - 280px)',
          height: 'calc(100vh - 78px - 48px - 49px)',
          overflow: 'scroll',
        }}
      >
        <MetricsOptionsContainer>
          <FormControl sx={{ width: 300 }}>
            <InputLabel>{'File type'}</InputLabel>
            <Select
              multiple
              value={selectedFileTypeOptions}
              onChange={(e) => {
                const {
                  target: { value },
                } = e;
                setSelectedFileTypeOptions(typeof value === 'string' ? value.split(',') : value);
              }}
              input={<OutlinedInput label="File type" />}
              renderValue={(selected) => selected.join(', ')}
            >
              {fileTypeOptions.map((type) => (
                <MenuItem key={type} value={type}>
                  <Checkbox checked={selectedFileTypeOptions.indexOf(type) > -1} />
                  <ListItemText primary={type} />
                </MenuItem>
              ))}
            </Select>
          </FormControl>
          <FormControl>
            <InputLabel>{'Size dimension'}</InputLabel>
            <Select
              value={sizeDimension.name}
              label="Size dimension"
              onChange={(e) =>
                setSizeDimension(
                  metricsTypeNames.find((typeName) => typeName.name === e.target.value) as MetricsTypeName
                )
              }
            >
              {metricsTypeNames.map((option, idx) => (
                <MenuItem key={idx} value={option.name}>
                  {option.name}
                </MenuItem>
              ))}
            </Select>
          </FormControl>
          <Button onClick={() => generateMetrics()} sx={{ textTransform: 'none' }}>
            {'Draw metrics'}
          </Button>
        </MetricsOptionsContainer>
        <Breadcrumbs
          sx={{
            padding: '5px',
            margin: '10px',
            border: (theme) => `1px solid ${theme.colors?.primary}`,
            borderRadius: '5px',
            width: 'max-content',
          }}
        >
          {fileInfo.path?.split('/').map((p, idx) => (
            <Typography
              key={idx}
              sx={{
                cursor: idx !== (fileInfo.path?.split('/').length as number) - 1 && idx !== 1 ? 'pointer' : '',
                ':hover':
                  idx !== (fileInfo.path?.split('/').length as number) - 1 && idx !== 1
                    ? {
                        color: (theme) => theme.backgroundColors?.secondary,
                      }
                    : {},
              }}
              onClick={() => {
                if (idx === (fileInfo.path?.split('/').length as number) - 1 || idx === 1) return;
                const filePath = fileInfo.path
                  ?.split('/')
                  .slice(0, idx + 1)
                  .join('/') as string;
                generateMetrics(filePath);
              }}
            >
              <div>{p}</div>
            </Typography>
          ))}
        </Breadcrumbs>
        <MetricsContainer>{renderTreeMap}</MetricsContainer>
      </StyledDiv>
    </>
  ) : (
    <StyledDiv sx={{ padding: '10px' }}>
      {'No directory selected. Right click on a directory in the file manager to generate Metrics.'}
    </StyledDiv>
  );
};
