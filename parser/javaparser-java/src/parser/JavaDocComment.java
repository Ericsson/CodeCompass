// $Id$
// Created by Aron Barath, 2014

package parser;

import javax.persistence.*;

@Entity
@Table(name="\"JavaDocComment\"")
public class JavaDocComment implements java.io.Serializable
{
	private static final long serialVersionUID = 5724573758447684648L;

	JavaDocComment() { }

	public JavaDocComment(long contentHash, String contentHTML, long mangledNameHash)
	{
		this.contentHash = contentHash;
		this.contentHTML = contentHTML;
		this.mangledNameHash = mangledNameHash;
	}

	@Column(name="id")
	@Id @GeneratedValue
	int id;
	public int getId() { return id; }
	private void setId(int id) { this.id = id; }

	@Column(name="\"contentHash\"")
	long contentHash;
	public long getContentHash() { return contentHash; }
	public void setContentHash(long contentHash) { this.contentHash = contentHash; }

	@Column(name="\"contentHTML\"")
	String contentHTML;
	public String getContentHTML() { return contentHTML; }
	public void setContentHTML(String contentHTML) { this.contentHTML = contentHTML; }

	@Column(name="\"mangledNameHash\"")
	long mangledNameHash;
	public long getMangledNameHash() { return mangledNameHash; }
	public void setMangledNameHash(long mangledNameHash) { this.mangledNameHash = mangledNameHash; }
}
