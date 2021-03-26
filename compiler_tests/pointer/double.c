double f()
{
    double x;
    double *y=&x;
    *y=64.4+3.0;
    *y = *y+4.0;
    return x;
}
