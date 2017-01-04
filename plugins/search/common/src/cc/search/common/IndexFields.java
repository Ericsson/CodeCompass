package cc.search.common;

import cc.search.analysis.tags.Tag;

/**
 * Common constants for index fields.
 */
public class IndexFields {
  /**
   * The name of the "path" field. Path contains the full path of the file.
   */
  public static final String filePathField = "path";
  /**
   * The name of directory path field.
   */
  public static final String fileDirPathField = "dirPath";
  /**
   * The name of file name field.
   */
  public static final String fileNameField = "fileName";
  /**
   * The name of the "file id" field.
   */
  public static final String fileDbIdField = "fid";
  /**
   * Definitions field for "definition search".
   */
  public static final String definitionsField = "defs";
  /**
   * Field for (original) tag kinds in the document.
   */
  public static final String tagKindsField = "tagKind";
  /**
   * Tags field (stored, serialized).
   */
  public static final String tagsField = "tags";
  /**
   * The name of the "content" field.
   */
  public static final String contentField = "content";
  /**
   * Field name for storing mime-type.
   */
  public static final String mimeTypeField = "mime";
  /**
   * Document boost value.
   */
  public static final String boostValue = "boost";
  
  /**
   * Maps a Tag.Kind to a document field name. 
   * @param kind_ tag kind.
   * @return document field name.
   */
  public static String getFieldNameForTagKind(Tag.Kind kind_) {
    switch (kind_) {
      case Other:
        return TAG_FIELD_OTHERTAG;
      case Type:
        return TAG_FIELD_TYPE;
      case Macro:
        return TAG_FIELD_MACRO;
      case Constant:
        return TAG_FIELD_CONST;
      case Function:
        return TAG_FIELD_FUNC;
      case Field:
        return TAG_FIELD_FIELD;
      case Prototype:
        return TAG_FIELD_PROTO;
      case Variable:
        return TAG_FIELD_VAR;
      case Label:
        return TAG_FIELD_LABEL;
      case Module:
        return TAG_FIELD_MODULE;
      default:
        throw new AssertionError(kind_.name());
    }
  }
  
  /**
   * Maps a document field name to a Tag.Kind.
   * @param fieldName_ document field name.
   * @return tag kind.
   * @throws IllegalArgumentException
   */
  public static Tag.Kind getTagKindForFieldName(String fieldName_) {
    switch (fieldName_) {
      case TAG_FIELD_OTHERTAG:
        return Tag.Kind.Other;
      case TAG_FIELD_TYPE:
        return Tag.Kind.Type;
      case TAG_FIELD_MACRO:
        return Tag.Kind.Macro;
      case TAG_FIELD_CONST:
        return Tag.Kind.Constant;
      case TAG_FIELD_FUNC:
        return Tag.Kind.Function;
      case TAG_FIELD_FIELD:
        return Tag.Kind.Field;
      case TAG_FIELD_PROTO:
        return Tag.Kind.Prototype;
      case TAG_FIELD_VAR:
        return Tag.Kind.Variable;
      case TAG_FIELD_LABEL:
        return Tag.Kind.Label;
      case TAG_FIELD_MODULE:
        return Tag.Kind.Module;
      default:
        throw new IllegalArgumentException();
    }
  }
  
  /**
   * @param fieldName_ document field name.
   * @return 
   */
  public static boolean isTagKindFieldName(String fieldName_) {
    try {
      getTagKindForFieldName(fieldName_);
      return true;
    } catch (IllegalArgumentException ex) {
      return false;
    }
  }
  
  private static final String TAG_FIELD_MODULE = "module";
  private static final String TAG_FIELD_LABEL = "label";
  private static final String TAG_FIELD_VAR = "var";
  private static final String TAG_FIELD_PROTO = "proto";
  private static final String TAG_FIELD_FIELD = "field";
  private static final String TAG_FIELD_FUNC = "func";
  private static final String TAG_FIELD_CONST = "const";
  private static final String TAG_FIELD_MACRO = "macro";
  private static final String TAG_FIELD_TYPE = "type";
  private static final String TAG_FIELD_OTHERTAG = "othertag";
}
