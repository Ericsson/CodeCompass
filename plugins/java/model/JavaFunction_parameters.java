package model;

import javax.persistence.*;

@Entity
@Table(name = "\"JavaFunction_parameters\"")
public class JavaFunction_parameters {
  @Id
  @GeneratedValue
  @Column(name = "id")
  private long id;

  @JoinColumn(name = "object_id")
  @OneToOne
  private JavaFunction object_id;

  @Column(name = "index")
  private long index;

  @JoinColumn(name = "value")
  @OneToOne
  private JavaVariable value;

  // Getters and setters

  public long getId() {
    return id;
  }

  public void setId(long id) {
    this.id = id;
  }

  public JavaFunction getObject_id() {
    return object_id;
  }

  public void setObject_id(JavaFunction object_id) {
    this.object_id = object_id;
  }

  public long getIndex() {
    return index;
  }

  public void setIndex(long index) {
    this.index = index;
  }

  public JavaVariable getValue() {
    return value;
  }

  public void setValue(JavaVariable value) {
    this.value = value;
  }
}
