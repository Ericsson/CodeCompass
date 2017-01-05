int first()
{
  int i, sum, done;
  
  i=1;
  sum=0;
  done=0;
  
  while( i<=10 && !done )
  {
    int j;
    j = 5;
    if(j >= 0)
    {
      sum += j;    
    
      if(sum > 100)
        done=1;
      else
        ++i;    
    }
  }
  ++sum;
  return 0;
}

void incrementtest()
{
  int x;
  int y;
  
  x = 0;
  y = 1;
  
  ++x;
  y++;
  int z;
  int zz;
  
  z = x;
  zz = y;
  
}

void for_test()
{
  int x;
  x = 3;
  for(int i=0; i<x; ++i)
  {
    --x;
  }
}

void do_while_test()
{
  int x;
  x = 3;
  do
  {
    ++x;
  } while(x < 10);
  x = 10;
}

void if_test()
{
  int x;
  x = 3;
  if(x > 0)
    ++x;
  else
    --x;
}

void while_test()
{
  int x;
  x = 3;
  while (x > 0)
  {
    --x;
  }
}

int helper(int a)
{
  return a+1;
}

void argument()
{
  int x, y;
  x = 0;
  y = x;
  x = helper(x);
  ++x;
}

void decltest1()
{
  int x = 5;
  ++x;
  --x;
}

void decltest2()
{
  int x = 5, y;
  ++x;
  y = x;
  --y;
}

void decltest3()
{
  int x = 5;  
  ++x;
  int y = x;
  --y;
}

int returntest1()
{
  int x = 9;
  ++x;
  return x+3;
}

int returntest2(int a, int b)
{
  int x = 5;
  ++x;
  int y = 0;
  ++y;
  int z = 1;
  z = x + a;
  return z + b;
}
