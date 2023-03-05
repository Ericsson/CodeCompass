import { createContext, useContext, useEffect, useState } from 'react';
import { createSearchClient, getSearchTypes } from 'service/search-service';
import { ProjectContext } from './project-context';

type SearchContextType = {
  searchOption: number;
  setSearchOption: (_val: number) => void;
};

export const SearchContext = createContext<SearchContextType>({
  searchOption: 1,
  setSearchOption: (_val) => {},
});

export const SearchContextController = ({ children }: { children: JSX.Element | JSX.Element[] }): JSX.Element => {
  const projectCtx = useContext(ProjectContext);
  const [searchOption, setSearchOption] = useState<number>(1);

  useEffect(() => {
    if (!projectCtx.currentWorkspace) {
      return;
    }
    const init = async () => {
      createSearchClient(projectCtx.currentWorkspace);
      const searchTypes = await getSearchTypes();
      setSearchOption(searchTypes[0].id as number);
    };
    init();
  }, [projectCtx.currentWorkspace]);

  const searchContext = {
    searchOption,
    setSearchOption,
  };

  return <SearchContext.Provider value={searchContext}>{children}</SearchContext.Provider>;
};
