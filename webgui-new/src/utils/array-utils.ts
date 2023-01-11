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
