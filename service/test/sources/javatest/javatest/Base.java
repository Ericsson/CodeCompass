package javatest;

import java.util.Objects;

class Base
{
	public static enum Ty
	{
		TyDefault,
		TyInt
	}

	public final Ty ty;

	public Base()
	{
		ty = Ty.TyDefault;
	}

	public Base(int value)
	{
		ty = Ty.TyInt;
		this.value = value;
	}

	private int value = 0;
	public int getValue() { return value; }
	public void setValue(int value) { this.value = value; }

	public void foo()
	{
		System.out.println("Base.foo()");
	}

	public String toString()
	{
		return "{ty=" + ty + ",val=" + value + "}";
	}

	public int hashCode()
	{
		int hash = 7;
		hash = 83 * hash + Objects.hashCode(this.ty);
		return hash;
	}

	public boolean equals(Object other)
	{
		if(null==other || !(other instanceof Base))
		{
			return false;
		}

		return value == ((Base)other).value;
	}
}

