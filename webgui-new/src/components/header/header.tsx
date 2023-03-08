import { IconButton, InputAdornment, TextField, Tooltip, styled } from '@mui/material';
import { ProjectSelect } from 'components/project-select/project-select';
import { Search, Settings, LightMode, DarkMode, Info } from '@mui/icons-material';
import { KeyboardEvent, useContext, useState } from 'react';
import { SearchOptions, SearchMethods, SearchMainLanguages, SearchTypes } from 'enums/search-enum';
import { enumToArray } from 'utils/array-utils';
import { ThemeContext } from 'global-context/theme-context';
import Logo from '../../../public/logo.png';
import { SettingsMenu } from 'components/settings-menu/settings-menu';
import { getTooltipText } from './get-tooltip-text';
import { getSearchResults } from 'service/search-service';
import { SearchContext } from 'global-context/search-context';
import { FileSearchResult, SearchResult } from '@thrift-generated';

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
  const searchCtx = useContext(SearchContext);

  const searchMainLanguages = enumToArray(SearchMainLanguages);
  const searchTypes = enumToArray(SearchTypes);

  const [searchLanguage, setSearchLanguage] = useState<string>(searchMainLanguages[0]);
  const [selectedTypes, setSelectedTypes] = useState<string[]>(searchTypes);
  const [settingsAnchorEl, setSettingsAnchorEl] = useState<null | HTMLElement>(null);

  const handleSearch = async (e: KeyboardEvent<HTMLDivElement>) => {
    if (e.key === 'Enter') {
      const query = createQueryString(searchCtx.searchQuery);
      const searchResults = (await getSearchResults(
        searchCtx.searchCurrentOption?.name === SearchOptions.FILE_NAME.toString(),
        searchCtx.searchCurrentOption?.id as number,
        query,
        searchCtx.searchStart,
        searchCtx.searchSize
      )) as SearchResult | FileSearchResult;
      searchCtx.setSearchPage(0);
      searchCtx.setSearchSize(10);
      searchCtx.setSearchResult(searchResults);
      searchCtx.setIsFileSearch(searchCtx.searchCurrentOption?.name === SearchOptions.FILE_NAME.toString());
      localStorage.setItem(
        'isFileSearch',
        JSON.stringify(searchCtx.searchCurrentOption?.name === SearchOptions.FILE_NAME.toString())
      );
      localStorage.setItem('searchResults', JSON.stringify(searchResults));
      localStorage.setItem('currentSearchPage', JSON.stringify(0));
      localStorage.setItem('currentSearchSize', JSON.stringify(10));
      localStorage.setItem('currentSearchQuery', searchCtx.searchQuery);
      localStorage.removeItem('expandedPathNodes');
      localStorage.removeItem('expandedFileNodes');
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

  const createQueryString = (queryString: string): string => {
    let modifiedQueryString: string = '';
    const allTypesSelected = searchTypes.every((t) => selectedTypes.includes(t));

    if (searchCtx.searchCurrentOption?.name === SearchOptions.DEFINITION.toString()) {
      modifiedQueryString = `defs:(${queryString})`;

      if (!allTypesSelected) {
        modifiedQueryString += ` AND (${getSearchTypeQuery(selectedTypes[0])}:(${queryString})`;

        if (selectedTypes.length > 1) {
          for (let i = 1; i < selectedTypes.length; ++i) {
            modifiedQueryString += ` OR ${getSearchTypeQuery(selectedTypes[i])}:(${queryString})`;
          }
        }

        modifiedQueryString += ')';
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
        <IconButton
          onClick={() => {
            localStorage.setItem('theme', theme === 'dark' ? 'light' : 'dark');
            setTheme(theme === 'dark' ? 'light' : 'dark');
          }}
        >
          {theme === 'dark' ? <LightMode /> : <DarkMode />}
        </IconButton>
      </HeaderContent>
    </StyledHeader>
  );
};
