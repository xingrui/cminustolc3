int x;
int y[100];
int z;

int f(int a)
{
    return a * a * a * a;
}

int main(void)
{
    int i;
    int j;
    int a[13];
    i = 0;
    j = input();
    output(j);

    while (i < j) {
        a[i] = f(i);
        i = i + 1;
    }

    i = 0;

    while (i < j) {
        output(a[i]);
        i = i + 1;
    }
}
