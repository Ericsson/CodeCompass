// $Id$
// Created by Aron Barath, 2013

package parser;

import javax.persistence.*;

@Entity
@Table(name="\"FileContent\"")
public class FileContent implements java.io.Serializable
{
	private static final long serialVersionUID = 1991375932464233463L;

	public FileContent() { }

	@Column(name="hash", nullable=false)
	@Id
	String hash;
	public String getHash() { return hash; }
	public void setHash(String hash) { this.hash = hash; }

	@Column(name="content", nullable=false)
	String content;
	public String getContent() { return content; }
	public void setContent(String content) { this.content = content; }

	//--------------------------------

	@Override
	public boolean equals(Object other)
	{
		if(this==other) { return true; }
		if(other==null || !(other instanceof FileContent)) { return false; }
		return hash.equals(((FileContent)other).hash);
	}

	@Override
	public int hashCode()
	{
		return hash.hashCode();
	}
}
