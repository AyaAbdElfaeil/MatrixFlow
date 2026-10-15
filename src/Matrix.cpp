#include "../include/Matrix.h"
#include <iomanip>
#include <sstream>
#include <stdexcept>

Matrix::Matrix() : numRows(0), numCols(0) {}

Matrix::Matrix(size_t rows, size_t cols)
    : numRows(rows), numCols(cols), data(rows, std::vector<Fraction>(cols, Fraction(0))) {}

Matrix::Matrix(size_t rows, size_t cols, const std::vector<std::vector<Fraction>>& values)
    : numRows(rows), numCols(cols), data(values) {
    if (data.size() != rows) {
        data.resize(rows, std::vector<Fraction>(cols, Fraction(0)));
    }
    for (size_t r = 0; r < rows; ++r) {
        if (data[r].size() != cols) {
            data[r].resize(cols, Fraction(0));
        }
    }
}

Fraction& Matrix::at(size_t r, size_t c) {
    if (r >= numRows || c >= numCols) {
        throw std::out_of_range("Matrix index out of bounds: (" + std::to_string(r) + ", " + std::to_string(c) + ")");
    }
    return data[r][c];
}

const Fraction& Matrix::at(size_t r, size_t c) const {
    if (r >= numRows || c >= numCols) {
        throw std::out_of_range("Matrix index out of bounds: (" + std::to_string(r) + ", " + std::to_string(c) + ")");
    }
    return data[r][c];
}

void Matrix::swapRows(size_t r1, size_t r2) {
    if (r1 >= numRows || r2 >= numRows) {
        throw std::out_of_range("Invalid row indices for swap.");
    }
    if (r1 == r2) return;
    std::swap(data[r1], data[r2]);
}

void Matrix::scaleRow(size_t r, const Fraction& scalar) {
    if (r >= numRows) {
        throw std::out_of_range("Invalid row index for scale.");
    }
    if (scalar.isZero()) {
        throw std::invalid_argument("Row operation error: cannot scale a row by zero.");
    }
    for (size_t c = 0; c < numCols; ++c) {
        data[r][c] *= scalar;
    }
}

void Matrix::addRowMultiple(size_t targetRow, size_t sourceRow, const Fraction& scalar) {
    if (targetRow >= numRows || sourceRow >= numRows) {
        throw std::out_of_range("Invalid row indices for addRowMultiple.");
    }
    if (targetRow == sourceRow || scalar.isZero()) return;
    
    for (size_t c = 0; c < numCols; ++c) {
        data[targetRow][c] += scalar * data[sourceRow][c];
    }
}

bool Matrix::isRowAllZeros(size_t r, bool includeRhs) const {
    if (r >= numRows) return true;
    size_t limit = includeRhs ? numCols : (numCols > 1 ? numCols - 1 : numCols);
    for (size_t c = 0; c < limit; ++c) {
        if (!data[r][c].isZero()) return false;
    }
    return true;
}

int Matrix::getLeadingEntryColumn(size_t r) const {
    if (r >= numRows) return -1;
    size_t coeffCols = (numCols > 1) ? numCols - 1 : numCols;
    for (size_t c = 0; c < coeffCols; ++c) {
        if (!data[r][c].isZero()) {
            return static_cast<int>(c);
        }
    }
    return -1;
}

bool Matrix::isAlreadyREF() const {
    int lastLeadingCol = -1;
    bool foundZeroRow = false;
    size_t coeffCols = (numCols > 1) ? numCols - 1 : numCols;

    for (size_t r = 0; r < numRows; ++r) {
        int leadingCol = -1;
        for (size_t c = 0; c < coeffCols; ++c) {
            if (!data[r][c].isZero()) {
                leadingCol = static_cast<int>(c);
                break;
            }
        }

        if (leadingCol == -1) {
            foundZeroRow = true;
        } else {
            if (foundZeroRow) {
                // Nonzero row appears after a zero row -> not REF
                return false;
            }
            if (leadingCol <= lastLeadingCol) {
                // Pivot is not strictly to the right of the previous pivot -> not REF
                return false;
            }
            // All entries below this leading entry must be zero
            for (size_t i = r + 1; i < numRows; ++i) {
                if (!data[i][leadingCol].isZero()) {
                    return false;
                }
            }
            lastLeadingCol = leadingCol;
        }
    }
    return true;
}

bool Matrix::isAlreadyRREF() const {
    if (!isAlreadyREF()) return false;

    size_t coeffCols = (numCols > 1) ? numCols - 1 : numCols;
    for (size_t r = 0; r < numRows; ++r) {
        int leadingCol = -1;
        for (size_t c = 0; c < coeffCols; ++c) {
            if (!data[r][c].isZero()) {
                leadingCol = static_cast<int>(c);
                break;
            }
        }

        if (leadingCol != -1) {
            // Leading entry must be 1
            if (!data[r][leadingCol].isOne()) {
                return false;
            }
            // Leading entry must be the ONLY nonzero entry in its column
            for (size_t i = 0; i < numRows; ++i) {
                if (i != r && !data[i][leadingCol].isZero()) {
                    return false;
                }
            }
        }
    }
    return true;
}

std::string Matrix::toString() const {
    std::ostringstream oss;
    for (size_t r = 0; r < numRows; ++r) {
        oss << "[ ";
        for (size_t c = 0; c < numCols; ++c) {
            if (c == numCols - 1 && numCols > 1) {
                oss << "| ";
            }
            oss << std::setw(6) << data[r][c].toString() << " ";
        }
        oss << "]\n";
    }
    return oss.str();
}

std::string Matrix::toJson() const {
    std::ostringstream oss;
    oss << "{\"rows\":" << numRows << ",\"cols\":" << numCols << ",\"data\":[";
    for (size_t r = 0; r < numRows; ++r) {
        if (r > 0) oss << ",";
        oss << "[";
        for (size_t c = 0; c < numCols; ++c) {
            if (c > 0) oss << ",";
            oss << "{\"num\":" << data[r][c].num 
                << ",\"den\":" << data[r][c].den
                << ",\"str\":\"" << data[r][c].toString() << "\""
                << ",\"dec\":\"" << data[r][c].toDecimalString() << "\""
                << ",\"val\":" << data[r][c].toDouble() << "}";
        }
        oss << "]";
    }
    oss << "]}";
    return oss.str();
}
