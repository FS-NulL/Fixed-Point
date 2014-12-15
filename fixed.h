#ifndef __FIXED_POINT__47598035
#define __FIXED_POINT__47598035

#include <cstddef> // for size_t

namespace FixedPoint
{
	template <size_t dps,typename MyType = long long int> struct Fixed;

	namespace details
	{
		// Negative numbers crash the compiler by design
		template<int N> struct Factor	 { enum { value = (Factor<N - 1>::value * 10) }; };
		template<>		struct Factor< 0>{ enum { value = 1 }; };

		template<size_t a, size_t b> struct Max{ enum { value = (a>b ? a : b) }; };

		namespace ctors
		{

			template <size_t dps1, size_t dps2, bool dps1_greater>
			struct scaleFixedImpl
			{
				inline static void f(Fixed<dps1>& a, const Fixed<dps2>& b) {
					// Round and scale down
					auto f = details::Factor<dps2 - dps1>::value;
					if (b.m_Value >= 0) a.m_Value = (b.m_Value + f / 2) / f;
					else a.m_Value = (b.m_Value - f / 2) / f;
				}
			};

			template <size_t dps1, size_t dps2>
			struct scaleFixedImpl < dps1, dps2, true >
			{
				inline static void f(Fixed<dps1>& a, const Fixed<dps2>& b) {
					auto fac = details::Factor<dps1 - dps2>::value;
					a.m_Value = b.m_Value * fac;
				}
			};

			template <size_t dps1, size_t dps2>
			inline void scaleFixed(Fixed<dps1>& a, const Fixed<dps2>& b) {
				//a.m_Value = 0;
				scaleFixedImpl<dps1, dps2, (dps1 >= dps2)>::f(a, b);
			}
		}

		namespace ops
		{
			// add
			template <size_t dps1, size_t dps2, bool dps1_greater>
			struct addImpl {
				inline static Fixed<(dps1 > dps2 ? dps1 : dps2)>
					f(const Fixed<dps1>& a, const Fixed<dps2>& b) {
					return (b.m_Value + Fixed<dps2>(a).m_Value);
				}
			};

			template <size_t dps1, size_t dps2>
			struct addImpl < dps1, dps2, true >{
				inline static Fixed<(dps1 > dps2 ? dps1 : dps2)>
					f(const Fixed<dps1>& a, const Fixed<dps2>& b) {
					return (Fixed<dps>(b).m_Value + a.m_Value);
				}
			};

			template <size_t dps1, size_t dps2>
			inline Fixed<(dps1 > dps2 ? dps1 : dps2)>
				add(const Fixed<dps1>& a, const Fixed<dps2>& b) {
				return addImpl<dps1, dps2, (dps1 > dps2)>::f(a, b);
			}

			// sub
			template <size_t dps1, size_t dps2, bool dps1_greater>
			struct subImpl {
				inline static Fixed<(dps1 > dps2 ? dps1 : dps2)>
					f(const Fixed<dps1>& a, const Fixed<dps2>& b) {
					return (b.m_Value - Fixed<dps2>(a).m_Value);
				}
			};

			template <size_t dps1, size_t dps2>
			struct subImpl < dps1, dps2, true >{
				inline static Fixed<(dps1 > dps2 ? dps1 : dps2)>
					f(const Fixed<dps1>& a, const Fixed<dps2>& b) {
					return (Fixed<dps1>(b).m_Value - a.m_Value);
				}
			};

			template <size_t dps1, size_t dps2>
			inline Fixed<(dps1 > dps2 ? dps1 : dps2)>
				sub(Fixed<dps1>& a, const Fixed<dps2>& b) {
				return subImpl<dps1, dps2, (dps1 > dps2)>::f(a, b);
			}

			// equal
			template <size_t dps1, size_t dps2, bool dps1_greater>
			struct eqImpl {
				inline static bool f(const Fixed<dps1>& a, const Fixed<dps2>& b)
				{
					return Fixed<details::Max<dps1, dps2>::value>(a).m_Value
						== b.m_Value;
				}
			};

			template <size_t dps1, size_t dps2>
			struct eqImpl < dps1, dps2, true >{
				inline static bool f(const Fixed<dps1>& a, const Fixed<dps2>& b)
				{
					return a.m_Value ==
						Fixed<details::Max<dps1, dps2>::value>(b).m_Value;
				}
			};

			template <size_t dps1, size_t dps2>
			inline bool equal(const Fixed<dps1>& a, const Fixed<dps2>& b) {
				return eqImpl<dps1, dps2, (dps1 > dps2)>::f(a, b);
			}

			// >
			template <size_t dps1, size_t dps2, bool dps1_greater>
			struct gtImpl {
				inline static bool f(const Fixed<dps1>& a, const Fixed<dps2>& b)
				{
					return Fixed<details::Max<dps1, dps2>::value>(a).m_Value
					> b.m_Value;
				}
			};

			template <size_t dps1, size_t dps2>
			struct gtImpl < dps1, dps2, true >{
				inline static bool f(const Fixed<dps1>& a, const Fixed<dps2>& b)
				{
					return a.m_Value >
						Fixed<details::Max<dps1, dps2>::value>(b).m_Value;
				}
			};

			template <size_t dps1, size_t dps2>
			inline bool greater(const Fixed<dps1>& a, const Fixed<dps2>& b) {
				return gtImpl<dps1, dps2, (dps1 > dps2)>::f(a, b);
			}


			// <
			template <size_t dps1, size_t dps2, bool dps1_greater>
			struct ltImpl {
				inline static bool f(const Fixed<dps1>& a, const Fixed<dps2>& b)
				{
					return Fixed<details::Max<dps1, dps2>::value>(a).m_Value
						< b.m_Value;
				}
			};

			template <size_t dps1, size_t dps2>
			struct ltImpl < dps1, dps2, true >{
				inline static bool f(const Fixed<dps1>& a, const Fixed<dps2>& b)
				{
					return a.m_Value <
						Fixed<details::Max<dps1, dps2>::value>(b).m_Value;
				}
			};

			template <size_t dps1, size_t dps2>
			inline bool lesser(const Fixed<dps1>& a, const Fixed<dps2>& b) {
				return ltImpl<dps1, dps2, (dps1 > dps2)>::f(a, b);
			}
		}

	}

	template <size_t dps, typename MyType>
	struct Fixed
	{
		//typedef long long int MyType;
		MyType m_Value;

		// Constructors

		explicit Fixed(float f)
		{
			f *= details::Factor<dps>::value;
			if (f < 0) f -= 0.5f;
			else f += 0.5f;
			m_Value = f;
		}

		explicit Fixed(double f)
		{
			f *= details::Factor<dps>::value;
			if (f < 0) f -= 0.5;
			else f += 0.5;
			m_Value = static_cast<MyType>(f);
		}

		Fixed() : m_Value(0) { }
		Fixed(MyType v) : m_Value(v/**details::Factor<dps>::value*/) { }
		Fixed(const Fixed<dps>& d) { m_Value = d.m_Value; }
		template <size_t dps2> Fixed(const Fixed<dps2>& d)
		{
			// Convert to our dps format
			details::ctors::scaleFixed(*this, d);
		}

		// Mathematical operators
		inline Fixed<dps> operator +(const Fixed<dps>& d)
		{
			return (m_Value + d.m_Value);
		}
		inline Fixed<dps> operator -(const Fixed<dps>& d)
		{
			return (m_Value - d.m_Value);
		}

		template <size_t dps2>
		inline Fixed<(dps > dps2 ? dps : dps2)>  operator +(const Fixed<dps2> d)
		{
			return details::ops::add(*this, d);       
		}
		template <size_t dps2>
		inline Fixed<(dps > dps2 ? dps : dps2)>  operator -(const Fixed<dps2> d)
		{
			return details::ops::sub(*this, d);      
		}
		template <size_t dps2>
		Fixed<(dps > dps2 ? dps : dps2)>  operator *(const Fixed<dps2> d)
		{
			auto temp = m_Value * d.m_Value;
			auto f = details::Factor<(dps <= dps2 ? dps : dps2)>::value;
			if (temp >= 0) temp += f / 2; else temp -= f / 2;
			return temp / f;
		}
		template <size_t dps2>
		Fixed<(dps > dps2 ? dps : dps2)>  operator /(const Fixed<dps2> d)
		{
			// find max dps       
			const auto max = details::Max<dps, dps2>::value;

			// convert numerator to (2*max)+1
			// Force this to use a wide value to avoid overflows
			Fixed<2 * max + 1, long long int> num = *this; 

			// convert denum to max
			Fixed<max> denum = d;

			// divide
			num.m_Value /= denum.m_Value;

			// + 5
			if (num.m_Value >= 0) num.m_Value += 5; else num.m_Value -= 5;

			// / 10
			denum.m_Value = num.m_Value / 10;

			return denum; // return denum so the return type is correct and doesn't force a conversion
		}
		inline bool operator ==(const Fixed<dps>& d) const
		{
			return m_Value == d.m_Value;
		}
		template <size_t dps2>
		inline bool operator ==(const Fixed<dps2>& d) const
		{
			return details::ops::equal(*this, d);
		}

		inline Fixed<dps> operator *(const int v) { return m_Value * v; }
		Fixed<dps> operator /(const int v)
		{
			auto temp = ((m_Value * 10) / v);
			if (temp >= 0) temp += 5; else temp -= 5;
			return temp / 10;
		}
		inline Fixed<dps> operator +(const int v)
		{
			return m_Value + v * details::Factor<dps>::value;
		}
		inline Fixed<dps> operator -(const int v)
		{
			return m_Value - v * details::Factor<dps>::value;
		}

		inline Fixed<dps> operator +=(const int v)
		{
			m_Value += v * details::Factor<dps>::value; return *this;
		}
		inline Fixed<dps> operator -=(const int v)
		{
			m_Value -= v * details::Factor<dps>::value; return *this;
		}
		inline Fixed<dps> operator *=(const int v) { m_Value *= v; return *this; }
		Fixed<dps> operator /=(const int v)
		{
			auto temp = ((m_Value * 10) / v);
			if (temp >= 0) temp += 5; else temp -= 5;
			m_Value = temp / 10;
			return *this;
		}

		inline Fixed<dps> operator +=(const Fixed<dps>& d) { m_Value += d.m_Value; return *this; }
		inline Fixed<dps> operator -=(const Fixed<dps>& d) { m_Value -= d.m_Value; return *this; }
		inline Fixed<dps> operator *=(const Fixed<dps>& d) { return *this = *this * d; }
		inline Fixed<dps> operator /=(const Fixed<dps>& d) { return *this = *this / d; }

		template <size_t dps2>
		Fixed<dps> operator +=(const Fixed<dps>& d)
		{
			m_Value += Fixed<dps>(d).m_Value;
			return *this;
		}
		template <size_t dps2>
		Fixed<dps> operator -=(const Fixed<dps>& d)
		{
			m_Value -= Fixed<dps>(d).m_Value;
			return *this;
		}
		template <size_t dps2>
		Fixed<dps> operator *=(const Fixed<dps>& d)
		{
			return this = this * d;
		}
		template <size_t dps2>
		Fixed<dps> operator /=(const Fixed<dps>& d)
		{
			return this = this / d;
		}

		// Comparisons
		inline bool operator > (const Fixed<dps>& d) { return m_Value > d.m_Value; }
		inline bool operator < (const Fixed<dps>& d) { return m_Value < d.m_Value; }
		inline bool operator >=(const Fixed<dps>& d) { return !(m_Value < d.m_Value); }
		inline bool operator <=(const Fixed<dps>& d) { return !(m_Value > d.m_Value); }

		template <size_t dps2>
		bool operator >(const Fixed<dps2>& d)
		{
			return details::ops::greater(*this, d);
		}
		template <size_t dps2>
		bool operator < (const Fixed<dps2>& d)
		{
			return details::ops::lesser(*this, d);
		}
		template <size_t dps2>
		bool operator <=(const Fixed<dps2>& d) { return !(*this > d); }
		template <size_t dps2>
		bool operator >=(const Fixed<dps2>& d) { return !(*this < d); }

		template <int decimalPlaces>
		void round()
		{
			const auto factor = 
				details::Factor<static_cast<int>(dps) - decimalPlaces>::value;
			if (m_Value < 0) m_Value -= (factor / 2);
			else m_Value += (factor / 2);
			m_Value /= factor;
			m_Value *= factor;
		}

		// Set the integral part, clears the fractional component
		inline void set_integral(MyType value)
		{
			m_Value = details::Factor<dps>::value * value;
		}

		// Sets the integral part, keeps the fractional component
		inline void append_integral(MyType value)
		{
			m_Value = (m_Value % details::Factor<dps>::value) +
				details::Factor<dps>::value * value;
		}

		// Sets the fractional part, clears the integral component
		inline void set_fractional(MyType value)
		{
			m_Value = value % details::Factor<dps>::value;
		}

		// Sets the fractional part, keeps the integral component
		inline void append_fractional(MyType value)
		{			
			m_Value = (m_Value / details::Factor<dps>::value)
				* details::Factor<dps>::value 
				+ value % details::Factor<dps>::value;
		}

		inline MyType get_integral()
		{
			return m_Value / details::Factor<dps>::value;
		}

		inline MyType get_fractional()
		{
			return m_Value % details::Factor<dps>::value;
		}

		inline size_t get_dp()
		{
			return dps;
		}

	};

	template <typename Stream, size_t N, typename intfmt>
	Stream& operator<<(Stream& s, Fixed<N, intfmt>& value)
	{
		auto fillch = s.fill();
		auto width = s.width();
		s << value.get_integral() << ".";
		s.fill('0');
		s.width(N);
		s << value.get_fractional();
		s.width(width);
		s.fill(fillch);
		return s;
	}

}

#endif
