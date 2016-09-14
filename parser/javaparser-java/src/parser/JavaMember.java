// $Id$
// Created by Aron Barath, 2013

package parser;

import javax.persistence.*;

@Entity
@Table(name="\"JavaMember\"")
public class JavaMember implements java.io.Serializable
{
	private static final long serialVersionUID = 8371821735538252063L;

	public JavaMember() { }

	public JavaMember(JavaType type, JavaAstNode node, JavaType fieldType, int mods)
	{
		this.type = type;
		this.fieldType = fieldType;
		this.astNode = node;
		this.modifiers = mods;
	}

	@Column(name="id")
	@Id @GeneratedValue
	int id;
	public int getId() { return id; }
	public void setId(int id) { this.id = id; }

	@Column(name="name")
	String name;
	public String getName() { return name; }
	public void setName(String name) { this.name = name; }

	/**
	 * This member/field is in this type/class.
	 */
	@JoinColumn(name="type")
	@ManyToOne
	JavaType type;
	public JavaType getType() { return type; }
	public void setType(JavaType type) { this.type = type; }

	/**
	 * Type of the current field.
	 */
	@JoinColumn(name="\"fieldType\"")
	@ManyToOne
	JavaType fieldType;
	public JavaType getFieldType() { return fieldType; }
	public void setFieldType(JavaType fieldType) { this.fieldType = fieldType; }

	@JoinColumn(name="\"astNodePtr\"") // TODO: rename to astNode
	@ManyToOne
	JavaAstNode astNode;
	public JavaAstNode getAstNode() { return astNode; }
	public void setAstNode(JavaAstNode astNode) { this.astNode = astNode; }

	@Column(name="modifiers")
	int modifiers;
	public int getModifiers() { return modifiers; }
	public void setModifiers(int modifiers) { this.modifiers = modifiers; }
}
