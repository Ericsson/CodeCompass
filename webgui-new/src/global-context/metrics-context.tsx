import { FileInfo, MetricsTypeName } from '@thrift-generated';
import { createContext, useContext, useEffect, useState } from 'react';
import { ProjectContext } from './project-context';
import { createMetricsClient, getMetricsTypeNames } from 'service/metrics-service';

type MetricsContextType = {
  metricsTypeNames: MetricsTypeName[];
  setMetricsTypeNames: (_val: MetricsTypeName[]) => void;
  metricsFileInfo: FileInfo | undefined;
  setMetricsFileInfo: (_val: FileInfo | undefined) => void;
};

export const MetricsContext = createContext<MetricsContextType>({
  metricsTypeNames: [],
  setMetricsTypeNames: (_val) => {},
  metricsFileInfo: undefined,
  setMetricsFileInfo: (_val) => {},
});

export const MetricsContextController = ({ children }: { children: JSX.Element | JSX.Element[] }): JSX.Element => {
  const projectCtx = useContext(ProjectContext);

  const [metricsTypeNames, setMetricsTypeNames] = useState<MetricsTypeName[]>([]);
  const [metricsFileInfo, setMetricsFileInfo] = useState<FileInfo | undefined>(undefined);

  useEffect(() => {
    if (!projectCtx.currentWorkspace) {
      return;
    }
    const init = async () => {
      createMetricsClient(projectCtx.currentWorkspace);

      const metricsTypeNamesData = await getMetricsTypeNames();
      setMetricsTypeNames(metricsTypeNamesData);

      setMetricsFileInfo(undefined);
    };
    init();
  }, [projectCtx.currentWorkspace]);

  const metricsContext = {
    metricsTypeNames,
    setMetricsTypeNames,
    metricsFileInfo,
    setMetricsFileInfo,
  };

  return <MetricsContext.Provider value={metricsContext}>{children}</MetricsContext.Provider>;
};
