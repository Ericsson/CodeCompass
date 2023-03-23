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
import { MetricsContext } from 'global-context/metrics-context';
import { useContext, useEffect, useMemo, useState } from 'react';
import { getMetrics } from 'service/metrics-service';
import { MetricsType, MetricsTypeName } from '@thrift-generated';
import { Treemap } from 'recharts';
import { ProjectContext } from 'global-context/project-context';

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
  const projectCtx = useContext(ProjectContext);
  const metricsCtx = useContext(MetricsContext);

  const fileTypeOptions = ['Unknown', 'Dir', 'Binary', 'CPP'];

  const [data, setData] = useState<DataType[] | undefined>(undefined);
  const [currentDataFolder, setCurrentDataFolder] = useState<string>('');
  const [selectedFileTypeOptions, setSelectedFileTypeOptions] = useState<string[]>(fileTypeOptions);
  const [sizeDimension, setSizeDimension] = useState<MetricsTypeName>(metricsCtx.metricsTypeNames[0]);

  useEffect(() => {
    setCurrentDataFolder('');
    setData(undefined);
  }, [metricsCtx.metricsFileInfo, projectCtx.currentWorkspace]);

  const generateMetrics = async () => {
    const metricsRes = await getMetrics(
      metricsCtx.metricsFileInfo?.id as string,
      selectedFileTypeOptions,
      sizeDimension.type as MetricsType
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

  return metricsCtx.metricsFileInfo ? (
    <div>
      <FileName
        fileName={metricsCtx.metricsFileInfo ? (metricsCtx.metricsFileInfo.name as string) : ''}
        filePath={metricsCtx.metricsFileInfo ? (metricsCtx.metricsFileInfo.path as string) : ''}
        parseStatus={metricsCtx.metricsFileInfo ? (metricsCtx.metricsFileInfo.parseStatus as number) : 4}
        info={metricsCtx.metricsFileInfo ?? undefined}
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
              setSizeDimension(
                metricsCtx.metricsTypeNames.find((typeName) => typeName.name === e.target.value) as MetricsTypeName
              )
            }
          >
            {metricsCtx.metricsTypeNames.map((option, idx) => (
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
