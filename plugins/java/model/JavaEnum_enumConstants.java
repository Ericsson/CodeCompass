package model;

import javax.persistence.*;

@Entity
@Table(name = "\"JavaEnum_enumConstant\"")
public class JavaEnum_enumConstants {
  @Id
  @GeneratedValue
  @Column(name = "id")
  private long id;

  @JoinColumn(name = "object_id")
  @OneToOne
  private JavaEnum object_id;

  @Column(name = "index")
  private long index;

  @JoinColumn(name = "value")
  @OneToOne
  private JavaEnumConstant value;

  // Getters and setters

  public long getId() {
    return id;
  }

  public void setId(long id) {
    this.id = id;
  }

  public JavaEnum getObject_id() {
    return object_id;
  }

  public void setObject_id(JavaEnum object_id) {
    this.object_id = object_id;
  }

  public long getIndex() {
    return index;
  }

  public void setIndex(long index) {
    this.index = index;
  }

  public JavaEnumConstant getValue() {
    return value;
  }

  public void setValue(JavaEnumConstant value) {
    this.value = value;
  }
}
