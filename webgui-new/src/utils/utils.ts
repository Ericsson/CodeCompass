import { RouterQueryType } from './types';

export const enumToArray = <T>(enumToConvert: { [s: string]: T }): T[] => {
  return Array.from(Object.values(enumToConvert));
};

export const removeFromArray = <T>(array: T[], elem: T): T[] => {
  return [
    ...array.splice(
      0,
      array.findIndex((e) => e === elem)
    ),
    ...array.splice(array.findIndex((e) => e === elem) + 1, array.length - 1 - array.findIndex((e) => e === elem)),
  ];
};

export const getFileFolderPath = (path?: string): string => {
  return path
    ? path
        ?.split('/')
        .splice(0, path.split('/').length - 1)
        .join('/')
    : '';
};

export const formatDate = (date: Date): string => {
  const year = date.getFullYear();
  const month = String(date.getMonth() + 1).padStart(2, '0');
  const day = String(date.getDate()).padStart(2, '0');
  const formattedDate = `${year}. ${month}. ${day}`;
  return formattedDate;
};

export const updateUrlWithParams = (params: RouterQueryType) => {
  const url = new URL(window.location.href);
  const searchParams = new URLSearchParams(url.search);

  Object.entries(params).forEach(([key, value]) => {
    const existingValue = searchParams.get(key);
    const valueString = String(value);

    if (existingValue !== valueString) {
      if (existingValue) {
        searchParams.set(key, valueString);
      } else {
        searchParams.append(key, valueString);
      }
    }
  });

  url.search = searchParams.toString();
  return url.toString();
};
