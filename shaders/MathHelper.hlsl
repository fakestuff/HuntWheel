#ifndef MATH_HELPER
#define MATH_HELPER
#define PI 3.1415926
float Pow2(float x)
{
  return x*x;
}

float Pow4(float x)
{
  float x2 = Pow2(x);
  return x2 * x2;
}

float Pow5(float x)
{
  return Pow4(x) * x;
}

float Square(float x)
{
    return x*x;
}


#endif