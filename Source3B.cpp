// Exercise 3B

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <array>

using namespace std;
void backwards(vector <string> arr);

int main() {
    string str;
    int i = 0;
    int j = 0;


    vector<string> arr(10);


    cout << "Enter the string, please\n";
    while ((cin >> str) && str != "stop") {


        arr.push_back(str);
        cout << "Enter the string, please\n";
        j++;
    }


    cout << "List of Strings:";

    for (i = 0; i < arr.size(); i++) {
        cout << arr[i] << "\n";
    }



    cout << "\n";

    cout << "List of Strings after the function:" << "\n";

    backwards(arr);


    cout << "Number of strings: " << j << "\n";





}

void backwards(vector <string> arr) {

    int i = 0;

    for (i = 0; i < arr.size(); i++) {
        reverse(arr[i].begin(), arr[i].end());
    }
    for (i = 0; i < arr.size(); i++) {
        sort(arr.begin(), arr.end(), greater <>());
    }
    for (i = 0; i < arr.size(); i++) {
        cout << arr[i] << "\n";
    }

}
