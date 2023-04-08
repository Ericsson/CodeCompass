import { IconButton, InputAdornment, TextField, Tooltip, styled } from '@mui/material';
import { ProjectSelect } from 'components/project-select/project-select';
import { Search, Settings, LightMode, DarkMode, Info } from '@mui/icons-material';
import { KeyboardEvent, useContext, useState } from 'react';
import { SearchOptions, SearchMethods, SearchMainLanguages, SearchTypes } from 'enums/search-enum';
import { enumToArray } from 'utils/utils';
import { ThemeContext } from 'global-context/theme-context';
import Logo from '../../../public/logo.png';
import { SettingsMenu } from 'components/settings-menu/settings-menu';
import { getTooltipText } from './get-tooltip-text';
import { getSearchResultCount, getSearchResults } from 'service/search-service';
import { FileSearchResult, SearchResult } from '@thrift-generated';
import { AccordionLabel } from 'enums/accordion-enum';
import { removeStore } from 'utils/store';
import { AppContext } from 'global-context/app-context';

const StyledHeader = styled('header')(({ theme }) => ({
  display: 'grid',
  gridTemplateColumns: '280px 1fr',
  gridTemplateRows: '1fr',
  borderBottom: `1px solid ${theme.colors?.primary}`,
  minWidth: '1460px',
}));

const HeaderLogo = styled('div')(({ theme }) => ({
  height: '75px',
  backgroundImage: `url('${Logo.src}')`,
  backgroundRepeat: 'no-repeat',
  backgroundSize: '90%',
  backgroundPosition: 'center center',
  borderRight: `1px solid ${theme.colors?.primary}`,
}));

const HeaderContent = styled('div')({
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
  flexGrow: '1',
  padding: '10px 10px 10px 15px',
});

const SettingsContainer = styled('div')({
  display: 'flex',
  gap: '1rem',
});

export const Header = (): JSX.Element => {
  const { theme, setTheme } = useContext(ThemeContext);
  const appCtx = useContext(AppContext);

  const searchTypes = enumToArray(SearchTypes);
  const [settingsAnchorEl, setSettingsAnchorEl] = useState<null | HTMLElement>(null);

  const handleSearch = async (e: KeyboardEvent<HTMLDivElement>) => {
    if (e.key === 'Enter') {
      const query = createQueryString(appCtx.searchQuery);
      if (query === '') return;

      const isFileSearch = appCtx.searchCurrentOption?.name === SearchOptions.FILE_NAME.toString();
      const searchResultCount = (await getSearchResultCount(
        isFileSearch,
        appCtx.searchCurrentOption?.id as number,
        query,
        appCtx.searchFileFilterQuery,
        appCtx.searchDirFilterQuery
      )) as number;
      const searchResults = (await getSearchResults(
        isFileSearch,
        appCtx.searchCurrentOption?.id as number,
        query,
        0,
        10,
        appCtx.searchFileFilterQuery,
        appCtx.searchDirFilterQuery
      )) as SearchResult | FileSearchResult;

      appCtx.setSearchStart(0);
      appCtx.setSearchSize(10);
      appCtx.setSearchResultCount(searchResultCount);
      appCtx.setSearchResult(searchResults);
      appCtx.setIsFileSearch(isFileSearch);
      appCtx.setActiveAccordion(AccordionLabel.SEARCH_RESULTS);
      removeStore(['storedExpandedSearchFileNodes', 'storedExpandedSearchPathNodes']);
    } else {
      return;
    }
  };

  const getSearchTypeQuery = (searchType: string): string => {
    if (searchType === SearchTypes.TYPE) {
      return 'type';
    } else if (searchType === SearchTypes.FUNCTION) {
      return 'func';
    } else if (searchType === SearchTypes.CONSTANT) {
      return 'const';
    } else if (searchType === SearchTypes.VARIABLE) {
      return 'var';
    } else if (searchType === SearchTypes.FIELD) {
      return 'field';
    } else if (searchType === SearchTypes.LABEL) {
      return 'label';
    } else if (searchType === SearchTypes.MACRO) {
      return 'macro';
    } else if (searchType === SearchTypes.MODULE) {
      return 'module';
    } else {
      return '';
    }
  };

  const getSearchLangQuery = (language: string): string => {
    if (language === SearchMainLanguages.C_CPP) {
      return '(mime:("text/x-c") OR mime:("text/x-c++"))';
    } else if (language === SearchMainLanguages.JAVA) {
      return '(mime:("text/x-java") OR mime:("text/x-java-source"))';
    } else if (language === SearchMainLanguages.JAVASCRIPT) {
      return '(mime:("text/x-javascript") OR mime:("application/javascript"))';
    } else if (language === SearchMainLanguages.SHELLSCRIPT) {
      return '(mime:("text/x-shellscript") OR mime:("application/x-shellscript"))';
    } else if (language === SearchMainLanguages.PEARL) {
      return '(mime:("text/x-perl"))';
    } else if (language === SearchMainLanguages.PYTHON) {
      return '(mime:("text/x-python"))';
    } else if (language === 'Any') {
      return '';
    } else {
      return `(mime:("${language}"))`;
    }
  };

  const createQueryString = (queryString: string): string => {
    let modifiedQueryString: string = '';
    const textSearch = appCtx.searchCurrentOption?.name === SearchOptions.TEXT.toString();
    const definitionSearch = appCtx.searchCurrentOption?.name === SearchOptions.DEFINITION.toString();
    const allTypesSelected = searchTypes.every((t) => appCtx.selectedSearchTypes.includes(t));
    const anyLanguage = appCtx.searchLanguage === 'Any';

    if (definitionSearch) {
      modifiedQueryString = queryString === '' ? '' : `defs:(${queryString})`;
      if (!allTypesSelected) {
        modifiedQueryString += queryString === '' ? '' : ' AND ';
        modifiedQueryString += `(${getSearchTypeQuery(appCtx.selectedSearchTypes[0])}:(${queryString})`;
        if (appCtx.selectedSearchTypes.length > 1) {
          for (let i = 1; i < appCtx.selectedSearchTypes.length; ++i) {
            modifiedQueryString += ` OR ${getSearchTypeQuery(appCtx.selectedSearchTypes[i])}:(${queryString})`;
          }
        }
        modifiedQueryString += ')';
      }
      if (!anyLanguage) {
        modifiedQueryString +=
          queryString === '' && modifiedQueryString === ''
            ? `${getSearchLangQuery(appCtx.searchLanguage)}`
            : ` AND ${getSearchLangQuery(appCtx.searchLanguage)}`;
      }
    } else if (textSearch) {
      if (!anyLanguage) {
        modifiedQueryString =
          queryString === ''
            ? `${getSearchLangQuery(appCtx.searchLanguage)}`
            : `${queryString} AND ${getSearchLangQuery(appCtx.searchLanguage)}`;
      } else {
        modifiedQueryString = queryString;
      }
    } else {
      modifiedQueryString = queryString;
    }
    return modifiedQueryString;
  };

  return (
    <StyledHeader>
      <HeaderLogo />
      <HeaderContent>
        <SettingsContainer>
          <ProjectSelect />
          <TextField
            value={appCtx.searchQuery}
            onChange={(e) => appCtx.setSearchQuery(e.target.value)}
            onKeyDown={(e) => handleSearch(e)}
            placeholder={
              appCtx.searchCurrentOption?.name === SearchOptions.FILE_NAME
                ? 'File name regex'
                : appCtx.searchCurrentOption?.name === SearchOptions.LOG
                ? 'Arbitrary log message'
                : 'Search by expression'
            }
            InputProps={{
              startAdornment: (
                <InputAdornment position={'start'}>
                  <Search />
                </InputAdornment>
              ),
              endAdornment: (
                <InputAdornment position={'end'}>
                  <IconButton onClick={(e) => setSettingsAnchorEl(e.currentTarget)}>
                    <Settings />
                  </IconButton>
                  <Tooltip title={getTooltipText(SearchMethods.EXPRESSION)}>
                    <Info />
                  </Tooltip>
                </InputAdornment>
              ),
            }}
          />
          <TextField
            value={appCtx.searchFileFilterQuery}
            onChange={(e) => appCtx.setSearchFileFilterQuery(e.target.value)}
            onKeyDown={(e) => handleSearch(e)}
            placeholder={'File name filter regex'}
            InputProps={{
              startAdornment: (
                <InputAdornment position={'start'}>
                  <Search />
                </InputAdornment>
              ),
              endAdornment: (
                <InputAdornment position={'end'}>
                  <Tooltip title={getTooltipText(SearchMethods.FILE_REGEX)}>
                    <Info />
                  </Tooltip>
                </InputAdornment>
              ),
            }}
          />
          <TextField
            value={appCtx.searchDirFilterQuery}
            onChange={(e) => appCtx.setSearchDirFilterQuery(e.target.value)}
            onKeyDown={(e) => handleSearch(e)}
            placeholder={'Path filter regex'}
            InputProps={{
              startAdornment: (
                <InputAdornment position={'start'}>
                  <Search />
                </InputAdornment>
              ),
              endAdornment: (
                <InputAdornment position={'end'}>
                  <Tooltip title={getTooltipText(SearchMethods.PATH_REGEX)}>
                    <Info />
                  </Tooltip>
                </InputAdornment>
              ),
            }}
          />
          <SettingsMenu anchorEl={settingsAnchorEl} setAnchorEl={setSettingsAnchorEl} />
        </SettingsContainer>
        <IconButton onClick={() => setTheme(theme === 'dark' ? 'light' : 'dark')}>
          {theme === 'dark' ? <LightMode /> : <DarkMode />}
        </IconButton>
      </HeaderContent>
    </StyledHeader>
  );
};
