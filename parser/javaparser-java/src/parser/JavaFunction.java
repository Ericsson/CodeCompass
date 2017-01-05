// $Id$
// Created by Aron Barath, 2013

package parser;

import java.util.LinkedList;
import java.util.List;

import javax.persistence.*;

@Entity
@Table(name="\"JavaFunction\"")
public class JavaFunction implements java.io.Serializable
{
	private static final long serialVersionUID = 8381855565578252763L;

	public JavaFunction() { }

	// @Column(name="id")
	// @Id @GeneratedValue
	// int id;
	// public int getId() { return id; }
	// public void setId(int id) { this.id = id; }

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

	@Column(name="modifiers")
	int modifiers;
	public int getModifiers() { return modifiers; }
	public void setModifiers(int modifiers) { this.modifiers = modifiers; }

	@Column(name="name")
	String name;
	public String getName() { return name; }
	public void setName(String name) { this.name = name; }

	@Id
	@Column(name="\"mangledNameHash\"")
	long mangledNameHash;
	public long getMangledNameHash() { return mangledNameHash; }
	public void setMangledNameHash(long mnh) { this.mangledNameHash = mnh; }

	@Column(name="\"mangledName\"")
	String mangledName;
	public String getMangledName() { return mangledName; }
	public void setMangledName(String mangledName) { this.mangledName = mangledName; }

	@Column(name="signature")
	String signature; // <function name>(<arguments>)
	public String getSignature() { return signature; }
	public void setSignature(String signature) { this.signature = signature; }

	@Column(name="\"qualifiedName\"")
	String qualifiedName;
	public String getQualifiedName() { 	return qualifiedName; }
	public void setQualifiedName(String qualifiedName) { this.qualifiedName = qualifiedName; }

	@Column(name="\"isGeneric\"")
	boolean isGeneric;
	public boolean getIsGeneric() { return isGeneric; }
	public void setIsGeneric(boolean isGeneric) { this.isGeneric = isGeneric; }

	@JoinColumn(name="\"genericImpl\"")
	JavaFunction genericImpl;
	public JavaFunction getGenericImpl() { return genericImpl; }
	public void setGenericImpl(JavaFunction genericImpl) { this.genericImpl = genericImpl; }

	@JoinColumn(name="\"returnType\"")
	@ManyToOne
	JavaType returnType;
	public JavaType getReturnType() { return returnType; }
	public void setReturnType(JavaType returnType) { this.returnType = returnType; }

	@JoinColumn(name="parameters")
	@OneToMany(mappedBy="paramInFunc")
	List<JavaVariable> parameters = new LinkedList<JavaVariable>();
	public List<JavaVariable> getParameters() { return parameters; }
	public void setParameters(List<JavaVariable> parameters) { this.parameters = parameters; }

	@JoinColumn(name="locals")
	@OneToMany(mappedBy="localInFunc")
	List<JavaVariable> locals = new LinkedList<JavaVariable>();
	public List<JavaVariable> getLocals() { return locals; }
	public void setLocals(List<JavaVariable> locals) { this.locals = locals; }

	@OneToMany(fetch=FetchType.LAZY, mappedBy="genericImpl")
	List<JavaFunction> genericInst = new LinkedList<JavaFunction>();
	public List<JavaFunction> getGenericInst() { return genericInst; }

	@JoinColumn(name="\"typeParams\"")
	@ManyToMany
	@JoinTable(name="\"JavaFunction_TypeParams\"",
		joinColumns = @JoinColumn(name = "function", referencedColumnName = "mangledNameHash"), 
		inverseJoinColumns = @JoinColumn(name = "param", referencedColumnName = "mangledNameHash")
	)
	List<JavaType> typeParams = new LinkedList<JavaType>();
	public List<JavaType> getTypeParams() { return typeParams; }
	public void setTypeParams(List<JavaType> typeParams) { this.typeParams = typeParams; }
}
