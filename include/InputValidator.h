#ifndef INPUT_VALIDATOR_H
#define INPUT_VALIDATOR_H

#include "Matrix.h"
#include <string>
#include <vector>

struct EquationInput {
    std::vector<std::string> coefficients; // length m
    std::string rhs;                       // 1 constant
};

struct ValidationResult {
    bool isValid;
    std::string generalError;
    // Map of (rowIndex, colIndex) to error message. ColIndex == m means RHS.
    struct CellError {
        size_t row;
        size_t col; // 0..m-1 for coeff, m for RHS
        std::string message;
    };
    std::vector<CellError> cellErrors;
    Matrix parsedMatrix;

    std::string toJson() const;
};

class InputValidator {
public:
    static const size_t MIN_ROWS = 1;
    static const size_t MAX_ROWS = 10;
    static const size_t MIN_VARS = 1;
    static const size_t MAX_VARS = 10;

    static bool validateDimensions(size_t rows, size_t vars, std::string& outError);
    static ValidationResult validateAndBuild(size_t rows, size_t vars, 
                                            const std::vector<EquationInput>& equations);
};

#endif // INPUT_VALIDATOR_H
