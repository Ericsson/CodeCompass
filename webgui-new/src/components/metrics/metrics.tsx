import {
  Button,
  Checkbox,
  FormControl,
  InputLabel,
  ListItemText,
  MenuItem,
  OutlinedInput,
  Select,
  styled,
} from '@mui/material';
import { FileName } from 'components/file-name/file-name';
import { MetricsContext } from 'global-context/metrics-context';
import { useContext, useState } from 'react';
import { getMetrics } from 'service/metrics-service';
import { MetricsType } from '@thrift-generated';

const StyledDiv = styled('div')({});

const MetricsOptionsContainer = styled('div')({
  display: 'flex',
  alignItems: 'center',
  gap: '1rem',
  padding: '10px',
});

export const Metrics = (): JSX.Element => {
  const metricsCtx = useContext(MetricsContext);

  const fileTypeOptions = ['Unknown', 'Dir', 'Binary', 'CPP'];
  const dimensionOptions = ['Original lines of code', 'Nonblank lines of code', 'Lines of pure code'];

  const [metrics, setMetrics] = useState<string>('');
  const [selectedFileTypeOptions, setSelectedFileTypeOptions] = useState<string[]>(fileTypeOptions);
  const [sizeDimension, setSizeDimension] = useState<string>(dimensionOptions[0]);
  const [colorDimension, setColorDimension] = useState<string>(dimensionOptions[0]);

  const generateMetrics = async () => {
    const metricsRes = await getMetrics(
      metricsCtx.metricsFileInfo?.id as string,
      selectedFileTypeOptions,
      metricsCtx.metricsTypeNames[0].type as MetricsType
    );
    setMetrics(metricsRes);
  };

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
          <Select value={sizeDimension} label="Size dimension" onChange={(e) => setSizeDimension(e.target.value)}>
            {dimensionOptions.map((option, idx) => (
              <MenuItem key={idx} value={option}>
                {option}
              </MenuItem>
            ))}
          </Select>
        </FormControl>
        <FormControl>
          <InputLabel>{'Color dimension'}</InputLabel>
          <Select value={colorDimension} label="Color dimension" onChange={(e) => setColorDimension(e.target.value)}>
            {dimensionOptions.map((option, idx) => (
              <MenuItem key={idx} value={option}>
                {option}
              </MenuItem>
            ))}
          </Select>
        </FormControl>
        <Button onClick={() => generateMetrics()} sx={{ textTransform: 'none' }}>
          {'Draw metrics'}
        </Button>
      </MetricsOptionsContainer>
    </div>
  ) : (
    <StyledDiv sx={{ padding: '10px' }}>
      {'No file/directory selected. Right click on a file/directory in the file manager to generate Metrics.'}
    </StyledDiv>
  );
};
