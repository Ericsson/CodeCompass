import { TextSnippet } from '@mui/icons-material';

export const FileIcon = ({ fileName }: { fileName: string }): JSX.Element => {
  const fileExtension = fileName.split('.').reverse()[0];

  switch (fileExtension) {
    default:
      return <TextSnippet></TextSnippet>;
  }
};
