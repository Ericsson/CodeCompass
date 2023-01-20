import { IconButton, styled } from '@mui/material';
import { Sidebar, Menu, MenuItem, SubMenu, useProSidebar } from 'react-pro-sidebar';
import { ArrowCircleRight, ArrowCircleLeft, Folder, Search, Info, GitHub } from '@mui/icons-material';
import Logo from '../../../public/logo.png';
import { FileTree } from '../file-tree/file-tree';

const SidebarContainer = styled('div')(({ theme }) => ({
  display: 'flex',
  flexDirection: 'column',
  borderRight: `1px solid ${theme.colors?.primary}`,
}));

const SidebarHeader = styled('div')(({ theme }) => ({
  display: 'flex',
  justifyContent: 'center',
  alignItems: 'center',
  padding: '10px',
  height: '80px',
  color: theme.colors?.primary,
  backgroundColor: theme.backgroundColors?.primary,
  borderBottom: `1px solid ${theme.colors?.primary}`,
  backgroundImage: `url('${Logo.src}')`,
  backgroundRepeat: 'no-repeat',
  backgroundPosition: 'left center',
  backgroundSize: '100%',

  '& > div': {
    backgroundColor: theme.backgroundColors?.primary,
    borderRadius: '5px',
  },
}));

const StyledMenu = styled(Menu)(({ theme }) => ({
  '& > aside': {
    border: 'none',
  },
  '& a': {
    color: theme.colors?.primary,
    backgroundColor: theme.backgroundColors?.primary,
  },
  '& a:hover': {
    color: theme.backgroundColors?.primary,
    backgroundColor: theme.colors?.primary,
  },
}));

const StyledHeading = styled('h1')(({ theme }) => ({
  fontSize: '1.2rem',
}));

const IconLabel = styled('div')({
  display: 'flex',
  alignItems: 'center',
  gap: '0.6rem',
});

export const SidebarMenu = (): JSX.Element => {
  const { collapseSidebar, toggleSidebar, collapsed, toggled, broken, rtl } = useProSidebar();

  return (
    <SidebarContainer>
      <Sidebar>
        <SidebarHeader>
          <IconLabel>
            <IconButton onClick={() => collapseSidebar()}>
              {collapsed ? <ArrowCircleRight /> : <ArrowCircleLeft />}
            </IconButton>
            {collapsed ? '' : <StyledHeading>{'CodeCompass'}</StyledHeading>}
          </IconLabel>
        </SidebarHeader>
        <StyledMenu>
          <SubMenu
            label={
              <IconLabel>
                <Folder />
                {collapsed ? '' : 'File manager'}
              </IconLabel>
            }
          >
            <FileTree />
          </SubMenu>
          <SubMenu
            label={
              <IconLabel>
                <Search />
                {collapsed ? '' : 'Search results'}
              </IconLabel>
            }
          >
            <MenuItem>main.c</MenuItem>
          </SubMenu>
          <SubMenu
            label={
              <IconLabel>
                <Info />
                {collapsed ? '' : 'Info tree'}
              </IconLabel>
            }
          >
            <MenuItem>Info 1</MenuItem>
            <MenuItem>Info 2</MenuItem>
          </SubMenu>
          <SubMenu
            label={
              <IconLabel>
                <GitHub />
                {collapsed ? '' : 'Revision control'}
              </IconLabel>
            }
          >
            <MenuItem>Content</MenuItem>
          </SubMenu>
        </StyledMenu>
      </Sidebar>
    </SidebarContainer>
  );
};
