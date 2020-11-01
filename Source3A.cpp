#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <array>


using namespace std;


int main() {
    double str;
    int i = 0;
    int j = 0;



    vector<double> arr(10);


    cout << "Enter the string, please\n";
    while ((cin >> str) && str > 0) {

        arr.push_back(str);
        cout << "Enter the double, please\n";
        j++;
    }

    sort(arr.begin(), arr.end());

    cout << "List of Numbers:";

    for (i = 0; i < arr.size(); i++) {
        cout << std::setprecision(3) << arr[i] << "\n";
    }


    cout << "Number of strings: " << j << "\n";




}