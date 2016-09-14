// $Id$
// Created by Aron Barath, 2013

package parser;

import javax.persistence.*;

@Entity
@Table(name="\"JavaEnumConstant\"")
public class JavaEnumConstant implements java.io.Serializable
{
	private static final long serialVersionUID = 8975521735538252063L;

	@Column(name="id")
	@Id @GeneratedValue
	int id;
	public int getId() { return id; }
	public void setId(int id) 	{ this.id = id; }

	@JoinColumn(name="\"astNodePtr\"") // TODO: rename to astNode
	@ManyToOne
	JavaAstNode astNode;
	public JavaAstNode getAstNode() { return astNode; }
	public void setAstNode(JavaAstNode astNode) { this.astNode = astNode; }

	@JoinColumn(name="\"enumType\"")
	@ManyToOne
	JavaType enumType;
	public JavaType getEnumType() { return enumType; }
	public void setEnumType(JavaType enumType) { this.enumType = enumType; }

	@Column(name="\"mangledNameHash\"")
	long mangledNameHash;
	public long getMangledNameHash() { return mangledNameHash; }
	public void setMangledNameHash(long mangledNameHash) { this.mangledNameHash = mangledNameHash; }

	@Column(name="name")
	String name;
	public String getName() { return name; }
	public void setName(String name) { this.name = name; }

	@Column(name="\"qualifiedName\"")
	String qualifiedName;
	public String getQualifiedName() { 	return qualifiedName; }
	public void setQualifiedName(String qualifiedName) { this.qualifiedName = qualifiedName; }
}
