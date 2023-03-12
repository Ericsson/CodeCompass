import { AccordionLabel } from 'enums/accordion-enum';
import { TabName } from 'enums/tab-enum';
import { createContext, useEffect, useState } from 'react';
import { createConfig } from 'service/config';
import { getStore, setStore } from 'utils/store';

type ConfigContextType = {
  activeAccordion: string;
  setActiveAccordion: (_val: string) => void;
  activeTab: number;
  setActiveTab: (_val: number) => void;
  treeViewOption: boolean;
  setTreeViewOption: (_val: boolean) => void;
};

export const ConfigContext = createContext<ConfigContextType>({
  activeAccordion: '',
  setActiveAccordion: (_val) => {},
  activeTab: 0,
  setActiveTab: (_val) => {},
  treeViewOption: false,
  setTreeViewOption: (_val) => {},
});

export const ConfigContextController = ({ children }: { children: JSX.Element | JSX.Element[] }): JSX.Element => {
  const [activeAccordion, setActiveAccordion] = useState<string | undefined>(undefined);
  const [activeTab, setActiveTab] = useState<number | undefined>(undefined);
  const [treeViewOption, setTreeViewOption] = useState<boolean | undefined>(undefined);

  useEffect(() => {
    const wHost = window.location.hostname;
    const wPort = window.location.port;
    const wHTTPS = window.location.protocol === 'https:';
    const wPath = window.location.pathname;

    createConfig({
      webserver_host: wHost,
      webserver_port: wHTTPS && !wPort ? 443 : parseInt(wPort),
      webserver_https: wHTTPS,
      webserver_path: wPath === '/new' ? '' : wPath.slice(0, wPath.lastIndexOf('/new')),
    });

    const { storedActiveAccordion, storedActiveTab, storedTreeViewOption } = getStore();
    setActiveAccordion(storedActiveAccordion ?? AccordionLabel.FILE_MANAGER);
    setActiveTab(storedActiveTab ?? TabName.WELCOME);
    setTreeViewOption(storedTreeViewOption ?? false);
  }, []);

  useEffect(() => {
    setStore({
      storedActiveAccordion: activeAccordion,
      storedActiveTab: activeTab,
      storedTreeViewOption: treeViewOption,
    });
  }, [activeAccordion, activeTab, treeViewOption]);

  const configContext = {
    activeAccordion: activeAccordion as string,
    activeTab: activeTab as number,
    treeViewOption: treeViewOption as boolean,
    setActiveAccordion,
    setActiveTab,
    setTreeViewOption,
  };

  return <ConfigContext.Provider value={configContext}>{children}</ConfigContext.Provider>;
};
