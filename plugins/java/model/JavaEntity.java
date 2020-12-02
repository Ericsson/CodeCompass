package model;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.Table;

@Entity
@Table(name = "\"JavaEntity\"")
public class JavaEntity {
    @Id
    @Column(name = "id")
    private int id;

    @JoinColumn(name="\"astNode\"")
    @ManyToOne
    JavaAstNode astNode;

    @Column(name = "\"typeId\"")
    private int typeId;

    @Column(name = "\"astNodeId\"")
    private int astNodeId;

    @Column(name = "\"mangledNameHash\"")
    private int mangledNameHash;

    @Column(name = "name")
    private String name;

    @Column(name = "\"qualifiedName\"")
    private String qualifiedName;

    // Getters and setters

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public int getTypeId() {
        return typeId;
    }

    public void setTypeId(int typeId) {
        this.typeId = typeId;
    }

    public int getAstNodeId() {
        return astNodeId;
    }

    public void setAstNodeId(int astNodeId) {
        this.astNodeId = astNodeId;
    }

    public int getMangledNameHash() {
        return mangledNameHash;
    }

    public void setMangledNameHash(int mangledNameHash) {
        this.mangledNameHash = mangledNameHash;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getQualifiedName() {
        return qualifiedName;
    }

    public void setQualifiedName(String qualifiedName) {
        this.qualifiedName = qualifiedName;
    }

    public JavaAstNode getAstNode() {
        return astNode;
    }

    public void setAstNode(JavaAstNode astNode) {
        this.astNode = astNode;
    }
}
