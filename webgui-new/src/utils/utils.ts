import { Range, Position } from '@thrift-generated';

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

export const convertSelectionStringToRange = (selectionString: string): Range => {
  const selection = selectionString.split('|');
  const startLine = parseInt(selection[0]);
  const startCol = parseInt(selection[1]);
  const endLine = parseInt(selection[2]);
  const endCol = parseInt(selection[3]);

  const startpos = new Position({
    line: startLine,
    column: startCol,
  });

  const endpos = new Position({
    line: endLine,
    column: endCol,
  });

  const range = new Range({
    startpos,
    endpos,
  });

  return range;
};

export const convertSelectionRangeToString = (selectionRange: Range | undefined): string => {
  if (selectionRange && selectionRange.startpos && selectionRange.endpos) {
    const { line: startLine, column: startCol } = selectionRange.startpos;
    const { line: endLine, column: endCol } = selectionRange.endpos;
    return `${startLine}|${startCol}|${endLine}|${endCol}`;
  } else {
    return '';
  }
};
