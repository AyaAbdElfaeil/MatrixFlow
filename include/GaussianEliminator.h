#ifndef GAUSSIAN_ELIMINATOR_H
#define GAUSSIAN_ELIMINATOR_H

#include "Matrix.h"
#include "Step.h"
#include <vector>

enum class SolveMode {
    REF,
    RREF
};

/**
 * @class GaussianEliminator
 * @brief Performs step-by-step Gaussian Elimination and Gauss-Jordan Elimination.
 * 
 * Supports both Row Echelon Form (REF) and Reduced Row Echelon Form (RREF).
 * Automatically handles partial pivoting, column skipping, rectangular systems,
 * and records detailed pedagogical explanations at every single row operation.
 */
class GaussianEliminator {
public:
    struct Result {
        Matrix initialMatrix;
        Matrix finalMatrix;
        SolveMode mode;
        std::vector<Step> steps;
        std::vector<std::pair<size_t, size_t>> pivotPositions; // (row, col) for each pivot
        bool wasAlreadyInForm;
        std::string summaryMessage;
    };

    static Result solve(const Matrix& matrix, SolveMode mode);

private:
    static void solveREF(Matrix& mat, std::vector<Step>& steps, 
                         std::vector<std::pair<size_t, size_t>>& pivots, int& stepCounter);
    static void solveRREF(Matrix& mat, std::vector<Step>& steps, 
                          std::vector<std::pair<size_t, size_t>>& pivots, int& stepCounter);
};

#endif // GAUSSIAN_ELIMINATOR_H
