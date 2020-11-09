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

    friend ostream& operator<<(ostream& out, const Time& c);

    Time operator+(const Time& a) {
        Time b;

        b.m = m + a.m;
        b.h = h + a.h;

        if (b.m > 60) {
            b.m -= 60;
            b.h += 1;
        }
        else if (b.h > 23) {
            b.h = b.h % 24;
        }

 
        return b;
    }
    Time operator-(const Time& a) {
        Time b;
        b.m = m - a.m;
        b.h = h - a.h;


        if (b.h < 0) {
            b.h = b.h * (-1);
            if (b.m < 0) {
                b.m = b.m * (-1);
            }
        }
        else if (b.m < 0) {
            b.m = b.m * (-1);
            b.m = 60 - b.m;
            b.h = b.h - 1;
        }

        return b;
    }
    bool operator < (const Time& a) {
        if (h < a.h) {
            return true;
        }
        else if (h == a.h && m < a.m) {
            return true;
        }
        else if (h == a.h && m > a.m) {
            return false;
        }
        else {
            return false;
        }
    }
    Time& operator++ (int);
    Time& operator++ ();

private:
    int h = 0;
    int m = 0;

};

ostream& operator << (ostream& out, const Time& c)
{
    out << c.h << ":" << c.m;
    return out;
}

Time& Time::operator++(int) {
    Time abc = *this;
    m++;
    return *this;
}
Time& Time::operator++() {
    Time abc = *this;
    ++m;
    return *this;
}


void print(const vector<Time>& v)
{
    for (auto& t : v) {
        cout << t << endl;
    }
}



int main() {
    Time time1, time2, duration;

    time1.read("Enter time 1: ");
    time2.read("Enter time 2: ");
    if (time1 < time2) {
        duration = time2 - time1;
        cout << "Starting time was " << time1 << endl;
    }
    else {
        duration = time1 - time2;
        cout << "Starting time was " << time2 << endl;
    }
    cout << "Duration was " << duration << endl;

    vector<Time> tv(5);
    for (auto& t : tv) {
        t.read("Enter time:");
    }

    cout << "Times: " << endl;
    print(tv);

    Time sum;
    for (auto t : tv) {
        sum = sum + t;
    }

    cout << "Sum of times: " << sum << endl;

    cout << "Post-increment: " << endl;
    print(tv);
    for (auto& t : tv) {
        cout << t++ << endl;
    }

    print(tv);

    cout << "Pre-increment: " << endl;
    for (auto& t : tv) {
        cout << ++t << endl;
    }

    sort(tv.begin(), tv.end());

    cout << "Sorted times: " << endl;
    print(tv);

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
