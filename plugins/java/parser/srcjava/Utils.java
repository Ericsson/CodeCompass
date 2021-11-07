package parser.srcjava;

import model.*;
import model.enums.*;
import org.eclipse.jdt.core.Flags;
import org.eclipse.jdt.core.dom.ITypeBinding;
import parser.srcjava.PositionInfo;

import java.util.Arrays;

public abstract class Utils {
  public static Visibility getVisibility(int modifiers) {
    if (Flags.isPublic(modifiers)) {
      return Visibility.PUBLIC;
    } else if (Flags.isProtected(modifiers)) {
      return Visibility.PROTECTED;
    } else if (Flags.isPrivate(modifiers)) {
      return Visibility.PRIVATE;
    }

    return Visibility.PACKAGE_PRIVATE;
  }

  public static String[] getParameterTypeNames(ITypeBinding[] parameters) {
    String[] parameterTypeNames = new String[parameters.length];

    for (int i = 0; i < parameters.length; i++) {
      parameterTypeNames[i] = parameters[i].getQualifiedName();
    }

    return parameterTypeNames;
  }

  public static String getParameterTypeNamesStr(ITypeBinding[] parameters) {
    return Arrays.toString(getParameterTypeNames(parameters));
  }

  public static void setJavaRelationFields(
    JavaRelation javaRelation, int leftEntityHash,
    int rightEntityHash, RelationKind kind)
  {
    javaRelation.setLhs(leftEntityHash);
    javaRelation.setRhs(rightEntityHash);
    javaRelation.setKind(kind);
  }

  public static void setJavaDocCommentFields(
    JavaDocComment javaDocComment, String commentString, long entityHash)
  {
    javaDocComment.setContent(commentString);
    javaDocComment.setContentHash(commentString.hashCode());
    javaDocComment.setEntityHash(entityHash);
  }

  public static void setJavaRecordFields(JavaRecord javaRecord, int modifiers) {
    javaRecord.setAbstract(Flags.isAbstract(modifiers));
    javaRecord.setFinal(Flags.isFinal(modifiers));
  }

  public static <T extends JavaTypedEntity> void setJavaTypedEntityFields(
    T javaTypedEntity, int modifiers, long typeHash, String qualifiedType)
  {
    javaTypedEntity.setFinal(Flags.isFinal(modifiers));
    javaTypedEntity.setStatic(Flags.isStatic(modifiers));
    javaTypedEntity.setTypeHash(typeHash);
    javaTypedEntity.setQualifiedType(qualifiedType);
  }

  public static <T extends JavaEntity> void setJavaEntityFields(
    T javaEntity, long astNodeId,
    long entityHash,String name, String qualifiedName)
  {
    javaEntity.setAstNodeId(astNodeId);
    javaEntity.setEntityHash(entityHash);
    javaEntity.setName(name);
    javaEntity.setQualifiedName(qualifiedName);
  }

  public static void setJavaInheritanceFields(
    JavaInheritance javaInheritance, int baseEntityHash, int derivedEntityHash)
  {
    javaInheritance.setBase(baseEntityHash);
    javaInheritance.setDerived(derivedEntityHash);
  }

  public static void setJavaMemberTypeFields(
    JavaMemberType javaMemberType, int typeHash, int memberTypeHash,
    MemberTypeKind memberTypeKind, int modifiers, JavaAstNode javaAstNode)
  {
    javaMemberType.setTypeHash(typeHash);
    javaMemberType.setMemberTypeHash(memberTypeHash);
    javaMemberType.setKind(memberTypeKind);
    javaMemberType.setVisibility(getVisibility(modifiers));
    javaMemberType.setMemberAstNode(javaAstNode);
  }

  public static void setJavaAstNodeFields(
    JavaAstNode javaAstNode, String astValue, PositionInfo positionInfo,
    long fileId, int entityHash, SymbolType symbolType,
    AstType astType, boolean visibleInSourceCode)
  {
    javaAstNode.setAstValue(astValue);
    javaAstNode.setLocation_range_start_line(positionInfo.getStartLine());
    javaAstNode.setLocation_range_start_column(positionInfo.getStartColumn());
    javaAstNode.setLocation_range_end_line(positionInfo.getEndLine());
    javaAstNode.setLocation_range_end_column(positionInfo.getEndColumn());
    javaAstNode.setLocation_file(fileId);
    javaAstNode.setEntityHash(entityHash);
    javaAstNode.setSymbolType(symbolType);
    javaAstNode.setAstType(astType);
    javaAstNode.setVisibleInSourceCode(visibleInSourceCode);
  }

  public static String getMethodHashStr(
    String className, String type, String name, String parameters)
  {
    return String.join(className, type, name, parameters);
  }

  public static String getEnumConstantHashStr(
    String enumName, String name)
  {
    return String.join(enumName, name);
  }

  public static String getFieldHashStr(
    String className, String type, String name)
  {
    return String.join(className, type, name);
  }

  public static String getVariableHashStr(
    String methodHashStr, String type, String name)
  {
    return String.join(methodHashStr, type, name);
  }
}
