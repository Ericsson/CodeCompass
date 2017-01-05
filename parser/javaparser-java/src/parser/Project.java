// $Id$
// Created by Aron Barath, 2013

package parser;

import javax.persistence.*;

@Entity
@Table(name="\"Project\"")
public class Project implements java.io.Serializable
{
	private static final long serialVersionUID = 1429867279426833463L;

	public Project() { }
	public Project(long id) { this.id = id; }

	@Column(name="id")
	@Id @GeneratedValue
	long id;
	public long getId() { return id; }
	public void setId(long id) { this.id = id; }

	//--------------------------------

	@Override
	public boolean equals(Object other)
	{
		if(this==other) { return true; }
		if(other==null || !(other instanceof Project)) { return false; }
		return id == ((Project)other).id;
	}

	@Override
	public int hashCode()
	{
		return (int)id;
	}
}
