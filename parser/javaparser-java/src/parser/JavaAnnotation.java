// $Id$
// Created by Aron Barath, 2014

package parser;

import javax.persistence.*;

@Entity
@Table(name="\"JavaAnnotation\"")
public class JavaAnnotation implements java.io.Serializable
{
	private static final long serialVersionUID = 4284374279595275554L;

	JavaAnnotation() { }

	public JavaAnnotation(JavaAstNode node, String name)
	{
		this.astNode = node;
		this.mangledNameHash = node.getMangledNameHash();
		this.name = name;
	}

	@Column(name="id")
	@Id @GeneratedValue
	int id;
	public int getId() { return id; }
	private void setId(int id) { this.id = id; }

	@JoinColumn(name="\"astNodePtr\"") // TODO: rename to astNode
	@ManyToOne
	JavaAstNode astNode;
	public JavaAstNode getAstNode() { return astNode; }
	public void setAstNode(JavaAstNode astNode) { this.astNode = astNode; }

	@Column(name="\"mangledNameHash\"")
	long mangledNameHash;
	public long getMangledNameHash() { return mangledNameHash; }
	public void setMangledNameHash(long mangledNameHash) { this.mangledNameHash = mangledNameHash; }

	@Column(name="name")
	String name;
	public String getName() { return name; }
	public void setName(String name) { this.name = name; }
}
