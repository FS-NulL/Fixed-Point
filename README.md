# Fixed-Point
A base10 fixed point numeric library for c++

Useful in code bases where decimal numbers are being held in integers with arbitary number of decimal places. eg 209 being used to represent 20.9. Supports basic arithmatic and output streaming.

## Usage
		Fixed<4> pi = 31415;
		Fixed<3> pie = 3142;

With custom internal storage type:


		Fixed<3, int> a(3.141);
		Fixed<2, long long> b(3.142);
