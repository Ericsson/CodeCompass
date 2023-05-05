import React from 'react';
import * as SC from './styled-components';

export const Welcome = (): JSX.Element => {
  return (
    <SC.OuterContainer>
      <SC.StyledHeading1>{'CodeCompass'}</SC.StyledHeading1>
      <div>{'CodeCompass is a pluginable code comprehension tool which helps you to understand large code bases.'}</div>
    </SC.OuterContainer>
  );
};
