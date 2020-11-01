// Exercise 2B

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;


int main() {
    string str;
    int i = 0;
    int j = 0;
    int length[50];
    int k = 0;
    int count = 0;
    string count1[50];
    int size;
    int capacity;


    vector<string> arr(10);


    cout << "Enter the string, please\n";
    while ((cin >> str) && str != "stop") {
        length[j] = str.length();
        count1[j] = str;
        arr.push_back(str);
        //arr[j]=str;   second way of doing that
        cout << "Enter the string, please\n";
        j++;
    }

    cout << "List of Strings:";

    for (i = 0; i < arr.size(); i++) {
        cout << arr[i] << "\n";
    }

    cout << "Number of strings: " << j << "\n";



    for (i = 0; i < j; i++) {
        if (length[i] > k) {
            k = length[i];
            count = i;
        }
    }

    cout << "Largest String: " << count1[count] << "\n";
    cout << "Length of the largest string: " << k << " characters\n";


    size = arr.size();
    cout << size << "\n";
    capacity = arr.capacity();
    cout << capacity;

}
