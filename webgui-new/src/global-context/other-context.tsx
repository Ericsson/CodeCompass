import { AccordionLabel } from 'enums/accordion-enum';
import { TabName } from 'enums/tab-enum';
import { createContext, useEffect, useState } from 'react';

type OtherContextType = {
  activeAccordion: string;
  setActiveAccordion: (_val: string) => void;
  activeTab: number;
  setActiveTab: (_val: number) => void;
};

export const OtherContext = createContext<OtherContextType>({
  activeAccordion: '',
  setActiveAccordion: (_val) => {},
  activeTab: 0,
  setActiveTab: (_val) => {},
});

export const OtherContextController = ({ children }: { children: JSX.Element | JSX.Element[] }): JSX.Element => {
  const [activeAccordion, setActiveAccordion] = useState<string>(AccordionLabel.FILE_MANAGER);
  const [activeTab, setActiveTab] = useState<number>(TabName.WELCOME);

  useEffect(() => {
    const init = async () => {
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

  const otherContext = {
    activeAccordion,
    setActiveAccordion,
    activeTab,
    setActiveTab,
  };

  return <OtherContext.Provider value={otherContext}>{children}</OtherContext.Provider>;
};
