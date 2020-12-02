package model;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.Table;

@Entity
@Table(name="\"JavaTypedef\"")
public class JavaVariable {
    @Id
    @Column(name = "id")
    private int id;

    @JoinColumn(name="\"astNode\"")
    @ManyToOne
    JavaAstNode astNode;

    // Getters and Setters

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public JavaAstNode getAstNode() {
        return astNode;
    }

    public void setAstNode(JavaAstNode astNode) {
        this.astNode = astNode;
    }
}
