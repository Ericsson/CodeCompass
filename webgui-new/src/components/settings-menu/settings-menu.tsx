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
import { enumToArray, removeFromArray } from 'utils/utils';
import { Info, Close } from '@mui/icons-material';
import { SearchContext } from 'global-context/search-context';
import { getStore, setStore } from 'utils/store';
import { AppContext } from 'global-context/app-context';

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
  anchorEl,
  setAnchorEl,
}: {
  anchorEl: null | HTMLElement;
  setAnchorEl: Dispatch<SetStateAction<null | HTMLElement>>;
}): JSX.Element => {
  const appCtx = useContext(AppContext);

  const searchMainLanguages = enumToArray(SearchMainLanguages);
  const searchOtherLanguages = enumToArray(SearchOtherLanguages) as string[];
  const searchTypes = enumToArray(SearchTypes);

  const [searchOtherLanguage, setSearchOtherLanguage] = useState<string | undefined>(undefined);

  const [searchLanguagesDisabled, setSearchLanguagesDisabled] = useState<boolean>(
    appCtx.searchCurrentOption?.name !== SearchOptions.TEXT.toString() ||
      appCtx.searchCurrentOption?.name !== SearchOptions.DEFINITION.toString()
  );

  const [searchTypesDisabled, setSearchTypesDisabled] = useState<boolean>(
    appCtx.searchCurrentOption?.name !== SearchOptions.DEFINITION.toString()
  );

  useEffect(() => {
    const { storedSearchOtherLanguage } = getStore();
    setSearchOtherLanguage(storedSearchOtherLanguage ?? 'text/x-asm');
  }, []);

  useEffect(() => {
    setSearchLanguagesDisabled(
      appCtx.searchCurrentOption?.name !== SearchOptions.TEXT.toString() &&
        appCtx.searchCurrentOption?.name !== SearchOptions.DEFINITION.toString()
    );
    setSearchTypesDisabled(appCtx.searchCurrentOption?.name !== SearchOptions.DEFINITION.toString());
  }, [appCtx.searchCurrentOption]);

  useEffect(() => {
    setStore({
      storedSearchOtherLanguage: searchOtherLanguage,
    });
  }, [searchOtherLanguage]);

  const Options = (): JSX.Element => {
    return (
      <div>
        <FormLabel>{'Search options'}</FormLabel>
        <RadioGroup value={appCtx.searchCurrentOption?.name ?? appCtx.searchOptions[0].name}>
          {appCtx.searchOptions.map((elem, idx) => {
            return (
              <FormControlLabel
                key={idx}
                onClick={() => appCtx.setSearchCurrentOption(elem)}
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
        <RadioGroup value={appCtx.searchLanguage ?? searchMainLanguages[0]}>
          {searchMainLanguages.map((elem, idx) => {
            return (
              <FormControlLabel
                disabled={searchLanguagesDisabled}
                key={idx}
                onClick={() => {
                  if (searchLanguagesDisabled) return;
                  appCtx.setSearchLanguage(elem);
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
                if (!searchLanguagesDisabled) {
                  appCtx.setSearchLanguage('Any');
                }
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
                if (!searchLanguagesDisabled) {
                  appCtx.setSearchLanguage(searchOtherLanguage as string);
                }
              }}
              value={''}
              control={<Radio />}
              label={''}
              checked={searchOtherLanguages.includes(appCtx.searchLanguage)}
            />
            <FormControl>
              <InputLabel>{'Other'}</InputLabel>
              <Select
                disabled={searchLanguagesDisabled}
                value={searchOtherLanguage}
                label="Other"
                onChange={(e) => {
                  if (!searchLanguagesDisabled) {
                    setSearchOtherLanguage(e.target.value);

                    if (searchOtherLanguages.includes(appCtx.searchLanguage)) {
                      appCtx.setSearchLanguage(e.target.value);
                    }
                  }
                }}
                MenuProps={{
                  PaperProps: {
                    style: {
                      maxHeight: 200,
                      width: 200,
                      marginTop: '5px',
                      border: '1px solid white',
                    },
                  },
                }}
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
                      ? appCtx.setSelectedSearchTypes(
                          e.currentTarget.checked
                            ? [...appCtx.selectedSearchTypes, elem]
                            : removeFromArray(appCtx.selectedSearchTypes, elem)
                        )
                      : ''
                  }
                  checked={appCtx.selectedSearchTypes.includes(elem)}
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
                !searchTypesDisabled ? appCtx.setSelectedSearchTypes(e.currentTarget.checked ? searchTypes : []) : ''
              }
              checked={searchTypes.every((t) => appCtx.selectedSearchTypes.includes(t))}
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
