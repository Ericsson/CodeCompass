import React from 'react';
import * as SC from './styled-components';
import { useTranslation } from 'react-i18next';

export const Credits = (): JSX.Element => {
  const { t } = useTranslation();

  return (
    <SC.OuterContainer>
      <SC.StyledHeading2>{t('credits.projectMembers.title')}</SC.StyledHeading2>
      {Array.from({ length: 10 }, (_, i) => i).map((n) => (
        <SC.ProjectMemberContainer key={n}>
          <div>{t(`credits.projectMembers.n${n + 1}.name`)}</div>
          <div>{t(`credits.projectMembers.n${n + 1}.contribution`)}</div>
        </SC.ProjectMemberContainer>
      ))}
      <SC.StyledHeading2 sx={{ marginTop: '30px' }}>{t('credits.thanks.title')}</SC.StyledHeading2>
      <div>{t('credits.thanks.text')}</div>
      <SC.StyledHeading2 sx={{ marginTop: '30px' }}>{t('credits.specialThanks.title')}</SC.StyledHeading2>
      <div>{t('credits.specialThanks.text')}</div>
    </SC.OuterContainer>
  );
};
