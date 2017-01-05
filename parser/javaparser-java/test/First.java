public class First
{
  int xx = 3;
  public int yy = 4;

  public First()
  {
    gggg();
  }

  private void priv(int parameter)
  {
    if(false)
    {
      priv(parameter);
    }
  }

  private boolean zero(long longParam)
  {
    return longParam == 0l;
  }

  public int ffff()
  {
    int tt = 0;
    priv(123);
    for(int i=1;i<=10;++i) { tt += i; }
    zero(tt);
    return tt;
  }

  int gggg()
  {
    priv(321);
    ++yy;
    xx = xx + yy;
    callFirst();
    return xx;
  }

  public void callFirst()
  {
    boolean defineLater = zero(0);
  }
}

