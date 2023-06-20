import { useState, useEffect, Dispatch, SetStateAction } from 'react';
import { useRouter } from 'next/router';
import { RouterQueryType } from 'utils/types';

export const useUrlState = (
  key: keyof RouterQueryType,
  initialValue: string
): [string, Dispatch<SetStateAction<string>>] => {
  const router = useRouter();
  const urlParam = router.query[key] as string | undefined;

  const [state, setState] = useState<string>(() => {
    return urlParam ?? initialValue;
  });

  useEffect(() => {
    setState(urlParam ?? initialValue);
  }, [urlParam, initialValue]);

  return [state, setState];
};
