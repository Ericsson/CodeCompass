import { IconButton, InputAdornment, TextField, Tooltip } from '@mui/material';
import { ProjectSelect } from 'components/project-select/project-select';
import { Search, Settings, LightMode, DarkMode, Info } from '@mui/icons-material';
import React, { KeyboardEvent, useContext, useEffect, useState } from 'react';
import { SearchOptions, SearchMethods, SearchMainLanguages, SearchTypeOptions } from 'enums/search-enum';
import { enumToArray } from 'utils/utils';
import { ThemeContext } from 'global-context/theme-context';
import { SettingsMenu } from 'components/settings-menu/settings-menu';
import { getTooltipText } from './get-tooltip-text';
import { getSearchTypes } from 'service/search-service';
import { SearchType } from '@thrift-generated';
import { AccordionLabel } from 'enums/accordion-enum';
import { getStore, removeStore, setStore } from 'utils/store';
import { AppContext } from 'global-context/app-context';
import * as SC from './styled-components';

export const Header = () => {
  const appCtx = useContext(AppContext);
  const { theme, setTheme } = useContext(ThemeContext);

  const searchTypeOptions = enumToArray(SearchTypeOptions);
  const [settingsAnchorEl, setSettingsAnchorEl] = useState<null | HTMLElement>(null);
  const [searchLanguage, setSearchLanguage] = useState<string | undefined>(undefined);
  const [searchType, setSearchType] = useState<SearchType | undefined>(undefined);
  const [searchTypes, setSearchTypes] = useState<SearchType[]>([]);
  const [selectedSearchTypeOptions, setSelectedSearchTypeOptions] = useState<string[] | undefined>(undefined);
  const [searchQuery, setSearchQuery] = useState<string | undefined>(undefined);
  const [searchFileFilterQuery, setSearchFileFilterQuery] = useState<string | undefined>(undefined);
  const [searchDirFilterQuery, setSearchDirFilterQuery] = useState<string | undefined>(undefined);

  useEffect(() => {
    const init = async () => {
      const initSearchTypes = await getSearchTypes();
      const initSearchTypeOptions = enumToArray(SearchTypeOptions);
      const initSearchLanguage = enumToArray(SearchMainLanguages)[0];

      const { storedSearchLanguage, storedSearchType, storedSelectedSearchTypeOptions, storedSearchProps } = getStore();
      setSearchLanguage(storedSearchLanguage ?? initSearchLanguage);
      setSearchType(storedSearchType ?? initSearchTypes[0]);
      setSearchTypes(initSearchTypes);
      setSelectedSearchTypeOptions(storedSelectedSearchTypeOptions ?? initSearchTypeOptions);

      if (storedSearchProps) {
        setSearchQuery(storedSearchProps.initialQuery);
        setSearchFileFilterQuery(storedSearchProps.fileFilter);
        setSearchDirFilterQuery(storedSearchProps.dirFilter);
      }
    };
    init();
  }, []);

  useEffect(() => {
    setStore({
      storedSearchType: searchType,
      storedSearchLanguage: searchLanguage,
      storedSelectedSearchTypeOptions: selectedSearchTypeOptions,
    });
  }, [searchType, searchLanguage, selectedSearchTypeOptions]);

  const handleSearch = async (e: KeyboardEvent<HTMLDivElement>) => {
    if (e.key === 'Enter') {
      if (!searchQuery) return;

      const query = createQueryString(searchQuery);
      if (query === '') return;

      const fileSearch = searchType?.name === SearchOptions.FILE_NAME.toString();
      const initSearchProps = {
        initialQuery: searchQuery,
        fileSearch: fileSearch,
        type: searchType?.id as number,
        query: query,
        fileFilter: searchFileFilterQuery ?? '',
        dirFilter: searchDirFilterQuery ?? '',
        start: 0,
        size: 10,
      };

      appCtx.setSearchProps(initSearchProps);
      appCtx.setActiveAccordion(AccordionLabel.SEARCH_RESULTS);
      removeStore(['storedExpandedSearchFileNodes', 'storedExpandedSearchPathNodes']);
    } else {
      return;
    }
  };

  const getSearchTypeOptionQuery = (searchType: string): string => {
    if (searchType === SearchTypeOptions.TYPE) {
      return 'type';
    } else if (searchType === SearchTypeOptions.FUNCTION) {
      return 'func';
    } else if (searchType === SearchTypeOptions.CONSTANT) {
      return 'const';
    } else if (searchType === SearchTypeOptions.VARIABLE) {
      return 'var';
    } else if (searchType === SearchTypeOptions.FIELD) {
      return 'field';
    } else if (searchType === SearchTypeOptions.LABEL) {
      return 'label';
    } else if (searchType === SearchTypeOptions.MACRO) {
      return 'macro';
    } else if (searchType === SearchTypeOptions.MODULE) {
      return 'module';
    } else {
      return '';
    }
  };

  const getSearchLanguageQuery = (language: string): string => {
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
    if (!searchLanguage || !selectedSearchTypeOptions) return '';

    let modifiedQueryString: string = '';
    const textSearch = searchType?.name === SearchOptions.TEXT.toString();
    const definitionSearch = searchType?.name === SearchOptions.DEFINITION.toString();
    const allTypesSelected = searchTypeOptions.every((t) => selectedSearchTypeOptions.includes(t));
    const anyLanguage = searchLanguage === 'Any';

    if (definitionSearch) {
      modifiedQueryString = queryString === '' ? '' : `defs:(${queryString})`;
      if (!allTypesSelected) {
        modifiedQueryString += queryString === '' ? '' : ' AND ';
        modifiedQueryString += `(${getSearchTypeOptionQuery(selectedSearchTypeOptions[0])}:(${queryString})`;
        if (selectedSearchTypeOptions.length > 1) {
          for (let i = 1; i < selectedSearchTypeOptions.length; ++i) {
            modifiedQueryString += ` OR ${getSearchTypeOptionQuery(selectedSearchTypeOptions[i])}:(${queryString})`;
          }
        }
        modifiedQueryString += ')';
      }
      if (!anyLanguage) {
        modifiedQueryString +=
          queryString === '' && modifiedQueryString === ''
            ? `${getSearchLanguageQuery(searchLanguage)}`
            : ` AND ${getSearchLanguageQuery(searchLanguage)}`;
      }
    } else if (textSearch) {
      if (!anyLanguage) {
        modifiedQueryString =
          queryString === ''
            ? `${getSearchLanguageQuery(searchLanguage)}`
            : `${queryString} AND ${getSearchLanguageQuery(searchLanguage)}`;
      } else {
        modifiedQueryString = queryString;
      }
    } else {
      modifiedQueryString = queryString;
    }
    return modifiedQueryString;
  };

  return (
    <SC.StyledHeader>
      <SC.HeaderLogo />
      <SC.HeaderContent>
        <SC.SettingsContainer>
          <ProjectSelect />
          <TextField
            value={searchQuery ?? ''}
            onChange={(e) => setSearchQuery(e.target.value)}
            onKeyDown={(e) => handleSearch(e)}
            placeholder={
              searchType?.name === SearchOptions.FILE_NAME
                ? 'File name regex'
                : searchType?.name === SearchOptions.LOG
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
            value={searchFileFilterQuery ?? ''}
            onChange={(e) => setSearchFileFilterQuery(e.target.value)}
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
            value={searchDirFilterQuery ?? ''}
            onChange={(e) => setSearchDirFilterQuery(e.target.value)}
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
            anchorEl={settingsAnchorEl}
            setAnchorEl={setSettingsAnchorEl}
            searchTypes={searchTypes}
            searchType={searchType as SearchType}
            setSearchType={setSearchType}
            searchLanguage={searchLanguage as string}
            setSearchLanguage={setSearchLanguage}
            selectedSearchTypeOptions={selectedSearchTypeOptions as string[]}
            setSelectedSearchTypeOptions={setSelectedSearchTypeOptions}
          />
        </SC.SettingsContainer>
        <IconButton onClick={() => setTheme(theme === 'dark' ? 'light' : 'dark')}>
          {theme === 'dark' ? <LightMode /> : <DarkMode />}
        </IconButton>
      </SC.HeaderContent>
    </SC.StyledHeader>
  );
};
