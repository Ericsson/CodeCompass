import { TextSnippet } from '@mui/icons-material';
import {
  Storage,
  AccountTree,
  Sell,
  Commit,
  Call,
  WifiCalling,
  PhoneCallback,
  DataObject,
  LocalActivity,
  MenuBook,
  Edit,
} from '@mui/icons-material';

export const FileIcon = ({ fileName }: { fileName: string }): JSX.Element => {
  const fileExtension = fileName.split('.').reverse()[0];

  switch (fileExtension) {
    default:
      return <TextSnippet sx={{ width: '20px', height: '20px' }} />;
  }
};

export const RefIcon = ({ refName }: { refName: string }): JSX.Element => {
  switch (refName) {
    case 'Callee':
      return <WifiCalling />;
    case 'Caller':
      return <Call />;
    case 'Declaration':
      return <DataObject />;
    case 'Definition':
      return <TextSnippet />;
    case 'Function pointer call':
      return <TextSnippet />;
    case 'Local variables':
      return <LocalActivity />;
    case 'Overridden by':
      return <TextSnippet />;
    case 'Overrides':
      return <TextSnippet />;
    case 'Parameters':
      return <TextSnippet />;
    case 'Return type':
      return <TextSnippet />;
    case 'This calls':
      return <PhoneCallback />;
    case 'Usage':
      return <TextSnippet />;
    case 'Virtual call':
      return <TextSnippet />;
    case 'Reads':
      return <MenuBook />;
    case 'Writes':
      return <Edit />;
    default:
      return <TextSnippet />;
  }
};

export const GitIcon = ({ name }: { name: string }): JSX.Element => {
  switch (name) {
    case 'repository':
      return <Storage />;
    case 'branch':
      return <AccountTree />;
    case 'tag':
      return <Sell />;
    case 'commit':
      return <Commit />;
    default:
      return <TextSnippet />;
  }
};
