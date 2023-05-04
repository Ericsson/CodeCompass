import React from 'react';
import { styled } from '@mui/material';

const OuterContainer = styled('div')({
  padding: '10px',
});

const ProjectMemberContainer = styled('div')({
  display: 'flex',
  alignItems: 'center',
  gap: '3rem',
  marginBottom: '5px',

  '& > div:nth-of-type(1)': {
    width: '200px',
  },
});

const StyledHeading2 = styled('h2')({
  fontSize: '1.1rem',
  marginBottom: '10px',
});

export const Credits = () => {
  return (
    <OuterContainer>
      <StyledHeading2>{'Project members'}</StyledHeading2>
      <ProjectMemberContainer>
        <div>{'Zoltán Borók-Nagy'}</div>
        <div>{'C++ plugin'}</div>
      </ProjectMemberContainer>
      <ProjectMemberContainer>
        <div>{'Tibor Brunner'}</div>
        <div>{'Parser & Service infrastructure, C++ plugin, Metrics plugin, Web GUI, Diagrams'}</div>
      </ProjectMemberContainer>
      <ProjectMemberContainer>
        <div>{'Máté Cserép'}</div>
        <div>{'Parser infrastructure, C++ plugin, CodeBites, Diagrams'}</div>
      </ProjectMemberContainer>
      <ProjectMemberContainer>
        <div>{'Tamás Cséri'}</div>
        <div>{'Git plugin'}</div>
      </ProjectMemberContainer>
      <ProjectMemberContainer>
        <div>{'Márton Csordás'}</div>
        <div>{'Build system, Parser & Service infrastructure, Web GUI'}</div>
      </ProjectMemberContainer>
      <ProjectMemberContainer>
        <div>{'Anett Fekete'}</div>
        <div>{'C++ plugin'}</div>
      </ProjectMemberContainer>
      <ProjectMemberContainer>
        <div>{'Gábor Alex Ispánovics'}</div>
        <div>{'C++ plugin, Search plugin'}</div>
      </ProjectMemberContainer>
      <ProjectMemberContainer>
        <div>{'Zoltán Porkoláb'}</div>
        <div>{'Project owner, C++ expert'}</div>
      </ProjectMemberContainer>
      <ProjectMemberContainer>
        <div>{'Richárd Szalay'}</div>
        <div>{'C++ plugin, Webserver'}</div>
      </ProjectMemberContainer>
      <ProjectMemberContainer>
        <div>{'Deme Maráki'}</div>
        <div>{'New Web GUI'}</div>
      </ProjectMemberContainer>
      <StyledHeading2 sx={{ marginTop: '30px' }}>{'Thanks for Support'}</StyledHeading2>
      <div>{'Thanks for MTAS and MiniLink for support!'}</div>
      <StyledHeading2 sx={{ marginTop: '30px' }}>{'Special Thanks'}</StyledHeading2>
      <div>{'Special thanks to the Coffee Machine!'}</div>
    </OuterContainer>
  );
};
