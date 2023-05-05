import React from 'react';
import * as SC from './styled-components';

export const Credits = (): JSX.Element => {
  return (
    <SC.OuterContainer>
      <SC.StyledHeading2>{'Project members'}</SC.StyledHeading2>
      <SC.ProjectMemberContainer>
        <div>{'Zoltán Borók-Nagy'}</div>
        <div>{'C++ plugin'}</div>
      </SC.ProjectMemberContainer>
      <SC.ProjectMemberContainer>
        <div>{'Tibor Brunner'}</div>
        <div>{'Parser & Service infrastructure, C++ plugin, Metrics plugin, Web GUI, Diagrams'}</div>
      </SC.ProjectMemberContainer>
      <SC.ProjectMemberContainer>
        <div>{'Máté Cserép'}</div>
        <div>{'Parser infrastructure, C++ plugin, CodeBites, Diagrams'}</div>
      </SC.ProjectMemberContainer>
      <SC.ProjectMemberContainer>
        <div>{'Tamás Cséri'}</div>
        <div>{'Git plugin'}</div>
      </SC.ProjectMemberContainer>
      <SC.ProjectMemberContainer>
        <div>{'Márton Csordás'}</div>
        <div>{'Build system, Parser & Service infrastructure, Web GUI'}</div>
      </SC.ProjectMemberContainer>
      <SC.ProjectMemberContainer>
        <div>{'Anett Fekete'}</div>
        <div>{'C++ plugin'}</div>
      </SC.ProjectMemberContainer>
      <SC.ProjectMemberContainer>
        <div>{'Gábor Alex Ispánovics'}</div>
        <div>{'C++ plugin, Search plugin'}</div>
      </SC.ProjectMemberContainer>
      <SC.ProjectMemberContainer>
        <div>{'Zoltán Porkoláb'}</div>
        <div>{'Project owner, C++ expert'}</div>
      </SC.ProjectMemberContainer>
      <SC.ProjectMemberContainer>
        <div>{'Richárd Szalay'}</div>
        <div>{'C++ plugin, Webserver'}</div>
      </SC.ProjectMemberContainer>
      <SC.ProjectMemberContainer>
        <div>{'Deme Maráki'}</div>
        <div>{'New Web GUI'}</div>
      </SC.ProjectMemberContainer>
      <SC.StyledHeading2 sx={{ marginTop: '30px' }}>{'Thanks for Support'}</SC.StyledHeading2>
      <div>{'Thanks for MTAS and MiniLink for support!'}</div>
      <SC.StyledHeading2 sx={{ marginTop: '30px' }}>{'Special Thanks'}</SC.StyledHeading2>
      <div>{'Special thanks to the Coffee Machine!'}</div>
    </SC.OuterContainer>
  );
};
