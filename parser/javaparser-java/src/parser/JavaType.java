// $Id$
// Created by Aron Barath, 2013

package parser;

import javax.persistence.*;

import java.util.*;

@Entity
@Table(name="\"JavaType\"")
public class JavaType implements java.io.Serializable
{
	private static final long serialVersionUID = 8375521735538252063L;

	// @Column(name="id")
	// @Id @GeneratedValue
	// long id;
	// public long getId() { return id; }
	// public void setId(long id) { this.id = id; }

	@JoinColumn(name="\"astNodePtr\"") // TODO: rename to astNode
	@ManyToOne
	JavaAstNode astNode;
	public JavaAstNode getAstNode() { return astNode; }
	public void setAstNode(JavaAstNode astNode) { this.astNode = astNode; }

	@Id
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

	@JoinColumn(name="\"superClass\"")
	JavaType superClass;
	public JavaType getSuperClass() { return superClass; }
	public void setSuperClass(JavaType superClass) { this.superClass = superClass; }

	@JoinColumn(name="interfaces")
	@ManyToMany
	@JoinTable(name="\"JavaType_Interfaces\"",
		joinColumns = @JoinColumn(name = "type", referencedColumnName = "mangledNameHash"), 
		inverseJoinColumns = @JoinColumn(name = "iface", referencedColumnName = "mangledNameHash")
	)
	List<JavaType> interfaces = new LinkedList<JavaType>();
	public List<JavaType> getInterfaces() { return interfaces; }
	public void setInterfaces(List<JavaType> interfaces) { this.interfaces = interfaces; }

	@JoinColumn(name="fields")
	@OneToMany(mappedBy="type")
	List<JavaMember> fields = new LinkedList<JavaMember>();
	public List<JavaMember> getFields() { return fields; }
	public void setFields(List<JavaMember> fields) { this.fields = fields; }

	@JoinColumn(name="functions")
	@OneToMany(mappedBy="type")
	List<JavaFunction> functions = new LinkedList<JavaFunction>();
	public List<JavaFunction> getFunctions() { return functions; }
	public void setFunctions(List<JavaFunction> functions) { this.functions = functions; }

	@Column(name="\"isEnum\"")
	boolean isEnum;
	public boolean getIsEnum() { return isEnum; }
	public void setIsEnum(boolean isEnum) { this.isEnum = isEnum; }

	@Column(name="\"isGeneric\"")
	boolean isGeneric;
	public boolean getIsGeneric() { return isGeneric; }
	public void setIsGeneric(boolean isGeneric) { this.isGeneric = isGeneric; }

	@JoinColumn(name="\"enumConstants\"")
	@OneToMany(mappedBy="enumType")
	List<JavaEnumConstant> enumConstants = new LinkedList<JavaEnumConstant>();
	public List<JavaEnumConstant> getEnumConstants() { return enumConstants; }
	public void setEnumConstants(List<JavaEnumConstant> enumConstants) { this.enumConstants = enumConstants; }

	@JoinColumn(name="\"genericImpl\"")
	JavaType genericImpl;
	public JavaType getGenericImpl() { return genericImpl; }
	public void setGenericImpl(JavaType genericImpl) { this.genericImpl = genericImpl; }

	@OneToMany(fetch=FetchType.LAZY, mappedBy="genericImpl")
	List<JavaType> genericInst = new LinkedList<JavaType>();
	public List<JavaType> getGenericInst() { return genericInst; }

	@JoinColumn(name="\"typeParams\"")
	@ManyToMany
	@JoinTable(name="\"JavaType_TypeParams\"",
		joinColumns = @JoinColumn(name = "type", referencedColumnName = "mangledNameHash"), 
		inverseJoinColumns = @JoinColumn(name = "param", referencedColumnName = "mangledNameHash")
	)
	List<JavaType> typeParams = new LinkedList<JavaType>();
	public List<JavaType> getTypeParams() { return typeParams; }
	public void setTypeParams(List<JavaType> typeParams) { this.typeParams = typeParams; }
}
