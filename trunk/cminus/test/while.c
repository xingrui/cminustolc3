int x;
int y[100];
int z;

int f(int a[])
{
	int x;
	x = 0;
	output(a[0]);
	output(a[1]);
	output(a[2]);
	a[0] = a[0] * a[0];
	a[1] = a[1] * a[1];
	a[2] = a[2] * a[2];
	output(a);
	return 0;
}

int main(void)
{
	int i;
	int j;
	int a[3];
	j = 3;
	i = 15;
	a[0] = 1;
	a[1] = 2;
	a[2] = 3;
	output(a);
	f(a);
	i = 0;

	while (i < j)
	{
		output(a[i]);
		i = i + 1;
	}
}
