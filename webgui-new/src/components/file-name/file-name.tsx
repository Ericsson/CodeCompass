import { styled } from '@mui/material';

const Container = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  gap: '0.6rem',
  paddingLeft: '15px',
  minHeight: '49px',
  borderBottom: `1px solid ${theme.colors?.primary}`,
}));

const ParseStatus = styled('div')({
  borderRadius: '5px',
  padding: '5px',
});

export const FileName = ({
  fileName,
  filePath,
  parseStatus,
}: {
  fileName: string;
  filePath: string;
  parseStatus: number;
}): JSX.Element => {
  const getParseStatusText = (status: number): string => {
    if (status === 2) {
      return 'Partially parsed';
    } else if (status === 3) {
      return 'Fully parsed';
    } else {
      return 'Not parsed';
    }
  };
  return (
    <Container>
      <ParseStatus
        sx={{
          color: (theme) =>
            parseStatus === 2 ? theme.colors?.warning : parseStatus === 3 ? theme.colors?.success : theme.colors?.error,
          border: (theme) =>
            `1px solid ${
              parseStatus === 2
                ? theme.colors?.warning
                : parseStatus === 3
                ? theme.colors?.success
                : theme.colors?.error
            }`,
        }}
      >
        {getParseStatusText(parseStatus)}
      </ParseStatus>
      <div>{fileName}</div>
      <div>{'::'}</div>
      <div>{filePath}</div>
    </Container>
  );
};
