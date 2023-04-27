#include <iostream>
#include <trs/MatrixN.h>

using namespace std;

int main(void) {
	int n;
	cin >> n;
	
	TRS::VectorN<float> v1(n);
	for (size_t i = 0; i < n; i++)
		cin >> v1[i];

	TRS::VectorN<float> v2(n);
	for (size_t i = 0; i < n; i++)
		cin >> v2[i];

	string cmd;
	cin >> cmd;

	if (cmd == "ADD") {
		auto res = v1 + v2;
		for (size_t i = 0; i < n; i++)
			cout << res[i] << ' ';
		cout << endl;
	}
	else if (cmd == "SUB") {
		auto res = v1 - v2;
		for (size_t i = 0; i < n; i++)
			cout << res[i] << ' ';
		cout << endl;
	}
	else if (cmd == "DOT") {
		float res = v1 * v2;
		cout << res << endl;
	}

	return 0;
}