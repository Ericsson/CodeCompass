package service.srcjava;

import cc.service.core.FilePosition;
import cc.service.core.FileRange;
import model.*;
import model.enums.AstType;
import model.enums.MemberTypeKind;
import model.enums.RelationKind;
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
    Path<Long> startLine =
      root.get("location_range_start_line");
    Path<Long> endLine =
      root.get("location_range_end_line");
    Path<Long> startColumn =
      root.get("location_range_start_column");
    Path<Long> endColumn =
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

  public static List<JavaAstNode> queryJavaAstNodesByFileRange(
    FileRange fileRange)
  {
    long fileId = Long.parseUnsignedLong(fileRange.file);
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);

    long rStartLine = fileRange.range.startpos.line;
    long rEndLine = fileRange.range.endpos.line;

    Path<Long> locationFile =
      root.get("location_file");
    Path<Long> startLine =
      root.get("location_range_start_line");
    Path<Long> endLine =
      root.get("location_range_end_line");
    Path<Boolean> visibility = root.get("visibleInSourceCode");

    Predicate sameFile = cb.equal(locationFile, fileId);
    Predicate startLineGreaterEqualLine = cb.ge(startLine, rStartLine);
    Predicate endLineLessThanLine = cb.lt(endLine, rEndLine);
    Predicate endLineNotEqualInitial = cb.notEqual(endLine, (long) -1);
    Predicate visibleInSourceCode = cb.isTrue(visibility);

    cr
      .select(root)
      .where(
        cb.and(
          sameFile, startLineGreaterEqualLine, endLineLessThanLine,
          endLineNotEqualInitial, visibleInSourceCode
        )
      );

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

  public static List<JavaAstNode> queryJavaAstNodes(long entityHash) {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);

    cr
      .select(root)
      .where(cb.equal(root.get("entityHash"), entityHash));

    return em.createQuery(cr).getResultList();
  }

  public static <T extends Collection<? extends JavaEntity>>
  List<JavaAstNode> queryJavaAstNodes(T javaEntities) {
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

  public static List<JavaAstNode> queryJavaAstNodes(
    long entityHash, CriteriaQuery<JavaAstNode> cr,
    Root<JavaAstNode> root, Predicate customPredicate)
  {
    Predicate entityHashPredicate = cb.equal(
      root.get("entityHash"), entityHash
    );

    cr.select(root).where(cb.and(entityHashPredicate, customPredicate));

    return em.createQuery(cr).getResultList();
  }

  public static List<JavaAstNode> queryJavaAstNodesInFile(long fileId) {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);

    cr
      .select(root)
      .where(cb.equal(root.get("location_file"), fileId));

    return em.createQuery(cr).getResultList();
  }

  public static List<JavaAstNode> queryJavaAstNodesInFile(
    long fileId, CriteriaQuery<JavaAstNode> cr,
    Root<JavaAstNode> root, Predicate customPredicate)
  {
    Predicate entityHashPredicate = cb.equal(
      root.get("location_file"), fileId
    );

    cr.select(root).where(cb.and(entityHashPredicate, customPredicate));

    return em.createQuery(cr).getResultList();
  }

  public static List<JavaAstNode> queryJavaImportNodesInFile(long fileId) {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);

    Predicate predicate = cb.equal(root.get("symbolType"), SymbolType.FILE);

    return queryJavaAstNodesInFile(fileId, cr, root, predicate);
  }

  public static List<JavaAstNode> queryJavaTypeNodesInFile(long fileId) {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);

    Predicate predicate =
      cb.and(
        cb.equal(root.get("symbolType"), SymbolType.TYPE),
        cb.equal(root.get("astType"), AstType.DEFINITION)
      );

    return queryJavaAstNodesInFile(fileId, cr, root, predicate);
  }

  public static List<JavaAstNode> queryJavaConstructorNodesInFile(long fileId) {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);

    Predicate predicate =
      cb.and(
        cb.equal(root.get("symbolType"), SymbolType.CONSTRUCTOR),
        cb.equal(root.get("astType"), AstType.DEFINITION)
      );

    return queryJavaAstNodesInFile(fileId, cr, root, predicate);
  }

  public static List<JavaAstNode> queryJavaMethodNodesInFile(long fileId) {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);

    Predicate predicate =
      cb.and(
        cb.equal(root.get("symbolType"), SymbolType.METHOD),
        cb.or(
          cb.equal(root.get("astType"), AstType.DEFINITION),
          cb.equal(root.get("astType"), AstType.DECLARATION)
        )
      );

    return queryJavaAstNodesInFile(fileId, cr, root, predicate);
  }

  public static List<JavaAstNode> queryJavaMemberTypeDefinitionNodes(
    JavaAstNode javaAstNode, MemberTypeKind memberTypeKind)
  {
    List<JavaMemberType> javaMemberTypes =
      queryJavaMemberTypes(javaAstNode, memberTypeKind);

    return javaMemberTypes.stream()
      .map(JavaMemberType::getMemberAstNode)
      .filter(
        m ->
          m.getAstType() == AstType.DECLARATION ||
          m.getAstType() == AstType.DEFINITION)
      .collect(Collectors.toList());
  }

  public static List<JavaAstNode> queryDefinitionNodes(JavaAstNode javaAstNode)
  {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);
    Predicate predicate =
      cb.equal(root.get("astType"), AstType.DEFINITION);

    return queryJavaAstNodes(javaAstNode, cr, root, predicate);
  }

  public static List<JavaAstNode> queryDefinitionNodes(long entityHash)
  {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);
    Predicate predicate =
      cb.equal(root.get("astType"), AstType.DEFINITION);

    return queryJavaAstNodes(entityHash, cr, root, predicate);
  }

  public static List<JavaAstNode> queryDeclOrDefNodes(JavaAstNode javaAstNode)
  {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);
    Predicate predicate =
      cb.or(
        cb.equal(root.get("astType"), AstType.DECLARATION),
        cb.equal(root.get("astType"), AstType.DEFINITION)
      );

    return queryJavaAstNodes(javaAstNode, cr, root, predicate);
  }

  public static List<JavaAstNode> queryDeclOrDefNodes(long entityHash)
  {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);
    Predicate predicate =
      cb.or(
        cb.equal(root.get("astType"), AstType.DECLARATION),
        cb.equal(root.get("astType"), AstType.DEFINITION)
      );

    return queryJavaAstNodes(entityHash, cr, root, predicate);
  }

  public static List<JavaMemberType> queryJavaMemberTypes(
    JavaAstNode recordJavaAstNode)
  {
    CriteriaQuery<JavaMemberType> cr = cb.createQuery(JavaMemberType.class);
    Root<JavaMemberType> root = cr.from(JavaMemberType.class);

    cr
      .select(root)
      .where(
        cb.equal(root.get("entityHash"), recordJavaAstNode.getEntityHash())
      );

    return em.createQuery(cr).getResultList();
  }

  public static List<JavaMemberType> queryJavaMemberTypes(
    JavaAstNode recordJavaAstNode, CriteriaQuery<JavaMemberType> cr,
    Root<JavaMemberType> root, Predicate customPredicate)
  {
    Predicate entityHashPredicate =
      cb.equal(root.get("typeHash"), recordJavaAstNode.getEntityHash());

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
    Path<MemberTypeKind> kind = root.get("kind");

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

  public static List<JavaAstNode> queryVisibleDeclarationNodes(
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

  public static List<JavaAstNode> queryUsageNodes(JavaAstNode javaAstNode) {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);
    Predicate predicate =
      cb.or(
        cb.equal(root.get("astType"), AstType.USAGE),
        cb.equal(root.get("astType"), AstType.READ),
        cb.equal(root.get("astType"), AstType.WRITE)
      );

    return queryJavaAstNodes(javaAstNode, cr, root, predicate);
  }

  public static List<JavaAstNode> queryReadNodes(JavaAstNode javaAstNode) {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);
    Predicate predicate = cb.equal(root.get("astType"), AstType.READ);

    return queryJavaAstNodes(javaAstNode, cr, root, predicate);
  }

  public static List<JavaAstNode> queryWriteNodes(JavaAstNode javaAstNode) {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);
    Predicate predicate = cb.equal(root.get("astType"), AstType.WRITE);

    return queryJavaAstNodes(javaAstNode, cr, root, predicate);
  }

  public static List<JavaAstNode> queryCalleeNodes(JavaAstNode javaAstNode) {
    List<JavaAstNode> calls = queryCallNodes(javaAstNode);

    return calls.stream()
      .flatMap(c -> queryDefinitionNodes(c).stream())
      .collect(Collectors.toList());
  }

  public static List<JavaAstNode> queryCallerNodes(JavaAstNode javaAstNode) {
    List<JavaAstNode> usages = queryUsageNodes(javaAstNode);

    return usages.stream()
      .flatMap(u -> queryCallerNode(u).stream())
      .collect(Collectors.toList());
  }

  public static List<JavaAstNode> queryCallerNode(JavaAstNode usage) {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);

    Path<AstType> astType = root.get("astType");
    Path<SymbolType> symbolType = root.get("symbolType");
    Path<Long> locationFile =
      root.get("location_file");
    Path<Long> startLine =
      root.get("location_range_start_line");
    Path<Long> endLine =
      root.get("location_range_end_line");
    Path<Long> startColumn =
      root.get("location_range_start_column");
    Path<Long> endColumn =
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

  public static List<JavaAstNode> queryCallNodes(JavaAstNode javaAstNode) {
    JavaAstNode definition;

    if (javaAstNode.getAstType() == AstType.DEFINITION) {
      definition = javaAstNode;
    } else {
      List<JavaAstNode> definitions = queryDefinitionNodes(javaAstNode);

      if (definitions.isEmpty()) {
        return definitions;
      }

      definition = definitions.get(0);
    }

    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);

    Path<AstType> astType = root.get("astType");
    Path<SymbolType> symbolType = root.get("symbolType");
    Path<Long> locationFile =
      root.get("location_file");
    Path<Long> startLine =
      root.get("location_range_start_line");
    Path<Long> endLine =
      root.get("location_range_end_line");
    Path<Long> startColumn =
      root.get("location_range_start_column");
    Path<Long> endColumn =
      root.get("location_range_end_column");

    long dFile = definition.getLocation_file();
    long dStartLine = definition.getLocation_range_start_line();
    long dEndLine = definition.getLocation_range_end_line();
    long dStartColumn = definition.getLocation_range_start_column();
    long dEndColumn = definition.getLocation_range_end_column();

    Predicate usage = cb.equal(astType, AstType.USAGE);
    Predicate methodOrConstructor =
      cb.or(
        cb.equal(symbolType, SymbolType.METHOD),
        cb.equal(symbolType, SymbolType.CONSTRUCTOR)
      );
    Predicate sameFile = cb.equal(locationFile, dFile);
    Predicate startPosGreaterEqualPos =
      cb.or(
        cb.and(
          cb.equal(startLine, dStartLine),
          cb.ge(startColumn, dStartColumn)
        ),
        cb.gt(startLine, dStartLine)
      );
    Predicate posGreaterEqualEndPos =
      cb.or(
        cb.and(
          cb.equal(endLine, dEndLine),
          cb.le(endColumn, dEndColumn)
        ),
        cb.lt(endLine, dEndLine)
      );

    cr
      .select(root)
      .where(
        cb.and(
          usage, methodOrConstructor, sameFile,
          startPosGreaterEqualPos, posGreaterEqualEndPos
        )
      );

    return em.createQuery(cr).getResultList();
  }

  public static List<JavaAstNode> queryParameterNodes(JavaAstNode javaAstNode) {
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

  public static List<JavaAstNode> queryLocalVarNodes(JavaAstNode javaAstNode) {
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

  public static List<JavaAstNode> queryReturnTypeNodes(JavaAstNode javaAstNode)
  {
    List<JavaMethod> javaMethods = queryJavaMethods(javaAstNode);

    if (!javaMethods.isEmpty()) {
      JavaMethod javaMethod = javaMethods.get(0);
      List<JavaRecord> javaRecords = queryJavaRecords(javaMethod);

      if (!javaRecords.isEmpty()) {
        JavaRecord javaRecord = javaRecords.get(0);

        return queryDefinitionNodes(javaRecord.getEntityHash());
      }
    }

    return new ArrayList<>();
  }

  public static List<JavaAstNode> queryRelationNodes(
    JavaAstNode javaAstNode, RelationKind relationKind, boolean reverse)
  {
    CriteriaQuery<JavaRelation> cr = cb.createQuery(JavaRelation.class);
    Root<JavaRelation> root = cr.from(JavaRelation.class);
    Predicate relationKindPredicate = cb.equal(
      root.get("kind"), relationKind
    );

    List<JavaRelation> javaRelations =
      queryJavaRelations(javaAstNode, reverse, cr, root, relationKindPredicate);

    return javaRelations.stream()
      .flatMap(
        r ->  queryDeclOrDefNodes(reverse ? r.getRhs() : r.getLhs()).stream())
      .collect(Collectors.toList()
      );
  }

  public static List<JavaAstNode> queryTypeNodes(JavaAstNode javaAstNode) {
    List<JavaVariable> javaVariables = queryJavaVariables(javaAstNode);

    if (!javaVariables.isEmpty()) {
      JavaVariable javaVariable = javaVariables.get(0);
      List<JavaRecord> javaRecords = queryJavaRecords(javaVariable);

      if (!javaRecords.isEmpty()) {
        JavaRecord javaRecord = javaRecords.get(0);

        return queryDefinitionNodes(javaRecord.getEntityHash());
      }
    }

    return new ArrayList<>();
  }

  public static List<JavaAstNode> queryInheritFromNodes(
    JavaAstNode javaAstNode)
  {
    List<JavaInheritance> javaInheritances =
      queryInheritancesDerived(javaAstNode);

    return javaInheritances.stream()
      .flatMap(i -> queryDefinitionNodes(i.getBase()).stream())
      .collect(Collectors.toList());
  }

  public static List<JavaAstNode> queryInheritedByNodes(
    JavaAstNode javaAstNode)
  {
    List<JavaInheritance> javaInheritances =
      queryInheritancesBase(javaAstNode);

    return javaInheritances.stream()
      .flatMap(i -> queryDefinitionNodes(i.getDerived()).stream())
      .collect(Collectors.toList());
  }

  public static List<JavaAstNode> queryJavaEnumConstantNodes(
    JavaAstNode javaAstNode)
  {
    List<JavaEnum> javaEnums = queryJavaEnums(javaAstNode);

    if (!javaEnums.isEmpty()) {
      JavaEnum javaEnum = javaEnums.get(0);
      Set<JavaEnumConstant> javaEnumConstants = javaEnum.getJavaEnumConstants();

      return javaEnumConstants.stream()
        .map(c ->
          queryJavaAstNode(c.getAstNodeId())).collect(Collectors.toList()
        );
    }

    return new ArrayList<>();
  }

  public static List<JavaInheritance> queryInheritancesDerived(
    JavaAstNode javaAstNode)
  {
    CriteriaQuery<JavaInheritance> cr = cb.createQuery(JavaInheritance.class);
    Root<JavaInheritance> root = cr.from(JavaInheritance.class);

    cr
      .select(root)
      .where(cb.equal(root.get("derived"), javaAstNode.getEntityHash()));

    return em.createQuery(cr).getResultList();
  }

  public static List<JavaInheritance> queryInheritancesDerived(
    JavaAstNode javaAstNode, CriteriaQuery<JavaInheritance> cr,
    Root<JavaInheritance> root, Predicate customPredicate)
  {
    Predicate entityHashPredicate = cb.equal(
      root.get("derived"), javaAstNode.getEntityHash()
    );

    cr.select(root).where(cb.and(entityHashPredicate, customPredicate));

    return em.createQuery(cr).getResultList();
  }

  public static List<JavaInheritance> queryInheritancesBase(
    JavaAstNode javaAstNode)
  {
    CriteriaQuery<JavaInheritance> cr = cb.createQuery(JavaInheritance.class);
    Root<JavaInheritance> root = cr.from(JavaInheritance.class);

    cr
      .select(root)
      .where(cb.equal(root.get("base"), javaAstNode.getEntityHash()));

    return em.createQuery(cr).getResultList();
  }

  public static List<JavaInheritance> queryInheritancesBase(
    JavaAstNode javaAstNode, CriteriaQuery<JavaInheritance> cr,
    Root<JavaInheritance> root, Predicate customPredicate)
  {
    Predicate entityHashPredicate = cb.equal(
      root.get("base"), javaAstNode.getEntityHash()
    );

    cr.select(root).where(cb.and(entityHashPredicate, customPredicate));

    return em.createQuery(cr).getResultList();
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

  public static List<JavaEnum> queryJavaEnums(JavaAstNode javaAstNode) {
    CriteriaQuery<JavaEnum> cr = cb.createQuery(JavaEnum.class);
    Root<JavaEnum> root = cr.from(JavaEnum.class);

    cr
      .select(root)
      .where(cb.equal(root.get("entityHash"), javaAstNode.getEntityHash()));

    List<JavaEnum> javaEnums = em.createQuery(cr).getResultList();

    if (javaEnums.isEmpty()) {
      LOGGER.log(
        Level.WARNING,
        "Database query result was not expected to be empty. " +
          getCurrentPath() + ", line #" + getCurrentLineNumber()
      );
    }

    return javaEnums;
  }

  public static List<JavaEnum> queryJavaEnums(
    JavaAstNode javaAstNode, CriteriaQuery<JavaEnum> cr,
    Root<JavaEnum> root, Predicate customPredicate)
  {
    Predicate entityHashPredicate = cb.equal(
      root.get("entityHash"), javaAstNode.getEntityHash()
    );

    cr.select(root).where(cb.and(entityHashPredicate, customPredicate));

    return em.createQuery(cr).getResultList();
  }

  public static List<JavaRelation> queryJavaRelations(
    JavaAstNode javaAstNode, boolean reverse)
  {
    CriteriaQuery<JavaRelation> cr = cb.createQuery(JavaRelation.class);
    Root<JavaRelation> root = cr.from(JavaRelation.class);

    Path<Long> entityHash = reverse ? root.get("lhs") : root.get("rhs");

    cr
      .select(root)
      .where(cb.equal(entityHash, javaAstNode.getEntityHash()));

    return em.createQuery(cr).getResultList();
  }

  public static List<JavaRelation> queryJavaRelations(
    JavaAstNode javaAstNode, boolean reverse,
    CriteriaQuery<JavaRelation> cr,
    Root<JavaRelation> root, Predicate customPredicate)
  {
    Path<Long> entityHash = reverse ? root.get("lhs") : root.get("rhs");

    Predicate entityHashPredicate = cb.equal(
      entityHash, javaAstNode.getEntityHash()
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

  public static List<JavaDocComment> queryJavaDocComments(
    long entityHash)
  {
    CriteriaQuery<JavaDocComment> cr =
      cb.createQuery(JavaDocComment.class);
    Root<JavaDocComment> root = cr.from(JavaDocComment.class);

    cr
      .select(root)
      .where(cb.equal(root.get("entityHash"), entityHash));

    return em.createQuery(cr).getResultList();
  }

  private static String getCurrentPath() {
    return new Throwable().getStackTrace()[1].getFileName();
  }

  private static int getCurrentLineNumber() {
    return new Throwable().getStackTrace()[1].getLineNumber();
  }
}
