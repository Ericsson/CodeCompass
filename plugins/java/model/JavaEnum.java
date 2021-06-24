package model;

import javax.persistence.*;
import java.util.Set;

@Entity
@Table(name = "\"JavaEnum\"")
public class JavaEnum extends JavaEntity {

  @ManyToMany
  @JoinTable(
    name = "\"JavaEnum_enumConstants\"",
    joinColumns = @JoinColumn(name = "object_id"),
    inverseJoinColumns = @JoinColumn(name = "value")
  )
  Set<JavaEnumConstant> javaEnumConstants;
}
