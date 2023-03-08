import {
  Checkbox,
  FormControl,
  FormControlLabel,
  FormGroup,
  FormLabel,
  IconButton,
  InputLabel,
  Menu,
  MenuItem,
  Radio,
  RadioGroup,
  Select,
  Tooltip,
  styled,
} from '@mui/material';
import { Dispatch, SetStateAction, useContext, useEffect, useState } from 'react';
import { SearchMainLanguages, SearchOptions, SearchOtherLanguages, SearchTypes } from 'enums/search-enum';
import { enumToArray, removeFromArray } from 'utils/array-utils';
import { Info, Close } from '@mui/icons-material';
import { SearchContext } from 'global-context/search-context';

const Container = styled('div')(({ theme }) => ({
  padding: '10px',
  border: `1px solid ${theme.colors?.primary}`,
  color: theme.colors?.primary,
  backgroundColor: theme.backgroundColors?.primary,
  borderRadius: '5px',
}));

const StyledMenu = styled(Menu)({
  '& ul': {
    padding: '0',
  },
});

const RadioWithInfo = styled('div')({
  display: 'flex',
  alignItems: 'center',
});

const ExpressionSearchSettings = styled('div')({
  display: 'flex',
  justifyContent: 'space-between',
  gap: '2rem',
});

const OtherLanguagesContainer = styled('div')(({ theme }) => ({
  display: 'flex',
  alignItems: 'center',
  marginTop: '10px',
  paddingTop: '20px',
  borderTop: `1px solid ${theme.colors?.primary}`,
}));

export const SettingsMenu = ({
  searchLanguage,
  setSearchLanguage,
  selectedTypes,
  setSelectedTypes,
  anchorEl,
  setAnchorEl,
}: {
  searchLanguage: string;
  setSearchLanguage: Dispatch<SetStateAction<string>>;
  selectedTypes: string[];
  setSelectedTypes: Dispatch<SetStateAction<string[]>>;
  anchorEl: null | HTMLElement;
  setAnchorEl: Dispatch<SetStateAction<null | HTMLElement>>;
}): JSX.Element => {
  const searchCtx = useContext(SearchContext);

  const searchMainLanguages = enumToArray(SearchMainLanguages);
  const searchOtherLanguages = enumToArray(SearchOtherLanguages);
  const searchTypes = enumToArray(SearchTypes);

  const [searchOtherLanguage, setSearchOtherLanguage] = useState<string>(searchOtherLanguages[0]);
  const [isOtherLanguage, setIsOtherLanguage] = useState<boolean>(false);

  const [searchLanguagesDisabled, setSearchLanguagesDisabled] = useState<boolean>(
    searchCtx.searchCurrentOption?.name !== SearchOptions.TEXT.toString() ||
      searchCtx.searchCurrentOption?.name !== SearchOptions.DEFINITION.toString()
  );

  const [searchTypesDisabled, setSearchTypesDisabled] = useState<boolean>(
    searchCtx.searchCurrentOption?.name !== SearchOptions.DEFINITION.toString()
  );

  useEffect(() => {
    setSearchLanguagesDisabled(
      searchCtx.searchCurrentOption?.name !== SearchOptions.TEXT.toString() &&
        searchCtx.searchCurrentOption?.name !== SearchOptions.DEFINITION.toString()
    );
    setSearchTypesDisabled(searchCtx.searchCurrentOption?.name !== SearchOptions.DEFINITION.toString());
  }, [searchCtx.searchCurrentOption]);

  useEffect(() => {
    if (isOtherLanguage) {
      setSearchLanguage(searchOtherLanguage);
    }
  }, [isOtherLanguage, searchOtherLanguage, setSearchLanguage]);

  const Options = (): JSX.Element => {
    return (
      <div>
        <FormLabel>{'Search options'}</FormLabel>
        <RadioGroup value={searchCtx.searchCurrentOption?.name ?? searchCtx.searchOptions[0].name}>
          {searchCtx.searchOptions.map((elem, idx) => {
            return (
              <FormControlLabel
                key={idx}
                onClick={() => {
                  searchCtx.setSearchCurrentOption(elem);
                  localStorage.setItem('currentSearchOption', JSON.stringify(elem));
                }}
                value={elem.name}
                control={<Radio />}
                label={elem.name}
              />
            );
          })}
        </RadioGroup>
      </div>
    );
  };

  const Languages = (): JSX.Element => {
    return (
      <div>
        <FormLabel>{'Languages'}</FormLabel>
        <RadioGroup value={searchLanguage ?? searchMainLanguages[0]}>
          {searchMainLanguages.map((elem, idx) => {
            return (
              <FormControlLabel
                disabled={searchLanguagesDisabled}
                key={idx}
                onClick={() => {
                  if (searchLanguagesDisabled) return;
                  setSearchLanguage(elem);
                  setIsOtherLanguage(false);
                }}
                value={elem}
                control={<Radio />}
                label={elem}
              />
            );
          })}
          <RadioWithInfo>
            <FormControlLabel
              disabled={searchLanguagesDisabled}
              onClick={() => {
                if (searchLanguagesDisabled) return;
                setSearchLanguage('Any');
                setIsOtherLanguage(false);
              }}
              value={'Any'}
              control={<Radio />}
              label={'Any'}
            />
            <Tooltip
              title={
                <span>
                  In case of <i>Any</i>, search happens in every file, not only in listed ones.
                </span>
              }
            >
              <Info />
            </Tooltip>
          </RadioWithInfo>
          <OtherLanguagesContainer>
            <FormControlLabel
              disabled={searchLanguagesDisabled}
              onClick={() => {
                if (searchLanguagesDisabled) return;
                setSearchLanguage(searchOtherLanguage);
                setIsOtherLanguage(true);
              }}
              value={''}
              control={<Radio />}
              label={''}
              checked={isOtherLanguage}
            />
            <FormControl>
              <InputLabel>{'Other'}</InputLabel>
              <Select
                disabled={searchLanguagesDisabled}
                value={searchOtherLanguage}
                label="Other"
                onChange={(e) => (!searchLanguagesDisabled ? setSearchOtherLanguage(e.target.value) : '')}
              >
                {searchOtherLanguages.map((elem, idx) => {
                  return (
                    <MenuItem key={idx} value={elem}>
                      {elem}
                    </MenuItem>
                  );
                })}
              </Select>
            </FormControl>
          </OtherLanguagesContainer>
        </RadioGroup>
      </div>
    );
  };

  const Types = (): JSX.Element => {
    return (
      <FormGroup>
        <FormLabel>{'Types'}</FormLabel>
        {searchTypes.map((elem, idx) => {
          return (
            <FormControlLabel
              disabled={searchTypesDisabled}
              key={idx}
              control={
                <Checkbox
                  onChange={(e) =>
                    !searchTypesDisabled
                      ? setSelectedTypes(
                          e.currentTarget.checked ? [...selectedTypes, elem] : removeFromArray(selectedTypes, elem)
                        )
                      : ''
                  }
                  checked={selectedTypes.includes(elem)}
                />
              }
              label={elem}
            />
          );
        })}
        <FormControlLabel
          disabled={searchTypesDisabled}
          control={
            <Checkbox
              onChange={(e) =>
                !searchTypesDisabled ? setSelectedTypes(e.currentTarget.checked ? searchTypes : []) : ''
              }
              checked={searchTypes.every((t) => selectedTypes.includes(t))}
            />
          }
          label={'All'}
        />
      </FormGroup>
    );
  };

  return (
    <StyledMenu anchorEl={anchorEl} open={Boolean(anchorEl)} onClose={() => setAnchorEl(null)}>
      <Container>
        <FormControl>
          <ExpressionSearchSettings>
            <Options />
            <Languages />
            <Types />
          </ExpressionSearchSettings>
        </FormControl>
        <IconButton onClick={() => setAnchorEl(null)}>
          <Close />
        </IconButton>
      </Container>
    </StyledMenu>
  );
};
