package two;

public class Foo
{
  Bar bar = new Bar();

  static final int VALUE = 42;

  public Foo()
  {
    Bar.setLogic(true);
    bar.bar(false);
  }

  boolean foo()
  {
    bar.shared = 3;
    return bar.bar();
  }
}

