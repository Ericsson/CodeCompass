package javatest;

import javatest.Base;
import javatest.Deriv;

class Main
{
	public enum MyEnum
	{
		Value1,
		Value2,
		Value3,
		Value4
	}

	private static void enumTest()
	{
		MyEnum e = MyEnum.Value1;
		e = MyEnum.Value2;

		if(MyEnum.Value3 == e)
		{
			assert(false);
		}
	}

	private static boolean test(int x)
	{
		return (x%2)==0;
	}

	public static void main(String[] args)
	{
		java.util.List<Base> list = new java.util.ArrayList<Base>();

		if(args!=null)
		{
			for(int i=0;i<args.length;++i)
			{
				list.add(new Deriv(args[i]));
			}
		}

		for(Base b : list)
		{
			b.foo();
		}

		int y = 2; // write 'y'
		y = 3;     // write 'y'
		test(y);   // read 'y'
		int z = y; // read 'y'

		enumTest();
	}
}

