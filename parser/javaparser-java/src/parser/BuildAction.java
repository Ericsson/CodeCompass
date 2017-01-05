// $Id$

package parser;

import java.util.LinkedList;
import java.util.List;

import javax.persistence.*;

@Entity
@Table(name="\"BuildAction\"")
public class BuildAction implements java.io.Serializable
{
	private static final long serialVersionUID = 1570473735385383463L;

	public BuildAction() { }

	public class Type
	{
		public static final int Compile = 0;
		public static final int Link    = 1;
		public static final int Other   = 2;
	}

	@Column(name="id")
	@Id
	long id;
	public long getId() { return id; }
	public void setId(long id) { this.id = id; }

	@Column(name="label", nullable=false)
	String label;
	public String getLabel() { return label; }
	public void setLabel(String label) { this.label = label; }

	@JoinColumn(name="project", nullable=false)
	Project project;
	public Project getProject() { return project; }
	public void setProject(Project project) { this.project = project; }

	@Column(name="type")
	int type;
	public int getType() { return type; }
	public void setType(int type) { this.type = type; }

	@JoinColumn(name="parameters")
	@OneToMany(mappedBy="action")
	List<BuildParameter> parameters = new LinkedList<BuildParameter>();
	public List<BuildParameter> getParameters() { return parameters; }
	public void setParameters(List<BuildParameter> parameters) { this.parameters = parameters; }

	@JoinColumn(name="sources")
	@OneToMany(mappedBy="action")
	List<BuildSource> sources = new LinkedList<BuildSource>();
	public List<BuildSource> getSources() { return sources; }
	public void setSources(List<BuildSource> sources) { this.sources = sources; }

	@JoinColumn(name="targets")
	@OneToMany(mappedBy="action")
	List<BuildTarget> targets = new LinkedList<BuildTarget>();
	public List<BuildTarget> getTargets() { return targets; }
	public void setTargets(List<BuildTarget> targets) { this.targets = targets; }

	@JoinColumn(name="log")
	@OneToMany(mappedBy="action")
	List<BuildLog> log = new LinkedList<BuildLog>();
	public List<BuildLog> getLog() { return log; }
	public void setLog(List<BuildLog> log) { this.log = log; }
}
