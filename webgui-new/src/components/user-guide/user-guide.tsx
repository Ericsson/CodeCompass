import React from 'react';
import * as SC from './styled-components';
import { useTranslation } from 'react-i18next';

export const UserGuide = (): JSX.Element => {
  const { t } = useTranslation();

  return (
    <SC.OuterContainer>
      <SC.StyledHeading1>{t('userGuide.title')}</SC.StyledHeading1>
      <SC.StyledHeading2>{t('userGuide.header.title')}</SC.StyledHeading2>
      <SC.StyledParagraph>{t('userGuide.header.desc')}</SC.StyledParagraph>
      <SC.StyledHeading2>{t('userGuide.navigation.title')}</SC.StyledHeading2>
      <SC.StyledParagraph>{t('userGuide.navigation.desc')}</SC.StyledParagraph>
      <SC.StyledUl>
        <li>{t('userGuide.navigation.tabs.n1')}</li>
        <li>{t('userGuide.navigation.tabs.n2')}</li>
        <li>{t('userGuide.navigation.tabs.n3')}</li>
        <li>{t('userGuide.navigation.tabs.n4')}</li>
        <li>{t('userGuide.navigation.tabs.n5')}</li>
        <li>{t('userGuide.navigation.tabs.n6')}</li>
      </SC.StyledUl>
      <SC.StyledHeading2>{t('userGuide.fileManager.title')}</SC.StyledHeading2>
      <SC.StyledParagraph>{t('userGuide.fileManager.desc1')}</SC.StyledParagraph>
      <SC.StyledParagraph>{t('userGuide.fileManager.desc2')}</SC.StyledParagraph>
      <SC.StyledHeading2>{t('userGuide.searchResults.title')}</SC.StyledHeading2>
      <SC.StyledParagraph>{t('userGuide.searchResults.desc1')}</SC.StyledParagraph>
      <SC.StyledParagraph>{t('userGuide.searchResults.desc2')}</SC.StyledParagraph>
      <SC.StyledParagraph>{t('userGuide.searchResults.desc3')}</SC.StyledParagraph>
      <SC.StyledHeading2>{t('userGuide.diagrams.title')}</SC.StyledHeading2>
      <SC.StyledParagraph>{t('userGuide.diagrams.desc1')}</SC.StyledParagraph>
      <SC.StyledParagraph>{t('userGuide.diagrams.desc2')}</SC.StyledParagraph>
      <SC.StyledParagraph>{t('userGuide.diagrams.desc3')}</SC.StyledParagraph>
      <SC.StyledHeading2>{t('userGuide.metrics.title')}</SC.StyledHeading2>
      <SC.StyledParagraph>{t('userGuide.metrics.desc1')}</SC.StyledParagraph>
      <SC.StyledParagraph>{t('userGuide.metrics.desc2')}</SC.StyledParagraph>
      <SC.StyledHeading2>{t('userGuide.infoTree.title')}</SC.StyledHeading2>
      <SC.StyledParagraph>{t('userGuide.infoTree.desc1')}</SC.StyledParagraph>
      <SC.StyledParagraph>{t('userGuide.infoTree.desc2')}</SC.StyledParagraph>
      <SC.StyledParagraph>{t('userGuide.infoTree.desc3')}</SC.StyledParagraph>
      <SC.StyledHeading2>{t('userGuide.revControl.title')}</SC.StyledHeading2>
      <SC.StyledParagraph>{t('userGuide.revControl.desc1')}</SC.StyledParagraph>
      <SC.StyledParagraph>{t('userGuide.revControl.desc2')}</SC.StyledParagraph>
      <SC.StyledParagraph>{t('userGuide.revControl.desc3')}</SC.StyledParagraph>
    </SC.OuterContainer>
  );
};
