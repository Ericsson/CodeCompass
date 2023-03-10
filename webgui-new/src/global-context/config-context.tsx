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
      createConfig({
        webserver_host: window.location.hostname,
        webserver_port:
          window.location.protocol === 'https:' && !window.location.port ? 443 : parseInt(window.location.port),
        webserver_https: window.location.protocol === 'https:',
        webserver_path: '',
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
