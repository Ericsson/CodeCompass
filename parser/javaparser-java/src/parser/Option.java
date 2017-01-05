// $Id$

package parser;

import javax.persistence.*;

@Entity
@Table(name="\"Option\"")
public class Option implements java.io.Serializable
{
	private static final long serialVersionUID = 1570473735355383463L;

	public Option() { }

	@Column(name="id")
	@Id @GeneratedValue
	long id;
	public long getId() { return id; }
	public void setId(long id) { this.id = id; }

	@Column(name="key", nullable=false)
	String key;
	public String getKey() { return key; }
	public void setKey(String key) { this.key = key; }

	@Column(name="value", nullable=false)
	String value;
	public String getValue() { return value; }
	public void setValue(String value) { this.value = value; }
}
