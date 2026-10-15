#include "../include/Fraction.h"
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <stdexcept>
#include <algorithm>

long long Fraction::gcd(long long a, long long b) {
    a = std::abs(a);
    b = std::abs(b);
    while (b != 0) {
        long long temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

void Fraction::simplify() {
    if (den == 0) {
        throw std::invalid_argument("Fraction denominator cannot be zero.");
    }
    if (num == 0) {
        den = 1;
        return;
    }
    if (den < 0) {
        num = -num;
        den = -den;
    }
    long long common = gcd(num, den);
    if (common > 1) {
        num /= common;
        den /= common;
    }
}

Fraction::Fraction() : num(0), den(1) {}

Fraction::Fraction(int n) : num(n), den(1) {}

Fraction::Fraction(long long n) : num(n), den(1) {}

Fraction::Fraction(int n, int d) : num(n), den(d) {
    simplify();
}

Fraction::Fraction(long long n, long long d) : num(n), den(d) {
    simplify();
}

Fraction::Fraction(double value) {
    if (std::isnan(value) || std::isinf(value)) {
        num = 0;
        den = 1;
        return;
    }
    // Continued fraction approximation for clean rational conversion
    double eps = 1e-9;
    double x = value;
    long long sign = (x < 0) ? -1 : 1;
    x = std::abs(x);

    long long h1 = 1, h0 = 0;
    long long k1 = 0, k0 = 1;
    double b = x;

    for (int i = 0; i < 20; ++i) {
        long long a = static_cast<long long>(std::floor(b));
        long long h2 = a * h1 + h0;
        long long k2 = a * k1 + k0;

        if (k2 > 10000000) break; // Limit denominator to reasonable magnitude

        h0 = h1; h1 = h2;
        k0 = k1; k1 = k2;

        if (std::abs(x - static_cast<double>(h1) / static_cast<double>(k1)) < eps) {
            break;
        }

        double diff = b - a;
        if (std::abs(diff) < 1e-12) break;
        b = 1.0 / diff;
    }

    num = sign * h1;
    den = k1;
    simplify();
}

static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

Fraction Fraction::fromString(const std::string& str) {
    Fraction f;
    std::string err;
    if (!tryParse(str, f, err)) {
        throw std::invalid_argument(err);
    }
    return f;
}

bool Fraction::tryParse(const std::string& rawStr, Fraction& outFrac, std::string& outError) {
    std::string s = trim(rawStr);
    if (s.empty()) {
        outError = "Input cannot be empty.";
        return false;
    }

    // Check for fraction format: "A/B"
    size_t slashPos = s.find('/');
    if (slashPos != std::string::npos) {
        std::string numPart = trim(s.substr(0, slashPos));
        std::string denPart = trim(s.substr(slashPos + 1));

        if (numPart.empty() || denPart.empty()) {
            outError = "Invalid fraction format: '" + s + "'";
            return false;
        }

        try {
            size_t idx1 = 0, idx2 = 0;
            long long n = std::stoll(numPart, &idx1);
            long long d = std::stoll(denPart, &idx2);

            if (idx1 != numPart.length() || idx2 != denPart.length()) {
                outError = "Non-numeric characters in fraction: '" + s + "'";
                return false;
            }
            if (d == 0) {
                outError = "Division by zero in fraction: '" + s + "'";
                return false;
            }
            outFrac = Fraction(n, d);
            return true;
        } catch (...) {
            outError = "Number out of range in fraction: '" + s + "'";
            return false;
        }
    }

    // Check for decimal format or standard integer
    try {
        size_t idx = 0;
        if (s.find('.') != std::string::npos) {
            double d = std::stod(s, &idx);
            if (idx != s.length()) {
                outError = "Non-numeric characters in number: '" + s + "'";
                return false;
            }
            outFrac = Fraction(d);
            return true;
        } else {
            long long n = std::stoll(s, &idx);
            if (idx != s.length()) {
                outError = "Non-numeric characters in integer: '" + s + "'";
                return false;
            }
            outFrac = Fraction(n);
            return true;
        }
    } catch (...) {
        outError = "Invalid number format: '" + s + "'";
        return false;
    }
}

Fraction Fraction::operator+(const Fraction& other) const {
    long long n = num * other.den + other.num * den;
    long long d = den * other.den;
    return Fraction(n, d);
}

Fraction Fraction::operator-(const Fraction& other) const {
    long long n = num * other.den - other.num * den;
    long long d = den * other.den;
    return Fraction(n, d);
}

Fraction Fraction::operator*(const Fraction& other) const {
    long long n = num * other.num;
    long long d = den * other.den;
    return Fraction(n, d);
}

Fraction Fraction::operator/(const Fraction& other) const {
    if (other.isZero()) {
        throw std::invalid_argument("Division by zero fraction.");
    }
    long long n = num * other.den;
    long long d = den * other.num;
    return Fraction(n, d);
}

Fraction Fraction::operator-() const {
    return Fraction(-num, den);
}

Fraction& Fraction::operator+=(const Fraction& other) {
    *this = *this + other;
    return *this;
}

Fraction& Fraction::operator-=(const Fraction& other) {
    *this = *this - other;
    return *this;
}

Fraction& Fraction::operator*=(const Fraction& other) {
    *this = *this * other;
    return *this;
}

Fraction& Fraction::operator/=(const Fraction& other) {
    *this = *this / other;
    return *this;
}

bool Fraction::operator==(const Fraction& other) const {
    return num == other.num && den == other.den;
}

bool Fraction::operator!=(const Fraction& other) const {
    return !(*this == other);
}

bool Fraction::operator<(const Fraction& other) const {
    return num * other.den < other.num * den;
}

bool Fraction::operator>(const Fraction& other) const {
    return num * other.den > other.num * den;
}

bool Fraction::operator<=(const Fraction& other) const {
    return !(*this > other);
}

bool Fraction::operator>=(const Fraction& other) const {
    return !(*this < other);
}

Fraction Fraction::abs() const {
    return Fraction(std::abs(num), den);
}

bool Fraction::isZero() const {
    return num == 0;
}

bool Fraction::isOne() const {
    return num == 1 && den == 1;
}

bool Fraction::isInteger() const {
    return den == 1;
}

double Fraction::toDouble() const {
    return static_cast<double>(num) / static_cast<double>(den);
}

std::string Fraction::toString() const {
    if (den == 1) {
        return std::to_string(num);
    }
    return std::to_string(num) + "/" + std::to_string(den);
}

std::string Fraction::toDecimalString(int precision) const {
    double val = toDouble();
    // Clean -0.000 display
    if (std::abs(val) < 1e-9) {
        val = 0.0;
    }
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << val;
    std::string str = oss.str();
    
    // Remove trailing zeros after decimal point if clean
    if (str.find('.') != std::string::npos) {
        while (str.back() == '0') {
            str.pop_back();
        }
        if (str.back() == '.') {
            str.pop_back();
        }
    }
    if (str == "-0") str = "0";
    return str;
}

std::ostream& operator<<(std::ostream& os, const Fraction& f) {
    os << f.toString();
    return os;
}
