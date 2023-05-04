import {
  Storage,
  Sell,
  Commit,
  Call,
  WifiCalling3,
  PhoneCallback,
  PhoneForwarded,
  PhoneMissed,
  DataObject,
  MenuBook,
  Edit,
  TextSnippet,
  TextSnippetOutlined,
  Badge,
  Person,
  DriveFileRenameOutline,
  People,
  DataUsage,
  FolderOpen,
  Folder,
} from '@mui/icons-material';
import { TbMathFunction, TbMacro, TbTableAlias } from 'react-icons/tb';
import { HiArrowsExpand, HiHashtag } from 'react-icons/hi';
import { FiType } from 'react-icons/fi';
import { GrInherit } from 'react-icons/gr';
import {
  VscRepo,
  VscSourceControl,
  VscJson,
  VscSymbolEnum,
  VscSymbolEnumMember,
  VscSymbolVariable,
  VscVariableGroup,
  VscSymbolParameter,
  VscSymbolField,
  VscSymbolMethod,
} from 'react-icons/vsc';
import * as Devicon from 'devicons-react';
import { styled } from '@mui/material';
import React from 'react';

export const FolderIcon = ({ open }: { open?: boolean }) => {
  const icon: React.FunctionComponent = open ? FolderOpen : Folder;

  const IconWithStyles = styled(icon)(({ theme }) => ({
    width: '20px',
    height: '20px',
    color: theme.iconColors?.folder,
  }));

  return <IconWithStyles />;
};

export const FileIcon = ({ fileName }: { fileName: string }) => {
  let icon: React.FunctionComponent;

  if (fileName.startsWith('.git')) {
    icon = Devicon.GitOriginal;

    const IconWithStyles = styled(icon)({
      width: '20px',
      height: '20px',
    });

    return <IconWithStyles />;
  }

  const fileExtension = fileName.split('.').reverse()[0];

  switch (fileExtension) {
    case 'h':
    case 'c':
      icon = Devicon.COriginal;
      break;
    case 'hpp':
    case 'cpp':
      icon = Devicon.CplusplusOriginal;
      break;
    case 'cmake':
      icon = Devicon.CmakeOriginal;
      break;
    case 'cs':
      icon = Devicon.CsharpOriginal;
      break;
    case 'jar':
    case 'java':
      icon = Devicon.JavaOriginal;
      break;
    case 'js':
      icon = Devicon.JavascriptOriginal;
      break;
    case 'json':
      icon = VscJson;
      break;
    case 'ts':
      icon = Devicon.TypescriptOriginal;
      break;
    case 'html':
      icon = Devicon.Html5Original;
      break;
    case 'css':
      icon = Devicon.Css3Original;
      break;
    case 'sh':
      icon = Devicon.BashOriginal;
      break;
    case 'php':
      icon = Devicon.PhpOriginal;
      break;
    case 'py':
      icon = Devicon.PythonOriginal;
      break;
    case 'pl':
      icon = Devicon.PerlOriginal;
      break;
    case 'lua':
      icon = Devicon.LuaOriginal;
      break;
    case 'rb':
      icon = Devicon.RubyOriginal;
      break;
    case 'md':
      icon = Devicon.MarkdownOriginal;
      break;
    default:
      icon = TextSnippetOutlined;
      break;
  }

  const IconWithStyles = styled(icon)({
    width: '20px',
    height: '20px',
  });

  return <IconWithStyles />;
};

export const RefIcon = ({ refName }: { refName: string }) => {
  let icon: React.FunctionComponent;

  switch (refName) {
    case 'Functions':
    case 'Function':
      icon = TbMathFunction;
      break;
    case 'Variable':
      icon = VscSymbolVariable;
      break;
    case 'Types':
    case 'Type':
    case 'Underlying type':
    case 'Return type':
    case 'Typedef':
      icon = FiType;
      break;
    case 'Enum':
      icon = VscSymbolEnum;
      break;
    case 'Macros':
    case 'Macro':
      icon = TbMacro;
      break;
    case 'Includes':
      icon = HiHashtag;
      break;
    case 'Name':
      icon = Badge;
      break;
    case 'Qualified name':
      icon = Person;
      break;
    case 'Signature':
      icon = DriveFileRenameOutline;
      break;
    case 'Callee':
      icon = PhoneForwarded;
      break;
    case 'Caller':
      icon = Call;
      break;
    case 'Declaration':
      icon = DataObject;
      break;
    case 'Definition':
    case 'Undefinitions':
      icon = VscSymbolField;
      break;
    case 'Function pointer call':
      icon = PhoneMissed;
      break;
    case 'Local variables':
      icon = VscVariableGroup;
      break;
    case 'Overridden by':
    case 'Overrides':
      icon = TbMathFunction;
      break;
    case 'Parameters':
      icon = VscSymbolParameter;
      break;
    case 'This calls':
      icon = PhoneCallback;
      break;
    case 'Usage':
      icon = DataUsage;
      break;
    case 'Virtual call':
      icon = WifiCalling3;
      break;
    case 'Reads':
      icon = MenuBook;
      break;
    case 'Writes':
      icon = Edit;
      break;
    case 'Aliases':
      icon = TbTableAlias;
      break;
    case 'Inherits from':
      icon = GrInherit;
      break;
    case 'Inherited by':
      icon = GrInherit;
      break;
    case 'Data member':
      icon = DataUsage;
      break;
    case 'Method':
      icon = VscSymbolMethod;
      break;
    case 'Friends':
      icon = People;
      break;
    case 'Enum constants':
      icon = VscSymbolEnumMember;
      break;
    case 'Expansions':
      icon = HiArrowsExpand;
      break;
    default:
      icon = TextSnippet;
      break;
  }

  const IconWithStyles = styled(icon)(({ theme }) => ({
    width: '20px',
    height: '20px',
    color: theme.iconColors?.ref,
  }));

  return <IconWithStyles />;
};

export const GitIcon = ({ name }: { name: string }) => {
  let icon: React.FunctionComponent;

  switch (name) {
    case 'repository':
      icon = VscRepo;
      break;
    case 'branch':
      icon = VscSourceControl;
      break;
    case 'tag':
      icon = Sell;
      break;
    case 'commit':
      icon = Commit;
      break;
    case 'repolist':
      icon = Storage;
      break;
    default:
      icon = TextSnippet;
      break;
  }

  const IconWithStyles = styled(icon)(({ theme }) => ({
    width: '20px',
    height: '20px',
    color: theme.iconColors?.git,
  }));

  return <IconWithStyles />;
};
