import {
  Checkbox,
  FormControl,
  FormControlLabel,
  FormGroup,
  FormLabel,
  InputLabel,
  MenuItem,
  Radio,
  RadioGroup,
  Select,
  styled,
} from '@mui/material';
import Modal from '@mui/material/Modal';
import { Dispatch, SetStateAction, useEffect, useState } from 'react';
import {
  SearchMainLanguages,
  SearchMethods,
  SearchOptions,
  SearchOtherLanguages,
  SearchTypes,
} from '../../../enums/settings-enum';
import { enumToArray, removeFromArray } from '../../../utils/array-utils';
import InfoIcon from '@mui/icons-material/Info';

const ModalWindow = styled('div')(({ theme }) => ({
  position: 'absolute',
  top: '50%',
  left: '50%',
  transform: 'translate(-50%, -50%)',
  width: '80vw',
  height: '85vh',
  maxWidth: '600px',
  maxHeight: '700px',
  padding: '1.5rem',
  border: `2px solid ${theme.colors?.primary}`,
  borderRadius: '5px',
  overflowY: 'scroll',
}));

const SearchMethodContainer = styled('div')({
  margin: '20px 0',
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

export const SettingsModal = ({
  modalOpen,
  setModalOpen,
  searchOption,
  setSearchOption,
  searchMethod,
  setSearchMethod,
  searchLanguage,
  setSearchLanguage,
  selectedTypes,
  setSelectedTypes,
}: {
  modalOpen: boolean;
  setModalOpen: Dispatch<SetStateAction<boolean>>;
  searchOption: string;
  setSearchOption: Dispatch<SetStateAction<string>>;
  searchMethod: string;
  setSearchMethod: Dispatch<SetStateAction<string>>;
  searchLanguage: string;
  setSearchLanguage: Dispatch<SetStateAction<string>>;
  selectedTypes: string[];
  setSelectedTypes: Dispatch<SetStateAction<string[]>>;
}): JSX.Element => {
  const searchOptions = enumToArray(SearchOptions);
  const searchMethods = enumToArray(SearchMethods);
  const searchMainLanguages = enumToArray(SearchMainLanguages);
  const searchOtherLanguages = enumToArray(SearchOtherLanguages);
  const searchTypes = enumToArray(SearchTypes);

  const [searchOtherLanguage, setSearchOtherLanguage] = useState<string>(searchOtherLanguages[0]);
  const [isOtherLanguage, setIsOtherLanguage] = useState<boolean>(false);

  useEffect(() => {
    if (isOtherLanguage) {
      setSearchLanguage(searchOtherLanguage);
    }
  }, [isOtherLanguage, searchOtherLanguage, setSearchLanguage]);

  const Options = (): JSX.Element => {
    return (
      <div>
        <FormLabel>{'Search options'}</FormLabel>
        <RadioGroup value={searchOption ?? searchOptions[0]}>
          {searchOptions.map((elem, idx) => {
            return (
              <FormControlLabel
                key={idx}
                onClick={() => setSearchOption(elem)}
                value={elem}
                control={<Radio />}
                label={elem}
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
                key={idx}
                onClick={() => {
                  setSearchLanguage(elem);
                  setIsOtherLanguage(false);
                }}
                value={elem}
                control={<Radio />}
                label={elem}
              />
            );
          })}
          <OtherLanguagesContainer>
            <FormControlLabel
              onClick={() => {
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
                value={searchOtherLanguage}
                label='Other'
                onChange={(e) => setSearchOtherLanguage(e.target.value)}
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
              key={idx}
              control={
                <Checkbox
                  disabled={searchOption === 'Text search'}
                  onChange={(e) =>
                    setSelectedTypes(
                      e.currentTarget.checked ? [...selectedTypes, elem] : removeFromArray(selectedTypes, elem)
                    )
                  }
                  checked={selectedTypes.includes(elem)}
                />
              }
              label={elem}
            />
          );
        })}
        <FormControlLabel
          control={
            <Checkbox
              disabled={searchOption === 'Text search'}
              onChange={(e) => setSelectedTypes(e.currentTarget.checked ? searchTypes : [])}
              checked={searchTypes.every((t) => selectedTypes.includes(t))}
            />
          }
          label={'All'}
        />
      </FormGroup>
    );
  };

  return (
    <Modal open={modalOpen} onClose={() => setModalOpen(false)}>
      <ModalWindow>
        <h2>{'Search settings'}</h2>
        <FormControl>
          <SearchMethodContainer>
            <FormLabel>{'Search method'}</FormLabel>
            <RadioGroup value={searchMethod ?? searchMethods[0]}>
              {searchMethods.map((elem, idx) => {
                return (
                  <FormControlLabel
                    key={idx}
                    onClick={() => setSearchMethod(elem)}
                    value={elem}
                    control={<Radio />}
                    label={elem}
                  />
                );
              })}
            </RadioGroup>
          </SearchMethodContainer>
          {searchMethod === SearchMethods.EXPRESSION ? (
            <ExpressionSearchSettings>
              <Options />
              {searchOption === SearchOptions.TEXT || searchOption === SearchOptions.DEFINITION ? (
                <>
                  <Languages />
                  <Types />
                </>
              ) : (
                ''
              )}
            </ExpressionSearchSettings>
          ) : (
            ''
          )}
        </FormControl>
      </ModalWindow>
    </Modal>
  );
};
