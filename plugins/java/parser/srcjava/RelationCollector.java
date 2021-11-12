package parser.srcjava;

import model.JavaRelation;
import model.enums.RelationKind;
import model.enums.Visibility;
import org.eclipse.jdt.core.dom.IMethodBinding;
import org.eclipse.jdt.core.dom.ITypeBinding;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Optional;

import static parser.srcjava.Utils.*;

public class RelationCollector {
  private final IMethodBinding methodBinding;
  private final String name;
  private final Visibility visibility;
  private final int entityHash;

  public RelationCollector(
    IMethodBinding methodBinding, String name,
    Visibility visibility, int entityHash)
  {
    this.methodBinding = methodBinding;
    this.name = name;
    this.visibility = visibility;
    this.entityHash = entityHash;
  }

  public List<JavaRelation> collectBaseMethods(
    ITypeBinding superclassBinding, ITypeBinding[] superInterfaceBindings)
  {
    List<JavaRelation> javaRelations = new ArrayList<>();
    JavaRelation javaExtension = collectExtension(superclassBinding);
    JavaRelation javaImplementation =
      collectImplementation(superInterfaceBindings);

    if (javaExtension != null) {
      javaRelations.add(javaExtension);
    }

    if (javaImplementation != null) {
      javaRelations.add(javaImplementation);
    }

    return javaRelations;
  }

  public JavaRelation collectExtension(ITypeBinding superclassBinding) {
    if (superclassBinding == null) {
      return null;
    }

    JavaRelation javaRelation =
      getRelation(superclassBinding, RelationKind.OVERRIDE);

    if (javaRelation != null) {
      return javaRelation;
    }

    return collectExtension(superclassBinding.getSuperclass());
  }

  public JavaRelation collectImplementation(
    ITypeBinding[] superInterfaceBindings)
  {
    for (ITypeBinding superInterfaceBinding : superInterfaceBindings) {
      JavaRelation javaRelation =
        getRelation(superInterfaceBinding, RelationKind.IMPLEMENT);

      if (javaRelation != null) {
        return javaRelation;
      }

      return collectImplementation(superInterfaceBinding.getInterfaces());
    }

    return null;
  }

  private JavaRelation getRelation(
    ITypeBinding superclassBinding, RelationKind relationKind)
  {
    IMethodBinding[] declaredMethods = superclassBinding.getDeclaredMethods();
    Optional<IMethodBinding> superMethodBinding =
      Arrays.stream(declaredMethods)
        .filter(this::isOverriddenMethod)
        .findFirst();

    if (superMethodBinding.isPresent()) {
      JavaRelation javaRelation = new JavaRelation();
      String superclassQualifiedName = superclassBinding.getQualifiedName();
      String methodQualifiedType =
        methodBinding.getReturnType().getQualifiedName();
      String parametersStr =
        getParameterTypeNamesStr(methodBinding.getParameterTypes());
      String superMethodEntityHashStr =
        getMethodHashStr(
          superclassQualifiedName, methodQualifiedType, name, parametersStr);
      int superMethodEntityHash = superMethodEntityHashStr.hashCode();

      setJavaRelationFields(
        javaRelation, superMethodEntityHash,
        entityHash, relationKind
      );

      return javaRelation;
    }

    return null;
  }

  private boolean isOverriddenMethod(IMethodBinding superMethodBinding)
  {
    Visibility mVisibility = getVisibility(superMethodBinding.getModifiers());
    String mName = superMethodBinding.getName();

    return mName.equals(name) &&
      mVisibility.getValue() <=
        visibility.getValue() &&
      Arrays.equals(
        getParameterTypeNames(superMethodBinding.getParameterTypes()),
        getParameterTypeNames(methodBinding.getParameterTypes())
      );
  }
}
