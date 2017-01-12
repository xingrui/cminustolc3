int main(void)
{
    int x;
    int y;
    x = 5;
    y = 0;

    if (x)
        if (y)
            y = 1;
        else
            y = 2;
    else
        y = 3;

    output(y);
    return y;
}
