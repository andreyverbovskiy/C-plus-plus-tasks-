#include <iostream>
#include <string>
#include <cstdlib>
using namespace std;


float mean(int* array, int n);


float mean(int* array, int n) {

    float sum = 0;

    for (int j = 0; j < n; j++) {
        sum = sum + (float)array[j];
    }

    sum = sum / n;
    return sum;
}


int main() {

    int str1;
    int* a;
    int i;
    int sum1;
    float x;
    cout << "Enter amount of numbers:\n";
    cin >> str1;


    a = new int[str1];

    for (i = 0; i < str1; i++) {
        a[i] = rand() % 100;
        cout << a[i] << "\n";
    }

    x = mean(a, str1);
    cout << x;
    delete a;

}