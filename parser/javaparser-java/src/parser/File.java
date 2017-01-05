// $Id$
// Created by Aron Barath, 2013

package parser;

import javax.persistence.*;

@Entity
@Table(name="\"File\"")
public class File implements java.io.Serializable
{
	public static final int Unknown      = 1;
	public static final int GenericFile  = 2;
	public static final int Directory    = 3;
	public static final int CSource      = 100;
	public static final int CxxSource    = 101;
	public static final int JavaSource   = 102;

	public static final int BashScript   = 201;
	public static final int PerlScript   = 202;
	public static final int PythonScript = 203;
	public static final int RubyScript   = 204;
	public static final int SqlScript    = 205;
	public static final int JavaScript   = 205;

	public static final int JavaClass    = 301;

	public static final int PSNone              = 0;
	public static final int PSPartiallyParsed   = 1;
	public static final int PSFullyParsed       = 2;
	public static final int PSVCView            = 10000;

	private static final long serialVersionUID = 1573473735355383463L;

	public File() { }

	@Column(name="id")
	@Id @GeneratedValue
	long id;
	public long getId() { return id; }
	public void setId(long id) { this.id = id; }

	@Column(name="type", nullable=false)
	int type;
	public int getType() { return type; }
	public void setType(int type) { this.type = type; }

	@Column(name="path", nullable=false)
	String path;
	public String getPath() { return path; }
	public void setPath(String path) { this.path = path; }

	@Column(name="filename")
	String filename;
	public String getFilename() { return filename; }
	public void setFilename(String filename) { this.filename = filename; }

	@Column(name="timestamp")
	long timestamp;
	public long getTimestamp() { return timestamp; }
	public void setTimestamp(long timestamp) { this.timestamp = timestamp; }

	@JoinColumn(name="content")
	FileContent content;
	public FileContent getContent() { return content; }
	public void setContent(FileContent content) { this.content = content; }

	@JoinColumn(name="project", nullable=false)
	@ManyToOne
	Project project;
	public Project getProject() { return project; }
	public void setProject(Project project) { this.project = project; }

	@JoinColumn(name="parent")
	@ManyToOne
	File parent;
	public File getParent() { return parent; }
	public void setParent(File parent) { this.parent = parent; }

	@Column(name="\"parseStatus\"", nullable=false)
	int parseStatus;
	public int getParseStatus() { return parseStatus; }
	public void setParseStatus(int parseStatus) { this.parseStatus = parseStatus; }

	@Column(name="\"inSearchIndex\"")
	boolean inSearchIndex;
	public boolean getInSearchIndex() { return inSearchIndex; }
	public void setInSearchIndex(boolean inSearchIndex) { this.inSearchIndex = inSearchIndex; }

	//--------------------------------

	@Override
	public boolean equals(Object other)
	{
		if(this==other) { return true; }
		if(other==null || !(other instanceof File)) { return false; }
		return id == ((File)other).id;
	}

	@Override
	public int hashCode()
	{
		return (int)id;
	}
}
