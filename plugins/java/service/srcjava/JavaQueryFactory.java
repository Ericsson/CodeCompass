package service.srcjava;

import cc.service.core.FilePosition;
import model.*;
import model.enums.AstType;
import model.enums.MemberTypeKind;
import model.enums.SymbolType;

import javax.persistence.EntityManager;
import javax.persistence.NoResultException;
import javax.persistence.criteria.*;
import java.util.*;
import java.util.logging.Level;
import java.util.stream.Collectors;

import static logger.Logger.LOGGER;
import static model.EMFactory.createEntityManager;

public abstract class JavaQueryFactory {
  private static final EntityManager em =
    createEntityManager(System.getProperty("rawDbContext"));
  private static final CriteriaBuilder cb = em.getCriteriaBuilder();

  public static List<JavaAstNode> queryJavaAstNodeByPosition(
    FilePosition fpos)
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

    return em.createQuery(cr).getResultList();
  }

  public static List<JavaAstNode> queryDefinitions(JavaAstNode javaAstNode) {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);
    Predicate predicate =
      cb.equal(root.get("astType"), AstType.DEFINITION);

    return queryJavaAstNodes(javaAstNode, cr, root, predicate);
  }

  public static List<JavaAstNode> queryDefinitions(long javaAstNodeId) {
    JavaAstNode javaAstNode = queryJavaAstNode(javaAstNodeId);

    return queryDefinitions(javaAstNode);
  }

  public static List<JavaMemberType> queryJavaMemberTypes(
    JavaAstNode recordJavaAstNode)
  {
    CriteriaQuery<JavaMemberType> cr = cb.createQuery(JavaMemberType.class);
    Root<JavaMemberType> root = cr.from(JavaMemberType.class);

    cr
      .select(root)
      .where(cb.equal(root.get("entityHash"), recordJavaAstNode.getEntityHash()));

    return em.createQuery(cr).getResultList();
  }

  public static List<JavaMemberType> queryJavaMemberTypes(
    JavaAstNode recordJavaAstNode, CriteriaQuery<JavaMemberType> cr,
    Root<JavaMemberType> root, Predicate customPredicate)
  {
    Predicate entityHashPredicate = cb.equal(
      root.get("typeHash"), recordJavaAstNode.getEntityHash()
    );

    cr.select(root).where(cb.and(entityHashPredicate, customPredicate));

    return em.createQuery(cr).getResultList();
  }

  public static List<JavaMemberType> queryJavaMemberTypes(
    JavaAstNode javaAstNode, JavaAstNode definition,
    MemberTypeKind memberTypeKind)
  {
    CriteriaQuery<JavaMemberType> cr = cb.createQuery(JavaMemberType.class);
    Root<JavaMemberType> root = cr.from(JavaMemberType.class);

    Path<Long> memberAstNodeId = root.get("memberAstNode").get("id");
    Path<Integer> kind = root.get("kind");

    Predicate idEqualNodeOrDefId =
      cb.or(
        cb.equal(memberAstNodeId, definition.getId()),
        cb.equal(memberAstNodeId, javaAstNode.getId())
      );
    Predicate kindEqualParameter = cb.equal(kind, memberTypeKind);

    cr
      .select(root)
      .where(cb.and(idEqualNodeOrDefId, kindEqualParameter));

    return em.createQuery(cr).getResultList();
  }

  public static List<JavaAstNode> queryVisibleDeclarations(
    JavaAstNode javaAstNode)
  {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);
    Predicate predicate =
      cb.and(
        cb.equal(root.get("astType"), AstType.DECLARATION),
        cb.isTrue(root.get("visibleInSourceCode"))
      );

    return queryJavaAstNodes(javaAstNode, cr, root, predicate);
  }

  public static List<JavaAstNode> queryUsages(JavaAstNode javaAstNode) {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);
    Predicate predicate = cb.equal(root.get("astType"), AstType.USAGE);

    return queryJavaAstNodes(javaAstNode, cr, root, predicate);
  }

  public static List<JavaAstNode> queryCallees(JavaAstNode javaAstNode) {
    List<JavaAstNode> calls = queryCalls(javaAstNode);

    return calls.stream()
      .flatMap(c -> queryDefinitions(c).stream())
      .collect(Collectors.toList());
  }

  public static List<JavaAstNode> queryCallers(JavaAstNode javaAstNode) {
    List<JavaAstNode> usages = queryUsages(javaAstNode);

    return usages.stream()
      .flatMap(u -> queryCaller(u).stream())
      .collect(Collectors.toList());
  }

  public static List<JavaAstNode> queryCaller(JavaAstNode usage) {
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

  public static List<JavaAstNode> queryCalls(JavaAstNode javaAstNode) {
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

  public static List<JavaAstNode> queryParameters(JavaAstNode javaAstNode) {
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

  public static List<JavaAstNode> queryLocalVars(JavaAstNode javaAstNode) {
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

  public static List<JavaAstNode> queryReturnType(JavaAstNode javaAstNode) {
    List<JavaMethod> javaMethods = queryJavaMethods(javaAstNode);

    if (!javaMethods.isEmpty()) {
      JavaMethod javaMethod = javaMethods.get(0);
      List<JavaRecord> javaRecords = queryJavaRecords(javaMethod);

      if (!javaRecords.isEmpty()) {
        JavaRecord javaRecord = javaRecords.get(0);

        return queryDefinitions(javaRecord.getAstNodeId());
      }
    }

    return new ArrayList<>();
  }

  public static List<JavaAstNode> queryType(JavaAstNode javaAstNode) {
    List<JavaVariable> javaVariables = queryJavaVariables(javaAstNode);

    if (!javaVariables.isEmpty()) {
      JavaVariable javaVariable = javaVariables.get(0);
      List<JavaRecord> javaRecords = queryJavaRecords(javaVariable);

      if (!javaRecords.isEmpty()) {
        JavaRecord javaRecord = javaRecords.get(0);

        return queryDefinitions(javaRecord.getAstNodeId());
      }
    }

    return new ArrayList<>();
  }

  public static List<JavaRecord> queryJavaRecords(
    JavaTypedEntity javaTypedEntity)
  {
    CriteriaQuery<JavaRecord> cr = cb.createQuery(JavaRecord.class);
    Root<JavaRecord> root = cr.from(JavaRecord.class);

    cr
      .select(root)
      .where(cb.equal(root.get("entityHash"), javaTypedEntity.getTypeHash()));

    return em.createQuery(cr).getResultList();
  }

  public static List<JavaRecord> queryJavaRecords(JavaAstNode javaAstNode) {
    CriteriaQuery<JavaRecord> cr = cb.createQuery(JavaRecord.class);
    Root<JavaRecord> root = cr.from(JavaRecord.class);

    cr
      .select(root)
      .where(cb.equal(root.get("entityHash"), javaAstNode.getEntityHash()));

    List<JavaRecord> javaRecords = em.createQuery(cr).getResultList();

    if (javaRecords.isEmpty()) {
      LOGGER.log(
        Level.WARNING,
          "Database query result was not expected to be empty. " +
          getCurrentPath() + ", line #" + getCurrentLineNumber()
      );
    }

    return javaRecords;
  }

  public static List<JavaRecord> queryJavaRecords(
    JavaAstNode javaAstNode, CriteriaQuery<JavaRecord> cr,
    Root<JavaRecord> root, Predicate customPredicate)
  {
    Predicate entityHashPredicate = cb.equal(
      root.get("entityHash"), javaAstNode.getEntityHash()
    );

    cr.select(root).where(cb.and(entityHashPredicate, customPredicate));

    return em.createQuery(cr).getResultList();
  }

  public static JavaAstNode queryJavaAstNode(long javaAstNodeId) {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);

    cr
      .select(root)
      .where(cb.equal(root.get("id"), javaAstNodeId));

    return em.createQuery(cr).getSingleResult();
  }

  public static List<JavaAstNode> queryJavaAstNodes(JavaAstNode javaAstNode) {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);

    cr
      .select(root)
      .where(cb.equal(root.get("entityHash"), javaAstNode.getEntityHash()));

    return em.createQuery(cr).getResultList();
  }

  public static <T extends Collection<? extends JavaEntity>>
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

  public static List<JavaAstNode> queryJavaAstNodes(
    JavaAstNode javaAstNode, CriteriaQuery<JavaAstNode> cr,
    Root<JavaAstNode> root, Predicate customPredicate)
  {
    Predicate entityHashPredicate = cb.equal(
      root.get("entityHash"), javaAstNode.getEntityHash()
    );

    cr.select(root).where(cb.and(entityHashPredicate, customPredicate));

    return em.createQuery(cr).getResultList();
  }

  public static List<JavaMethod> queryJavaMethods(JavaAstNode javaAstNode) {
    CriteriaQuery<JavaMethod> cr = cb.createQuery(JavaMethod.class);
    Root<JavaMethod> root = cr.from(JavaMethod.class);

    cr
      .select(root)
      .where(cb.equal(root.get("entityHash"), javaAstNode.getEntityHash()));

    List<JavaMethod> javaMethods = em.createQuery(cr).getResultList();

    if (javaMethods.isEmpty()) {
      LOGGER.log(
        Level.WARNING,
          "Database query result was not expected to be empty. " +
          getCurrentPath() + ", line #" + getCurrentLineNumber()
      );
    }

    return javaMethods;
  }

  public static List<JavaMethod> queryJavaMethods(
    JavaAstNode javaAstNode, CriteriaQuery<JavaMethod> cr,
    Root<JavaMethod> root, Predicate customPredicate)
  {
    Predicate entityHashPredicate = cb.equal(
      root.get("entityHash"), javaAstNode.getEntityHash()
    );

    cr.select(root).where(cb.and(entityHashPredicate, customPredicate));

    return em.createQuery(cr).getResultList();
  }

  public static List<JavaVariable> queryJavaVariables(JavaAstNode javaAstNode) {
    CriteriaQuery<JavaVariable> cr = cb.createQuery(JavaVariable.class);
    Root<JavaVariable> root = cr.from(JavaVariable.class);

    cr
      .select(root)
      .where(cb.equal(root.get("entityHash"), javaAstNode.getEntityHash()));

    List<JavaVariable> javaVariables = em.createQuery(cr).getResultList();

    if (javaVariables.isEmpty()) {
      LOGGER.log(
        Level.WARNING,
          "Database query result was not expected to be empty. " +
          getCurrentPath() + ", line #" + getCurrentLineNumber()
      );
    }

    return javaVariables;
  }

  public static List<JavaVariable> queryJavaVariables(
    JavaAstNode javaAstNode, CriteriaQuery<JavaVariable> cr,
    Root<JavaVariable> root, Predicate customPredicate)
  {
    Predicate entityHashPredicate = cb.equal(
      root.get("entityHash"), javaAstNode.getEntityHash()
    );

    cr.select(root).where(cb.and(entityHashPredicate, customPredicate));

    return em.createQuery(cr).getResultList();
  }

  public static List<JavaConstructor> queryJavaConstructors(
    JavaAstNode javaAstNode)
  {
    CriteriaQuery<JavaConstructor> cr =
      cb.createQuery(JavaConstructor.class);
    Root<JavaConstructor> root = cr.from(JavaConstructor.class);

    cr
      .select(root)
      .where(cb.equal(root.get("entityHash"), javaAstNode.getEntityHash()));

    List<JavaConstructor> javaConstructors = em.createQuery(cr).getResultList();

    if (javaConstructors.isEmpty()) {
      LOGGER.log(
        Level.WARNING,
          "Database query result was not expected to be empty. " +
          getCurrentPath() + ", line #" + getCurrentLineNumber()
      );
    }

    return javaConstructors;
  }

  public static List<JavaConstructor> queryJavaConstructors(
    JavaAstNode javaAstNode, CriteriaQuery<JavaConstructor> cr,
    Root<JavaConstructor> root, Predicate customPredicate)
  {
    Predicate entityHashPredicate = cb.equal(
      root.get("entityHash"), javaAstNode.getEntityHash()
    );

    cr.select(root).where(cb.and(entityHashPredicate, customPredicate));

    return em.createQuery(cr).getResultList();
  }

  public static List<JavaEnumConstant> queryJavaEnumConstants(
    JavaAstNode javaAstNode)
  {
    CriteriaQuery<JavaEnumConstant> cr =
      cb.createQuery(JavaEnumConstant.class);
    Root<JavaEnumConstant> root = cr.from(JavaEnumConstant.class);

    cr
      .select(root)
      .where(cb.equal(root.get("entityHash"), javaAstNode.getEntityHash()));

    List<JavaEnumConstant> javaEnumConstants =
      em.createQuery(cr).getResultList();

    if (javaEnumConstants.isEmpty()) {
      LOGGER.log(
        Level.WARNING,
          "Database query result was not expected to be empty. " +
          getCurrentPath() + ", line #" + getCurrentLineNumber()
      );
    }

    return  javaEnumConstants;
  }

  public static List<JavaEnumConstant> queryJavaEnumConstants(
    JavaAstNode javaAstNode, CriteriaQuery<JavaEnumConstant> cr,
    Root<JavaEnumConstant> root, Predicate customPredicate)
  {
    Predicate entityHashPredicate = cb.equal(
      root.get("entityHash"), javaAstNode.getEntityHash()
    );

    cr.select(root).where(cb.and(entityHashPredicate, customPredicate));

    return em.createQuery(cr).getResultList();
  }

  public static List<JavaMemberType> queryJavaMemberTypes(
    JavaAstNode recordJavaAstNode, MemberTypeKind memberTypeKind)
  {
    CriteriaQuery<JavaMemberType> cr = cb.createQuery(JavaMemberType.class);
    Root<JavaMemberType> root = cr.from(JavaMemberType.class);
    Predicate predicate =
      cb.equal(root.get("kind"), memberTypeKind);

    return queryJavaMemberTypes(recordJavaAstNode, cr, root, predicate);
  }

  public static List<JavaDocComment> queryJavaDocComments(
    JavaAstNode javaAstNode)
  {
    CriteriaQuery<JavaDocComment> cr =
      cb.createQuery(JavaDocComment.class);
    Root<JavaDocComment> root = cr.from(JavaDocComment.class);

    cr
      .select(root)
      .where(cb.equal(root.get("entityHash"), javaAstNode.getEntityHash()));

    return em.createQuery(cr).getResultList();
  }

  public static List<JavaAstNode> getJavaAstNodesFromMemberTypes(
    List<JavaMemberType> javaMemberTypes)
  {
    return javaMemberTypes.stream()
      .map(JavaMemberType::getMemberAstNode)
      .collect(Collectors.toList());
  }

  private static String getCurrentPath() {
    return new Throwable().getStackTrace()[1].getFileName();
  }

  private static int getCurrentLineNumber() {
    return new Throwable().getStackTrace()[1].getLineNumber();
  }
}
