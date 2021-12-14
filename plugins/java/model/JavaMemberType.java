package model;

import model.enums.MemberTypeKind;
import model.enums.Visibility;

import javax.persistence.*;

@Entity
@Table(name = "\"JavaMemberType\"")
public class JavaMemberType {
  @Id
  @GeneratedValue(strategy = GenerationType.IDENTITY)
  @Column(name = "id")
  private int id;

  @Column(name = "\"typeHash\"")
  private long typeHash;

  @OneToOne(cascade = CascadeType.PERSIST, fetch = FetchType.EAGER)
  @JoinColumn(name = "\"memberAstNode\"")
  private JavaAstNode memberAstNode;

  @Column(name = "\"memberTypeHash\"")
  private long memberTypeHash;

  @Column(name = "kind")
  private MemberTypeKind kind;

  @Column(name = "visibility")
  private Visibility visibility;


  // Getters and setters

  public int getId() {
    return id;
  }

  public void setId(int id) {
    this.id = id;
  }

  public long getTypeHash() {
    return typeHash;
  }

  public void setTypeHash(long typeHash) {
    this.typeHash = typeHash;
  }

  public JavaAstNode getMemberAstNode() {
    return memberAstNode;
  }

  public void setMemberAstNode(JavaAstNode memberAstNode) {
    this.memberAstNode = memberAstNode;
  }

  public long getMemberTypeHash() {
    return memberTypeHash;
  }

  public void setMemberTypeHash(long memberTypeHash) {
    this.memberTypeHash = memberTypeHash;
  }

  public MemberTypeKind getKind() {
    return kind;
  }

  public void setKind(MemberTypeKind kind) {
    this.kind = kind;
  }

  public Visibility getVisibility() {
    return visibility;
  }

  public void setVisibility(Visibility visibility) {
    this.visibility = visibility;
  }
}
