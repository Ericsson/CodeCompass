package parser.srcjava;

import model.*;
import model.enums.RelationKind;
import model.enums.Visibility;
import org.eclipse.jdt.core.Flags;
import org.eclipse.jdt.core.dom.ITypeBinding;

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
}
