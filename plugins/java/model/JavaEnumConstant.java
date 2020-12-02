package model;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.Table;

@Entity
@Table(name="\"JavaEnumConstant\"")
public class JavaEnumConstant {
    @Id
    @Column(name = "id")
    private int id;

    @JoinColumn(name="\"astNode\"")
    @ManyToOne
    JavaAstNode astNode;

    @Column(name = "value")
    private int value;

    // Getters and setters

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public int getValue() {
        return value;
    }

    public void setValue(int value) {
        this.value = value;
    }

    public JavaAstNode getAstNode() {
        return astNode;
    }

    public void setAstNode(JavaAstNode astNode) {
        this.astNode = astNode;
    }
}
