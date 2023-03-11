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
