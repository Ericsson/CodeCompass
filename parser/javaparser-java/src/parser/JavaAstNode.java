// $Id$
// Created by Aron Barath, 2013

package parser;

import javax.persistence.*;

@Entity
@Table(name="\"JavaAstNode\"")
public class JavaAstNode implements java.io.Serializable
{
	private static final long serialVersionUID = 1988594095685833463L;

	public JavaAstNode() { }
	public JavaAstNode(long id) { this.id = id; }

	public class Modifier
	{
		public static final int Public       = 0x00000001;
		public static final int Protected    = 0x00000002;
		public static final int Private      = 0x00000004;
		public static final int Static       = 0x00000008;
		public static final int Abstract     = 0x00000010;
		public static final int Final        = 0x00000020;
		public static final int Native       = 0x00000040;
		public static final int Synchronized = 0x00000080;
		public static final int Transient    = 0x00000100;
		public static final int Volatile     = 0x00000200;
		public static final int Strictfp     = 0x00000400;
	}

	public enum SymbolType
	{
		Variable(0),
		Function(1),
		FunctionPointer(2),
		Type(3),
		Typedef(4),
		Macro(5),
		Enum(6),
		EnumConstant(7),
		Import(8),
		File(500),
		Other(1000); // TODO: should be the first (for value zero)

		@SuppressWarnings("unused")
		private int value;

		private SymbolType(int value)
		{
			this.value = value;
		}
	}

	public enum AstType
	{
		Statement(0),
		TypeLocation(1),
		Declaration(2),
		Definition(3),
		UnDefinition(4), // for Macros
		Usage(5),
		Read(6),
		Write(7),
		VirtualCall(8),
		ParameterTypeLoc(9),
		ReturnTypeLoc(10),
		FieldTypeLoc(11),
		GlobalTypeLoc(12),
		LocalTypeLoc(13),
		Other(1000); // TODO: should be the first (for value zero)

		@SuppressWarnings("unused")
		private int value;

		private AstType(int value)
		{
			this.value = value;
		}
	}

	@Column(name="id")
	@Id
	long id;
	public long getId() { return id; }
	private void setId(long id) { this.id = id; }

	@Override
	public int hashCode()
	{
		final int prime = 31;
		int result = 1;
		result = prime * result + (int) (id ^ (id >>> 32));
		return result;
	}

	@Override
	public boolean equals(Object obj)
	{
		if(this == obj)
			return true;
		if(obj == null)
			return false;
		if(getClass() != obj.getClass())
			return false;
		JavaAstNode other = (JavaAstNode) obj;
		if(id != other.id)
			return false;
		return true;
	}

	@JoinColumn(name="file")
	@ManyToOne
	File file;
	public File getFile() { return file; }
	public void setFile(File f) { file = f; }

	@Column(name="\"astValue\"")
	String astValue;
	public String getAstValue() { return astValue; }
	public void setAstValue(String s) { astValue = s; }

	@Column(name="loc_start_line")
	int loc_start_line;
	public int getLoc_start_line() { return loc_start_line; }
	public void setLoc_start_line(int loc_start_line) { this.loc_start_line = loc_start_line; }

	@Column(name="loc_start_col")
	int loc_start_col;
	public int getLoc_start_col() { return loc_start_col; }
	public void setLoc_start_col(int loc_start_col) { this.loc_start_col = loc_start_col; }

	@Column(name="loc_end_line")
	int loc_end_line;
	public int getLoc_end_line() { return loc_end_line; }
	public void setLoc_end_line(int loc_end_line) { this.loc_end_line = loc_end_line; }

	@Column(name="loc_end_col")
	int loc_end_col;
	public int getLoc_end_col() { return loc_end_col; }
	public void setLoc_end_col(int loc_end_col) { this.loc_end_col = loc_end_col; }

	@Column(name="\"mangledName\"")
	String mangledName;
	public String getMangledName() { return mangledName; }
	public void setMangledName(String mn) { mangledName = mn; }

	@Column(name="\"mangledNameHash\"")
	long mangledNameHash;
	public long getMangledNameHash() { return mangledNameHash; }
	public void setMangledNameHash(long mnh) { this.mangledNameHash = mnh; }

	@Column(name="\"symbolType\"")
	SymbolType symbolType;
	public SymbolType getSymbolType() { return symbolType; }
	public void setSymbolType(SymbolType st) { this.symbolType = st; }

	@Column(name="\"astType\"")
	AstType astType;
	public AstType getAstType() { return astType; }
	public void setAstType(AstType astType) { this.astType = astType; 	}
}
