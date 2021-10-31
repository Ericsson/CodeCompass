package service.srcjava.enums;

public enum FileReferenceType {
  IMPORTS, /*!< Included source files in the current source file after the
    inclusion directive. */

  TYPES, /*!< User defined data types such as classes, structs etc. */

  CONSTRUCTORS, /*!< Methods in the current source file. */

  METHODS /*!< Methods in the current source file. */
}
