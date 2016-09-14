// $Id$

package parser;

import javax.persistence.*;

@Entity
@Table(name="\"BuildParameter\"")
public class BuildParameter implements java.io.Serializable
{
	private static final long serialVersionUID = 1570473935248383463L;

	public BuildParameter() { }

	@Column(name="id")
	@Id @GeneratedValue
	long id;
	public long getId() { return id; }
	public void setId(long id) { this.id = id; }

	@Column(name="param", nullable=false)
	String param;
	public String getParam() { return param; }
	public void setParam(String param) { this.param = param; }

	@JoinColumn(name="action", nullable=false)
	BuildAction action;
	public BuildAction getAction() { return action; }
	public void setAction(BuildAction action) { this.action = action; }
}
