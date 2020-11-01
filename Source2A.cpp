// Exercise 2A

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

string find_field(const string& xml, string tag_name);

int main() {
	string page, line, location, temperature;
	ifstream inputFile("weather.xml");

	while (getline(inputFile, line)) {
		page.append(line);
		line.erase();
	}

	location = find_field(page, "location");
	temperature = find_field(page, "temp_c");

	cout << "Location: " << location << endl;
	cout << "Temperature: " << temperature << endl;

}


string find_field(const string& xml, string tag_name)
{
	int size;
	string a;
	ifstream myfile;
	myfile.open("weather.xml");
	int i;


	size = tag_name.size();


	int start = xml.find("<" + tag_name + ">");
	int end = xml.find("</" + tag_name + ">");
	if (start != string::npos && end != string::npos) {
		i = end - start;
		a = xml;
		a.erase(0, start);
		cout << a;
		a.erase(0, size + 2);
		a.erase(i);
		a.erase(a.end() - 2 - size, a.end() - 0);
		myfile.close();
		return a;
	}
	else {
		return "Not found";
	}
}