#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <array>


using namespace std;

class Time {
public:
	void read(string a);
	bool lessThan(Time x);
	Time subtract(Time y);
	void display();
private:
	int h;
	int m;

};

int main() {
	Time time1, time2, duration;

	time1.read("Enter time 1: ");
	time2.read("Enter time 2: ");
	if (time1.lessThan(time2)) {
		duration = time2.subtract(time1);
		cout << "Starting time was ";
		cout << "\n" << "this1\n";
		time1.display();
	}
	else {
		duration = time1.subtract(time2);
		cout << "Starting time was ";
		cout << "\n" << "this2\n";

		time2.display();
	}
	cout << "\n";
	cout << "Duration was ";
	duration.display();

	return 0;
}


void Time::read(string a) {
	char dots;
	cout << a;
	cin >> h >> dots >> m;
}

bool Time::lessThan(Time x) {
	if (h > x.h || (h == x.h && m > x.m)) {
		return false;
	}
	else {
		return true;
	}
}

Time Time::subtract(Time y) {
	Time sub;
	int mins;
	int hours;

	if (y.m < m && y.h < h) {
		mins = m - y.m;
		hours = h - y.h;
	}
	else {
		m += 60;
		mins = m - y.m;
		h -= 1;
		hours = h - y.h;
	}

	sub.h = hours;
	sub.m = mins;
	if (sub.h < 0) {
		sub.h = sub.h * (-1);
	}
	return sub;


}

void Time::display() {
	cout << h << ":" << m;
}
