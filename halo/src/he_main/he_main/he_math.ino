/*
    Project : HALO-1
    Author  : Jeff King
    Date    : 10/1/2018
*/

/* ----------------------------------------------
    getSumFloat : Sum of an array of float types
-----------------------------------------------*/
// float arr[]  : Array to be summed
// int size     : Size of array
// ----------------------------------------------
float getSumFloat(float arr[], int size) {
    float sum = 0.0;
    for (int i=0; i < size; i++)
    {
        sum += arr[i];
    }

    return sum;
}

/* ----------------------------------------------
    getSumPowFloat : Sum of squares of an array of float types
-----------------------------------------------*/
// float arr[]  : Array to be squared then summed
// int size     : Size of array
// int power    : Exponent to raise to this power
// ----------------------------------------------
float getSumPowFloat(float arr[], int size, int power) {
    float sumPow = 0.0;
    for (int i=0; i < size; i++)
    {
        sumPow += pow(arr[i], power);
    }

    return sumPow;
}
