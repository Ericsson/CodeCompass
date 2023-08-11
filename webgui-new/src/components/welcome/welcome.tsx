import React, { useState } from 'react';
import * as SC from './styled-components';
import Link from 'next/link';
import { useTranslation } from 'react-i18next';
import CC_Overview from '@images/cc_overview.png';
import CC_InfoTree from '@images/cc_info_tree.png';
import CC_CodeBites from '@images/cc_codebites.png';
import CC_Metrics from '@images/cc_metrics.png';
import CC_Diagrams from '@images/cc_diagrams.png';
import CC_RevControl from '@images/cc_revcontrol.png';
import { StaticImageData } from 'next/image';
import { Modal } from '@mui/material';

export const Welcome = (): JSX.Element => {
  const { t } = useTranslation();
  const [currentImg, setCurrentImg] = useState<StaticImageData | null>(null);
  const [modalOpen, setModalOpen] = useState<boolean>(false);

  const ImageContainer = ({ image }: { image: StaticImageData }): JSX.Element => {
    return (
      <SC.ImageContainer
        sx={{
          backgroundImage: `url(${image.src})`,
          width: image.width / 2,
          height: image.height / 2,
        }}
        onClick={() => {
          setModalOpen(true);
          setCurrentImg(image);
        }}
      />
    );
  };

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
        <ImageContainer image={CC_Overview} />
      </SC.Container>

      <SC.StyledHeading2>{t('welcome.featureHighlights')}</SC.StyledHeading2>

      <SC.Container>
        <SC.FeatureDescription>
          <SC.StyledHeading3>{t('welcome.infoTree.title')}</SC.StyledHeading3>
          <SC.StyledParagraph>{t('welcome.infoTree.desc')}</SC.StyledParagraph>
        </SC.FeatureDescription>
        <ImageContainer image={CC_InfoTree} />
      </SC.Container>

      <SC.Container>
        <SC.FeatureDescription>
          <SC.StyledHeading3>{t('welcome.codeBites.title')}</SC.StyledHeading3>
          <SC.StyledParagraph>{t('welcome.codeBites.desc')}</SC.StyledParagraph>
        </SC.FeatureDescription>
        <ImageContainer image={CC_CodeBites} />
      </SC.Container>

      <SC.Container>
        <SC.FeatureDescription>
          <SC.StyledHeading3>{t('welcome.metrics.title')}</SC.StyledHeading3>
          <SC.StyledParagraph>{t('welcome.metrics.desc')}</SC.StyledParagraph>
        </SC.FeatureDescription>
        <ImageContainer image={CC_Metrics} />
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
        <ImageContainer image={CC_Diagrams} />
      </SC.Container>

      <SC.Container>
        <SC.FeatureDescription>
          <SC.StyledHeading3>{t('welcome.revControl.title')}</SC.StyledHeading3>
          <SC.StyledParagraph>{t('welcome.revControl.desc')}</SC.StyledParagraph>
        </SC.FeatureDescription>
        <ImageContainer image={CC_RevControl} />
      </SC.Container>

      <Modal open={modalOpen} onClose={() => setModalOpen(false)}>
        <SC.ModalBox>
          {currentImg ? (
            <SC.ImageContainer
              sx={{
                backgroundImage: `url(${currentImg.src})`,
                width: currentImg.width,
                height: currentImg.height,
              }}
            />
          ) : (
            <></>
          )}
        </SC.ModalBox>
      </Modal>
    </SC.OuterContainer>
  );
};
