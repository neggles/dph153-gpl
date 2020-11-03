#include <math.h>

TYPE
test1 (TYPE a[], TYPE b[], unsigned int size)
{
	TYPE result = 0;
	unsigned int i;

	for(i = 0; i < size; ++i)
		result += a[i] * b[i];

	return result;
}

/* The following test should produce a msub, but gcc cannot generate
   such instruction due to a bug as of 4.2.  */
TYPE
test1_1 (TYPE a[], TYPE b[], unsigned int size)
{
        TYPE result = (TYPE) 100;
        unsigned int i;

        for(i = 0; i < size; ++i)
                result -= a[i] * b[i];

        return result;
}

TYPE
test2 (TYPE x)
{
	return sqrt (x);
}

TYPE
test3 (TYPE x)
{
	return ((TYPE) 1) / sqrt (x);
}

TYPE
test4 (TYPE x)
{
	return ((TYPE) 1) / x;
}

TYPE
test5 (TYPE x, TYPE y)
{
	return x * y;
}
