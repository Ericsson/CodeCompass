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
  AccountTreeOutlined,
  CommitOutlined,
  SellOutlined,
  StorageOutlined,
  TextSnippet,
  TextSnippetOutlined,
  CallOutlined,
  Badge,
  Person,
  DriveFileRenameOutline,
  People,
  PeopleOutlined,
  DataUsage,
  DataUsageOutlined,
} from '@mui/icons-material';
import { TbMathFunction, TbMacro } from 'react-icons/tb';
import { HiVariable, HiHashtag } from 'react-icons/hi';
import { FiType } from 'react-icons/fi';
import * as Devicon from 'devicons-react';

export const FileIcon = ({ fileName, outlined }: { fileName: string; outlined?: boolean }): JSX.Element => {
  if (fileName.startsWith('.git')) {
    return <Devicon.GitOriginal />;
  }

  const fileExtension = fileName.split('.').reverse()[0];

  switch (fileExtension) {
    case 'h':
    case 'c':
      return <Devicon.COriginal />;
    case 'hpp':
    case 'cpp':
      return <Devicon.CplusplusOriginal />;
    case 'cmake':
      return <Devicon.CmakeOriginal />;
    case 'cs':
      return <Devicon.CsharpOriginal />;
    case 'jar':
    case 'java':
      return <Devicon.JavaOriginal />;
    case 'js':
      return <Devicon.JavascriptOriginal />;
    case 'json':
      return <DataObject sx={{ width: '20px', height: '20px' }} />;
    case 'ts':
      return <Devicon.TypescriptOriginal />;
    case 'html':
      return <Devicon.Html5Original />;
    case 'css':
      return <Devicon.Css3Original />;
    case 'sh':
      return <Devicon.BashOriginal />;
    case 'php':
      return <Devicon.PhpOriginal />;
    case 'py':
      return <Devicon.PythonOriginal />;
    case 'pl':
      return <Devicon.PerlOriginal />;
    case 'lua':
      return <Devicon.LuaOriginal />;
    case 'rb':
      return <Devicon.RubyOriginal />;
    case 'md':
      return <Devicon.MarkdownOriginal />;
    default:
      return outlined ? (
        <TextSnippetOutlined sx={{ width: '20px', height: '20px' }} />
      ) : (
        <TextSnippet sx={{ width: '20px', height: '20px' }} />
      );
  }
};

export const RefIcon = ({ refName, outlined }: { refName: string; outlined?: boolean }): JSX.Element => {
  switch (refName) {
    case 'Functions':
    case 'Function':
      return <TbMathFunction style={{ width: '20px', height: '20px' }} />;
    case 'Variable':
      return <HiVariable style={{ width: '20px', height: '20px' }} />;
    case 'Types':
    case 'Type':
    case 'Typedef':
      return <FiType style={{ width: '20px', height: '20px' }} />;
    case 'Enum':
      return <TextSnippet sx={{ width: '20px', height: '20px' }} />;
    case 'Macros':
    case 'Macro':
      return <TbMacro style={{ width: '20px', height: '20px' }} />;
    case 'Includes':
      return <HiHashtag style={{ width: '20px', height: '20px' }} />;
    case 'Name':
      return <Badge sx={{ width: '20px', height: '20px' }} />;
    case 'Qualified name':
      return <Person sx={{ width: '20px', height: '20px' }} />;
    case 'Signature':
      return <DriveFileRenameOutline sx={{ width: '20px', height: '20px' }} />;
    case 'Callee':
      return <WifiCalling />;
    case 'Caller':
      return outlined ? <CallOutlined /> : <Call />;
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
    case 'Aliases':
      return <TextSnippet />;
    case 'Inherits from':
      return <TextSnippet />;
    case 'Inherited by':
      return <TextSnippet />;
    case 'Data member':
      return outlined ? <DataUsageOutlined /> : <DataUsage />;
    case 'Method':
      return <TextSnippet />;
    case 'Friends':
      return outlined ? <PeopleOutlined /> : <People />;
    case 'Underlying type':
      return <TextSnippet />;
    case 'Enum constants':
      return <TextSnippet />;
    case 'Expansions':
      return <TextSnippet />;
    case 'Undefinitions':
      return <TextSnippet />;
    default:
      return <TextSnippet />;
  }
};

export const GitIcon = ({ name, outlined }: { name: string; outlined?: boolean }): JSX.Element => {
  switch (name) {
    case 'repository':
      return outlined ? <StorageOutlined /> : <Storage />;
    case 'branch':
      return outlined ? <AccountTreeOutlined /> : <AccountTree />;
    case 'tag':
      return outlined ? <SellOutlined /> : <Sell />;
    case 'commit':
      return outlined ? <CommitOutlined /> : <Commit />;
    default:
      return outlined ? <TextSnippetOutlined /> : <TextSnippet />;
  }
};
