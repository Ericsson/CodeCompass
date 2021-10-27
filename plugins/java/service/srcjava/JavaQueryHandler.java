package service.srcjava;

import cc.service.core.*;
import cc.service.java.JavaService;
import cc.service.language.AstNodeInfo;
import model.*;
import model.enums.AstType;
import model.enums.SymbolType;
import org.apache.thrift.TException;
import service.srcjava.enums.ReferenceType;

import javax.persistence.EntityManager;
import javax.persistence.NoResultException;
import javax.persistence.criteria.*;

import java.util.*;
import java.util.logging.Level;
import java.util.stream.Collectors;

import static logger.Logger.LOGGER;
import static model.EMFactory.createEntityManager;

class JavaQueryHandler implements JavaService.Iface {
  private static final EntityManager em =
    createEntityManager(System.getProperty("rawDbContext"));
  private static final CriteriaBuilder cb = em.getCriteriaBuilder();

  @Override
  public AstNodeInfo getAstNodeInfoByPosition(FilePosition fpos)
    throws TException
  {
    long fileId = Long.parseUnsignedLong(fpos.file);

    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);

    int line = fpos.pos.line;
    int column = fpos.pos.column;
    Path<Long> locationFile =
      root.get("location_file");
    Path<Integer> startLine =
      root.get("location_range_start_line");
    Path<Integer> endLine =
      root.get("location_range_end_line");
    Path<Integer> startColumn =
      root.get("location_range_start_column");
    Path<Integer> endColumn =
      root.get("location_range_end_column");

    Predicate sameFile = cb.equal(locationFile, fileId);
    Predicate startPosLessEqualPos =
      cb.or(
        cb.and(cb.equal(startLine, line), cb.le(startColumn, column)),
        cb.lt(startLine, line)
      );
    Predicate posLessThanEndPos =
      cb.or(
        cb.and(cb.equal(endLine, line), cb.gt(endColumn, column)),
        cb.gt(endLine, line)
      );

    cr
      .select(root)
      .where(cb.and(sameFile, startPosLessEqualPos, posLessThanEndPos));

    List<JavaAstNode> javaAstNodes = em.createQuery(cr).getResultList();
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
        LOGGER.log(
          Level.WARNING,
          "Database query result was not expected to be empty. " +
          getCurrentPath() + ", line #" + getCurrentLineNumber()
        );
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
        LOGGER.log(
          Level.WARNING,
          "Database query result was not expected to be empty. " +
          getCurrentPath() + ", line #" + getCurrentLineNumber()
        );
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
        LOGGER.log(
          Level.WARNING,
          "Database query result was not expected to be empty. " +
          getCurrentPath() + ", line #" + getCurrentLineNumber()
        );
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
        LOGGER.log(
          Level.WARNING,
          "Database query result was not expected to be empty. " +
          getCurrentPath() + ", line #" + getCurrentLineNumber()
        );
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
        LOGGER.log(
          Level.WARNING,
          "Database query result was not expected to be empty. " +
          getCurrentPath() + ", line #" + getCurrentLineNumber()
        );
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

  private List<JavaAstNode> queryDefinitions(JavaAstNode javaAstNode) {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);
    Predicate predicate =
      cb.equal(root.get("astType"), AstType.DEFINITION);

    return queryJavaAstNodes(javaAstNode, cr, root, predicate);
  }

  private List<JavaAstNode> queryDefinitions(long javaAstNodeId) {
    JavaAstNode javaAstNode = queryJavaAstNode(javaAstNodeId);

    return queryDefinitions(javaAstNode);
  }

  private List<JavaAstNode> queryVisibleDeclarations(JavaAstNode javaAstNode) {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);
    Predicate predicate =
      cb.and(
        cb.equal(root.get("astType"), AstType.DECLARATION),
        cb.isTrue(root.get("visibleInSourceCode"))
      );

    return queryJavaAstNodes(javaAstNode, cr, root, predicate);
  }

  private List<JavaAstNode> queryUsages(JavaAstNode javaAstNode) {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);
    Predicate predicate = cb.equal(root.get("astType"), AstType.USAGE);

    return queryJavaAstNodes(javaAstNode, cr, root, predicate);
  }

  private List<JavaAstNode> queryCallees(JavaAstNode javaAstNode) {
    Set<JavaAstNode> callees = new HashSet<>();
    List<JavaAstNode> calls = queryCalls(javaAstNode);

    calls.forEach(c -> callees.addAll(queryDefinitions(c)));

    return new ArrayList<>(callees);
  }

  private List<JavaAstNode> queryCallers(JavaAstNode javaAstNode) {
    List<JavaAstNode> callers = new ArrayList<>();
    List<JavaAstNode> usages = queryUsages(javaAstNode);

    usages.forEach(u -> callers.addAll(queryCaller(u)));

    return callers;
  }

  private List<JavaAstNode> queryCaller(JavaAstNode usage) {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);

    Path<AstType> astType = root.get("astType");
    Path<SymbolType> symbolType = root.get("symbolType");
    Path<Long> locationFile =
      root.get("location_file");
    Path<Integer> startLine =
      root.get("location_range_start_line");
    Path<Integer> endLine =
      root.get("location_range_end_line");
    Path<Integer> startColumn =
      root.get("location_range_start_column");
    Path<Integer> endColumn =
      root.get("location_range_end_column");

    long uStartLine = usage.getLocation_range_start_line();
    long uEndLine = usage.getLocation_range_end_line();
    long uStartColumn = usage.getLocation_range_start_line();
    long uEndColumn = usage.getLocation_range_start_line();

    Predicate definition = cb.equal(astType, AstType.DEFINITION);
    Predicate methodOrConstructor =
      cb.or(
        cb.equal(symbolType, SymbolType.METHOD),
        cb.equal(symbolType, SymbolType.CONSTRUCTOR)
      );
    Predicate sameFile = cb.equal(locationFile, usage.getLocation_file());
    Predicate startPosLessEqualPos =
      cb.or(
        cb.and(
          cb.equal(startLine, uStartLine),
          cb.le(startColumn, uStartColumn)
        ),
        cb.lt(startLine, usage.getLocation_range_start_line())
      );
    Predicate posLessThanEndPos =
      cb.or(
        cb.and(cb.equal(endLine, uEndLine), cb.gt(endColumn, uEndColumn)),
        cb.gt(endLine, uEndLine)
      );

    cr
      .select(root)
      .where(
        cb.and(
          definition, methodOrConstructor, sameFile,
          startPosLessEqualPos, posLessThanEndPos
        )
      );

    return em.createQuery(cr).getResultList();
  }

  private List<JavaAstNode> queryCalls(JavaAstNode javaAstNode) {
    List<JavaAstNode> definitions = queryDefinitions(javaAstNode);

    if (definitions.isEmpty()) {
      return definitions;
    }

    JavaAstNode definition = definitions.get(0);

    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);

    Path<AstType> astType = root.get("astType");
    Path<SymbolType> symbolType = root.get("symbolType");
    Path<Long> locationFile =
      root.get("location_file");
    Path<Integer> startLine =
      root.get("location_range_start_line");
    Path<Integer> endLine =
      root.get("location_range_end_line");
    Path<Integer> startColumn =
      root.get("location_range_start_column");
    Path<Integer> endColumn =
      root.get("location_range_end_column");

    long uStartLine = definition.getLocation_range_start_line();
    long uEndLine = definition.getLocation_range_end_line();
    long uStartColumn = definition.getLocation_range_start_line();
    long uEndColumn = definition.getLocation_range_start_line();

    Predicate usage = cb.equal(astType, AstType.USAGE);
    Predicate methodOrConstructor =
      cb.or(
        cb.equal(symbolType, SymbolType.METHOD),
        cb.equal(symbolType, SymbolType.CONSTRUCTOR)
      );
    Predicate sameFile = cb.equal(locationFile, definition.getLocation_file());
    Predicate startPosGreaterEqualPos =
      cb.or(
        cb.and(
          cb.equal(startLine, uStartLine),
          cb.ge(startColumn, uStartColumn)
        ),
        cb.gt(startLine, definition.getLocation_range_start_line())
      );
    Predicate posGreaterThanEndPos =
      cb.or(
        cb.and(cb.equal(endLine, uEndLine), cb.lt(endColumn, uEndColumn)),
        cb.lt(endLine, uEndLine)
      );

    cr
      .select(root)
      .where(
        cb.and(
          usage, methodOrConstructor, sameFile,
          startPosGreaterEqualPos, posGreaterThanEndPos
        )
      );

    return em.createQuery(cr).getResultList();
  }

  private List<JavaAstNode> queryParameters(JavaAstNode javaAstNode) {
    CriteriaQuery<JavaEntity> cr = cb.createQuery(JavaEntity.class);
    Root<JavaEntity> root = cr.from(JavaEntity.class);

    cr
      .select(root)
      .where(cb.equal(root.get("astNodeId"), javaAstNode.getId()));

    try {
      JavaEntity javaEntity = em.createQuery(cr).getSingleResult();

      if (javaEntity instanceof JavaConstructor) {
        return queryJavaAstNodes(
          ((JavaConstructor) javaEntity).getJavaConVarParams());
      } else if (javaEntity instanceof JavaMethod) {
        return queryJavaAstNodes(
          ((JavaMethod) javaEntity).getJavaMetVarParams());
      }
    } catch (NoResultException ex) {
      LOGGER.log(
        Level.WARNING,
        "Database query result was not expected to be empty. " +
          getCurrentPath() + ", line #" + getCurrentLineNumber()
      );
    }

    return new ArrayList<>();
  }

  private List<JavaAstNode> queryLocalVars(JavaAstNode javaAstNode) {
    CriteriaQuery<JavaEntity> cr = cb.createQuery(JavaEntity.class);
    Root<JavaEntity> root = cr.from(JavaEntity.class);

    cr
      .select(root)
      .where(cb.equal(root.get("astNodeId"), javaAstNode.getId()));

    try {
      JavaEntity javaEntity = em.createQuery(cr).getSingleResult();

      if (javaEntity instanceof JavaConstructor) {
        return queryJavaAstNodes(
          ((JavaConstructor) javaEntity).getJavaConVarLocals());
      } else if (javaEntity instanceof JavaMethod) {
        return queryJavaAstNodes(
          ((JavaMethod) javaEntity).getJavaMetVarLocals());
      }
    } catch (NoResultException ex) {
      LOGGER.log(
        Level.WARNING,
        "Database query result was not expected to be empty. " +
          getCurrentPath() + ", line #" + getCurrentLineNumber()
      );
    }

    return new ArrayList<>();
  }

  private  List<JavaAstNode> queryReturnType(JavaAstNode javaAstNode) {
    List<JavaMethod> javaMethods = queryJavaMethods(javaAstNode);

    if (!javaMethods.isEmpty()) {
      JavaMethod javaMethod = javaMethods.get(0);
      List<JavaRecord> javaRecords = queryJavaRecords(javaMethod);

      if (!javaRecords.isEmpty()) {
        JavaRecord javaRecord = javaRecords.get(0);

        return queryDefinitions(javaRecord.getAstNodeId());
      }
    } else {
      LOGGER.log(
        Level.WARNING,
          "Database query result was not expected to be empty. " +
          getCurrentPath() + ", line #" + getCurrentLineNumber()
      );
    }

    return new ArrayList<>();
  }

  private List<JavaAstNode> queryType(JavaAstNode javaAstNode) {
    List<JavaVariable> javaVariables = queryJavaVariables(javaAstNode);

    if (!javaVariables.isEmpty()) {
      JavaVariable javaVariable = javaVariables.get(0);
      List<JavaRecord> javaRecords = queryJavaRecords(javaVariable);

      if (!javaRecords.isEmpty()) {
        JavaRecord javaRecord = javaRecords.get(0);

        return queryDefinitions(javaRecord.getAstNodeId());
      }
    } else {
      LOGGER.log(
        Level.WARNING,
          "Database query result was not expected to be empty. " +
          getCurrentPath() + ", line #" + getCurrentLineNumber()
      );
    }

    return new ArrayList<>();
  }

  private List<JavaRecord> queryJavaRecords(JavaTypedEntity javaTypedEntity) {
    CriteriaQuery<JavaRecord> cr = cb.createQuery(JavaRecord.class);
    Root<JavaRecord> root = cr.from(JavaRecord.class);

    cr
      .select(root)
      .where(cb.equal(root.get("entityHash"), javaTypedEntity.getTypeHash()));

    return em.createQuery(cr).getResultList();
  }

  private List<JavaRecord> queryJavaRecords(JavaAstNode javaAstNode) {
    CriteriaQuery<JavaRecord> cr = cb.createQuery(JavaRecord.class);
    Root<JavaRecord> root = cr.from(JavaRecord.class);

    cr
      .select(root)
      .where(cb.equal(root.get("entityHash"), javaAstNode.getEntityHash()));


    return em.createQuery(cr).getResultList();
  }

  private List<JavaRecord> queryJavaRecords(
    JavaAstNode javaAstNode, CriteriaQuery<JavaRecord> cr,
    Root<JavaRecord> root, Predicate customPredicate)
  {
    Predicate entityHashPredicate = cb.equal(
      root.get("entityHash"), javaAstNode.getEntityHash()
    );

    cr.select(root).where(cb.and(entityHashPredicate, customPredicate));

    return em.createQuery(cr).getResultList();
  }

  private JavaAstNode queryJavaAstNode(long javaAstNodeId) {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);

    cr
      .select(root)
      .where(cb.equal(root.get("id"), javaAstNodeId));

    return em.createQuery(cr).getSingleResult();
  }

  private List<JavaAstNode> queryJavaAstNodes(JavaAstNode javaAstNode) {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);

    cr
      .select(root)
      .where(cb.equal(root.get("entityHash"), javaAstNode.getEntityHash()));

    return em.createQuery(cr).getResultList();
  }

  private <T extends Collection<? extends JavaEntity>>
  List<JavaAstNode> queryJavaAstNodes(
    T javaEntities)
  {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);

    cr
      .select(root)
      .where(
        root.get("id")
          .in(javaEntities.stream()
            .map(JavaEntity::getAstNodeId)
            .collect(Collectors.toList())
          )
      );

    return em.createQuery(cr).getResultList();
  }

  private List<JavaAstNode> queryJavaAstNodes(
    JavaAstNode javaAstNode, CriteriaQuery<JavaAstNode> cr,
    Root<JavaAstNode> root, Predicate customPredicate)
  {
    Predicate entityHashPredicate = cb.equal(
      root.get("entityHash"), javaAstNode.getEntityHash()
    );

    cr.select(root).where(cb.and(entityHashPredicate, customPredicate));

    return em.createQuery(cr).getResultList();
  }

   private int queryJavaAstNodeCount(JavaAstNode javaAstNode) {
    CriteriaQuery<Long> cr = cb.createQuery(Long.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);
    Predicate entityHashPredicate =
      cb.equal(root.get("entityHash"), javaAstNode.getEntityHash());

    cr.select(cb.count(root)).where(entityHashPredicate);

    return em.createQuery(cr).getSingleResult().intValue();
  }

  private int queryJavaAstNodeCount(
    JavaAstNode javaAstNode, CriteriaQuery<Long> cr,
    Root<JavaAstNode> root, Predicate customPredicate)
  {
    Predicate entityHashPredicate =
      cb.equal(root.get("entityHash"), javaAstNode.getEntityHash());

    cr
      .select(cb.count(root))
      .where(cb.and(entityHashPredicate, customPredicate));

    return em.createQuery(cr).getSingleResult().intValue();
  }

  private <T> List<JavaMethod> queryJavaMethods(JavaAstNode javaAstNode) {
    CriteriaQuery<JavaMethod> cr = cb.createQuery(JavaMethod.class);
    Root<JavaMethod> root = cr.from(JavaMethod.class);

    cr
      .select(root)
      .where(cb.equal(root.get("entityHash"), javaAstNode.getEntityHash()));

    return em.createQuery(cr).getResultList();
  }

  private List<JavaMethod> queryJavaMethods(
    JavaAstNode javaAstNode, CriteriaQuery<JavaMethod> cr,
    Root<JavaMethod> root, Predicate customPredicate)
  {
    Predicate entityHashPredicate = cb.equal(
      root.get("entityHash"), javaAstNode.getEntityHash()
    );

    cr.select(root).where(cb.and(entityHashPredicate, customPredicate));

    return em.createQuery(cr).getResultList();
  }

  private List<JavaVariable> queryJavaVariables(JavaAstNode javaAstNode) {
    CriteriaQuery<JavaVariable> cr = cb.createQuery(JavaVariable.class);
    Root<JavaVariable> root = cr.from(JavaVariable.class);

    cr
      .select(root)
      .where(cb.equal(root.get("entityHash"), javaAstNode.getEntityHash()));

    return em.createQuery(cr).getResultList();
  }

  private List<JavaVariable> queryJavaVariables(
    JavaAstNode javaAstNode, CriteriaQuery<JavaVariable> cr,
    Root<JavaVariable> root, Predicate customPredicate)
  {
    Predicate entityHashPredicate = cb.equal(
      root.get("entityHash"), javaAstNode.getEntityHash()
    );

    cr.select(root).where(cb.and(entityHashPredicate, customPredicate));

    return em.createQuery(cr).getResultList();
  }

  private List<JavaConstructor> queryJavaConstructors(JavaAstNode javaAstNode) {
    CriteriaQuery<JavaConstructor> cr =
      cb.createQuery(JavaConstructor.class);
    Root<JavaConstructor> root = cr.from(JavaConstructor.class);

    cr
      .select(root)
      .where(cb.equal(root.get("entityHash"), javaAstNode.getEntityHash()));


    return em.createQuery(cr).getResultList();
  }

  private List<JavaConstructor> queryJavaConstructors(
    JavaAstNode javaAstNode, CriteriaQuery<JavaConstructor> cr,
    Root<JavaConstructor> root, Predicate customPredicate)
  {
    Predicate entityHashPredicate = cb.equal(
      root.get("entityHash"), javaAstNode.getEntityHash()
    );

    cr.select(root).where(cb.and(entityHashPredicate, customPredicate));

    return em.createQuery(cr).getResultList();
  }

  private List<JavaEnumConstant> queryJavaEnumConstants(
    JavaAstNode javaAstNode)
  {
    CriteriaQuery<JavaEnumConstant> cr =
      cb.createQuery(JavaEnumConstant.class);
    Root<JavaEnumConstant> root = cr.from(JavaEnumConstant.class);

    cr
      .select(root)
      .where(cb.equal(root.get("entityHash"), javaAstNode.getEntityHash()));


    return em.createQuery(cr).getResultList();
  }

  private List<JavaEnumConstant> queryJavaEnumConstants(
    JavaAstNode javaAstNode, CriteriaQuery<JavaEnumConstant> cr,
    Root<JavaEnumConstant> root, Predicate customPredicate)
  {
    Predicate entityHashPredicate = cb.equal(
      root.get("entityHash"), javaAstNode.getEntityHash()
    );

    cr.select(root).where(cb.and(entityHashPredicate, customPredicate));

    return em.createQuery(cr).getResultList();
  }

  private static String getCurrentPath() {
    return new Throwable().getStackTrace()[1].getFileName();
  }

  private static int getCurrentLineNumber() {
    return new Throwable().getStackTrace()[1].getLineNumber();
  }
}