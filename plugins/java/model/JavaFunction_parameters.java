package model;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Table;

@Entity
@Table(name="\"JavaFunction_parameters\"")
public class JavaFunction_parameters {
    @Column(name = "object_id")
    private int object_id;

    @Column(name = "index")
    private int index;

    @Column(name = "value")
    private int value;

    @JoinColumn(name="\"astNode\"")
    @ManyToOne
    JavaAstNode astNode;

    // Getters and setters

    public int getObject_id() {
        return object_id;
    }

    public void setObject_id(int object_id) {
        this.object_id = object_id;
    }

    public int getIndex() {
        return index;
    }

    public void setIndex(int index) {
        this.index = index;
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
