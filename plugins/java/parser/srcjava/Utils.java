package parser.srcjava;

import cc.parser.java.CmdArgs;
import cc.parser.java.ParseResult;
import cc.service.core.BuildLog;
import cc.service.core.MessageType;
import cc.service.core.Position;
import cc.service.core.Range;
import model.*;
import model.enums.*;
import org.eclipse.jdt.core.Flags;
import org.eclipse.jdt.core.JavaCore;
import org.eclipse.jdt.core.compiler.IProblem;
import org.eclipse.jdt.core.dom.*;

import java.util.Arrays;
import java.util.Hashtable;
import java.util.List;
import java.util.logging.Level;
import java.util.stream.Collectors;

import static logger.Logger.LOGGER;

public abstract class Utils {
  public static String getJavaVersion() {
    String javaCoreVersion = System.getProperty("java.version");
    int dot1 = javaCoreVersion.indexOf(".");

    if (javaCoreVersion.startsWith("1.")) {
      int dot2 = javaCoreVersion.indexOf(".", dot1 + 1);
      return javaCoreVersion.substring(0, dot2);
    }

    return javaCoreVersion.substring(0, dot1);
  }

  public static Hashtable<String, String> getJavaCoreOptions() {
    Hashtable<String, String> options = JavaCore.getOptions();
    JavaCore.setComplianceOptions(getJavaVersion(), options);

    return options;
  }

  public static ParseResult getParseResult(
    CompilationUnit cu, ArgParser argParser,
    String fileCounterStr, boolean errorDueParsing)
  {
    ParseResult parseResult = new ParseResult();
    CmdArgs cmdArgs = getCmdArgs(argParser);
    List<BuildLog> buildLogs =  Arrays.stream(
      cu.getProblems())
      .map(p -> getBuildLog(cu, p, fileCounterStr))
      .collect(Collectors.toList());

    parseResult.cmdArgs = cmdArgs;
    parseResult.buildLogs = buildLogs;
    parseResult.errorDueParsing = errorDueParsing;

    return parseResult;
  }

  public static CmdArgs getCmdArgs(ArgParser argParser) {
    CmdArgs cmdArgs = new CmdArgs();
    cmdArgs.directory = argParser.getDirectory();
    cmdArgs.classpath = argParser.getClasspath();
    cmdArgs.sourcepath = argParser.getSourcepath();
    cmdArgs.filepath = argParser.getFilepath();
    cmdArgs.filename = argParser.getFilename();
    cmdArgs.bytecodeDir = argParser.getBytecodePath();
    cmdArgs.bytecodesPaths = argParser.getBytecodesPaths();

    return cmdArgs;
  }

  public static BuildLog getBuildLog(
    CompilationUnit cu, IProblem problem, String fileCounterStr)
  {
    MessageType messageType;
    PositionInfo positionInfo =
      new PositionInfo(cu, problem.getSourceStart(), problem.getSourceEnd());
    Position startPos =
      new Position(
        positionInfo.getStartLine(), positionInfo.getStartColumn());
    Position endPos =
      new Position(
        positionInfo.getEndLine(), positionInfo.getEndColumn());
    String message = problem.getMessage();

    if (problem.isError()) {
      messageType = MessageType.Error;

      LOGGER.log(
        Level.SEVERE, String.join(
          " ", fileCounterStr,
          positionInfo.getStartLine() + ":" + positionInfo.getStartColumn(),
          message
        )
      );
    } else if (problem.isWarning()) {
      messageType = MessageType.Warning;

      LOGGER.log(
        Level.WARNING, String.join(
          " ", fileCounterStr,
          positionInfo.getStartLine() + ":" + positionInfo.getStartColumn(),
          message
        )
      );
    } else {
      messageType = MessageType.Note;

      LOGGER.log(
        Level.INFO, String.join(
          " ", fileCounterStr,
          positionInfo.getStartLine() + ":" + positionInfo.getStartColumn(),
          message
        )
      );
    }

    BuildLog buildLog = new BuildLog();
    buildLog.message = message;
    buildLog.messageType = messageType;
    buildLog.range = new Range(startPos, endPos);

    return buildLog;
  }

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
    javaRecord.setStatic(Flags.isStatic(modifiers));
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
    T javaEntity, long astNodeId, long entityHash)
  {
    javaEntity.setAstNodeId(astNodeId);
    javaEntity.setEntityHash(entityHash);
  }

  public static <T extends JavaEntity> void setJavaEntityFields(
    T javaEntity, long astNodeId,
    long entityHash, String name, String qualifiedName)
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

  public static void setJavaInitializerFields(
    JavaInitializer javaInitializer, int modifiers, int typeHash)
  {
    javaInitializer.setKind(
      Flags.isStatic(modifiers) ?
        InitializerKind.STATIC :
        InitializerKind.INSTANCE
    );
    javaInitializer.setTypeHash(typeHash);
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

  public static String getInitializerHashStr(
    String className, int startPosition)
  {
    return String.join(" ", className, String.valueOf(startPosition));
  }

  public static String getMethodHashStr(
    ITypeBinding declaringTypeBinding, IMethodBinding methodBinding)
  {
    String className = getQualifiedTypeName(declaringTypeBinding);
    String type = methodBinding.getReturnType().getQualifiedName();
    String name = methodBinding.getName();
    String parameters =
      getParameterTypeNamesStr(methodBinding.getParameterTypes());

    return String.join(" ", className, type, name, parameters);
  }

  public static String getMethodHashStr(
    ASTNode node, ITypeBinding declaringTypeBinding,
    IMethodBinding methodBinding)
  {
    ASTNode declaringNode = findDeclaringNode(node);
    String className = getQualifiedTypeName(declaringTypeBinding);
    String type = methodBinding.getReturnType().getQualifiedName();
    String name = methodBinding.getName();
    String parameters =
      getParameterTypeNamesStr(methodBinding.getParameterTypes());

    if (declaringNode instanceof LambdaExpression) {
      return String.join(
        " ", String.valueOf(declaringNode.getStartPosition()),
        className, type, name, parameters
      );
    }

    return String.join(" ", className, type, name, parameters);
  }

  public static String getEnumConstantHashStr(
    String enumName, String name)
  {
    return String.join(" ", enumName, name);
  }

  public static String getFieldHashStr(
    String className, String type, String name)
  {
    return String.join(" ", className, type, name);
  }

  public static String getVariableHashStr(
    String methodHashStr, String type, String name)
  {
    return String.join(" ", methodHashStr, type, name);
  }

  public static int getMainTypeHashForInnerType(ASTNode node) {
    ASTNode mainType = node.getParent();

    if (!(mainType instanceof AbstractTypeDeclaration)) {
      // TODO: Anonymous class declarations
      return 0;
    }

    String mainTypeQualifiedName =
      getQualifiedTypeName(
        ((AbstractTypeDeclaration) mainType).resolveBinding()
      );

    return mainTypeQualifiedName.hashCode();
  }

  public static String typeParametersArrayToString(
    ITypeBinding[] typeParametersArray)
  {
    if (typeParametersArray.length == 0) {
      return "";
    }

    StringBuilder sb = new StringBuilder("<");

    for (int i =0; i < typeParametersArray.length; ++i) {
      sb.append(typeParametersArray[i].getQualifiedName());

      if (!(i == typeParametersArray.length - 1)) {
        sb.append(",");
      }
    }

    sb.append(">");

    return sb.toString();
  }

  public static String getQualifiedTypeName(ITypeBinding typeBinding) {
    return typeBinding.getQualifiedName() +
      typeParametersArrayToString(typeBinding.getTypeParameters());
  }

  public static ASTNode findDeclaringNode(ASTNode node) {
    while (node != null) {
      if (
        node instanceof LambdaExpression ||
        node instanceof MethodDeclaration ||
        node instanceof AbstractTypeDeclaration ||
        node instanceof Initializer)
      {
        return node;
      }

      node = node.getParent();
    }

    return null;
  }
}
