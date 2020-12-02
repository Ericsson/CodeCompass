package model;

import javax.persistence.*;

@Entity
@Table(name="\"JavaAnnotation\"")
public class JavaAnnotation {
    @Id
    @GeneratedValue
    @Column(name = "id")
    private int id;

    @JoinColumn(name="\"astNode\"")
	@ManyToOne
	JavaAstNode astNode;

    // Getters and setters

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
