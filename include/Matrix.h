#ifndef MATRIX_H
#define MATRIX_H

#include "Fraction.h"
#include <vector>
#include <string>
#include <iostream>

/**
 * @class Matrix
 * @brief Represents an augmented matrix [A | b] with n rows and m+1 columns.
 * 
 * Provides elementary row operations:
 * - Row swapping: R_i <-> R_j
 * - Multiplying a row by a nonzero scalar: R_i -> k * R_i
 * - Adding a multiple of one row to another: R_i -> R_i + k * R_j
 */
class Matrix {
private:
    size_t numRows;
    size_t numCols; // m + 1 (m coefficients + 1 RHS constant)
    std::vector<std::vector<Fraction>> data;

public:
    // Constructors
    Matrix();
    Matrix(size_t rows, size_t cols);
    Matrix(size_t rows, size_t cols, const std::vector<std::vector<Fraction>>& values);

    // Dimension getters
    size_t getRows() const { return numRows; }
    size_t getCols() const { return numCols; }
    size_t getVarCount() const { return (numCols > 0) ? numCols - 1 : 0; }

    // Element access
    Fraction& at(size_t r, size_t c);
    const Fraction& at(size_t r, size_t c) const;

    // Elementary Row Operations (EROs)
    void swapRows(size_t r1, size_t r2);
    void scaleRow(size_t r, const Fraction& scalar);
    void addRowMultiple(size_t targetRow, size_t sourceRow, const Fraction& scalar);

    // Matrix properties and queries
    bool isRowAllZeros(size_t r, bool includeRhs = true) const;
    int getLeadingEntryColumn(size_t r) const; // returns -1 if row is all zeros (coefficients only)
    bool isAlreadyREF() const;
    bool isAlreadyRREF() const;

    // Utility formatting
    std::string toString() const;
    std::string toJson() const;
};

#endif // MATRIX_H
