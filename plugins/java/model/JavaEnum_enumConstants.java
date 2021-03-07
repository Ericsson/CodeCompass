package model;

import javax.persistence.*;

@Entity
@Table(name = "\"JavaEnum_enumConstant\"")
public class JavaEnum_enumConstants {
  @JoinColumn(name = "object_id")
  @OneToOne
  private JavaEnum object_id;

  @Column(name = "index")
  private int index;

  @JoinColumn(name = "value")
  @OneToOne
  private JavaEnumConstant value;

  // Getters and setters

  public JavaEnum getObject_id() {
    return object_id;
  }

  public void setObject_id(JavaEnum object_id) {
    this.object_id = object_id;
  }

  public int getIndex() {
    return index;
  }

  public void setIndex(int index) {
    this.index = index;
  }

  public JavaEnumConstant getValue() {
    return value;
  }

  public void setValue(JavaEnumConstant value) {
    this.value = value;
  }
}
