// $Id$
// Created by Aron Barath, 2013

package parser;

import javax.persistence.*;

@Entity
@Table(name="\"JavaVariable\"")
public class JavaVariable implements java.io.Serializable
{
	private static final long serialVersionUID = 8372742635538252063L;

	public JavaVariable() { }

	public JavaVariable(JavaType type, JavaAstNode node, String name, String qualName)
	{
		this.type = type;
		this.astNode = node;
		this.name = name;
		this.qualifiedName = qualName;
	}

	@Column(name="id")
	@Id @GeneratedValue
	int id;
	public int getId() { return id; }
	public void setId(int id) { this.id = id; }

	@JoinColumn(name="type")
	@ManyToOne
	JavaType type;
	public JavaType getType() { return type; }
	public void setType(JavaType type) { this.type = type; }

	@JoinColumn(name="\"astNodePtr\"") // TODO: rename to astNode
	@ManyToOne
	JavaAstNode astNode;
	public JavaAstNode getAstNode() { return astNode; }
	public void setAstNode(JavaAstNode astNode) { this.astNode = astNode; }

	@Column(name="name")
	String name;
	public String getName() { return name; }
	public void setName(String name) { this.name = name; }

	@Column(name="\"qualifiedName\"")
	String qualifiedName;
	public String getQualifiedName() { 	return qualifiedName; }
	public void setQualifiedName(String qualifiedName) { this.qualifiedName = qualifiedName; }

	@Column(name="\"mangledNameHash\"")
	long mangledNameHash;
	public long getMangledNameHash() { return mangledNameHash; }
	public void setMangledNameHash(long mangledNameHash) { this.mangledNameHash = mangledNameHash; }

	/**
	 * Connect this variable as a formal parameter to a function.
	 */
	@JoinColumn(name="\"paramInFunc\"")
	@ManyToOne
	JavaFunction paramInFunc;
	public JavaFunction getParamInFunc() { return paramInFunc; }
	public void setParamInFunc(JavaFunction paramInFunc) { this.paramInFunc = paramInFunc; }

	/**
	 * Connect this variable as a local variable to a function.
	 */
	@JoinColumn(name="\"localInFunc\"")
	@ManyToOne
	JavaFunction localInFunc;
	public JavaFunction getLocalInFunc() { return localInFunc; }
	public void setLocalInFunc(JavaFunction localInFunc) { this.localInFunc = localInFunc; }
}
