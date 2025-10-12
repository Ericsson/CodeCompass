import { SearchMethods } from 'enums/search-enum';
import { Trans } from 'react-i18next';
import React from 'react';

export const getTooltipText = (value: string): string | React.ReactNode => {
  switch (value) {
    case SearchMethods.FILE_REGEX:
      return <Trans i18nKey={'searchSettings.tooltips.fileRegexTooltip'} components={{ b: <b /> }} />;
    case SearchMethods.PATH_REGEX:
      return <Trans i18nKey={'searchSettings.tooltips.pathRegexTooltip'} components={{ b: <b /> }} />;
    case SearchMethods.EXPRESSION:
      return (
        <>
          <div>
            <Trans
              i18nKey={'searchSettings.tooltips.searchOptionTooltip.textAndDefinition'}
              components={{ b: <b /> }}
            />
          </div>
          <ul style={{ listStyle: 'inside' }}>
            <li>
              <Trans i18nKey={'searchSettings.tooltips.searchOptionTooltip.phrases'} components={{ b: <b /> }} />
            </li>
            <li>
              <Trans i18nKey={'searchSettings.tooltips.searchOptionTooltip.wildcars'} components={{ b: <b /> }} />
            </li>
            <li>
              <Trans i18nKey={'searchSettings.tooltips.searchOptionTooltip.regex'} components={{ b: <b /> }} />
            </li>
            <li>
              <Trans i18nKey={'searchSettings.tooltips.searchOptionTooltip.boolean'} components={{ b: <b /> }} />
              <ul style={{ listStyle: 'inside', marginLeft: '20px' }}>
                <li>
                  <Trans i18nKey={'searchSettings.tooltips.searchOptionTooltip.booleanOR'} components={{ b: <b /> }} />
                </li>
                <li>
                  <Trans i18nKey={'searchSettings.tooltips.searchOptionTooltip.booleanAND'} components={{ b: <b /> }} />
                </li>
              </ul>
            </li>
            <li>
              <Trans i18nKey={'searchSettings.tooltips.searchOptionTooltip.escaping'} components={{ b: <b /> }} />
            </li>
          </ul>
          <div>
            <Trans i18nKey={'searchSettings.tooltips.searchOptionTooltip.fileSearch'} components={{ b: <b /> }} />
          </div>
          <div>
            <Trans i18nKey={'searchSettings.tooltips.searchOptionTooltip.logSearch'} components={{ b: <b /> }} />
          </div>
        </>
      );
    default:
      return <></>;
  }
};
