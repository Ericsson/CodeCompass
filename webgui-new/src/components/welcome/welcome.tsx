import React from 'react';
import * as SC from './styled-components';
import Link from 'next/link';
import Image from 'next/image';

export const Welcome = (): JSX.Element => {
  return (
    <SC.OuterContainer>
      <SC.StyledHeading1>CodeCompass</SC.StyledHeading1>
      <SC.StyledParagraph sx={{ width: '60%' }}>
        CodeCompass is a pluginable code comprehension tool which helps you to understand large code bases.
      </SC.StyledParagraph>

      <SC.StyledHeading2>
        Highlights of the new{' '}
        <Link href={'https://github.com/ericsson/CodeCompass'} target="_blank">
          {'Flash'}
        </Link>{' '}
        release
      </SC.StyledHeading2>

      <SC.Container>
        <SC.StyledUl>
          <li>Pluginable interface</li>
          <li>New icon set</li>
          <li>Faster/parallel parsing of C/C++ files</li>
          <li>New Git Navigation/Commit View</li>
          <li>Faster diagram generation</li>
        </SC.StyledUl>
        <Image
          src={'/cc_overview.png'}
          alt={'Overview'}
          width={0}
          height={0}
          sizes="100vw"
          style={{ width: '25%', height: 'auto' }}
        />
      </SC.Container>

      <SC.StyledHeading2>Feature highlights</SC.StyledHeading2>

      <SC.Container>
        <SC.FeatureDescription>
          <SC.StyledHeading3>Info Tree</SC.StyledHeading3>
          <SC.StyledParagraph>
            Info Tree shows different information about given language elements such as its name, qualified name, and
            place of definition.
          </SC.StyledParagraph>
        </SC.FeatureDescription>
        <Image
          src={'/cc_info_tree.png'}
          alt={'Overview'}
          width={0}
          height={0}
          sizes="100vw"
          style={{ width: '10%', height: 'auto' }}
        />
      </SC.Container>

      <SC.Container>
        <SC.FeatureDescription>
          <SC.StyledHeading3>CodeBites</SC.StyledHeading3>
          <SC.StyledParagraph>
            By CodeBites diagram one can inspect the source code without loosing the visited path. CodeBites can be
            called on functions, variables, classes or macros. In each case the definition of the element is placed in a
            graph (tree) node.
          </SC.StyledParagraph>
        </SC.FeatureDescription>
        <Image
          src={'/cc_codebites.png'}
          alt={'Overview'}
          width={0}
          height={0}
          sizes="100vw"
          style={{ width: '25%', height: 'auto' }}
        />
      </SC.Container>

      <SC.Container>
        <SC.FeatureDescription>
          <SC.StyledHeading3>Metrics</SC.StyledHeading3>
          <SC.StyledParagraph>
            Spot your most problematic modules by asking for &quot;Metrics&quot; for a directory. A treemap view will
            show the the distribution of codechecker faults or McCabe complexity organized by directory.
          </SC.StyledParagraph>
        </SC.FeatureDescription>
        <Image
          src={'/cc_metrics.png'}
          alt={'Overview'}
          width={0}
          height={0}
          sizes="100vw"
          style={{ width: '25%', height: 'auto' }}
        />
      </SC.Container>

      <SC.Container>
        <SC.FeatureDescription>
          <SC.StyledHeading3>Diagrams</SC.StyledHeading3>
          <SC.StyledParagraph>CodeCompass can display many diagrams:</SC.StyledParagraph>
          <SC.StyledUl sx={{ marginTop: '5px' }}>
            <li>Function call diagram</li>
            <li>UML class diagram</li>
            <li>Collaboration diagram</li>
            <li>CodeBites</li>
            <li>etc.</li>
          </SC.StyledUl>
        </SC.FeatureDescription>
        <Image
          src={'/cc_diagrams.png'}
          alt={'Overview'}
          width={0}
          height={0}
          sizes="100vw"
          style={{ width: '25%', height: 'auto' }}
        />
      </SC.Container>

      <SC.Container>
        <SC.FeatureDescription>
          <SC.StyledHeading3>Revision Control Navigator</SC.StyledHeading3>
          <SC.StyledParagraph>
            The revision control navigator allows to search in the GIT repositories, visualize branches and merges, list
            and show commit diffs.
          </SC.StyledParagraph>
        </SC.FeatureDescription>
        <Image
          src={'/cc_revcontrol.png'}
          alt={'Overview'}
          width={0}
          height={0}
          sizes="100vw"
          style={{ width: '25%', height: 'auto' }}
        />
      </SC.Container>
    </SC.OuterContainer>
  );
};
