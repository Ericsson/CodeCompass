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
import { getSearchResults } from 'service/search-service';
import { SearchContext } from 'global-context/search-context';
import { FileSearchResult, SearchResult } from '@thrift-generated';
import { ConfigContext } from 'global-context/config-context';
import { AccordionLabel } from 'enums/accordion-enum';
import { removeStore } from 'utils/store';

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
  const configCtx = useContext(ConfigContext);
  const searchCtx = useContext(SearchContext);

  const searchTypes = enumToArray(SearchTypes);
  const [searchLanguage, setSearchLanguage] = useState<string>('Any');
  const [selectedTypes, setSelectedTypes] = useState<string[]>(searchTypes);
  const [settingsAnchorEl, setSettingsAnchorEl] = useState<null | HTMLElement>(null);

  const handleSearch = async (e: KeyboardEvent<HTMLDivElement>) => {
    if (e.key === 'Enter') {
      const query = createQueryString(searchCtx.searchQuery);
      if (query === '') return;
      const isFileSearch = searchCtx.searchCurrentOption?.name === SearchOptions.FILE_NAME.toString();
      const searchResults = (await getSearchResults(
        isFileSearch,
        searchCtx.searchCurrentOption?.id as number,
        query,
        searchCtx.searchStart,
        searchCtx.searchSize,
        searchCtx.searchFileFilterQuery,
        searchCtx.searchDirFilterQuery
      )) as SearchResult | FileSearchResult;
      searchCtx.setSearchResult(searchResults);
      searchCtx.setIsFileSearch(isFileSearch);
      configCtx.setActiveAccordion(AccordionLabel.SEARCH_RESULTS);
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
      return `(mime:(${language}))`;
    }
  };

  const createQueryString = (queryString: string): string => {
    let modifiedQueryString: string = '';
    const textSearch = searchCtx.searchCurrentOption?.name === SearchOptions.TEXT.toString();
    const definitionSearch = searchCtx.searchCurrentOption?.name === SearchOptions.DEFINITION.toString();
    const allTypesSelected = searchTypes.every((t) => selectedTypes.includes(t));
    const anyLanguage = searchLanguage === 'Any';

    if (definitionSearch) {
      modifiedQueryString = queryString === '' ? '' : `defs:(${queryString})`;
      if (!allTypesSelected) {
        modifiedQueryString += queryString === '' ? '' : ' AND ';
        modifiedQueryString += `(${getSearchTypeQuery(selectedTypes[0])}:(${queryString})`;
        if (selectedTypes.length > 1) {
          for (let i = 1; i < selectedTypes.length; ++i) {
            modifiedQueryString += ` OR ${getSearchTypeQuery(selectedTypes[i])}:(${queryString})`;
          }
        }
        modifiedQueryString += ')';
      }
      if (!anyLanguage) {
        modifiedQueryString +=
          queryString === '' && modifiedQueryString === ''
            ? `${getSearchLangQuery(searchLanguage)}`
            : ` AND ${getSearchLangQuery(searchLanguage)}`;
      }
    } else if (textSearch) {
      if (!anyLanguage) {
        modifiedQueryString =
          queryString === ''
            ? `${getSearchLangQuery(searchLanguage)}`
            : `${queryString} AND ${getSearchLangQuery(searchLanguage)}`;
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
            value={searchCtx.searchQuery}
            onChange={(e) => searchCtx.setSearchQuery(e.target.value)}
            onKeyDown={(e) => handleSearch(e)}
            placeholder={
              searchCtx.searchCurrentOption?.name === SearchOptions.FILE_NAME
                ? 'File name regex'
                : searchCtx.searchCurrentOption?.name === SearchOptions.LOG
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
            value={searchCtx.searchFileFilterQuery}
            onChange={(e) => searchCtx.setSearchFileFilterQuery(e.target.value)}
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
            value={searchCtx.searchDirFilterQuery}
            onChange={(e) => searchCtx.setSearchDirFilterQuery(e.target.value)}
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
          <SettingsMenu
            searchLanguage={searchLanguage}
            setSearchLanguage={setSearchLanguage}
            selectedTypes={selectedTypes}
            setSelectedTypes={setSelectedTypes}
            anchorEl={settingsAnchorEl}
            setAnchorEl={setSettingsAnchorEl}
          />
        </SettingsContainer>
        <IconButton onClick={() => setTheme(theme === 'dark' ? 'light' : 'dark')}>
          {theme === 'dark' ? <LightMode /> : <DarkMode />}
        </IconButton>
      </HeaderContent>
    </StyledHeader>
  );
};
