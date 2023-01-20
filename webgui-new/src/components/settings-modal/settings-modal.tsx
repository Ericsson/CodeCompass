import {
  Checkbox,
  FormControl,
  FormControlLabel,
  FormGroup,
  FormLabel,
  IconButton,
  InputLabel,
  MenuItem,
  Radio,
  RadioGroup,
  Select,
  Tooltip,
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
} from '../../enums/settings-enum';
import { enumToArray, removeFromArray } from '../../utils/array-utils';
import { Info, Close } from '@mui/icons-material';

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
  backgroundColor: theme.backgroundColors?.primary,
  borderRadius: '5px',
  overflowY: 'scroll',
}));

const Title = styled('div')({
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
});

const SearchMethodContainer = styled('div')({
  margin: '20px 0',
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

  const [searchOptionsDisabled, setSearchOptionsDisabled] = useState<boolean>(
    searchMethod !== SearchMethods.EXPRESSION.toString()
  );

  const [searchLanguagesDisabled, setSearchLanguagesDisabled] = useState<boolean>(
    searchMethod !== SearchMethods.EXPRESSION.toString() &&
      (searchOption !== SearchOptions.TEXT.toString() || searchOption !== SearchOptions.DEFINITION.toString())
  );

  const [searchTypesDisabled, setSearchTypesDisabled] = useState<boolean>(
    searchMethod !== SearchMethods.EXPRESSION.toString() || searchOption !== SearchOptions.DEFINITION.toString()
  );

  useEffect(() => {
    setSearchOptionsDisabled(searchMethod !== SearchMethods.EXPRESSION.toString());
    setSearchLanguagesDisabled(
      searchMethod !== SearchMethods.EXPRESSION.toString() ||
        (searchOption !== SearchOptions.TEXT.toString() && searchOption !== SearchOptions.DEFINITION.toString())
    );
    setSearchTypesDisabled(
      searchMethod !== SearchMethods.EXPRESSION.toString() || searchOption !== SearchOptions.DEFINITION.toString()
    );
  }, [searchMethod, searchOption]);

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
                disabled={searchOptionsDisabled}
                key={idx}
                onClick={() => (!searchOptionsDisabled ? setSearchOption(elem) : '')}
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
            <Tooltip title={getTooltipText('Any')}>
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
                label='Other'
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

  const getTooltipText = (value: string): string | JSX.Element => {
    switch (value) {
      case SearchMethods.FILE_REGEX:
        return (
          <>
            This filter is a regular expression of file names.
            <br />
            Example: .*cpp
          </>
        );
      case SearchMethods.PATH_REGEX:
        return (
          <>
            This filter is a regular expression of directory paths.
            <br />
            Example: /path/to/source/subdir/.*
          </>
        );
      case SearchMethods.EXPRESSION:
        return (
          <>
            <div>
              For <b>&quot;Text search&quot;</b> and <b>&quot;Definition search&quot;</b> you can use wildcards, regex,
              and logical operators:
            </div>
            <ul>
              <li>
                <b>Phrases:</b> a Phrase is a group of words surrounded by double quotes such as &quot;hello
                dolly&quot;. A Phrase query matches if the words are next to each other in the particular order.
              </li>
              <li>
                <b>Wildcards:</b> single and multiple character wildcard searches within single terms.
                <br />
                Examples: *test te?t test* te*t
                <br />
                <b>Note:</b> you cannot use wildcard searches within phrase queries!
              </li>
              <li>
                <b>Regular Expressions:</b> matching a pattern between forward slashes.
                <br />
                Example: /[mb]oat/
              </li>
              <li>
                <b>Boolean Operators:</b> Boolean operators allow terms to be combined through logic operators.
                <br />
                We support AND, &quot;+&quot;, OR, NOT and &quot;-&quot; as Boolean operators. (Note: Boolean operators
                must be ALL CAPS)
                <ul>
                  <li>
                    <b>OR:</b> The OR operator links two terms and finds a matching document if either of the terms
                    exist in a document. If there is no Boolean operator between two terms, the OR operator is used.
                    <br />
                    Example: &quot;some text&quot; OR other
                  </li>
                  <li>
                    <b>AND:</b> The AND operator matches documents where both terms exist anywhere in the text of a
                    single document.
                    <br />
                    Example: &quot;some text&quot; AND other
                  </li>
                </ul>
              </li>
              <li>
                <b>Escaping Special Characters:</b> The current list special characters are: + - && || ! ( ) {} [ ] ^
                &quot; ~ * ? : \ /
                <br />
                <b>The escape character is the &quot;\&quot; (back slash).</b>
              </li>
            </ul>
            <div>
              In <b>&quot;File name search&quot;</b> you can use a regular expression for matching on full (absolute)
              path of files in the database.
              <br />
              Example: /path/.*/te?t\.cpp
            </div>
            <div>
              For the documentation of <b>&quot;Log search&quot;</b> please see the user manual.
            </div>
          </>
        );
      case 'Any':
        return (
          <>
            In case of <i>Any</i>, search happens in every file, not only in listed ones.
          </>
        );
      default:
        return <></>;
    }
  };

  return (
    <Modal open={modalOpen} onClose={() => setModalOpen(false)}>
      <ModalWindow>
        <Title>
          <h2>{'Search settings'}</h2>
          <IconButton onClick={() => setModalOpen(false)}>
            <Close />
          </IconButton>
        </Title>
        <FormControl>
          <SearchMethodContainer>
            <FormLabel>{'Search method'}</FormLabel>
            <RadioGroup value={searchMethod ?? searchMethods[0]}>
              {searchMethods.map((elem, idx) => {
                return (
                  <RadioWithInfo key={idx}>
                    <FormControlLabel
                      onClick={() => setSearchMethod(elem)}
                      value={elem}
                      control={<Radio />}
                      label={elem}
                    />
                    <Tooltip title={getTooltipText(elem)}>
                      <Info />
                    </Tooltip>
                  </RadioWithInfo>
                );
              })}
            </RadioGroup>
          </SearchMethodContainer>
          <ExpressionSearchSettings>
            <Options />
            <Languages />
            <Types />
          </ExpressionSearchSettings>
        </FormControl>
      </ModalWindow>
    </Modal>
  );
};
