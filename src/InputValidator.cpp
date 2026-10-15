#include "../include/InputValidator.h"
#include <sstream>

std::string ValidationResult::toJson() const {
    std::ostringstream oss;
    oss << "{"
        << "\"isValid\":" << (isValid ? "true" : "false") << ","
        << "\"generalError\":\"" << generalError << "\","
        << "\"cellErrors\":[";
    for (size_t i = 0; i < cellErrors.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "{\"row\":" << cellErrors[i].row
            << ",\"col\":" << cellErrors[i].col
            << ",\"message\":\"" << cellErrors[i].message << "\"}";
    }
    oss << "]";
    if (isValid) {
        oss << ",\"matrix\":" << parsedMatrix.toJson();
    }
    oss << "}";
    return oss.str();
}

bool InputValidator::validateDimensions(size_t rows, size_t vars, std::string& outError) {
    if (rows < MIN_ROWS || rows > MAX_ROWS) {
        outError = "Number of equations (rows n) must be between " + 
                   std::to_string(MIN_ROWS) + " and " + std::to_string(MAX_ROWS) + ".";
        return false;
    }
    if (vars < MIN_VARS || vars > MAX_VARS) {
        outError = "Number of variables (columns m) must be between " + 
                   std::to_string(MIN_VARS) + " and " + std::to_string(MAX_VARS) + ".";
        return false;
    }
    return true;
}

ValidationResult InputValidator::validateAndBuild(size_t rows, size_t vars, 
                                                 const std::vector<EquationInput>& equations) {
    ValidationResult res;
    res.isValid = true;

    std::string dimErr;
    if (!validateDimensions(rows, vars, dimErr)) {
        res.isValid = false;
        res.generalError = dimErr;
        return res;
    }

    if (equations.size() != rows) {
        res.isValid = false;
        res.generalError = "Expected " + std::to_string(rows) + " equations, but received " + 
                           std::to_string(equations.size()) + ".";
        return res;
    }

    Matrix mat(rows, vars + 1);

    for (size_t r = 0; r < rows; ++r) {
        const auto& eq = equations[r];

        // Check coefficients
        for (size_t c = 0; c < vars; ++c) {
            std::string valStr = (c < eq.coefficients.size()) ? eq.coefficients[c] : "";
            Fraction frac;
            std::string err;
            if (!Fraction::tryParse(valStr, frac, err)) {
                res.isValid = false;
                res.cellErrors.push_back({r, c, "Equation " + std::to_string(r + 1) + 
                                         ", variable x_" + std::to_string(c + 1) + ": " + err});
            } else {
                mat.at(r, c) = frac;
            }
        }

        // Check RHS constant
        Fraction rhsFrac;
        std::string rhsErr;
        if (!Fraction::tryParse(eq.rhs, rhsFrac, rhsErr)) {
            res.isValid = false;
            res.cellErrors.push_back({r, vars, "Equation " + std::to_string(r + 1) + 
                                     ", constant (RHS): " + rhsErr});
        } else {
            mat.at(r, vars) = rhsFrac;
        }
    }

    if (res.isValid) {
        res.parsedMatrix = mat;
    } else {
        res.generalError = "Please correct the highlighted inputs before generating the matrix.";
    }

    return res;
}
