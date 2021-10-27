package service.srcjava;

import cc.service.core.*;
import cc.service.java.JavaService;
import cc.service.language.AstNodeInfo;
import model.*;
import org.apache.thrift.TException;
import service.srcjava.enums.ReferenceType;

import javax.persistence.EntityManager;
import javax.persistence.criteria.*;

import java.util.*;
import java.util.logging.Level;

import static logger.Logger.LOGGER;
import static model.EMFactory.createEntityManager;
import static service.srcjava.JavaQueryFactory.*;

class JavaQueryHandler implements JavaService.Iface {
  private static final EntityManager em =
    createEntityManager(System.getProperty("rawDbContext"));
  private static final CriteriaBuilder cb = em.getCriteriaBuilder();

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

    return createAstNodeInfo(minJavaAstNode);
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
            Boolean.toString(javaRecord.getIsAbstract()));
          properties.put("Name", javaRecord.getName());
          properties.put("Qualified name", javaRecord.getQualifiedName());

          return properties;
        }
        break;
      }
      case ENUM_CONSTANT: {
        List<JavaEnumConstant> javaEnumConstants =
          queryJavaEnumConstants(javaAstNode);

        if (!javaEnumConstants.isEmpty()) {
          JavaEnumConstant javaEnumConstant = javaEnumConstants.get(0);

          properties.put("Name", javaEnumConstant.getName());
          properties.put("Qualified name", javaEnumConstant.getQualifiedName());
          properties.put("Value", javaEnumConstant.getValue());

          return properties;
        }
        break;
      }
    }

    return properties;
  }

  @Override
  public String getDocumentation(String javaAstNodeId) {
    return "";
  }

  @Override
  public int getReferenceCount(String javaAstNodeId, int referenceId) {
    long javaAstNodeIdLong = Long.parseLong(javaAstNodeId);
    JavaAstNode javaAstNode = queryJavaAstNode(javaAstNodeIdLong);

    switch (ReferenceType.values()[referenceId]) {
      case DEFINITION:
        return queryDefinitions(javaAstNode).size();
      case DECLARATION:
        return queryVisibleDeclarations(javaAstNode).size();
      case USAGE:
        return queryUsages(javaAstNode).size();
      case THIS_CALLS:
        return queryCalls(javaAstNode).size();
      case CALLS_OF_THIS:
        return queryUsages(javaAstNode).size();
      case CALLEE:
        return queryCallees(javaAstNode).size();
      case CALLER:
        return queryCallers(javaAstNode).size();
      case PARAMETER:
        return queryParameters(javaAstNode).size();
      case LOCAL_VAR:
        return queryLocalVars(javaAstNode).size();
      case RETURN_TYPE:
        return queryReturnType(javaAstNode).size();
      case OVERRIDE:
        break;
      case OVERRIDDEN_BY:
        break;
      case READ:
        break;
      case WRITE:
        break;
      case TYPE:
        return queryType(javaAstNode).size();
      case INHERIT_FROM:
        break;
      case INHERIT_BY:
        break;
      case DATA_MEMBER:
        break;
      case METHOD:
        break;
      case ENUM_CONSTANTS:
        break;
    }

    return 0;
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

      case ENUM:
        referenceTypes.put(
          "Enum constants", ReferenceType.ENUM_CONSTANTS.ordinal());
        break;
    }

    return referenceTypes;
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
        javaAstNodes = queryDefinitions(javaAstNode);
        break;
      case DECLARATION:
        javaAstNodes = queryVisibleDeclarations(javaAstNode);
        break;
      case USAGE:
        javaAstNodes = queryUsages(javaAstNode);
        break;
      case THIS_CALLS:
        javaAstNodes = queryCalls(javaAstNode);
        break;
      case CALLS_OF_THIS:
        javaAstNodes = queryUsages(javaAstNode);
        break;
      case CALLEE:
        javaAstNodes = queryCallees(javaAstNode);
        break;
      case CALLER:
        javaAstNodes = queryCallers(javaAstNode);
        break;
      case PARAMETER:
        javaAstNodes = queryParameters(javaAstNode);
        break;
      case LOCAL_VAR:
        javaAstNodes = queryLocalVars(javaAstNode);
        break;
      case RETURN_TYPE:
        javaAstNodes = queryReturnType(javaAstNode);
        break;
      case OVERRIDE:
        break;
      case OVERRIDDEN_BY:
        break;
      case READ:
        break;
      case WRITE:
        break;
      case TYPE:
        javaAstNodes = queryType(javaAstNode);
        break;
      case INHERIT_FROM:
        break;
      case INHERIT_BY:
        break;
      case DATA_MEMBER:
        break;
      case METHOD:
        break;
      case ENUM_CONSTANTS:
        break;
    }

    javaAstNodeInfos = createAstNodeInfos(javaAstNodes);

    return javaAstNodeInfos;
  }

  private List<AstNodeInfo> createAstNodeInfos(List<JavaAstNode> javaAstNodes) {
    List<AstNodeInfo> javaAstNodeInfos = new ArrayList<>();

    javaAstNodes.forEach(p -> javaAstNodeInfos.add(createAstNodeInfo(p)));

    javaAstNodeInfos.sort((n1, n2) -> {
      Integer line1 = n1.range.range.endpos.line;
      Integer line2 = n2.range.range.endpos.line;
      int lineComp = line1.compareTo(line2);

      if (lineComp != 0) {
        return lineComp;
      }

      Integer col1 = n1.range.range.endpos.column;
      Integer col2 = n2.range.range.endpos.column;
      return col1.compareTo(col2);
    });

    return javaAstNodeInfos;
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
}