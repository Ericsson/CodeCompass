import { IconButton, styled } from '@mui/material';
import { Sidebar, Menu, MenuItem, SubMenu, useProSidebar } from 'react-pro-sidebar';
import { ArrowCircleRight, ArrowCircleLeft, Folder, Search, Info, GitHub } from '@mui/icons-material';

const SidebarContainer = styled('div')({
  display: 'flex',
  flexDirection: 'column',
});

const SidebarHeader = styled('div')(({ theme }) => ({
  display: 'flex',
  justifyContent: 'center',
  alignItems: 'center',
  gap: '1rem',
  borderBottom: `1px solid ${theme.colors?.primary}`,
  borderRight: `1px solid ${theme.colors?.primary}`,
  padding: '10px',
}));

const StyledMenu = styled(Menu)(({ theme }) => ({
  '& > aside': {
    borderRightStyle: 'none',
    borderRight: `1px solid ${theme.colors?.primary}`,
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

const IconLabel = styled('div')({
  display: 'flex',
  alignItems: 'center',
  gap: '0.6rem',
});

export const SidebarMenu = (): JSX.Element => {
  const { collapseSidebar, toggleSidebar, collapsed, toggled, broken, rtl } = useProSidebar();

  return (
    <SidebarContainer>
      <SidebarHeader>
        {collapsed ? '' : 'Menu'}
        <IconButton onClick={() => collapseSidebar()}>
          {collapsed ? <ArrowCircleRight /> : <ArrowCircleLeft />}
        </IconButton>
      </SidebarHeader>
      <Sidebar>
        <StyledMenu>
          <SubMenu
            label={
              <IconLabel>
                <Folder />
                {collapsed ? '' : 'File manager'}
              </IconLabel>
            }
          >
            <MenuItem>auth.c</MenuItem>
            <MenuItem>auth.h</MenuItem>
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
