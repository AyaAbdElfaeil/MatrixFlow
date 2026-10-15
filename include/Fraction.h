#ifndef FRACTION_H
#define FRACTION_H

#include <iostream>
#include <string>
#include <sstream>

/**
 * @class Fraction
 * @brief Represents an exact rational number (numerator / denominator).
 * 
 * In linear algebra, exact fraction arithmetic avoids floating-point drift
 * and yields standard textbook solutions (e.g. 2/3 instead of 0.666667).
 */
class Fraction {
public:
    long long num; ///< Numerator
    long long den; ///< Denominator (always kept positive)

    // Constructors
    Fraction();
    Fraction(int n);
    Fraction(long long n);
    Fraction(long long n, long long d);
    Fraction(int n, int d);
    Fraction(double value);
    
    // Parse string: accepts "5", "-3", "2/7", "-4/5", "1.25", "-0.5"
    static Fraction fromString(const std::string& str);
    static bool tryParse(const std::string& str, Fraction& outFrac, std::string& outError);

    // Arithmetic operators
    Fraction operator+(const Fraction& other) const;
    Fraction operator-(const Fraction& other) const;
    Fraction operator*(const Fraction& other) const;
    Fraction operator/(const Fraction& other) const;
    Fraction operator-() const;

    // Assignment arithmetic operators
    Fraction& operator+=(const Fraction& other);
    Fraction& operator-=(const Fraction& other);
    Fraction& operator*=(const Fraction& other);
    Fraction& operator/=(const Fraction& other);

    // Comparison operators
    bool operator==(const Fraction& other) const;
    bool operator!=(const Fraction& other) const;
    bool operator<(const Fraction& other) const;
    bool operator>(const Fraction& other) const;
    bool operator<=(const Fraction& other) const;
    bool operator>=(const Fraction& other) const;

    // Utility methods
    Fraction abs() const;
    bool isZero() const;
    bool isOne() const;
    bool isInteger() const;
    double toDouble() const;
    
    // Formatting methods
    std::string toString() const;                  // e.g. "5", "-3/4", "0"
    std::string toDecimalString(int precision = 3) const; // e.g. "0.750"

private:
    void simplify();
    static long long gcd(long long a, long long b);
};

// Stream output
std::ostream& operator<<(std::ostream& os, const Fraction& f);

#endif // FRACTION_H
