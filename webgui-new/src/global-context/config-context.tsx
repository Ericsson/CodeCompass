import { AccordionLabel } from 'enums/accordion-enum';
import { TabName } from 'enums/tab-enum';
import { createContext, useEffect, useState } from 'react';
import { createConfig } from 'service/config';

type ConfigContextType = {
  activeAccordion: string;
  setActiveAccordion: (_val: string) => void;
  activeTab: number;
  setActiveTab: (_val: number) => void;
};

export const ConfigContext = createContext<ConfigContextType>({
  activeAccordion: '',
  setActiveAccordion: (_val) => {},
  activeTab: 0,
  setActiveTab: (_val) => {},
});

export const ConfigContextController = ({ children }: { children: JSX.Element | JSX.Element[] }): JSX.Element => {
  const [activeAccordion, setActiveAccordion] = useState<string>(AccordionLabel.FILE_MANAGER);
  const [activeTab, setActiveTab] = useState<number>(TabName.WELCOME);

  useEffect(() => {
    const init = async () => {
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

      const storedActiveAccordion = localStorage.getItem('activeAccordion');
      if (storedActiveAccordion) {
        setActiveAccordion(storedActiveAccordion);
      }

      const storedActiveTab = localStorage.getItem('activeTab');
      if (storedActiveTab) {
        setActiveTab(JSON.parse(storedActiveTab));
      }
    };
    init();
  }, []);

  const configContext = {
    activeAccordion,
    setActiveAccordion,
    activeTab,
    setActiveTab,
  };

  return <ConfigContext.Provider value={configContext}>{children}</ConfigContext.Provider>;
};
