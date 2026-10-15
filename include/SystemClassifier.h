#ifndef SYSTEM_CLASSIFIER_H
#define SYSTEM_CLASSIFIER_H

#include "Matrix.h"
#include <string>
#include <vector>

enum class SystemType {
    UNIQUE_SOLUTION,
    INFINITELY_MANY,
    INCONSISTENT
};

struct ClassificationResult {
    SystemType type;
    std::string typeString; // "UNIQUE", "INFINITE", "INCONSISTENT"
    size_t rankA;
    size_t rankAb;
    size_t numVars;
    size_t numEquations;
    size_t numFreeVars;
    
    std::vector<size_t> pivotVars; // 0-indexed column indices
    std::vector<size_t> freeVars;  // 0-indexed column indices

    // Solutions
    std::vector<Fraction> uniqueSolutionValues; // if UNIQUE
    std::vector<std::string> parametricSolution; // if INFINITE (e.g. x1 = 3 - 2*t1)
    
    std::string explanation;
    std::string solutionSummary;

    std::string toJson() const;
};

class SystemClassifier {
public:
    static ClassificationResult classify(const Matrix& rrefMatrix);
};

#endif // SYSTEM_CLASSIFIER_H
