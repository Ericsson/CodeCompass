package parser.srcjava;

import model.JavaAstNode;
import model.JavaConstructor;
import model.JavaEntity;
import model.JavaMethod;
import model.enums.AstType;

import javax.persistence.EntityManager;
import javax.persistence.criteria.*;

public class QueryManager {
  private final EntityManager em;
  private final CriteriaBuilder cb;

  public QueryManager(EntityManager em) {
    this.em = em;
    this.cb = em.getCriteriaBuilder();
  }

  public JavaAstNode queryParentAstNode(
    JavaAstNode child, int methodEntityHash)
  {
    CriteriaQuery<JavaAstNode> cr = cb.createQuery(JavaAstNode.class);
    Root<JavaAstNode> root = cr.from(JavaAstNode.class);

    long childLocationFile = child.getLocation_file();
    long childStartLine = child.getLocation_range_start_line();
    long childStartColumn = child.getLocation_range_start_column();
    Path<Long> locationFile =
      root.get("location_file");
    Path<Long> entityHash =
      root.get("entityHash");
    Path<Long> astType =
      root.get("astType");
    Path<Integer> startLine =
      root.get("location_range_start_line");
    Path<Integer> endLine =
      root.get("location_range_end_line");
    Path<Integer> startColumn =
      root.get("location_range_start_column");
    Path<Integer> endColumn =
      root.get("location_range_end_column");

    Predicate sameFile = cb.equal(locationFile, childLocationFile);
    Predicate sameEntityHash = cb.equal(entityHash, methodEntityHash);
    Predicate definition = cb.equal(astType, AstType.DEFINITION);
    Predicate startPosLessEqualPos =
      cb.or(
        cb.and(
          cb.equal(startLine, childStartLine),
          cb.le(startColumn, childStartColumn)
        ),
        cb.lt(startLine, childStartLine)
      );
    Predicate posLessThanEndPos =
      cb.or(
        cb.and(
          cb.equal(endLine, childStartLine),
          cb.gt(endColumn, childStartColumn)
        ),
        cb.gt(endLine, childStartLine)
      );

    cr
      .select(root)
      .where(
        cb.and(
          sameFile, sameEntityHash, definition,
          startPosLessEqualPos, posLessThanEndPos
        )
      );

    return em.createQuery(cr).getSingleResult();
  }

  public <E extends JavaEntity> E queryJavaEntityByAstNodeId(
    CriteriaQuery<E> cr,
    Root<E> root,
    long astNodeId)
  {
    cr
      .select(root)
      .where(cb.equal(root.get("astNodeId"), astNodeId));

    return em.createQuery(cr).getSingleResult();
  }

  public JavaConstructor queryJavaConstructor(long javaAstNodeId) {
    CriteriaQuery<JavaConstructor> cr = cb.createQuery(JavaConstructor.class);
    Root<JavaConstructor> root = cr.from(JavaConstructor.class);

    return queryJavaEntityByAstNodeId(cr, root, javaAstNodeId);
  }

  public JavaMethod queryJavaMethod(long javaAstNodeId) {
    CriteriaQuery<JavaMethod> cr = cb.createQuery(JavaMethod.class);
    Root<JavaMethod> root = cr.from(JavaMethod.class);

    return queryJavaEntityByAstNodeId(cr, root, javaAstNodeId);
  }
}
