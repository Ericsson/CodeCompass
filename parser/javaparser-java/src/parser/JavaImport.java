// $Id$
// Created by Aron Barath, 2014

package parser;

import javax.persistence.*;

@Entity
@Table(name="\"JavaImport\"")
public class JavaImport implements java.io.Serializable
{
	private static final long serialVersionUID = 5473614587269514848L;

	JavaImport() { }

	public JavaImport(File file, JavaAstNode node, String name, boolean staticImport, boolean onDemand, boolean explicitImport)
	{
		this.file = file;
		this.astNode = node;
		this.mangledNameHash = node.getMangledNameHash();
		this.name = name;
		this.staticImport = staticImport;
		this.onDemand = onDemand;
		this.explicitImport = explicitImport;
	}

	@Column(name="id")
	@Id @GeneratedValue
	int id;
	public int getId() { return id; }
	private void setId(int id) { this.id = id; }

	@JoinColumn(name="file")
	@ManyToOne
	File file;
	public File getFile() { return file; }
	public void setFile(File f) { file = f; }

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

	@Column(name="\"staticImport\"")
	boolean staticImport;
	public boolean getStaticImport() { return staticImport; }
	public void setStaticImport(boolean staticImport) { this.staticImport = staticImport; }

	@Column(name="\"onDemand\"")
	boolean onDemand;
	public boolean getOnDemand() { return onDemand; }
	public void setOnDemand(boolean onDemand) { this.onDemand = onDemand; }

	@Column(name="\"explicitImport\"")
	boolean explicitImport;
	public boolean getExplicitImport() { return explicitImport; }
	public void setExplicitImport(boolean explicitImport) { this.explicitImport = explicitImport; }
}
