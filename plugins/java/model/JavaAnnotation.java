package model;

import javax.persistence.*;

@Entity
@Table(name = "\"JavaAnnotation\"")
public class JavaAnnotation extends JavaEntity {
  @Id
  @GeneratedValue
  @Column(name = "id")
  private int id;

  // Getters and setters

  public int getId() {
    return id;
  }

  public void setId(int id) {
    this.id = id;
  }
}
