#include <iostream>
#include <variant>
#include <trs/MatrixN.h>

using namespace std;

void ReadArgument(variant<TRS::VectorN<float>, TRS::MatrixN<float>>& arg, int n) {
	string cmd;
	cin >> cmd;

	if (cmd == "VEC") {
		arg.emplace<0>(n);
		auto& v = std::get<0>(arg);

		for (int i = 0; i < n; i++)
			cin >> v[i];
	}
	else if (cmd == "MAT") {
		arg.emplace<1>(n);
		auto& m = std::get<1>(arg);

		for (int i = 0; i < n; i++)
			for (int j = 0; j < n; j++)
				cin >> m[i][j];
	}
}

int main(void) {
	int n;
	cin >> n;
	cin.tie(0);
	ios::sync_with_stdio(false);

	variant<TRS::VectorN<float>, TRS::MatrixN<float>> arg1;
	ReadArgument(arg1, n);
	variant<TRS::VectorN<float>, TRS::MatrixN<float>> arg2;
	ReadArgument(arg2, n);


	string cmd;
	cin >> cmd;

	if (cmd == "MUL") {
		if (arg1.index() == 0 && arg2.index() == 0) {
			auto& v1 = std::get<0>(arg1);
			auto& v2 = std::get<0>(arg2);
			cout << v1 * v2 << '\n';
		}
		else if (arg1.index() == 0 && arg2.index() == 1) {
			auto& v1 = std::get<0>(arg1);
			auto& m2 = std::get<1>(arg2);
			auto res = v1 * m2;

			for (float v : res)
				cout << v << ' ';
			cout << '\n';
		}
		else if (arg1.index() == 1 && arg2.index() == 0) {
			auto& m1 = std::get<1>(arg1);
			auto& v2 = std::get<0>(arg2);
			auto res = m1 * v2;

			for (float v : res)
				cout << v << ' ';
			cout << '\n';
		}
		else {
			auto& m1 = std::get<1>(arg1);
			auto& m2 = std::get<1>(arg2);
			auto res = m1 * m2;

			for (auto it1 = res.begin(); it1 != res.end(); it1++) {
				for (auto it2 = it1->begin(); it2 != it1->end(); it2++)
					cout << *it2 << ' ';
				cout << '\n';
			}
		}
	}
	else if (cmd == "ADD") {
		if (arg1.index() != arg2.index())
			cerr << "Cannot add " << (arg1.index() ? "vector " : "matrix ") << "to " << (arg2.index() ? "vector " : "matrix ") << '\n';
		else if (arg1.index() == 0) {
			auto& v1 = std::get<0>(arg1);
			auto& v2 = std::get<0>(arg2);
			auto res = v1 + v2;

			for (float v : res)
				cout << v << ' ';
		}
		else {
			auto& m1 = std::get<1>(arg1);
			auto& m2 = std::get<1>(arg2);
			auto res = m1 + m2;

			for (auto it1 = res.begin(); it1 != res.end(); it1++) {
				for (auto it2 = it1->begin(); it2 != it1->end(); it2++)
					cout << *it2 << ' ';
				cout << '\n';
			}
		}
	}
	else if (cmd == "SUB") {
		if (arg1.index() != arg2.index())
			cerr << "Cannot substract " << (arg2.index() ? "vector " : "matrix ") << "from " << (arg1.index() ? "vector " : "matrix ") << '\n';
		else if (arg1.index() == 0) {
			auto& v1 = std::get<0>(arg1);
			auto& v2 = std::get<0>(arg2);
			auto res = v1 - v2;

			for (float v : res)
				cout << v << ' ';
		}
		else {
			auto& m1 = std::get<1>(arg1);
			auto& m2 = std::get<1>(arg2);
			auto res = m1 - m2;

			for (auto it1 = res.begin(); it1 != res.end(); it1++) {
				for (auto it2 = it1->begin(); it2 != it1->end(); it2++)
					cout << *it2 << ' ';
				cout << '\n';
			}
		}
	}
	else if (cmd == "DET") {
		if (arg1.index() != 1 || arg2.index() != 1)
			cerr << "Cannot find determinant from a vector\n";
		else {
			auto& m1 = std::get<1>(arg1);
			auto& m2 = std::get<1>(arg2);

			cout << "arg1 determinant is: " << m1.Determinant() << '\n';
			cout << "arg2 determinant is: " << m2.Determinant() << '\n';
		}
	}
	else if (cmd == "INV") {
		if (arg1.index() != 1 || arg2.index() != 1)
			cerr << "Cannot find inverse from a vector\n";
		else {
			auto& m1 = std::get<1>(arg1);
			auto& m2 = std::get<1>(arg2);

			float det1 = m1.Determinant();
			if (det1 == 0.0f) {
				cerr << "Matrix 1 is degenerate\n";
				goto end;
			}
			
			auto inv1 = m1.Inverse();

			float det2 = m2.Determinant();
			if (det2 == 0.0f) {
				cerr << "Matrix 2 is degenerate\n";
				goto end;
			}
			auto inv2 = m2.Inverse();

			cout << "First input matrix:\n";
			for (auto it = m1.begin(); it != m1.end(); it++) {
				for (auto it2 = it->begin(); it2 != it->end(); it2++)
					cout << *it2 << ' ';
				cout << '\n';
			}

			cout << "\nFirst matrix inverse:\n";
			for (auto it = inv1.begin(); it != inv1.end(); it++) {
				for (auto it2 = it->begin(); it2 != it->end(); it2++)
					cout << *it2 << ' ';
				cout << '\n';
			}

			cout << "\n\nSecond input matrix:\n";
			for (auto it = m2.begin(); it != m2.end(); it++) {
				for (auto it2 = it->begin(); it2 != it->end(); it2++)
					cout << *it2 << ' ';
				cout << '\n';
			}

			cout << "\nSecond matrix inverse:\n";
			for (auto it = inv2.begin(); it != inv2.end(); it++) {
				for (auto it2 = it->begin(); it2 != it->end(); it2++)
					cout << *it2 << ' ';
				cout << '\n';
			}
		}
	}

	cout.flush();
	end:
	return 0;
}