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
  parseStatus: string;
}): JSX.Element => {
  return (
    <Container>
      <ParseStatus
        sx={{
          color: (theme) =>
            parseStatus === 'Not parsed'
              ? theme.colors?.error
              : parseStatus === 'Partially parsed'
              ? theme.colors?.warning
              : parseStatus === 'Fully parsed'
              ? theme.colors?.success
              : theme.colors?.primary,
          border: (theme) =>
            `1px solid ${
              parseStatus === 'Not parsed'
                ? theme.colors?.error
                : parseStatus === 'Partially parsed'
                ? theme.colors?.warning
                : parseStatus === 'Fully parsed'
                ? theme.colors?.success
                : theme.colors?.primary
            }`,
        }}
      >
        {parseStatus}
      </ParseStatus>
      <div>{fileName}</div>
      <div>{'::'}</div>
      <div>{filePath}</div>
    </Container>
  );
};
