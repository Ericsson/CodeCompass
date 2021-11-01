package service.srcjava;

import cc.service.core.*;
import cc.service.java.JavaService;
import cc.service.language.AstNodeInfo;
import model.*;
import model.enums.MemberTypeKind;
import org.apache.thrift.TException;
import service.srcjava.enums.FileReferenceType;
import service.srcjava.enums.ReferenceType;

import java.util.*;
import java.util.stream.Collectors;

import static service.srcjava.JavaQueryFactory.*;

public class JavaQueryHandler implements JavaService.Iface {
  @Override
  public AstNodeInfo getAstNodeInfoByPosition(FilePosition fpos)
    throws TException
  {
    List<JavaAstNode> javaAstNodes = queryJavaAstNodeByPosition(fpos);
    JavaAstNode minJavaAstNode;

    if (javaAstNodes.isEmpty()) {
      InvalidPos ex = new InvalidPos();
      ex.msg = "There are no any JavaAstNode at this position.";
      ex.fpos = fpos;

      throw ex;
    }

    minJavaAstNode = javaAstNodes.get(0);

    for (JavaAstNode javaAstNode : javaAstNodes) {
      if (javaAstNode.isVisibleInSourceCode() &&
        javaAstNode.isRangeSmaller(minJavaAstNode)) {
        minJavaAstNode = javaAstNode;
      }
    }

    return createAstNodeInfo(
      minJavaAstNode, getTags(Collections.singletonList(minJavaAstNode))
    );
  }

  @Override
  public Map<String, String> getProperties(String javaAstNodeId) {
    Map<String, String> properties = new HashMap<>();
    long javaAstNodeIdLong = Long.parseLong(javaAstNodeId);
    JavaAstNode javaAstNode = queryJavaAstNode(javaAstNodeIdLong);

    switch (javaAstNode.getSymbolType()){
      case VARIABLE: {
        List<JavaVariable> javaVariables = queryJavaVariables(javaAstNode);

        if (!javaVariables.isEmpty()) {
          JavaVariable javaVariable = javaVariables.get(0);

          properties.put("Final variable",
            Boolean.toString(javaVariable.isFinal()));
          properties.put("Static variable",
            Boolean.toString(javaVariable.isStatic()));
          properties.put("Name", javaVariable.getName());
          properties.put("Qualified name", javaVariable.getQualifiedName());
          properties.put("Type", javaVariable.getQualifiedType());

          return properties;
        }
        break;
      }
      case CONSTRUCTOR: {
        List<JavaConstructor> javaConstructors =
          queryJavaConstructors(javaAstNode);

        if (!javaConstructors.isEmpty()) {
          JavaConstructor javaConstructor = javaConstructors.get(0);

          properties.put("Name", javaConstructor.getName());
          properties.put("Qualified name", javaConstructor.getQualifiedName());

          return properties;
        }
        break;
      }
      case METHOD: {
        List<JavaMethod> javaMethods = queryJavaMethods(javaAstNode);

        if (!javaMethods.isEmpty()) {
          JavaMethod javaMethod = javaMethods.get(0);

          properties.put("Final method",
            Boolean.toString(javaMethod.isFinal()));
          properties.put("Static method",
            Boolean.toString(javaMethod.isStatic()));
          properties.put("Name", javaMethod.getName());
          properties.put("Qualified name", javaMethod.getQualifiedName());
          properties.put("Type", javaMethod.getQualifiedType());

          return properties;
        }
        break;
      }
      case TYPE: {
        List<JavaRecord> javaRecords = queryJavaRecords(javaAstNode);

        if (!javaRecords.isEmpty()) {
          JavaRecord javaRecord = javaRecords.get(0);

          properties.put(
            "Abstract type",
            Boolean.toString(javaRecord.isAbstract()));
          properties.put(
            "Final type",
            Boolean.toString(javaRecord.isFinal()));
          properties.put("Name", javaRecord.getName());
          properties.put("Qualified name", javaRecord.getQualifiedName());

          return properties;
        }
        break;
      }
      case ENUM: {
        List<JavaEnum> javaEnums = queryJavaEnums(javaAstNode);

        if (!javaEnums.isEmpty()) {
          JavaEnum javaEnum = javaEnums.get(0);

          properties.put("Name", javaEnum.getName());
          properties.put("Qualified name", javaEnum.getQualifiedName());
        }
      }
      case ENUM_CONSTANT: {
        List<JavaEnumConstant> javaEnumConstants =
          queryJavaEnumConstants(javaAstNode);

        if (!javaEnumConstants.isEmpty()) {
          JavaEnumConstant javaEnumConstant = javaEnumConstants.get(0);

          properties.put("Name", javaEnumConstant.getName());
          properties.put("Qualified name", javaEnumConstant.getQualifiedName());
          properties.put(
            "Value", Integer.toString(javaEnumConstant.getValue())
          );

          return properties;
        }
        break;
      }
    }

    return properties;
  }

  @Override
  public String getDocumentation(String javaAstNodeId) {
    StringBuilder stringBuilder = new StringBuilder();

    long javaAstNodeIdLong = Long.parseLong(javaAstNodeId);
    JavaAstNode javaAstNode = queryJavaAstNode(javaAstNodeIdLong);

    List<JavaDocComment> javaDocComments = queryJavaDocComments(javaAstNode);

    if (!javaDocComments.isEmpty()) {
      stringBuilder
        .append("<div class=\"main-doc\">")
        .append(javaDocComments.get(0).getContent().replace("\n", "<br/>"))
        .append("</div>");
    }

    switch (javaAstNode.getSymbolType()) {
      case TYPE:
        break;
    }

    return stringBuilder.toString();
  }

  @Override
  public Map<String, Integer> getReferenceTypes(String javaAstNodeId) {
    long javaAstNodeIdLong = Long.parseLong(javaAstNodeId);
    HashMap<String, Integer> referenceTypes = new HashMap<>();
    JavaAstNode javaAstNode = queryJavaAstNode(javaAstNodeIdLong);

    referenceTypes.put("Definition", ReferenceType.DEFINITION.ordinal());
    referenceTypes.put("Declaration", ReferenceType.DECLARATION.ordinal());
    referenceTypes.put("Usage", ReferenceType.USAGE.ordinal());

    switch (javaAstNode.getSymbolType()) {
      case CONSTRUCTOR:
        referenceTypes.put(
          "This calls", ReferenceType.THIS_CALLS.ordinal());
        referenceTypes.put(
          "Callee", ReferenceType.CALLEE.ordinal());
        referenceTypes.put(
          "Caller", ReferenceType.CALLER.ordinal());
        referenceTypes.put(
          "Parameters", ReferenceType.PARAMETER.ordinal());
        referenceTypes.put(
          "Local variables", ReferenceType.LOCAL_VAR.ordinal());
        break;
      case METHOD:
        referenceTypes.put(
          "This calls", ReferenceType.THIS_CALLS.ordinal());
        referenceTypes.put(
          "Callee", ReferenceType.CALLEE.ordinal());
        referenceTypes.put(
          "Caller", ReferenceType.CALLER.ordinal());
        referenceTypes.put(
          "Parameters", ReferenceType.PARAMETER.ordinal());
        referenceTypes.put(
          "Local variables", ReferenceType.LOCAL_VAR.ordinal());
        referenceTypes.put(
          "Overrides", ReferenceType.OVERRIDE.ordinal());
        referenceTypes.put(
          "Overridden by", ReferenceType.OVERRIDDEN_BY.ordinal());
        referenceTypes.put(
          "Return type", ReferenceType.RETURN_TYPE.ordinal());
        break;
      case VARIABLE:
        referenceTypes.put(
          "Reads", ReferenceType.READ.ordinal());
        referenceTypes.put(
          "Writes", ReferenceType.WRITE.ordinal());
        referenceTypes.put(
          "Type", ReferenceType.TYPE.ordinal());
        break;

      case TYPE:
        referenceTypes.put(
          "Inherits from", ReferenceType.INHERIT_FROM.ordinal());
        referenceTypes.put(
          "Inherits by", ReferenceType.INHERIT_BY.ordinal());
        referenceTypes.put(
          "Constructor", ReferenceType.CONSTRUCTOR.ordinal());
        referenceTypes.put(
          "Data member", ReferenceType.DATA_MEMBER.ordinal());
        referenceTypes.put(
          "Method", ReferenceType.METHOD.ordinal());
        break;
      case ENUM:
        referenceTypes.put(
          "Enum constants", ReferenceType.ENUM_CONSTANTS.ordinal());
        referenceTypes.put(
          "Constructor", ReferenceType.CONSTRUCTOR.ordinal());
        referenceTypes.put(
          "Data member", ReferenceType.DATA_MEMBER.ordinal());
        referenceTypes.put(
          "Method", ReferenceType.METHOD.ordinal());
        break;
    }

    return referenceTypes;
  }

  @Override
  public int getReferenceCount(String javaAstNodeId, int referenceId) {
    long javaAstNodeIdLong = Long.parseLong(javaAstNodeId);
    JavaAstNode javaAstNode = queryJavaAstNode(javaAstNodeIdLong);

    switch (ReferenceType.values()[referenceId]) {
      case DEFINITION:
        return queryDefinitionNodes(javaAstNode).size();
      case DECLARATION:
        return queryVisibleDeclarationNodes(javaAstNode).size();
      case USAGE:
        return queryUsageNodes(javaAstNode).size();
      case THIS_CALLS:
        return queryCallNodes(javaAstNode).size();
      case CALLEE:
        return queryCalleeNodes(javaAstNode).size();
      case CALLER:
        return queryCallerNodes(javaAstNode).size();
      case PARAMETER:
        return queryParameterNodes(javaAstNode).size();
      case LOCAL_VAR:
        return queryLocalVarNodes(javaAstNode).size();
      case RETURN_TYPE:
        return queryReturnTypeNodes(javaAstNode).size();
      case OVERRIDE:
        break;
      case OVERRIDDEN_BY:
        break;
      case READ:
        return queryReadNodes(javaAstNode).size();
      case WRITE:
        return queryWriteNodes(javaAstNode).size();
      case TYPE:
        return queryTypeNodes(javaAstNode).size();
      case INHERIT_FROM:
        break;
      case INHERIT_BY:
        break;
      case CONSTRUCTOR:
        return queryJavaMemberTypeDefinitionNodes(
          javaAstNode, MemberTypeKind.CONSTRUCTOR).size();
      case DATA_MEMBER:
        return queryJavaMemberTypeDefinitionNodes(
          javaAstNode, MemberTypeKind.FIELD).size();
      case METHOD:
        return queryJavaMemberTypeDefinitionNodes(
          javaAstNode, MemberTypeKind.METHOD).size();
      case ENUM_CONSTANTS:
        return queryJavaEnumConstantNodes(javaAstNode).size();
    }

    return 0;
  }

  @Override
  public List<AstNodeInfo> getReferences(
    String javaAstNodeId, int referenceId, List<String> tags)
  {
    long javaAstNodeIdLong = Long.parseLong(javaAstNodeId);
    JavaAstNode javaAstNode = queryJavaAstNode(javaAstNodeIdLong);
    List<JavaAstNode> javaAstNodes = new ArrayList<>();
    List<AstNodeInfo> javaAstNodeInfos;

    switch (ReferenceType.values()[referenceId]) {
      case DEFINITION:
        javaAstNodes = queryDefinitionNodes(javaAstNode);
        break;
      case DECLARATION:
        javaAstNodes = queryVisibleDeclarationNodes(javaAstNode);
        break;
      case USAGE:
        javaAstNodes = queryUsageNodes(javaAstNode);
        break;
      case THIS_CALLS:
        javaAstNodes = queryCallNodes(javaAstNode);
        break;
      case CALLEE:
        javaAstNodes = queryCalleeNodes(javaAstNode);
        break;
      case CALLER:
        javaAstNodes = queryCallerNodes(javaAstNode);
        break;
      case PARAMETER:
        javaAstNodes = queryParameterNodes(javaAstNode);
        break;
      case LOCAL_VAR:
        javaAstNodes = queryLocalVarNodes(javaAstNode);
        break;
      case RETURN_TYPE:
        javaAstNodes = queryReturnTypeNodes(javaAstNode);
        break;
      case OVERRIDE:
        break;
      case OVERRIDDEN_BY:
        break;
      case READ:
        javaAstNodes = queryReadNodes(javaAstNode);
        break;
      case WRITE:
        javaAstNodes = queryWriteNodes(javaAstNode);
        break;
      case TYPE:
        javaAstNodes = queryTypeNodes(javaAstNode);
        break;
      case INHERIT_FROM:
        break;
      case INHERIT_BY:
        break;
      case CONSTRUCTOR:
        javaAstNodes =
          queryJavaMemberTypeDefinitionNodes(
            javaAstNode, MemberTypeKind.CONSTRUCTOR);
        break;
      case DATA_MEMBER:
        javaAstNodes =
          queryJavaMemberTypeDefinitionNodes(
            javaAstNode, MemberTypeKind.FIELD);
        break;
      case METHOD:
        javaAstNodes =
          queryJavaMemberTypeDefinitionNodes(
            javaAstNode, MemberTypeKind.METHOD);
        break;
      case ENUM_CONSTANTS:
        javaAstNodes = queryJavaEnumConstantNodes(javaAstNode);
        break;
    }

    javaAstNodeInfos = createAstNodeInfos(javaAstNodes);

    return javaAstNodeInfos;
  }

  @Override
  public Map<String, Integer> getFileReferenceTypes() {
    Map<String, Integer> fileReferenceTypes = new HashMap<>();

    fileReferenceTypes.put("Import", FileReferenceType.IMPORTS.ordinal());
    fileReferenceTypes.put("Type", FileReferenceType.TYPES.ordinal());
    fileReferenceTypes.put("Constructor",
      FileReferenceType.CONSTRUCTORS.ordinal());
    fileReferenceTypes.put("Method", FileReferenceType.METHODS.ordinal());

    return fileReferenceTypes;
  }

  @Override
  public int getFileReferenceCount(String fileId, int referenceId) {
    long fileIdLong = Long.parseUnsignedLong(fileId);

    switch (FileReferenceType.values()[referenceId]) {
      case IMPORTS:
        return queryJavaImportNodesInFile(fileIdLong).size();
      case TYPES:
        return queryJavaTypeNodesInFile(fileIdLong).size();
      case CONSTRUCTORS:
        return queryJavaConstructorNodesInFile(fileIdLong).size();
      case METHODS:
        return queryJavaMethodNodesInFile(fileIdLong).size();
    }

    return 0;
  }

  @Override
  public List<AstNodeInfo> getFileReferences(String fileId, int referenceId) {
    long fileIdLong = Long.parseUnsignedLong(fileId);
    List<JavaAstNode> javaAstNodes = new ArrayList<>();
    List<AstNodeInfo> javaAstNodeInfos;

    switch (FileReferenceType.values()[referenceId]) {
      case IMPORTS:
        javaAstNodes = queryJavaImportNodesInFile(fileIdLong);
        break;
      case TYPES:
        javaAstNodes = queryJavaTypeNodesInFile(fileIdLong);
        break;
      case CONSTRUCTORS:
        javaAstNodes = queryJavaConstructorNodesInFile(fileIdLong);
        break;
      case METHODS:
        javaAstNodes = queryJavaMethodNodesInFile(fileIdLong);
        break;
    }

    javaAstNodeInfos = createAstNodeInfos(javaAstNodes);

    return javaAstNodeInfos;
  }

  private List<AstNodeInfo> createAstNodeInfos(List<JavaAstNode> javaAstNodes) {
    Map<Long, List<String>> tags = getTags(javaAstNodes);

    return javaAstNodes.stream()
      .map(p -> createAstNodeInfo(p, tags))
      .sorted((n1, n2) -> {
        Integer line1 = n1.range.range.startpos.line;
        Integer line2 = n2.range.range.startpos.line;
        int lineComp = line1.compareTo(line2);

        if (lineComp != 0) {
          return lineComp;
        }

        Integer col1 = n1.range.range.startpos.column;
        Integer col2 = n2.range.range.startpos.column;
        return col1.compareTo(col2);
      })
      .collect(Collectors.toList());
  }

  private AstNodeInfo createAstNodeInfo(JavaAstNode javaAstNode) {
    AstNodeInfo astNodeInfo = new AstNodeInfo();
    FileRange fileRange = new FileRange();
    Range range = new Range();
    Position startPosition = new Position(
      (int) javaAstNode.getLocation_range_start_line(),
      (int) javaAstNode.getLocation_range_start_column()
    );
    Position endPosition = new Position(
      (int) javaAstNode.getLocation_range_end_line(),
      (int) javaAstNode.getLocation_range_end_column()
    );

    range.startpos = startPosition;
    range.endpos = endPosition;

    fileRange.file = String.valueOf(javaAstNode.getLocation_file());
    fileRange.range = range;

    astNodeInfo.id = String.valueOf(javaAstNode.getId());
    astNodeInfo.entityHash = javaAstNode.getEntityHash();
    astNodeInfo.astNodeType = javaAstNode.getAstType().getName();
    astNodeInfo.symbolType = javaAstNode.getSymbolType().getName();
    astNodeInfo.astNodeValue = javaAstNode.getAstValue();
    astNodeInfo.range = fileRange;

    return astNodeInfo;
  }

  private AstNodeInfo createAstNodeInfo(
    JavaAstNode javaAstNode, Map<Long, List<String>> tags)
  {
    AstNodeInfo astNodeInfo = createAstNodeInfo(javaAstNode);

    astNodeInfo.tags = tags.get(javaAstNode.getId());

    return astNodeInfo;
  }

  private Map<Long, List<String>> getTags(List<JavaAstNode> javaAstNodes) {
    Map<Long, List<String>> tags = new HashMap<>();

    javaAstNodes.forEach(node -> {
      List<JavaAstNode> definitions = queryDefinitionNodes(node);
      JavaAstNode definition =
        definitions.isEmpty() ? node : definitions.get(0);

      switch (node.getSymbolType()) {
        case TYPE:
          putTags(node, definition, MemberTypeKind.TYPE, tags);
          break;
        case CONSTRUCTOR:
          putTags(node, definition, MemberTypeKind.CONSTRUCTOR, tags);
          break;
        case VARIABLE:
          putTags(node, definition, MemberTypeKind.FIELD, tags);
          break;
        case METHOD:
          putTags(node, definition, MemberTypeKind.METHOD, tags);
          break;
        case ENUM:
          putTags(node, definition, MemberTypeKind.ENUM, tags);
          break;
        case ENUM_CONSTANT:
          putTags(node, definition, MemberTypeKind.ENUM_CONSTANT, tags);
          break;
      }
    });

    return tags;
  }

  private void putTags(
    JavaAstNode javaAstNode, JavaAstNode definition,
    MemberTypeKind memberTypeKind, Map<Long, List<String>> tags)
  {
    List<JavaMemberType> javaMemberTypes =
      queryJavaMemberTypes(javaAstNode, definition, memberTypeKind);

    javaMemberTypes.forEach(m -> {
      long nodeId = javaAstNode.getId();
      if (!tags.containsKey(nodeId)) {
        tags.put(
          nodeId,
          new ArrayList<>(
            Collections.singleton(m.getVisibility().getName())
          )
        );
      } else {
        tags.get(nodeId).add(m.getVisibility().getName());
      }
    });
  }
}