package service.srcjava;

import cc.service.core.*;
import cc.service.java.JavaService;
import cc.service.language.AstNodeInfo;
import model.JavaAstNode;
import model.JavaEntity;
import model.JavaMethod;
import model.enums.AstType;
import model.enums.SymbolType;
import org.apache.thrift.TException;
import service.srcjava.enums.ReferenceType;

import javax.persistence.EntityManager;
import javax.persistence.NoResultException;
import javax.persistence.Query;
import javax.persistence.TypedQuery;
import javax.persistence.criteria.CriteriaBuilder;
import javax.persistence.criteria.CriteriaQuery;
import javax.persistence.criteria.Predicate;
import javax.persistence.criteria.Root;

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
    TypedQuery<JavaAstNode> queryJavaAstNodeByPosition = em.createQuery(
      "SELECT " +
          "n " +
        "from " +
          "JavaAstNode n " +
        "where " +
          "n.location_file = :fileId and " +
          // StartPos <= Pos
          "((n.location_range_start_line = :line and " +
            "n.location_range_start_column <= :column) or " +
           "n.location_range_start_line < :line) and " +
          // Pos < EndPos
          "((n.location_range_end_line = :line and " +
            "n.location_range_end_column > :column) or " +
           "n.location_range_end_line > :line)",
      JavaAstNode.class
    )
      .setParameter("fileId", fileId)
      .setParameter("line", fpos.pos.line)
      .setParameter("column", fpos.pos.column);

    List<JavaAstNode> javaAstNodes = queryJavaAstNodeByPosition.getResultList();
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
  public int getReferenceCount(String javaAstNodeId, int referenceId) {
    long javaAstNodeIdLong = Long.parseLong(javaAstNodeId);
    JavaAstNode javaAstNode = queryJavaAstNode(javaAstNodeIdLong);

    switch (ReferenceType.values()[referenceId])
    {
      case DEFINITION:
        break;
      case DECLARATION:
        break;
      case USAGE:
        CriteriaQuery<Long> cr = cb.createQuery(Long.class);
        Root<JavaAstNode> root = cr.from(JavaAstNode.class);
        Predicate predicate =
          cb.equal(root.get("astType"), AstType.USAGE);

        return queryJavaAstNodeCount(javaAstNode, cr, root, predicate);
      case THIS_CALLS:
        break;
      case CALLS_OF_THIS:
        break;
      case CALLEE:
        break;
      case CALLER:
        break;
      case PARAMETER:
        TypedQuery<Long> queryParametersCount =
          em.createQuery(
            "SELECT " +
                "COUNT(m.javaMetVarParams) " +
              "FROM " +
                "JavaMethod m " +
              "WHERE " +
                "m.astNodeId = :nId",
            Long.class
          ).setParameter("nId", javaAstNode.getId());
        return queryParametersCount.getSingleResult().intValue();
      case LOCAL_VAR:
        break;
      case RETURN_TYPE:
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

    switch (javaAstNode.getSymbolType())
    {
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

    switch (ReferenceType.values()[referenceId])
    {
      case DEFINITION:
        break;
      case DECLARATION:
        break;
      case USAGE:
        CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
        Root<JavaAstNode> root = cr.from(JavaAstNode.class);
        Predicate predicate =
          cb.equal(root.get("astType"), AstType.USAGE);

        List<JavaAstNode> javaAstNodes =
          queryJavaAstNodes(javaAstNode, cr, root, predicate);
        javaAstNodeInfos = createAstNodeInfos(javaAstNodes);
        break;
      case THIS_CALLS:
        break;
      case CALLS_OF_THIS:
        break;
      case CALLEE:
        break;
      case CALLER:
        break;
      case PARAMETER:
        TypedQuery<JavaMethod> queryJavaMethod =
          em.createQuery(
            "SELECT m FROM JavaMethod m WHERE m.entityHash = :entityHash",
            JavaMethod.class
          ).setParameter("entityHash", javaAstNode.getEntityHash());

        try {
          JavaMethod javaMethod = queryJavaMethod.getSingleResult();
          javaAstNodeInfos =
            createAstNodeInfos(javaMethod.getJavaMetVarParams());
        } catch (NoResultException e) {
          LOGGER.log(
            Level.WARNING,
            "Database query result was not expected to be empty. " +
              getCurrentPath() + ", line #" + getCurrentLineNumber()
          );
        }
        break;
      case LOCAL_VAR:
        break;
      case RETURN_TYPE:
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

    javaAstNodeInfos.sort(
      Comparator.comparingInt(n -> n.range.range.endpos.column)
    );

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

  private JavaAstNode queryJavaAstNode(long javaAstNodeId) {
    TypedQuery<JavaAstNode> queryJavaAstNode = em.createQuery(
      "SELECT n from JavaAstNode n where n.id = :id",
      JavaAstNode.class
    ).setParameter("id", javaAstNodeId);

    return queryJavaAstNode.getSingleResult();
  }

  private List<JavaAstNode> queryJavaAstNodes(long javaAstNodeId) {
    JavaAstNode javaAstNode = queryJavaAstNode(javaAstNodeId);
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