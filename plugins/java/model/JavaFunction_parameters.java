package model;

import javax.persistence.*;

@Entity
@Table(name = "\"JavaFunction_parameters\"")
public class JavaFunction_parameters {
  @JoinColumn(name = "object_id")
  @OneToOne
  private JavaFunction object_id;

  @Column(name = "index")
  private int index;

  @JoinColumn(name = "value")
  @OneToOne
  private JavaVariable value;

  // Getters and setters

  public JavaFunction getObject_id() {
    return object_id;
  }

  public void setObject_id(JavaFunction object_id) {
    this.object_id = object_id;
  }

  public int getIndex() {
    return index;
  }

  public void setIndex(int index) {
    this.index = index;
  }

  public JavaVariable getValue() {
    return value;
  }

  public void setValue(JavaVariable value) {
    this.value = value;
  }
}
