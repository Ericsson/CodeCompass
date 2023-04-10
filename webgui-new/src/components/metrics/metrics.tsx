import {
  alpha,
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
} from '@mui/material';
import { FileName } from 'components/file-name/file-name';
import { useContext, useEffect, useMemo, useState } from 'react';
import { getMetrics, getMetricsTypeNames } from 'service/metrics-service';
import { FileInfo, MetricsType, MetricsTypeName } from '@thrift-generated';
import { Treemap } from 'recharts';
import { AppContext } from 'global-context/app-context';
import { getFileInfo } from 'service/project-service';

type RespType = {
  [key: string]: {
    [key: string]: string | RespType;
  };
};

type DataType = {
  name: string;
  size?: number;
  children?: DataType[];
};

type CustomTreeNodeProps = {
  x?: number;
  y?: number;
  width?: number;
  height?: number;
  size?: number;
  name?: string;
};

const StyledDiv = styled('div')({});

const StyledRect = styled('rect')({});

const StyledTreemap = styled(Treemap)({
  cursor: 'pointer',
});

const MetricsOptionsContainer = styled('div')({
  display: 'flex',
  alignItems: 'center',
  gap: '1rem',
  padding: '10px',
});

const MetricsContainer = styled('div')({
  overflow: 'scroll',
  padding: '10px',
  width: 'calc(100vw - 280px)',
  height: 'calc(100vh - 78px - 48px - 49px - 77px)',
});

const CustomizedContent = (props: CustomTreeNodeProps) => {
  const { x, y, width, height, size, name } = props as {
    x: number;
    y: number;
    width: number;
    height: number;
    size: number;
    name: string;
  };

  const mapColors = (size: number) => {
    if (size >= 1000) {
      return 'rgb(0, 0, 25)';
    } else if (size >= 800 && size < 1000) {
      return 'rgb(0, 0, 50)';
    } else if (size >= 600 && size < 800) {
      return 'rgb(0, 0, 100)';
    } else if (size >= 400 && size < 600) {
      return 'rgb(0, 0, 150)';
    } else if (size >= 200 && size < 400) {
      return 'rgb(0, 0, 200)';
    } else {
      return 'rgb(0, 0, 255)';
    }
  };

  return (
    <g>
      <Tooltip title={name}>
        <StyledRect
          x={x}
          y={y}
          width={width}
          height={height}
          sx={{
            fill: mapColors(size),
            ':hover': {
              cursor: 'pointer',
              fill: alpha('#ddd', 0.3),
            },
          }}
        />
      </Tooltip>
      <text x={x + width / 2} y={y + height / 2 + 7} textAnchor="middle" fill="#fff" fontSize={12}>
        {size}
      </text>
    </g>
  );
};

export const Metrics = (): JSX.Element => {
  const appCtx = useContext(AppContext);

  const fileTypeOptions = ['Unknown', 'Dir', 'Binary', 'CPP'];

  const [fileInfo, setFileInfo] = useState<FileInfo | undefined>(undefined);
  const [data, setData] = useState<DataType[] | undefined>(undefined);
  const [currentDataFolder, setCurrentDataFolder] = useState<string>('');
  const [selectedFileTypeOptions, setSelectedFileTypeOptions] = useState<string[]>(fileTypeOptions);
  const [sizeDimension, setSizeDimension] = useState<MetricsTypeName | undefined>(undefined);

  const [metricsTypeNames, setMetricsTypeNames] = useState<MetricsTypeName[]>([]);

  useEffect(() => {
    if (!appCtx.workspaceId) return;
    const init = async () => {
      const initMetricsTypeNames = await getMetricsTypeNames();
      setMetricsTypeNames(initMetricsTypeNames);
      setSizeDimension(initMetricsTypeNames[0]);
    };
    init();
  }, [appCtx.workspaceId]);

  useEffect(() => {
    const init = async () => {
      const initFileInfo = await getFileInfo(appCtx.metricsGenId);
      setFileInfo(initFileInfo);
      setCurrentDataFolder('');
      setData(undefined);
    };
    init();
  }, [appCtx.metricsGenId, appCtx.workspaceId]);

  const generateMetrics = async () => {
    const metricsRes = await getMetrics(
      fileInfo?.id as string,
      selectedFileTypeOptions,
      sizeDimension?.type as MetricsType
    );
    setCurrentDataFolder('');
    setData(convertResObject(JSON.parse(metricsRes)).children);
  };

  const convertResObject = (res: RespType): DataType => {
    const keys = Object.keys(res);
    const name = keys[0];
    const obj = res[name];
    const children = [];

    for (let key in obj) {
      const value = obj[key];

      if (typeof value === 'object') {
        const child = convertResObject({ [key]: value } as RespType);
        child.name = key;
        children.push(child);
      } else {
        children.push({ name: key, size: Number(value) });
      }
    }

    return { name, children };
  };

  const renderTreeMap: JSX.Element = useMemo(() => {
    return data ? (
      <StyledTreemap
        width={2250}
        height={980}
        data={data}
        dataKey="size"
        aspectRatio={16 / 9}
        stroke="#fff"
        fill="#8884d8"
        content={<CustomizedContent />}
        onClick={(node) => {
          setCurrentDataFolder(node.root.name);
          setData(node.root.children);
        }}
      />
    ) : (
      <></>
    );
  }, [data]);

  return appCtx.metricsGenId && fileInfo && sizeDimension ? (
    <div>
      <FileName
        fileName={fileInfo ? (fileInfo.name as string) : ''}
        filePath={fileInfo ? (fileInfo.path as string) : ''}
        parseStatus={fileInfo ? (fileInfo.parseStatus as number) : 4}
        info={fileInfo ?? undefined}
      />
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
              setSizeDimension(metricsTypeNames.find((typeName) => typeName.name === e.target.value) as MetricsTypeName)
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
      <MetricsContainer>
        {currentDataFolder ? <div>{`Directory: ${currentDataFolder}`}</div> : ''}
        {renderTreeMap}
      </MetricsContainer>
    </div>
  ) : (
    <StyledDiv sx={{ padding: '10px' }}>
      {'No file/directory selected. Right click on a file/directory in the file manager to generate Metrics.'}
    </StyledDiv>
  );
};
