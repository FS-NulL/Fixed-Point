#ifndef __FIXED_POINT__47598035
#define __FIXED_POINT__47598035

#include <cstddef> // for size_t
#include <iosfwd>

// Fixed point classes
// For storing decimal numbers with a fixed number of decimal places.
namespace FixedPoint
{
    // User configurable settings
    namespace config
    {
        // Fixed's default internal storage type
        // 32 or 64 bit ints are probable depending on platform
        typedef long long int DefaultType;
    }

    template <size_t dps, typename MyType> struct Fixed;

    namespace Utils
    {
        template <size_t dpsF, typename TF>
        constexpr Fixed<dpsF, TF> MkFxd(TF t);
    }

    // Fixed prototype
    template <size_t dps, typename MyType = config::DefaultType> struct Fixed;

    // Internal implementation 
    namespace details
    {
        // Negative numbers crash the compiler by design
        template<int N> struct Factor	 { enum { value = (Factor<N - 1>::value * 10) }; };
        template<>		struct Factor< 0>{ enum { value = 1 }; };

        static int RTFactor(size_t dp)
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

        // Gets the wider of two types
        // Usage: wider::widest<T1, T2>::type
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
                inline static constexpr T1 f(const T2 in)
                {
                    // round and scale input down
                    return  static_cast<T1>
                        ((in >= 0) ?
                        ((in + (details::Factor<dps2 - dps1>::value / 2)) 
                            / details::Factor<dps2 - dps1>::value)
                        :
                        ((in + (details::Factor<dps2 - dps1>::value / 2))
                            / details::Factor<dps2 - dps1>::value)
                        );
                }
            };

            template <size_t dps1, size_t dps2, typename T1, typename T2>
            struct scaleFixedImpl < dps1, dps2, true, T1, T2 >
            {
                inline static constexpr T1 f(const T2 in)
                {
                    // scale up
                    return static_cast<T1>
                        (in * details::Factor<dps1 - dps2>::value);
                }
            };

            template <size_t dps1, size_t dps2, typename T1, typename T2>
            inline static constexpr T1 scaleFixed(const T2 in)
            {
                return scaleFixedImpl<dps1, dps2, (dps1>=dps2), T1, T2>::f(in);
            }
        }

        namespace ops
        {

            // add
            template <size_t dps1, size_t dps2, bool dps1_greater, typename T1, typename T2>
            struct addImpl {
                inline static constexpr Fixed<(dps1 > dps2 ? dps1 : dps2), typename details::wider::widest<T1, T2>::type>
                    f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b) {
                    return Utils::MkFxd<(dps1 > dps2 ? dps1 : dps2), 
                        typename details::wider::widest<T1, T2>::type >
                        (b.m_Value + Fixed<dps2, T2>(a).m_Value);
                }
            };

            template <size_t dps1, size_t dps2, typename T1, typename T2>
            struct addImpl < dps1, dps2, true, T1, T2>{
                inline static constexpr Fixed<(dps1 > dps2 ? dps1 : dps2), typename details::wider::widest<T1, T2>::type>
                    f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b) {
                    return Utils::MkFxd<(dps1 > dps2 ? dps1 : dps2), 
                        typename details::wider::widest<T1, T2>::type>
                        (Fixed<dps1, T1>(b).m_Value + a.m_Value);
                }
            };

            template <size_t dps1, size_t dps2, bool dps_equal, typename T1, typename T2>
            struct addImplDpCompare
            {
                inline static constexpr
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
                inline static constexpr
                    Fixed<(dps1 > dps2 ? dps1 : dps2), typename details::wider::widest<T1, T2>::type>
                    f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
                {
                    // dps1 == dps2
                    // do work right here
                    return a.m_Value + b.m_Value;
                }
            };

            template <size_t dps1, size_t dps2, typename T1, typename T2>
            inline static constexpr
                Fixed<(dps1 > dps2 ? dps1 : dps2), typename details::wider::widest<T1, T2>::type>
                add(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
            {
                return addImplDpCompare <dps1, dps2, dps1 == dps2, T1, T2 > ::f(a, b);
            }

            // sub
            template <size_t dps1, size_t dps2, bool dps1_greater, typename T1, typename T2>
            struct subImpl {
                inline static constexpr Fixed<(dps1 > dps2 ? dps1 : dps2), typename details::wider::widest<T1, T2>::type>
                    f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b) {
                    return Utils::MkFxd<(dps1 > dps2 ? dps1 : dps2), 
                        typename details::wider::widest<T1, T2>::type>
                        (Fixed<dps2, T2>(a).m_Value - b.m_Value);
                }
            };

            template <size_t dps1, size_t dps2, typename T1, typename T2>
            struct subImpl < dps1, dps2, true, T1, T2>{
                inline static constexpr Fixed<(dps1 > dps2 ? dps1 : dps2), typename details::wider::widest<T1, T2>::type>
                    f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b) {
                    return Utils::MkFxd<(dps1 > dps2 ? dps1 : dps2), 
                        typename details::wider::widest<T1, T2>::type>
                        (a.m_Value - Fixed<dps1, T1>(b).m_Value);
                }
            };

            template <size_t dps1, size_t dps2, bool dps_equal, typename T1, typename T2>
            struct subImplDpCompare
            {
                inline static constexpr
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
                inline static constexpr
                    Fixed<(dps1 > dps2 ? dps1 : dps2), typename details::wider::widest<T1, T2>::type>
                    f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
                {
                    // dps1 == dps2
                    // do work right here
                    return Utils::MkFxd<(dps1 > dps2 ? dps1 : dps2), 
                        typename details::wider::widest<T1, T2>::type>
                        (a.m_Value - b.m_Value);
                }
            };

            template <size_t dps1, size_t dps2, typename T1, typename T2>
            inline static constexpr
                Fixed<(dps1 > dps2 ? dps1 : dps2), typename details::wider::widest<T1, T2>::type>
                sub(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
            {
                return subImplDpCompare <dps1, dps2, dps1 == dps2, T1, T2 > ::f(a, b);
            }

            // equal
            template <size_t dps1, size_t dps2, bool dps1_greater, typename T1, typename T2>
            struct eqImpl {
                inline static constexpr bool f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2> & b)
                {	//dps1 < dps2
                    return Fixed<details::Max<dps1, dps2>::value>(a).m_Value
                        == b.m_Value;
                }
            };

            template <size_t dps1, size_t dps2, typename T1, typename T2>
            struct eqImpl < dps1, dps2, true, T1, T2>{
                inline static constexpr bool f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
                {	//dps1 > dps2
                    return a.m_Value ==
                        Fixed<details::Max<dps1, dps2>::value>(b).m_Value;
                }
            };

            template <size_t dps1, size_t dps2, bool dps_equal, typename T1, typename T2>
            struct eqImplDpCompare
            {
                inline static constexpr bool
                    f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
                {	// dps1 != dps2
                    return eqImpl<dps1, dps2, (dps1 > dps2), T1, T2>::f(a, b);
                }
            };

            template <size_t dps1, size_t dps2, typename T1, typename T2>
            struct eqImplDpCompare < dps1, dps2, true, T1, T2 >
            {
                inline static constexpr bool
                    f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
                {
                    // dps1 == dps2 - do work right here
                    return a.m_Value == b.m_Value;
                }
            };

            template <size_t dps1, size_t dps2, typename T1, typename T2>
            inline static constexpr bool equal(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b) {
                
                return eqImplDpCompare <dps1, dps2, dps1 == dps2, T1, T2 > ::f(a, b);
            }

            // greater
            template <size_t dps1, size_t dps2, bool dps1_greater, typename T1, typename T2>
            struct gtImpl {
                inline static constexpr bool f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2> & b)
                {	//dps1 < dps2
                    return Fixed<details::Max<dps1, dps2>::value>(a).m_Value
                        > b.m_Value;
                }
            };

            template <size_t dps1, size_t dps2, typename T1, typename T2>
            struct gtImpl < dps1, dps2, true, T1, T2>{
                inline static constexpr bool f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
                {	//dps1 > dps2
                    return a.m_Value >
                        Fixed<details::Max<dps1, dps2>::value>(b).m_Value;
                }
            };

            template <size_t dps1, size_t dps2, bool dps_equal, typename T1, typename T2>
            struct gtImplDpCompare
            {
                inline static constexpr bool
                    f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
                {	// dps1 != dps2
                    return gtImpl<dps1, dps2, (dps1 > dps2), T1, T2>::f(a, b);
                }
            };

            template <size_t dps1, size_t dps2, typename T1, typename T2>
            struct gtImplDpCompare < dps1, dps2, true, T1, T2 >
            {
                inline static constexpr bool
                    f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
                {
                    // dps1 == dps2 - do work right here
                    return a.m_Value > b.m_Value;
                }
            };

            template <size_t dps1, size_t dps2, typename T1, typename T2>
            inline static constexpr bool greater(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b) {

                return gtImplDpCompare <dps1, dps2, dps1 == dps2, T1, T2 > ::f(a, b);
            }

            // lesser
            template <size_t dps1, size_t dps2, bool dps1_greater, typename T1, typename T2>
            struct ltImpl {
                inline static constexpr bool f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2> & b)
                {	//dps1 < dps2
                    return Fixed<details::Max<dps1, dps2>::value>(a).m_Value
                        < b.m_Value;
                }
            };

            template <size_t dps1, size_t dps2, typename T1, typename T2>
            struct ltImpl < dps1, dps2, true, T1, T2>{
                inline static constexpr bool f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
                {	//dps1 > dps2
                    return a.m_Value <
                        Fixed<details::Max<dps1, dps2>::value>(b).m_Value;
                }
            };

            template <size_t dps1, size_t dps2, bool dps_equal, typename T1, typename T2>
            struct ltImplDpCompare
            {
                inline static constexpr bool
                    f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
                {	// dps1 != dps2
                    return ltImpl<dps1, dps2, (dps1 > dps2), T1, T2>::f(a, b);
                }
            };

            template <size_t dps1, size_t dps2, typename T1, typename T2>
            struct ltImplDpCompare < dps1, dps2, true, T1, T2 >
            {
                inline static constexpr bool
                    f(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
                {
                    // dps1 == dps2 - do work right here
                    return a.m_Value < b.m_Value;
                }
            };

            template <size_t dps1, size_t dps2, typename T1, typename T2>
            inline static constexpr bool lesser(const Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b) {

                return ltImplDpCompare <dps1, dps2, dps1 == dps2, T1, T2 > ::f(a, b);
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
                inline static constexpr Fixed<dps1, T1>&
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
                inline static constexpr Fixed<dps1, T1>&
                    f(Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
                {	// dps1 != dps2
                    return plusEqImpl<dps1, dps2, (dps1 > dps2), T1, T2>::f(a, b);
                }
            };

            template <size_t dps1, size_t dps2, typename T1, typename T2>
            struct plusEqImplDpCompare < dps1, dps2, true, T1, T2 >
            {
                inline static constexpr Fixed<dps1, T1>&
                    f(Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b)
                {
                    // dps1 == dps2 - do work right here
                    a.m_Value += static_cast<T1>(b.m_Value);
                    return a;
                }
            };

            template <size_t dps1, size_t dps2, typename T1, typename T2>
            inline static constexpr Fixed<dps1, T1>& plusEqual(Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b) {

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
                inline static constexpr Fixed<dps1, T1>&
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
            inline static constexpr Fixed<dps1, T1>& minusEqual(Fixed<dps1, T1>& a, const Fixed<dps2, T2>& b) {

                return minusEqImplDpCompare <dps1, dps2, dps1 == dps2, T1, T2 > ::f(a, b);
            }
        }

    }

    // Stores a value with a fixed number of decimal places
    template <size_t dps, typename MyType> struct Fixed
    {
        // Value is stored here
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

    private:
        struct InternalValue {};
        constexpr Fixed(MyType v, InternalValue) : m_Value(v) { }

    public:

        constexpr Fixed() : m_Value(0) { }
        constexpr Fixed(MyType v) : m_Value(v * details::Factor<dps>::value) {} 
        constexpr Fixed(const Fixed<dps,MyType>& d) : m_Value(d.m_Value) {}

        template <size_t dps2, typename T> 
        constexpr Fixed(const Fixed<dps2, T>& d)
            : m_Value(details::ctors::scaleFixed<dps,dps2,MyType,T>(d.m_Value))
        { }


        // Operator Overloads

        // Fixed<> Inputs

        template<size_t dps2, typename T2>
        inline constexpr
            Fixed<(dps > dps2 ? dps : dps2), typename details::wider::widest<MyType, T2>::type>
            operator + (const Fixed<dps2,T2>& d) const
        {
            return details::ops::add(*this,d);
        }

        template<size_t dps2, typename T2>
        inline constexpr
            Fixed<(dps > dps2 ? dps : dps2), typename details::wider::widest<MyType, T2>::type>
            operator - (const Fixed<dps2, T2>& d) const
        {
            return details::ops::sub(*this, d);
        }

        template<size_t dps2, typename T2>
        Fixed<(dps > dps2 ? dps : dps2), typename details::wider::widest<MyType, T2>::type>
            operator *(const Fixed<dps2, T2>& d)
        {
            auto temp = m_Value * d.m_Value;
            const auto f = details::Factor<(dps <= dps2 ? dps : dps2)>::value;
            if (temp >= 0) temp += f / 2; else temp -= f / 2;
            return Utils::MkFxd<(dps > dps2 ? dps : dps2), 
                typename details::wider::widest<MyType, T2>::type>
                (temp / f);
        }

        template<size_t dps2, typename T2>
        Fixed<(dps > dps2 ? dps : dps2), typename details::wider::widest<MyType, T2>::type>
            operator /(const Fixed<dps2, T2> d) const
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
            return details::ops::equal(*this, d);
        }

        template <size_t dps2, typename T2>
        inline bool operator >(const Fixed<dps2, T2>& d) const
        {
            return details::ops::greater(*this, d);
        }

        template <size_t dps2, typename T2>
        inline bool operator <(const Fixed<dps2, T2>& d) const
        {
            return details::ops::lesser(*this, d);
        }

        template <size_t dps2, typename T2>
        inline bool operator >=(const Fixed<dps2, T2>& d) const
        {
            return !details::ops::lesser(*this, d);
        }

        template <size_t dps2, typename T2>
        inline bool operator <=(const Fixed<dps2, T2>& d) const
        {
            return !details::ops::greater(*this, d);
        }

        template <size_t dps2, typename T2>
        inline Fixed<dps, MyType>& operator +=(const Fixed<dps2, T2>& d)
        {
            return details::ops::plusEqual(*this, d);
        }

        template <size_t dps2, typename T2>
        inline Fixed<dps, MyType>& operator -=(const Fixed<dps2, T2>& d)
        {
            return details::ops::minusEqual(*this, d);
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
        // template with enable_if is_integral
        // so all integer types can be used

        
        template<typename T> inline Fixed<dps, MyType> operator *(T v) const
        { 
            return Utils::MkFxd<dps, MyType>(m_Value * v);
        }
        template<typename T> Fixed<dps, MyType> operator /(T v) const
        {
            auto temp = ((m_Value * 10) / v);
            if (temp >= 0) temp += 5; else temp -= 5;
            return Utils::MkFxd<dps, MyType>(temp / 10);
        }
        template<typename T> inline Fixed<dps, MyType> operator +(T v) const
        {
            return Utils::MkFxd<dps, MyType>
                (m_Value + v * details::Factor<dps>::value);
        }
        template<typename T> inline Fixed<dps, MyType> operator -(T v) const
        {
            return Utils::MkFxd<dps, MyType>
                (m_Value - v * details::Factor<dps>::value);
        }

        template<typename T> inline Fixed<dps, MyType>& operator +=(T v)
        {
            m_Value += v * details::Factor<dps>::value; return *this;
        }
        template<typename T> inline Fixed<dps, MyType>& operator -=(T v)
        {
            m_Value -= v * details::Factor<dps>::value; return *this;
        }
        template<typename T> inline Fixed<dps, MyType>& operator *=(T v)
        { 
            m_Value *= v; return *this; 
        }
        template<typename T> Fixed<dps, MyType>& operator /=(T v)
        {
            auto temp = ((m_Value * 10) / v);
            if (temp >= 0) temp += 5; else temp -= 5;
            m_Value = temp / 10;
            return *this;
        }

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


        template <size_t dpsF, typename TF>
        friend Fixed<dpsF, TF> Utils::MkFxd(TF t);

    };

    template<size_t dps, typename T>
    inline Fixed<dps, T> operator + (const int a, const Fixed<dps, T>& b) {
        return Utils::MkFxd<dps,T>
            ((a * details::Factor<dps>::value) + b.m_Value);
    }

    template<size_t dps, typename T>
    inline Fixed<dps, T> operator - (const int a, const Fixed<dps, T>& b) {
        return Utils::MkFxd<dps, T>
            ((a * details::Factor<dps>::value) - b.m_Value);
    }

    template<size_t dps, typename T>
    inline Fixed<dps, T> operator * (const int a, const Fixed<dps, T>& b) {
        return Utils::MkFxd<dps, T>(a * b.m_Value);
    }

    template<size_t dps, typename T>
    inline Fixed<dps, T> operator / (const int a, const Fixed<dps, T>& b) {
        // NOTE: This large factor is quite dangerous
        T temp = ((T)a) * details::Factor<2*dps+1>::value;
        temp /= b.m_Value;
        if (temp >= 0) temp += 5; else temp -= 5;
        temp /= 10;		
        return Utils::MkFxd<dps, T>(temp);
    }

    template<size_t dps, typename T>
    inline int operator +=(const int a, const Fixed<dps, T>& b) {
        return static_cast<int>(a + (b.m_Value / details::Factor<dps>::value));
    }

    template<size_t dps, typename T>
    inline int operator -=(const int a, const Fixed<dps, T>& b) {
        return static_cast<int>(a + (b.m_Value / details::Factor<dps>::value));
    }

    template<size_t dps, typename T>
    inline int operator *=(const int a, const Fixed<dps, T>& b) {
        return static_cast<int>((a * b.m_Value) / details::Factor<dps>::value);
    }

    template<size_t dps, typename T>
    inline int operator /=(const int a, const Fixed<dps, T>& b) {
        return static_cast<int>(((a * details::Factor<dps>::value) / b.m_Value));
    }

    template<size_t dps, typename T>
    inline bool operator ==(const int a, const Fixed<dps, T>& b) {
        return (a * details::Factor<dps>::value) == b.m_Value;
    }

    template<size_t dps, typename T>
    inline bool operator >(const int a, const Fixed<dps, T>& b) {
        return (a * details::Factor<dps>::value) > b.m_Value;
    }

    template<size_t dps, typename T>
    inline bool operator <(const int a, const Fixed<dps, T>& b) {
        return (a * details::Factor<dps>::value) < b.m_Value;
    }

    template<size_t dps, typename T>
    inline bool operator >=(const int a, const Fixed<dps, T>& b) {
        return !((a * details::Factor<dps>::value) < b.m_Value);
    }

    template<size_t dps, typename T>
    inline bool operator <=(const int a, const Fixed<dps, T>& b) {
        return !((a * details::Factor<dps>::value) > b.m_Value);
    }
    
    template <size_t N, typename intfmt>
    std::ostream& operator<<(std::ostream& s, const Fixed<N, intfmt>& value)
    {
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

    // Stores a fixed point value whos number of decimal places in not known
    // at compile time. Useful for example if a function needs to return
    // either a Fixed<2> or Fixed<3>.
    template<typename MyType = config::DefaultType>	struct RTFixed
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
        int mul = (value.m_Value < 0) ? -1 : 1;
        if ((value.m_Value < 0)) s << "-";
        auto fillch = s.fill();
        auto width = s.width();
        s << value.get_integral() * mul << ".";
        s.fill('0');
        s.width(value.m_dps);
        s << value.get_fractional() * mul;
        s.width(width);
        s.fill(fillch);
        return s;
    }

    namespace FixedTypes
    {
        typedef Fixed<1, int> Fixed1i;
        typedef Fixed<2, int> Fixed2i;
        typedef Fixed<3, int> Fixed3i;
        typedef Fixed<4, int> Fixed4i;
        typedef Fixed<5, int> Fixed5i;
        typedef Fixed<6, int> Fixed6i;

        typedef Fixed<1, long long int> Fixed1ll;
        typedef Fixed<2, long long int> Fixed2ll;
        typedef Fixed<3, long long int> Fixed3ll;
        typedef Fixed<4, long long int> Fixed4ll;
        typedef Fixed<5, long long int> Fixed5ll;
        typedef Fixed<6, long long int> Fixed6ll;
    }

    namespace Utils
    {
        // Make a fixed point number using the same internal storage type
        // of the passed in Fixed value
        template <size_t dps, size_t dps2, typename T>
        inline constexpr Fixed<dps, T> MkFxd(Fixed<dps2, T> t)
        {
            return Fixed<dps, T>(t);
        }

        // Make a fixed point number using internal storage type
        // that matches the argument type
        template <size_t dps, typename T>
        inline constexpr Fixed<dps, T> MkFxd(T t)
        {
            return Fixed<dps,T>(t, Fixed<dps,T>::InternalValue());
        }
    }

    namespace UserDefinedLiterals
    {
        constexpr auto operator "" _fxd1(long double lld) { return ::FixedPoint::Utils::MkFxd<1>((int)(lld * 10        )); }
        constexpr auto operator "" _fxd2(long double lld) { return ::FixedPoint::Utils::MkFxd<2>((int)(lld * 100       )); }
        constexpr auto operator "" _fxd3(long double lld) { return ::FixedPoint::Utils::MkFxd<3>((int)(lld * 1000      )); }
        constexpr auto operator "" _fxd4(long double lld) { return ::FixedPoint::Utils::MkFxd<4>((int)(lld * 10000     )); }
        constexpr auto operator "" _fxd5(long double lld) { return ::FixedPoint::Utils::MkFxd<5>((int)(lld * 100000    )); }
        constexpr auto operator "" _fxd6(long double lld) { return ::FixedPoint::Utils::MkFxd<6>((int)(lld * 1000000   )); }
        constexpr auto operator "" _fxd7(long double lld) { return ::FixedPoint::Utils::MkFxd<7>((int)(lld * 10000000  )); }
        constexpr auto operator "" _fxd8(long double lld) { return ::FixedPoint::Utils::MkFxd<8>((int)(lld * 100000000 )); }
        constexpr auto operator "" _fxd9(long double lld) { return ::FixedPoint::Utils::MkFxd<9>((int)(lld * 1000000000)); }
    }


}

#endif
