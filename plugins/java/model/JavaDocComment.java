package model;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.Table;

@Entity
@Table(name="\"JavaDocComment\"")
public class JavaDocComment {
    @Id
    @Column(name = "id")
    private int id;

    @JoinColumn(name="\"astNode\"")
    @ManyToOne
    JavaAstNode astNode;

    @Column(name = "\"contentHash\"")
    private int contentHash;

    @Column(name = "\"contentHTML\"")
    private int contentHTML;

    @Column(name = "\"mangledNameHash\"")
    private int mangledNameHash;

    // Getters and setters

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public int getContentHash() {
        return contentHash;
    }

    public void setContentHash(int contentHash) {
        this.contentHash = contentHash;
    }

    public int getContentHTML() {
        return contentHTML;
    }

    public void setContentHTML(int contentHTML) {
        this.contentHTML = contentHTML;
    }

    public int getMangledNameHash() {
        return mangledNameHash;
    }

    public void setMangledNameHash(int mangledNameHash) {
        this.mangledNameHash = mangledNameHash;
    }

    public JavaAstNode getAstNode() {
        return astNode;
    }

    public void setAstNode(JavaAstNode astNode) {
        this.astNode = astNode;
    }
}
