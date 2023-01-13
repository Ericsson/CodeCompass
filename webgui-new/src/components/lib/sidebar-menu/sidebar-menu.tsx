import { IconButton, styled } from '@mui/material';
import { Sidebar, Menu, MenuItem, SubMenu, useProSidebar } from 'react-pro-sidebar';
import { ArrowCircleRight, ArrowCircleLeft, Folder, Search, Info, GitHub } from '@mui/icons-material';

const SidebarContainer = styled('div')({});

const SidebarHeader = styled('div')(({ theme }) => ({
  display: 'flex',
  justifyContent: 'center',
  alignItems: 'center',
  gap: '1rem',
  borderBottom: `1px solid ${theme.colors?.primary}`,
  borderRight: `1px solid ${theme.colors?.primary}`,
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
        <Menu
          menuItemStyles={{
            button: ({ level, active, disabled }) => {
              return {
                color: '#fff',
                backgroundColor: '#000',
                ':hover': {
                  color: '#fff',
                  backgroundColor: '#000',
                },
              };
            },
          }}
        >
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
        </Menu>
      </Sidebar>
    </SidebarContainer>
  );
};
