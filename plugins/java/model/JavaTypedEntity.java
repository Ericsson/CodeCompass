package model;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.Table;

@Entity
@Table(name="\"JavaTypedEntity\"")
public class JavaTypedEntity {
    @Id
    @Column(name = "id")
    private int id;

    @JoinColumn(name="\"astNode\"")
    @ManyToOne
    JavaAstNode astNode;

    @Column(name = "\"typeHash\"")
    private int typeHash;

    @Column(name = "\"qualifiedType\"")
    private String qualifiedType;

    // Getters and setters

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public int getTypeHash() {
        return typeHash;
    }

    public void setTypeHash(int typeHash) {
        this.typeHash = typeHash;
    }

    public String getQualifiedType() {
        return qualifiedType;
    }

    public void setQualifiedType(String qualifiedType) {
        this.qualifiedType = qualifiedType;
    }

    public JavaAstNode getAstNode() {
        return astNode;
    }

    public void setAstNode(JavaAstNode astNode) {
        this.astNode = astNode;
    }
}
