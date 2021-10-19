package service.srcjava;

import cc.service.core.*;
import cc.service.java.JavaService;
import cc.service.language.AstNodeInfo;
import model.*;
import model.enums.AstType;
import org.apache.thrift.TException;
import service.srcjava.enums.ReferenceType;

import javax.persistence.EntityManager;
import javax.persistence.NoResultException;
import javax.persistence.criteria.*;

import java.util.*;
import java.util.logging.Level;

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
        cb.lt(startLine, line));
    Predicate posLessThanEndPos =
      cb.or(
        cb.and(cb.equal(endLine, line), cb.gt(endColumn, column)),
        cb.gt(endLine, line));

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
        CriteriaQuery<JavaVariable> cr = cb.createQuery(JavaVariable.class);
        Root<JavaVariable> root = cr.from(JavaVariable.class);

        cr
          .select(root)
          .where(cb.equal(root.get("entityHash"), javaAstNode.getEntityHash()));

        List<JavaVariable> javaVariables = em.createQuery(cr).getResultList();

        if (!javaVariables.isEmpty()) {
          JavaVariable javaVariable = javaVariables.get(0);

          properties.put("Name", javaVariable.getName());
          properties.put("Qualified name", javaVariable.getQualifiedName());
          properties.put("Type", javaVariable.getQualifiedType());

          return properties;
        } else {
          LOGGER.log(
            Level.WARNING,
            "Database query result was not expected to be empty. " +
              getCurrentPath() + ", line #" + getCurrentLineNumber()
          );
        }
      }
      case CONSTRUCTOR: {
        CriteriaQuery<JavaConstructor> cr =
          cb.createQuery(JavaConstructor.class);
        Root<JavaConstructor> root = cr.from(JavaConstructor.class);

        cr
          .select(root)
          .where(cb.equal(root.get("entityHash"), javaAstNode.getEntityHash()));


        List<JavaConstructor> javaConstructors = em.createQuery(cr).getResultList();

        if (!javaConstructors.isEmpty()) {
          JavaConstructor javaConstructor = javaConstructors.get(0);

          properties.put("Name", javaConstructor.getName());
          properties.put("Qualified name", javaConstructor.getQualifiedName());

          return properties;
        } else {
          LOGGER.log(
            Level.WARNING,
            "Database query result was not expected to be empty. " +
              getCurrentPath() + ", line #" + getCurrentLineNumber()
          );
        }
      }
      case METHOD: {
        CriteriaQuery<JavaMethod> cr = cb.createQuery(JavaMethod.class);
        Root<JavaMethod> root = cr.from(JavaMethod.class);

        cr
          .select(root)
          .where(cb.equal(root.get("entityHash"), javaAstNode.getEntityHash()));


        List<JavaMethod> javaMethods = em.createQuery(cr).getResultList();

        if (!javaMethods.isEmpty()) {
          JavaMethod javaMethod = javaMethods.get(0);

          properties.put("Name", javaMethod.getName());
          properties.put("Qualified name", javaMethod.getQualifiedName());
          properties.put("Type", javaMethod.getQualifiedType());

          return properties;
        } else {
          LOGGER.log(
            Level.WARNING,
            "Database query result was not expected to be empty. " +
              getCurrentPath() + ", line #" + getCurrentLineNumber()
          );
        }
      }
      // case TYPE: {
      //   break;
      // }
      case ENUM_CONSTANT: {
        CriteriaQuery<JavaEnumConstant> cr =
          cb.createQuery(JavaEnumConstant.class);
        Root<JavaEnumConstant> root = cr.from(JavaEnumConstant.class);

        cr
          .select(root)
          .where(cb.equal(root.get("entityHash"), javaAstNode.getEntityHash()));


        List<JavaEnumConstant> javaEnumConstants =
          em.createQuery(cr).getResultList();

        if (!javaEnumConstants.isEmpty()) {
          JavaEnumConstant javaEnumConstant = javaEnumConstants.get(0);

          properties.put("Name", javaEnumConstant.getName());
          properties.put("Qualified name", javaEnumConstant.getQualifiedName());
          properties.put("Value", javaEnumConstant.getValue());

          return properties;
        } else {
          LOGGER.log(
            Level.WARNING,
            "Database query result was not expected to be empty. " +
              getCurrentPath() + ", line #" + getCurrentLineNumber()
          );
        }
      }
    }

    return properties;
  }

  @Override
  public String getDocumentation(String javaAstNodeId) {
    return " ";
  }

  @Override
  public int getReferenceCount(String javaAstNodeId, int referenceId) {
    long javaAstNodeIdLong = Long.parseLong(javaAstNodeId);
    JavaAstNode javaAstNode = queryJavaAstNode(javaAstNodeIdLong);

    switch (ReferenceType.values()[referenceId]) {
      case DEFINITION: {
        CriteriaQuery<Long> cr = cb.createQuery(Long.class);
        Root<JavaAstNode> root = cr.from(JavaAstNode.class);
        Predicate predicate =
          cb.equal(root.get("astType"), AstType.DEFINITION);

        return queryJavaAstNodeCount(javaAstNode, cr, root, predicate);
      }
      case DECLARATION: {
        CriteriaQuery<Long> cr = cb.createQuery(Long.class);
        Root<JavaAstNode> root = cr.from(JavaAstNode.class);
        Predicate predicate =
          cb.and(
            cb.equal(root.get("astType"), AstType.DECLARATION),
            cb.isTrue(root.get("visibleInSourceCode"))
          );

        return queryJavaAstNodeCount(javaAstNode, cr, root, predicate);
      }
      case USAGE: {
        CriteriaQuery<Long> cr = cb.createQuery(Long.class);
        Root<JavaAstNode> root = cr.from(JavaAstNode.class);
        Predicate predicate =
          cb.equal(root.get("astType"), AstType.USAGE);

        return queryJavaAstNodeCount(javaAstNode, cr, root, predicate);
      }
      case THIS_CALLS: {
        break;
      }
      case CALLS_OF_THIS: {
        CriteriaQuery<Long> cr = cb.createQuery(Long.class);
        Root<JavaAstNode> root = cr.from(JavaAstNode.class);
        Predicate predicate =
          cb.equal(root.get("astType"), AstType.USAGE);

        return queryJavaAstNodeCount(javaAstNode, cr, root, predicate);
      }
      case CALLEE: {
        break;
      }
      case CALLER: {
        break;
      }
      case PARAMETER: {
        CriteriaQuery<Long> cr = cb.createQuery(Long.class);
        Root<JavaMethod> root = cr.from(JavaMethod.class);

        cr
          .select(cb.count(root.get("javaMetVarParams")))
          .where(cb.equal(root.get("astNodeId"), javaAstNode.getId()));

        return em.createQuery(cr).getSingleResult().intValue();
      }
      case LOCAL_VAR: {
        CriteriaQuery<Long> cr = cb.createQuery(Long.class);
        Root<JavaMethod> root = cr.from(JavaMethod.class);

        cr
          .select(cb.count(root.get("javaMetVarLocals")))
          .where(cb.equal(root.get("astNodeId"), javaAstNode.getId()));

        return em.createQuery(cr).getSingleResult().intValue();
      }
      case RETURN_TYPE: {
        break;
      }
      case OVERRIDE: {
        break;
      }
      case OVERRIDDEN_BY: {
        break;
      }
      case READ: {
        break;
      }
      case WRITE: {
        break;
      }
      case TYPE: {
        break;
      }
      case INHERIT_FROM: {
        break;
      }
      case INHERIT_BY: {
        break;
      }
      case DATA_MEMBER: {
        break;
      }
      case METHOD: {
        break;
      }
      case ENUM_CONSTANTS: {
        break;
      }
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
    List<AstNodeInfo> javaAstNodeInfos = new ArrayList<>();
    JavaAstNode javaAstNode = queryJavaAstNode(javaAstNodeIdLong);

    switch (ReferenceType.values()[referenceId]) {
      case DEFINITION: {
        List<JavaAstNode> javaAstNodes = queryDefinitions(javaAstNode);
        javaAstNodeInfos = createAstNodeInfos(javaAstNodes);
        break;
      }
      case DECLARATION: {
        CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
        Root<JavaAstNode> root = cr.from(JavaAstNode.class);
        Predicate predicate =
          cb.and(
            cb.equal(root.get("astType"), AstType.DECLARATION),
            cb.isTrue(root.get("visibleInSourceCode"))
          );

        List<JavaAstNode> javaAstNodes =
          queryJavaAstNodes(javaAstNode, cr, root, predicate);
        javaAstNodeInfos = createAstNodeInfos(javaAstNodes);
        break;
      }
      case USAGE: {
        CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
        Root<JavaAstNode> root = cr.from(JavaAstNode.class);
        Predicate predicate =
          cb.equal(root.get("astType"), AstType.USAGE);

        List<JavaAstNode> javaAstNodes =
          queryJavaAstNodes(javaAstNode, cr, root, predicate);
        javaAstNodeInfos = createAstNodeInfos(javaAstNodes);
        break;
      }
      case THIS_CALLS: {
        break;
      }
      case CALLS_OF_THIS: {
        break;
      }
      case CALLEE: {
        break;
      }
      case CALLER: {
        break;
      }
      case PARAMETER: {
        CriteriaQuery<JavaEntity> cr = cb.createQuery(JavaEntity.class);
        Root<JavaEntity> root = cr.from(JavaEntity.class);
        cr
          .select(root)
          .where(cb.equal(root.get("entityHash"), javaAstNode.getEntityHash()));

        try {
          JavaEntity javaEntity = em.createQuery(cr).getSingleResult();

          if (javaEntity instanceof JavaMethod) {
            javaAstNodeInfos =
              createAstNodeInfos(
                ((JavaMethod) javaEntity).getJavaMetVarParams());
          } else if (javaEntity instanceof JavaConstructor) {
            javaAstNodeInfos =
              createAstNodeInfos(
                ((JavaConstructor) javaEntity).getJavaConVarLocals());
          }
        } catch (NoResultException e) {
          LOGGER.log(
            Level.WARNING,
            "Database query result was not expected to be empty. " +
              getCurrentPath() + ", line #" + getCurrentLineNumber()
          );
        }
        break;
      }
      case LOCAL_VAR: {
        break;
      }
      case RETURN_TYPE: {
        break;
      }
      case OVERRIDE: {
        break;
      }
      case OVERRIDDEN_BY: {
        break;
      }
      case READ: {
        break;
      }
      case WRITE: {
        break;
      }
      case TYPE: {
        break;
      }
      case INHERIT_FROM: {
        break;
      }
      case INHERIT_BY: {
        break;
      }
      case DATA_MEMBER: {
        break;
      }
      case METHOD: {
        break;
      }
      case ENUM_CONSTANTS: {
        break;
      }
    }

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

  private List<AstNodeInfo> createAstNodeInfos(List<JavaAstNode> javaAstNodes) {
    List<AstNodeInfo> javaAstNodeInfos = new ArrayList<>();

    javaAstNodes.forEach(p -> javaAstNodeInfos.add(createAstNodeInfo(p)));

    return javaAstNodeInfos;
  }

  private <T extends Collection<? extends JavaEntity>>
  List<AstNodeInfo> createAstNodeInfos(
    T collection)
  {
    List<AstNodeInfo> javaAstNodeInfos = new ArrayList<>();

    collection.forEach(
      p -> javaAstNodeInfos.add(
        createAstNodeInfo(
          queryJavaAstNode(p.getAstNodeId())
        )
      )
    );

    return javaAstNodeInfos;
  }

  private AstNodeInfo createAstNodeInfo(JavaAstNode javaAstNode) {
    AstNodeInfo astNodeInfo = new AstNodeInfo();
    FileRange fileRange = new FileRange();
    Range range = new Range();
    Position startPosition = new Position();
    Position endPosition = new Position();

    startPosition.line = (int) javaAstNode.getLocation_range_start_line();
    startPosition.column = (int) javaAstNode.getLocation_range_end_column();
    endPosition.line = (int) javaAstNode.getLocation_range_end_line();
    endPosition.column = (int) javaAstNode.getLocation_range_end_column();

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

  private static String getCurrentPath() {
    return new Throwable().getStackTrace()[1].getFileName();
  }

  private static int getCurrentLineNumber() {
    return new Throwable().getStackTrace()[1].getLineNumber();
  }
}