import { styled } from '@mui/material';
import { Sidebar, Menu, MenuItem, SubMenu } from 'react-pro-sidebar';

const SidebarContainer = styled('div')({
  height: '90vh',
});

export const SidebarMenu = (): JSX.Element => {
  return (
    <SidebarContainer>
      <Sidebar style={{ height: '100%' }}>
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
          <SubMenu label='File manager'>
            <MenuItem>auth.c</MenuItem>
            <MenuItem>auth.h</MenuItem>
          </SubMenu>
          <SubMenu label='Search results'>
            <MenuItem>main.c</MenuItem>
          </SubMenu>
          <SubMenu label='Info tree'>
            <MenuItem>Info 1</MenuItem>
            <MenuItem>Info 2</MenuItem>
          </SubMenu>
          <SubMenu label='Revision control navigator'>
            <MenuItem>Content</MenuItem>
          </SubMenu>
        </Menu>
      </Sidebar>
    </SidebarContainer>
  );
};
