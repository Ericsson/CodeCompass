package two;

/**
 * Documentation comment of the Bar class.
 */
public class Bar
{
  private static boolean staticLogic = false;
  boolean logic = staticLogic;
  public int shared = 2;

  public Bar()
  {
  }

  public Bar(boolean l)
  {
    logic = l;
  }

  boolean bar()
  {
    return logic;
  }

  void bar(boolean l)
  {
    logic = l;
  }

  public static void setLogic(boolean b)
  {
    staticLogic = b;
  }

  /**
   * Documentation comment of the testFooVALUE method.
   */
  public boolean testFooVALUE(int other)
  {
    return other == Foo.VALUE;
  }
}

