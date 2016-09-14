package javatest;

import javatest.Base;

class Deriv extends Base implements Iface
{
	String str;

	public final int something;

	public Deriv(String s)
	{
		super(4);
		str = s;
		something = 6;
	}

	public void foo()
	{
		System.out.println("Deriv.foo() : " + str);
	}

	public boolean foo2()
	{
		return true;
	}
}

