import React from 'react';
import * as SC from './styled-components';
import Link from 'next/link';
import Image from 'next/image';
import { useTranslation } from 'react-i18next';

export const Welcome = (): JSX.Element => {
  const { t } = useTranslation();

  return (
    <SC.OuterContainer>
      <SC.StyledHeading1>{t('welcome.title')}</SC.StyledHeading1>
      <SC.StyledParagraph sx={{ width: '60%' }}>{t('welcome.desc')}</SC.StyledParagraph>

      <SC.StyledHeading2>
        {t('welcome.newRelease')} (
        <Link href={'https://github.com/ericsson/CodeCompass'} target="_blank">
          {'Flash'}
        </Link>
        )
      </SC.StyledHeading2>

      <SC.Container>
        <SC.StyledUl>
          <li>{t('welcome.features.n1')}</li>
          <li>{t('welcome.features.n2')}</li>
          <li>{t('welcome.features.n3')}</li>
          <li>{t('welcome.features.n4')}</li>
          <li>{t('welcome.features.n5')}</li>
        </SC.StyledUl>
        <Image
          src={'/cc_overview.png'}
          alt={t('welcome.title')}
          width={0}
          height={0}
          sizes="100vw"
          style={{ width: '25%', height: 'auto' }}
        />
      </SC.Container>

      <SC.StyledHeading2>{t('welcome.featureHighlights')}</SC.StyledHeading2>

      <SC.Container>
        <SC.FeatureDescription>
          <SC.StyledHeading3>{t('welcome.infoTree.title')}</SC.StyledHeading3>
          <SC.StyledParagraph>{t('welcome.infoTree.desc')}</SC.StyledParagraph>
        </SC.FeatureDescription>
        <Image
          src={'/cc_info_tree.png'}
          alt={t('welcome.infoTree.title')}
          width={0}
          height={0}
          sizes="100vw"
          style={{ width: '10%', height: 'auto' }}
        />
      </SC.Container>

      <SC.Container>
        <SC.FeatureDescription>
          <SC.StyledHeading3>{t('welcome.codeBites.title')}</SC.StyledHeading3>
          <SC.StyledParagraph>{t('welcome.codeBites.desc')}</SC.StyledParagraph>
        </SC.FeatureDescription>
        <Image
          src={'/cc_codebites.png'}
          alt={t('welcome.codeBites.title')}
          width={0}
          height={0}
          sizes="100vw"
          style={{ width: '25%', height: 'auto' }}
        />
      </SC.Container>

      <SC.Container>
        <SC.FeatureDescription>
          <SC.StyledHeading3>{t('welcome.metrics.title')}</SC.StyledHeading3>
          <SC.StyledParagraph>{t('welcome.metrics.desc')}</SC.StyledParagraph>
        </SC.FeatureDescription>
        <Image
          src={'/cc_metrics.png'}
          alt={t('welcome.metrics.title')}
          width={0}
          height={0}
          sizes="100vw"
          style={{ width: '25%', height: 'auto' }}
        />
      </SC.Container>

      <SC.Container>
        <SC.FeatureDescription>
          <SC.StyledHeading3>{t('welcome.diagrams.title')}</SC.StyledHeading3>
          <SC.StyledParagraph>{t('welcome.diagrams.desc')}</SC.StyledParagraph>
          <SC.StyledUl sx={{ marginTop: '5px' }}>
            <li>{t('welcome.diagrams.types.n1')}</li>
            <li>{t('welcome.diagrams.types.n2')}</li>
            <li>{t('welcome.diagrams.types.n3')}</li>
            <li>{t('welcome.diagrams.types.n4')}</li>
            <li>{t('welcome.diagrams.types.n5')}</li>
          </SC.StyledUl>
        </SC.FeatureDescription>
        <Image
          src={'/cc_diagrams.png'}
          alt={t('welcome.diagrams.title')}
          width={0}
          height={0}
          sizes="100vw"
          style={{ width: '25%', height: 'auto' }}
        />
      </SC.Container>

      <SC.Container>
        <SC.FeatureDescription>
          <SC.StyledHeading3>{t('welcome.revControl.title')}</SC.StyledHeading3>
          <SC.StyledParagraph>{t('welcome.revControl.desc')}</SC.StyledParagraph>
        </SC.FeatureDescription>
        <Image
          src={'/cc_revcontrol.png'}
          alt={t('welcome.revControl.title')}
          width={0}
          height={0}
          sizes="100vw"
          style={{ width: '25%', height: 'auto' }}
        />
      </SC.Container>
    </SC.OuterContainer>
  );
};
