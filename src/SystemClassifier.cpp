#include "../include/SystemClassifier.h"
#include <sstream>
#include <map>

std::string ClassificationResult::toJson() const {
    std::ostringstream oss;
    oss << "{"
        << "\"type\":\"" << typeString << "\","
        << "\"rankA\":" << rankA << ","
        << "\"rankAb\":" << rankAb << ","
        << "\"numVars\":" << numVars << ","
        << "\"numEquations\":" << numEquations << ","
        << "\"numFreeVars\":" << numFreeVars << ","
        << "\"explanation\":\"" << explanation << "\","
        << "\"solutionSummary\":\"" << solutionSummary << "\","
        << "\"pivotVars\":[";
    for (size_t i = 0; i < pivotVars.size(); ++i) {
        if (i > 0) oss << ",";
        oss << pivotVars[i];
    }
    oss << "],\"freeVars\":[";
    for (size_t i = 0; i < freeVars.size(); ++i) {
        if (i > 0) oss << ",";
        oss << freeVars[i];
    }
    oss << "],\"uniqueSolution\":[";
    for (size_t i = 0; i < uniqueSolutionValues.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "{\"var\":\"x_" << (i + 1) << "\",\"val\":\"" << uniqueSolutionValues[i].toString() 
            << "\",\"dec\":\"" << uniqueSolutionValues[i].toDecimalString() << "\"}";
    }
    oss << "],\"parametricSolution\":[";
    for (size_t i = 0; i < parametricSolution.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "\"" << parametricSolution[i] << "\"";
    }
    oss << "]}";
    return oss.str();
}

ClassificationResult SystemClassifier::classify(const Matrix& rrefMatrix) {
    ClassificationResult res;
    size_t rows = rrefMatrix.getRows();
    size_t cols = rrefMatrix.getCols();
    size_t m = rrefMatrix.getVarCount();

    res.numEquations = rows;
    res.numVars = m;

    // Step 1: Detect contradiction row [0 0 ... 0 | c] where c != 0
    bool hasContradiction = false;
    size_t contradictionRow = 0;
    Fraction contradictionVal = 0;

    size_t rankA_count = 0;
    size_t rankAb_count = 0;

    // Map pivot column -> row index
    std::map<size_t, size_t> pivotColToRow;
    std::vector<bool> isPivotCol(m, false);

    for (size_t r = 0; r < rows; ++r) {
        bool allZeroA = true;
        int leadingCol = -1;

        for (size_t c = 0; c < m; ++c) {
            if (!rrefMatrix.at(r, c).isZero()) {
                allZeroA = false;
                if (leadingCol == -1) {
                    leadingCol = static_cast<int>(c);
                }
            }
        }

        bool zeroB = rrefMatrix.at(r, cols - 1).isZero();

        if (!allZeroA) {
            rankA_count++;
            rankAb_count++;
            if (leadingCol != -1) {
                pivotColToRow[leadingCol] = r;
                isPivotCol[leadingCol] = true;
                res.pivotVars.push_back(leadingCol);
            }
        } else {
            if (!zeroB) {
                // Inconsistent contradiction
                rankAb_count++;
                if (!hasContradiction) {
                    hasContradiction = true;
                    contradictionRow = r;
                    contradictionVal = rrefMatrix.at(r, cols - 1);
                }
            }
        }
    }

    res.rankA = rankA_count;
    res.rankAb = rankAb_count;

    // Step 2: Categorize system
    if (hasContradiction || res.rankA < res.rankAb) {
        res.type = SystemType::INCONSISTENT;
        res.typeString = "INCONSISTENT";
        res.numFreeVars = 0;
        res.solutionSummary = "No solution exists (Inconsistent System).";
        res.explanation = "Row " + std::to_string(contradictionRow + 1) + 
                          " produces the contradictory equation: 0 = " + contradictionVal.toString() + 
                          ". Because rank(A) = " + std::to_string(res.rankA) + " < rank([A|b]) = " + 
                          std::to_string(res.rankAb) + ", no values of variables can satisfy this system.";
        return res;
    }

    // Identify free variables
    for (size_t c = 0; c < m; ++c) {
        if (!isPivotCol[c]) {
            res.freeVars.push_back(c);
        }
    }
    res.numFreeVars = res.freeVars.size();

    if (res.numFreeVars == 0 && res.rankA == m) {
        // Unique Solution
        res.type = SystemType::UNIQUE_SOLUTION;
        res.typeString = "UNIQUE";
        res.uniqueSolutionValues.resize(m);

        std::ostringstream summary;
        summary << "Unique Solution: ";

        for (size_t c = 0; c < m; ++c) {
            size_t r = pivotColToRow[c];
            Fraction val = rrefMatrix.at(r, cols - 1);
            res.uniqueSolutionValues[c] = val;
            if (c > 0) summary << ", ";
            summary << "x_" << (c + 1) << " = " << val.toString();
        }

        res.solutionSummary = summary.str();
        res.explanation = "The coefficient matrix has full column rank (rank(A) = " + 
                          std::to_string(res.rankA) + " = number of variables " + std::to_string(m) + 
                          "). Every variable is a pivot variable, yielding a single unique solution vector.";
    } else {
        // Infinitely Many Solutions
        res.type = SystemType::INFINITELY_MANY;
        res.typeString = "INFINITE";

        std::map<size_t, std::string> paramNames;
        for (size_t i = 0; i < res.freeVars.size(); ++i) {
            paramNames[res.freeVars[i]] = "t_" + std::to_string(i + 1);
        }

        res.parametricSolution.resize(m);
        for (size_t c = 0; c < m; ++c) {
            if (!isPivotCol[c]) {
                // Free variable
                res.parametricSolution[c] = "x_" + std::to_string(c + 1) + " = " + paramNames[c] + "  (free, " + paramNames[c] + " in R)";
            } else {
                // Basic / Pivot variable
                size_t r = pivotColToRow[c];
                Fraction constTerm = rrefMatrix.at(r, cols - 1);
                
                std::ostringstream eq;
                eq << "x_" << (c + 1) << " = ";
                bool hasTerm = false;

                if (!constTerm.isZero() || res.freeVars.empty()) {
                    eq << constTerm.toString();
                    hasTerm = true;
                }

                for (size_t fCol : res.freeVars) {
                    Fraction coeff = rrefMatrix.at(r, fCol);
                    if (!coeff.isZero()) {
                        Fraction termCoeff = -coeff; // move to RHS
                        std::string pName = paramNames[fCol];

                        if (termCoeff > Fraction(0)) {
                            if (hasTerm) eq << " + ";
                            if (termCoeff.isOne()) {
                                eq << pName;
                            } else {
                                eq << termCoeff.toString() << pName;
                            }
                        } else {
                            Fraction absCoeff = -termCoeff;
                            eq << " - ";
                            if (absCoeff.isOne()) {
                                eq << pName;
                            } else {
                                eq << absCoeff.toString() << pName;
                            }
                        }
                        hasTerm = true;
                    }
                }
                if (!hasTerm) {
                    eq << "0";
                }
                res.parametricSolution[c] = eq.str();
            }
        }

        std::ostringstream summary;
        summary << "Infinitely many solutions (" << res.numFreeVars << " free variable" 
                << (res.numFreeVars > 1 ? "s" : "") << "): ";
        for (size_t i = 0; i < res.freeVars.size(); ++i) {
            if (i > 0) summary << ", ";
            summary << "x_" << (res.freeVars[i] + 1) << " free (" << paramNames[res.freeVars[i]] << ")";
        }
        res.solutionSummary = summary.str();
        res.explanation = "System is consistent with rank(A) = " + std::to_string(res.rankA) + 
                          " < number of variables " + std::to_string(m) + 
                          ". There are " + std::to_string(res.numFreeVars) + 
                          " degrees of freedom (free variables), resulting in infinitely many solutions.";
    }

    return res;
}
