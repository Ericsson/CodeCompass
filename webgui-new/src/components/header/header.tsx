import { IconButton, InputAdornment, TextField, Tooltip, styled } from '@mui/material';
import { ProjectSelect } from 'components/project-select/project-select';
import { Search, Settings, LightMode, DarkMode, Info } from '@mui/icons-material';
import { KeyboardEvent, useContext, useState } from 'react';
import { SearchOptions, SearchMethods, SearchMainLanguages, SearchTypes } from 'enums/settings-enum';
import { enumToArray } from 'utils/array-utils';
import { ThemeContext } from 'global-context/theme-context';
import Logo from '../../../public/logo.png';
import { SettingsMenu } from 'components/settings-menu/settings-menu';
import { getTooltipText } from './get-tooltip-text';
import { getSearchResults } from 'service/search-service';
import { SearchContext } from 'global-context/search-context';
import { SearchResult } from '@thrift-generated';

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

  const searchOptions = enumToArray(SearchOptions);
  const searchMainLanguages = enumToArray(SearchMainLanguages);
  const searchTypes = enumToArray(SearchTypes);

  const [searchOption, setSearchOption] = useState<string>(searchOptions[0]);
  const [searchLanguage, setSearchLanguage] = useState<string>(searchMainLanguages[0]);
  const [selectedTypes, setSelectedTypes] = useState<string[]>(searchTypes);
  const [settingsAnchorEl, setSettingsAnchorEl] = useState<null | HTMLElement>(null);
  const [query, setQuery] = useState<string>('');

  const handleSearch = async (e: KeyboardEvent<HTMLDivElement>) => {
    if (e.key === 'Enter' && query) {
      searchCtx.setSearchStart(5);
      searchCtx.setSearchQuery(query);
      const searchResults = (await getSearchResults(
        searchCtx.searchOption,
        query,
        5,
        searchCtx.searchSize
      )) as SearchResult;
      console.log(searchResults);
      searchCtx.setSearchResult(searchResults);
      localStorage.setItem('searchResults', JSON.stringify(searchResults as SearchResult));
      localStorage.removeItem('expandedPathNodes');
      localStorage.removeItem('expandedFileNodes');
    } else {
      return;
    }
  };

  return (
    <StyledHeader>
      <HeaderLogo />
      <HeaderContent>
        <SettingsContainer>
          <ProjectSelect />
          <TextField
            onChange={(e) => setQuery(e.target.value)}
            onKeyDown={(e) => handleSearch(e)}
            placeholder={
              searchOption === SearchOptions.FILE_NAME
                ? 'File name regex'
                : searchOption === SearchOptions.LOG
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
            searchOption={searchOption}
            setSearchOption={setSearchOption}
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
