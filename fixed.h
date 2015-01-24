#ifndef __FIXED_POINT__47598035
#define __FIXED_POINT__47598035

#include <cstddef> // for size_t
#include <iosfwd>

namespace FixedPoint
{
	namespace details
	{
		typedef long long int DefaultType;
	}

	template <size_t dps, typename MyType = details::DefaultType> struct Fixed;

	namespace details
	{
		// Negative numbers crash the compiler by design
		template<int N> struct Factor	 { enum { value = (Factor<N - 1>::value * 10) }; };
		template<>		struct Factor< 0>{ enum { value = 1 }; };

		int RTFactor(size_t dp)
		{
			// 0 - 9 Are the only valid options
			// > 9 Overflows int return type
			switch (dp)
			{
			case 0: return 1;
			case 1: return 10;
			case 2: return 100;
			case 3: return 1000;
			case 4: return 10000;
			case 5: return 100000;
			case 6: return 1000000;
			case 7: return 10000000;
			case 8: return 100000000;
			case 9: return 1000000000;
			default: while (1); // This is an error
			}			
		}

		template<size_t a, size_t b> struct Max{ enum { value = (a>b ? a : b) }; };

		namespace wider
		{
			template<bool, typename T1, typename T2>
			struct is_cond {
				typedef T1 type;
			};

			template<typename T1, typename T2>
			struct is_cond<false, T1, T2> {
				typedef T2 type;
			};

			template<typename T1, typename T2>
			struct widest {
				typedef typename is_cond<(sizeof(T1)>sizeof(T2)), T1, T2>::type type;
			};
		}

		namespace ctors
		{

			template <size_t dps1, size_t dps2, bool dps1_greater, typename T1, typename T2>
			struct scaleFixedImpl
			{
				inline static void f(Fixed<dps1,T1>& a, const Fixed<dps2,T2>& b) {
					// Round and scale down
					auto f = details::Factor<dps2 - dps1>::value;
					if (b.m_Value >= 0) 
						 a.m_Value = static_cast<T1>((b.m_Value + f / 2) / f);
					else a.m_Value = static_cast<T1>((b.m_Value - f / 2) / f);
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			struct scaleFixedImpl < dps1, dps2, true, T1, T2 >
			{
				inline static void f(Fixed<dps1,T1>& a, const Fixed<dps2,T2>& b) {
					auto fac = details::Factor<dps1 - dps2>::value;
					a.m_Value = static_cast<T1>(b.m_Value * fac);
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			inline void scaleFixed(Fixed<dps1,T1>& a, const Fixed<dps2,T2>& b) {
				//a.m_Value = 0;
				scaleFixedImpl<dps1, dps2, (dps1 >= dps2), T1, T2>::f(a, b);
			}
		}

		namespace ops
		{
			// add
			template <size_t dps1, size_t dps2, bool dps1_greater, typename T1, typename T2>
			struct addImpl {
				inline static Fixed<(dps1 > dps2 ? dps1 : dps2)>
					f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b) {
					return (b.m_Value + Fixed<dps2,T1>(a).m_Value);
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			struct addImpl < dps1, dps2, true, T1, T2>{
				inline static Fixed<(dps1 > dps2 ? dps1 : dps2)>
					f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b) {
					return (Fixed<dps, T1>(b).m_Value + a.m_Value);
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			inline Fixed<(dps1 > dps2 ? dps1 : dps2)>
				add(const Fixed<dps1,T1>& a, const Fixed<dps2,T2>& b) {
				return addImpl<dps1, dps2, (dps1 > dps2), T1, T2 > ::f(a, b);
			}

			// sub
			template <size_t dps1, size_t dps2, bool dps1_greater, typename T1, typename T2>
			struct subImpl {
				inline static Fixed<(dps1 > dps2 ? dps1 : dps2)>
					f(const Fixed<dps1,T1>& a, const Fixed<dps2,T2>& b) {
					return (b.m_Value - Fixed<dps2,T2>(a).m_Value);
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			struct subImpl < dps1, dps2, true, T1, T2 >{
				inline static Fixed<(dps1 > dps2 ? dps1 : dps2)>
					f(const Fixed<dps1,T1>& a, const Fixed<dps2,T2>& b) {
					return (Fixed<dps1,T1>(b).m_Value - a.m_Value);
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			inline Fixed<(dps1 > dps2 ? dps1 : dps2)>
				sub(Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b) {
				return subImpl<dps1, dps2, (dps1 > dps2), T1, T2>::f(a, b);
			}

			// equal
			template <size_t dps1, size_t dps2, bool dps1_greater, typename T1, typename T2>
			struct eqImpl {
				inline static bool f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2> & b)
				{
					return Fixed<details::Max<dps1, dps2>::value>(a).m_Value
						== b.m_Value;
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			struct eqImpl < dps1, dps2, true, T1, T2>{
				inline static bool f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
				{
					return a.m_Value ==
						Fixed<details::Max<dps1, dps2>::value>(b).m_Value;
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			inline bool equal(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b) {
				return eqImpl<dps1, dps2, (dps1 > dps2), T1, T2>::f(a, b);
			}

			// >
			template <size_t dps1, size_t dps2, bool dps1_greater, typename T1, typename T2>
			struct gtImpl {
				inline static bool f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
				{
					return Fixed<details::Max<dps1, dps2>::value, T1>(a).m_Value
					> b.m_Value;
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			struct gtImpl < dps1, dps2, true, T1, T2 >{
				inline static bool f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
				{
					return a.m_Value >
						Fixed<details::Max<dps1, dps2>::value, T2>(b).m_Value;
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			inline bool greater(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b) {
				return gtImpl<dps1, dps2, (dps1 > dps2), T1, T2>::f(a, b);
			}


			// <
			template <size_t dps1, size_t dps2, bool dps1_greater, typename T1, typename T2>
			struct ltImpl {
				inline static bool f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
				{
					return Fixed<details::Max<dps1, dps2>::value, T1>(a).m_Value
						< b.m_Value;
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			struct ltImpl < dps1, dps2, true, T1, T2 >{
				inline static bool f(const Fixed<dps1,T1>& a, const Fixed<dps2, T1>& b)
				{
					return a.m_Value <
						Fixed<details::Max<dps1, dps2>::value, T2>(b).m_Value;
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			inline bool lesser(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b) {
				return ltImpl<dps1, dps2, (dps1 > dps2), T1, T2>::f(a, b);
			}
		}

		namespace ops2
		{
			// add base
			// add dps1 == dps2
			// add dps1 != dps2
			// add dps1 >  dps2
			// add dps1 <  dps2

			// add
			template <size_t dps1, size_t dps2, bool dps1_greater, typename T1, typename T2>
			struct addImpl {
				inline static Fixed<(dps1 > dps2 ? dps1 : dps2)>
					f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b) {
					return (b.m_Value + Fixed<dps2, T2>(a).m_Value);
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			struct addImpl < dps1, dps2, true, T1, T2>{
				inline static Fixed<(dps1 > dps2 ? dps1 : dps2)>
					f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b) {
					return (Fixed<dps1, T1>(b).m_Value + a.m_Value);
				}
			};

			template <size_t dps1, size_t dps2, bool dps_equal, typename T1, typename T2>
			struct addImplDpCompare
			{
				inline static 
					Fixed<(dps1 > dps2 ? dps1 : dps2), typename details::wider::widest<T1, T2>::type>
					f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
				{
					// dps1 != dps2
					return addImpl<dps1, dps2, (dps1 > dps2), T1, T2 > ::f(a, b);
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			struct addImplDpCompare < dps1, dps2, true, T1, T2 >
			{
				inline static
					Fixed<(dps1 > dps2 ? dps1 : dps2), typename details::wider::widest<T1, T2>::type>
					f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
				{
					// dps1 == dps2
					// do work right here
					return a.m_Value + b.m_Value;
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			inline
				Fixed<(dps1 > dps2 ? dps1 : dps2), typename details::wider::widest<T1, T2>::type>
				add(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
			{
				return addImplDpCompare <dps1, dps2, dps1 == dps2, T1, T2 > ::f(a, b);
			}

			// sub
			template <size_t dps1, size_t dps2, bool dps1_greater, typename T1, typename T2>
			struct subImpl {
				inline static Fixed<(dps1 > dps2 ? dps1 : dps2)>
					f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b) {
					return (Fixed<dps2, T2>(a).m_Value - b.m_Value);
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			struct subImpl < dps1, dps2, true, T1, T2>{
				inline static Fixed<(dps1 > dps2 ? dps1 : dps2)>
					f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b) {
					return (a.m_Value - Fixed<dps1, T1>(b).m_Value);
				}
			};

			template <size_t dps1, size_t dps2, bool dps_equal, typename T1, typename T2>
			struct subImplDpCompare
			{
				inline static
					Fixed<(dps1 > dps2 ? dps1 : dps2), typename details::wider::widest<T1, T2>::type>
					f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
				{
					// dps1 != dps2
					return subImpl<dps1, dps2, (dps1 > dps2), T1, T2 > ::f(a, b);
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			struct subImplDpCompare < dps1, dps2, true, T1, T2 >
			{
				inline static
					Fixed<(dps1 > dps2 ? dps1 : dps2), typename details::wider::widest<T1, T2>::type>
					f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
				{
					// dps1 == dps2
					// do work right here
					return a.m_Value - b.m_Value;
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			inline
				Fixed<(dps1 > dps2 ? dps1 : dps2), typename details::wider::widest<T1, T2>::type>
				sub(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
			{
				return subImplDpCompare <dps1, dps2, dps1 == dps2, T1, T2 > ::f(a, b);
			}

			// equal
			template <size_t dps1, size_t dps2, bool dps1_greater, typename T1, typename T2>
			struct eqImpl {
				inline static bool f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2> & b)
				{	//dps1 < dps2
					return Fixed<details::Max<dps1, dps2>::value>(a).m_Value
						== b.m_Value;
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			struct eqImpl < dps1, dps2, true, T1, T2>{
				inline static bool f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
				{	//dps1 > dps2
					return a.m_Value ==
						Fixed<details::Max<dps1, dps2>::value>(b).m_Value;
				}
			};

			template <size_t dps1, size_t dps2, bool dps_equal, typename T1, typename T2>
			struct eqImplDpCompare
			{
				inline static bool
					f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
				{	// dps1 != dps2
					return eqImpl<dps1, dps2, (dps1 > dps2), T1, T2>::f(a, b);
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			struct eqImplDpCompare < dps1, dps2, true, T1, T2 >
			{
				inline static bool 
					f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
				{
					// dps1 == dps2 - do work right here
					return a.m_Value == b.m_Value;
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			inline bool equal(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b) {
				
				return eqImplDpCompare <dps1, dps2, dps1 == dps2, T1, T2 > ::f(a, b);
			}

			// operator +=
			template <size_t dps1, size_t dps2, bool dps1_greater, typename T1, typename T2>
			struct plusEqImpl {
				inline static Fixed<dps1, T1>&
					f(Fixed<dps1, T1>& a, const Fixed<dps2, T2> & b)
				{	//dps1 < dps2
					const int f = details::Factor<dps2 - dps1>::value;
					if (b.m_Value > 0) 
						a.m_Value += static_cast<T1>((b.m_Value + (f / 2)) / f);
					else 
						a.m_Value += static_cast<T1>((b.m_Value - (f / 2)) / f);
					return a;
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			struct plusEqImpl < dps1, dps2, true, T1, T2>{
				inline static Fixed<dps1, T1>&
					f(Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
				{	//dps1 > dps2
					a.m_Value += static_cast<T1>(b.m_Value *
						(details::Factor<dps1 - dps2>::value));
					return a;
				}
			};

			template <size_t dps1, size_t dps2, bool dps_equal, typename T1, typename T2>
			struct plusEqImplDpCompare
			{
				inline static Fixed<dps1, T1>&
					f(Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
				{	// dps1 != dps2
					return plusEqImpl<dps1, dps2, (dps1 > dps2), T1, T2>::f(a, b);
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			struct plusEqImplDpCompare < dps1, dps2, true, T1, T2 >
			{
				inline static Fixed<dps1, T1>&
					f(Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
				{
					// dps1 == dps2 - do work right here
					a.m_Value += static_cast<T1>(b.m_Value);
					return a;
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			inline Fixed<dps1, T1>& plusEqual(Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b) {

				return plusEqImplDpCompare <dps1, dps2, dps1 == dps2, T1, T2 > ::f(a, b);
			}

			// operator -=
			template <size_t dps1, size_t dps2, bool dps1_greater, typename T1, typename T2>
			struct minusEqImpl {
				inline static Fixed<dps1, T1>&
					f(Fixed<dps1, T1>& a, const Fixed<dps2, T2> & b)
				{	//dps1 < dps2
					const int f = details::Factor<dps2 - dps1>::value;
					if (b.m_Value > 0)
						a.m_Value -= static_cast<T1>((b.m_Value + (f / 2)) / f);
					else
						a.m_Value -= static_cast<T1>((b.m_Value - (f / 2)) / f);
					return a;
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			struct minusEqImpl < dps1, dps2, true, T1, T2>{
				inline static Fixed<dps1, T1>&
					f(Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
				{	//dps1 > dps2
					a.m_Value -= static_cast<T1>(b.m_Value *
						(details::Factor<dps1 - dps2>::value));
					return a;
				}
			};

			template <size_t dps1, size_t dps2, bool dps_equal, typename T1, typename T2>
			struct minusEqImplDpCompare
			{
				inline static Fixed<dps1, T1>&
					f(Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
				{	// dps1 != dps2
					return minusEqImpl<dps1, dps2, (dps1 > dps2), T1, T2>::f(a, b);
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			struct minusEqImplDpCompare < dps1, dps2, true, T1, T2 >
			{
				inline static Fixed<dps1, T1>&
					f(Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
				{
					// dps1 == dps2 - do work right here
					a.m_Value -= static_cast<T1>(b.m_Value);
					return a;
				}
			};

			template <size_t dps1, size_t dps2, typename T1, typename T2>
			inline Fixed<dps1, T1>& minusEqual(Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b) {

				return minusEqImplDpCompare <dps1, dps2, dps1 == dps2, T1, T2 > ::f(a, b);
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
		Fixed(MyType v) : m_Value(v) { } // Required to be like this // TODO
		Fixed(const Fixed<dps,MyType>& d) { m_Value = d.m_Value; }

		template <size_t dps2> Fixed(const Fixed<dps2, MyType>& d)
		{
			// Convert to our dps format
			details::ctors::scaleFixed(*this, d);
		}

		template <size_t dps2, typename T> Fixed(const Fixed<dps2, T>& d)
		{
			// Convert to our dps format and out internal format
			details::ctors::scaleFixed(*this, d);
		}

		template<size_t dps2, typename T2>
		inline 
			Fixed<(dps > dps2 ? dps : dps2), typename details::wider::widest<MyType, T2>::type>
			operator + (const Fixed<dps2,T2>& d)
		{
			return details::ops2::add(*this,d);
		}

		template<size_t dps2, typename T2>
		inline
			Fixed<(dps > dps2 ? dps : dps2), typename details::wider::widest<MyType, T2>::type>
			operator - (const Fixed<dps2, T2>& d)
		{
			return details::ops2::sub(*this, d);
		}

		template<size_t dps2, typename T2>
		Fixed<(dps > dps2 ? dps : dps2), typename details::wider::widest<MyType, T2>::type>
			operator *(const Fixed<dps2, T2>& d)
		{
			auto temp = m_Value * d.m_Value;
			auto f = details::Factor<(dps <= dps2 ? dps : dps2)>::value;
			if (temp >= 0) temp += f / 2; else temp -= f / 2;
			return temp / f;
		}

		template<size_t dps2, typename T2>
		Fixed<(dps > dps2 ? dps : dps2), typename details::wider::widest<MyType, T2>::type>
			operator /(const Fixed<dps2, T2> d)
		{
			// find max dps       
			const auto max = details::Max<dps, dps2>::value;

			// convert numerator to (2*max)+1
			// Force this to use a wide value to avoid overflows
			Fixed<2 * max + 1, long long int> num = *this;

			// convert denum to max
			Fixed<max, details::wider::widest<MyType, T2>::type> denum = d;

			// divide
			num.m_Value /= denum.m_Value;

			// + 5
			if (num.m_Value >= 0) num.m_Value += 5; else num.m_Value -= 5;

			// / 10
			denum.m_Value = num.m_Value / 10;

			return denum; // return denum so the return type is correct and doesn't force a conversion
		}

		template <size_t dps2, typename T2>
		inline bool operator ==(const Fixed<dps2, T2>& d) const
		{
			return details::ops2::equal(*this, d);
		}

		template <size_t dps2, typename T2>
		inline Fixed<dps, MyType>& operator +=(const Fixed<dps2, T2>& d) 
		{
			return details::ops2::plusEqual(*this, d);
		}

		template <size_t dps2, typename T2>
		inline Fixed<dps, MyType>& operator -=(const Fixed<dps2, T2>& d)
		{
			return details::ops2::minusEqual(*this, d);
		}

		template <size_t dps2, typename T2>
		inline Fixed<dps, MyType>& operator *=(const Fixed<dps2, T2>& d)
		{
			details::wider::widest<MyType, T2>::type temp = m_Value;
			temp *= d.m_Value;
			temp /= details::Factor<dps2>::value;			
			m_Value =  static_cast<MyType>(temp);
			return *this;
		}

		template <size_t dps2, typename T2>
		inline Fixed<dps, MyType>& operator /=(const Fixed<dps2, T2>& d)
		{
			details::wider::widest<MyType, T2>::type temp = m_Value;
			temp *= details::Factor<dps2 + 1>::value;
			temp /= d.m_Value;
			if (temp >= 0) temp += 5; else temp -= 5;
			temp /= 10;
			m_Value = temp;
			return *this;
		}




		// INTEGER inputs

		inline Fixed<dps, MyType> operator *(const int v) { return m_Value * v; }
		Fixed<dps, MyType> operator /(const int v)
		{
			auto temp = ((m_Value * 10) / v);
			if (temp >= 0) temp += 5; else temp -= 5;
			return temp / 10;
		}
		inline Fixed<dps, MyType> operator +(const int v)
		{
			return m_Value + v * details::Factor<dps>::value;
		}
		inline Fixed<dps, MyType> operator -(const int v)
		{
			return m_Value - v * details::Factor<dps>::value;
		}

		inline Fixed<dps, MyType> operator +=(const int v)
		{
			m_Value += v * details::Factor<dps>::value; return *this;
		}
		inline Fixed<dps, MyType> operator -=(const int v)
		{
			m_Value -= v * details::Factor<dps>::value; return *this;
		}
		inline Fixed<dps, MyType> operator *=(const int v) { m_Value *= v; return *this; }
		Fixed<dps, MyType> operator /=(const int v)
		{
			auto temp = ((m_Value * 10) / v);
			if (temp >= 0) temp += 5; else temp -= 5;
			m_Value = temp / 10;
			return *this;
		}

		// TODO: templatize all this code on dps dps2 T1(MyType) T2 
		// Only for Fixed<> inputs
		// then specialise on dps == dps2 
		// and T1 == T2

		// operators: 
		// +  -  *  /  all operator on largest dp (and widest T), return largest + widest DONE
		// += -= *= /= all operator on largest dp (and widest T), return largest + widest DONE
		// == >  >= <  <= all operate on largest dp (width conversion is implicit)

		//template<size_t dps2, typename T2>
		//...

		// Same DP, Same Type // this will be made obsolete
		//inline Fixed<dps, MyType> operator +=(const Fixed<dps, MyType>& d) { m_Value += d.m_Value; return *this; }
		//inline Fixed<dps, MyType> operator -=(const Fixed<dps, MyType>& d) { m_Value -= d.m_Value; return *this; }
		//inline Fixed<dps, MyType> operator *=(const Fixed<dps, MyType>& d) { return *this = *this * d; }
		//inline Fixed<dps, MyType> operator /=(const Fixed<dps, MyType>& d) { return *this = *this / d; }
		/* 
		// This block isn't used, there is an implicit conversion before
		// calling the above functions
		// Different DP, Same Type
		template <size_t dps2>
		Fixed<dps, MyType> operator +=(const Fixed<dps, MyType>& d)
		{
			m_Value += Fixed<dps, MyType>(d).m_Value;
			return *this;
		}
		template <size_t dps2>
		Fixed<dps, MyType> operator -=(const Fixed<dps, MyType>& d)
		{
			m_Value -= Fixed<dps, MyType>(d).m_Value;
			return *this;
		}
		template <size_t dps2>
		Fixed<dps, MyType> operator *=(const Fixed<dps, MyType>& d)
		{
			return this = this * d;
		}
		template <size_t dps2>
		Fixed<dps, MyType> operator /=(const Fixed<dps, MyType>& d)
		{
			return this = this / d;
		}*/
		
		// Same DP, Different Type
		//template <typename T>
		//inline Fixed<dps, MyType> operator +=(const Fixed<dps, T>& d) { 
		//	m_Value += static_cast<MyType>(d.m_Value); return *this;
		//}
		//template <typename T>
		//inline Fixed<dps, MyType> operator -=(const Fixed<dps, T>& d) { 
		//	m_Value -= static_cast<MyType>(d.m_Value); return *this;
		//}		
		//template <typename T>
		//inline Fixed<dps, MyType> operator *=(const Fixed<dps, T>& d) { 
		//	return *this = *this * d; 
		//}		
		//template <typename T>
		//inline Fixed<dps, MyType> operator /=(const Fixed<dps, T>& d) { 
		//	return *this = *this / d; 
		//}

		// Comparisons		
		inline bool operator > (const Fixed<dps, MyType>& d) const { return m_Value > d.m_Value; }
		inline bool operator < (const Fixed<dps, MyType>& d) const { return m_Value < d.m_Value; }
		inline bool operator >=(const Fixed<dps, MyType>& d) const { return !(m_Value < d.m_Value); }
		inline bool operator <=(const Fixed<dps, MyType>& d) const { return !(m_Value > d.m_Value); }

		template <size_t dps2>
		bool operator >(const Fixed<dps2, MyType>& d) const
		{
			return details::ops::greater(*this, d);
		}
		template <size_t dps2>
		bool operator < (const Fixed<dps2, MyType>& d) const
		{
			return details::ops::lesser(*this, d);
		}
		template <size_t dps2>
		bool operator <=(const Fixed<dps2, MyType>& d) const { return !(*this > d); }
		template <size_t dps2>
		bool operator >=(const Fixed<dps2, MyType>& d) const { return !(*this < d); }

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
		inline void set_integral(const MyType value)
		{
			m_Value = details::Factor<dps>::value * value;
		}

		// Sets the integral part, keeps the fractional component
		inline void append_integral(const MyType value)
		{
			m_Value = (m_Value % details::Factor<dps>::value) +
				details::Factor<dps>::value * value;
		}

		// Sets the fractional part, clears the integral component
		inline void set_fractional(const MyType value)
		{
			m_Value = value % details::Factor<dps>::value;
		}

		// Sets the fractional part, keeps the integral component
		inline void append_fractional(const MyType value)
		{			
			m_Value = (m_Value / details::Factor<dps>::value)
				* details::Factor<dps>::value 
				+ value % details::Factor<dps>::value;
		}

		inline MyType get_integral() const
		{
			return m_Value / details::Factor<dps>::value;
		}

		inline MyType get_fractional() const
		{
			return m_Value % details::Factor<dps>::value;
		}

		inline size_t get_dp() const
		{
			return dps;
		}

	};

	// TODO: external operator overloads
	// int + Fixed
	// float + Fixed
	// double + fixed

	template <size_t N, typename intfmt>
	std::ostream& operator<<(std::ostream& s, const Fixed<N, intfmt>& value)
	{
		// TODO: This doesn't handle -0.04 for example
		// negative numbers are a problem here		
		int mul = (value.m_Value < 0) ? -1 : 1;
		if ((value.m_Value < 0)) s << "-";		
		auto fillch = s.fill();
		auto width = s.width();
		s << value.get_integral() * mul << ".";
		s.fill('0');
		s.width(N);
		s << value.get_fractional() * mul;
		s.width(width);
		s.fill(fillch);
		return s;
	}

	template<typename MyType = details::DefaultType>
	struct RTFixed
	{
		MyType m_Value;
		size_t m_dps;

		template<size_t N>
		RTFixed(const Fixed<N, MyType>& f)
			: m_Value(f.m_Value)
			, m_dps(f.get_dp())
		{ }

		inline MyType get_integral() const
		{
			return m_Value / details::RTFactor(m_dps);
		}

		inline MyType get_fractional() const
		{
			return m_Value % details::RTFactor(m_dps);
		}
	};

	template <typename T>
	std::ostream& operator<<(std::ostream& s, const RTFixed<T>& value)
	{
		auto fillch = s.fill();
		auto width = s.width();
		s << value.get_integral() << ".";
		s.fill('0');
		s.width(value.m_dps);
		s << value.get_fractional();
		s.width(width);
		s.fill(fillch);
		return s;
	}


}

#endif
