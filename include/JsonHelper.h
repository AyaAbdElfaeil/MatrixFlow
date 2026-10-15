#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include "InputValidator.h"
#include "GaussianEliminator.h"
#include <string>
#include <vector>

/**
 * @class JsonHelper
 * @brief Lightweight JSON parser and formatter designed for MatrixFlow payloads.
 */
class JsonHelper {
public:
    struct SolveRequest {
        size_t rows;
        size_t vars;
        SolveMode mode;
        std::vector<EquationInput> equations;
        bool parseSuccess;
        std::string errorMessage;
    };

    static SolveRequest parseSolveRequest(const std::string& json);
    static std::string escapeString(const std::string& s);
};

#endif // JSON_HELPER_H
