package model;

import model.enums.RelationKind;

import javax.persistence.*;

@Entity
@Table(name = "\"JavaRelation\"")
public class JavaRelation {
  @Id
  @GeneratedValue
  @Column(name = "id")
  private int id;

  @Column(name = "lhs")
  private long lhs;

  @Column(name = "rhs")
  private long rhs;

  @Column(name = "\"kind\"")
  @Enumerated(EnumType.ORDINAL)
  private RelationKind kind;

  // Getters and setters

  public int getId() {
    return id;
  }

  public void setId(int id) {
    this.id = id;
  }

  public long getLhs() {
    return lhs;
  }

  public void setLhs(long lhs) {
    this.lhs = lhs;
  }

  public long getRhs() {
    return rhs;
  }

  public void setRhs(long rhs) {
    this.rhs = rhs;
  }

  public RelationKind getKind() {
    return kind;
  }

  public void setKind(RelationKind kind) {
    this.kind = kind;
  }
}
