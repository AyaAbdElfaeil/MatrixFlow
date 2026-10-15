#include "../include/GaussianEliminator.h"
#include <cmath>

GaussianEliminator::Result GaussianEliminator::solve(const Matrix& initialMat, SolveMode mode) {
    Result res;
    res.initialMatrix = initialMat;
    res.mode = mode;
    Matrix current = initialMat;
    int stepCounter = 0;

    // Record Step 0: Initial Matrix
    Step initialStep;
    initialStep.stepNumber = stepCounter++;
    initialStep.title = "Initial Augmented Matrix";
    initialStep.operationType = "INITIAL";
    initialStep.notation = "None";
    initialStep.reason = "Starting system of linear equations [A | b]";
    initialStep.explanation = "Matrix constructed with " + std::to_string(current.getRows()) +
                              " equations and " + std::to_string(current.getVarCount()) + " variables.";
    initialStep.matrixBefore = current;
    initialStep.matrixAfter = current;
    res.steps.push_back(initialStep);

    // Check if already in form
    bool already = (mode == SolveMode::REF) ? current.isAlreadyREF() : current.isAlreadyRREF();
    if (already) {
        res.wasAlreadyInForm = true;
        res.finalMatrix = current;
        res.summaryMessage = (mode == SolveMode::REF)
            ? "The matrix is already in Row Echelon Form (REF). No operations required."
            : "The matrix is already in Reduced Row Echelon Form (RREF). No operations required.";
        
        Step doneStep;
        doneStep.stepNumber = stepCounter++;
        doneStep.title = "Already in " + std::string(mode == SolveMode::REF ? "REF" : "RREF");
        doneStep.operationType = "COMPLETE";
        doneStep.notation = "None";
        doneStep.reason = res.summaryMessage;
        doneStep.explanation = "All REF/RREF mathematical criteria are already satisfied by this matrix.";
        doneStep.matrixBefore = current;
        doneStep.matrixAfter = current;
        res.steps.push_back(doneStep);

        // Find pivots in this already-formed matrix
        for (size_t r = 0; r < current.getRows(); ++r) {
            int leadCol = current.getLeadingEntryColumn(r);
            if (leadCol != -1) {
                res.pivotPositions.push_back({r, static_cast<size_t>(leadCol)});
            }
        }
        return res;
    }

    res.wasAlreadyInForm = false;

    if (mode == SolveMode::REF) {
        solveREF(current, res.steps, res.pivotPositions, stepCounter);
    } else {
        solveRREF(current, res.steps, res.pivotPositions, stepCounter);
    }

    res.finalMatrix = current;
    res.summaryMessage = (mode == SolveMode::REF)
        ? "Successfully transformed matrix into Row Echelon Form (REF) in " + std::to_string(res.steps.size() - 1) + " operations."
        : "Successfully transformed matrix into Reduced Row Echelon Form (RREF) in " + std::to_string(res.steps.size() - 1) + " operations.";

    // Add Final Summary Step
    Step finalStep;
    finalStep.stepNumber = stepCounter++;
    finalStep.title = "Elimination Complete";
    finalStep.operationType = "COMPLETE";
    finalStep.notation = (mode == SolveMode::REF) ? "REF Complete" : "RREF Complete";
    finalStep.reason = (mode == SolveMode::REF)
        ? "All entries below leading pivots have been eliminated to zero."
        : "All pivots are normalized to 1 and are the sole non-zero entries in their columns.";
    finalStep.explanation = res.summaryMessage + " Number of pivots identified: " + std::to_string(res.pivotPositions.size()) + ".";
    finalStep.matrixBefore = current;
    finalStep.matrixAfter = current;
    res.steps.push_back(finalStep);

    return res;
}

void GaussianEliminator::solveREF(Matrix& mat, std::vector<Step>& steps, 
                                 std::vector<std::pair<size_t, size_t>>& pivots, int& stepCounter) {
    size_t rows = mat.getRows();
    size_t coeffCols = mat.getVarCount();
    size_t pivotRow = 0;

    for (size_t col = 0; col < coeffCols && pivotRow < rows; ++col) {
        // Step 1: Find best pivot in column `col` from `pivotRow` down
        size_t bestRow = pivotRow;
        Fraction maxVal = mat.at(pivotRow, col).abs();

        for (size_t r = pivotRow + 1; r < rows; ++r) {
            if (mat.at(r, col).abs() > maxVal) {
                maxVal = mat.at(r, col).abs();
                bestRow = r;
            }
        }

        // If the entire column below pivotRow is zero, skip to next column
        if (maxVal.isZero()) {
            continue;
        }

        // Step 2: Swap rows if needed
        if (bestRow != pivotRow) {
            Step swapStep;
            swapStep.stepNumber = stepCounter++;
            swapStep.title = "Row Swap: R" + std::to_string(pivotRow + 1) + " <-> R" + std::to_string(bestRow + 1);
            swapStep.operationType = "SWAP";
            swapStep.notation = "R_{" + std::to_string(pivotRow + 1) + "} \\leftrightarrow R_{" + std::to_string(bestRow + 1) + "}";
            
            if (mat.at(pivotRow, col).isZero()) {
                swapStep.reason = "The current pivot candidate at (" + std::to_string(pivotRow + 1) + ", " + 
                                  std::to_string(col + 1) + ") is 0. A non-zero pivot is required.";
            } else {
                swapStep.reason = "Partial pivoting: swap with Row " + std::to_string(bestRow + 1) + 
                                  " (value: " + mat.at(bestRow, col).toString() + ") for numerical stability.";
            }
            
            swapStep.explanation = "Swapped Row " + std::to_string(pivotRow + 1) + " and Row " + 
                                   std::to_string(bestRow + 1) + " to place a non-zero leading coefficient at column " + 
                                   std::to_string(col + 1) + ".";
            swapStep.matrixBefore = mat;
            swapStep.pivotRow = static_cast<int>(pivotRow);
            swapStep.pivotCol = static_cast<int>(col);
            swapStep.activeRow = static_cast<int>(pivotRow);
            swapStep.sourceRow = static_cast<int>(bestRow);

            mat.swapRows(pivotRow, bestRow);
            swapStep.matrixAfter = mat;
            steps.push_back(swapStep);
        }

        pivots.push_back({pivotRow, col});
        Fraction pivotVal = mat.at(pivotRow, col);

        // Step 3: Eliminate elements below pivot
        for (size_t r = pivotRow + 1; r < rows; ++r) {
            Fraction targetVal = mat.at(r, col);
            if (!targetVal.isZero()) {
                Fraction factor = targetVal / pivotVal;

                Step elimStep;
                elimStep.stepNumber = stepCounter++;
                elimStep.title = "Eliminate R" + std::to_string(r + 1) + " Entry";
                elimStep.operationType = "ELIMINATE";
                
                std::string factorStr = factor.toString();
                if (factor > Fraction(0)) {
                    elimStep.notation = "R_{" + std::to_string(r + 1) + "} \\to R_{" + std::to_string(r + 1) + "} - (" + factorStr + ") R_{" + std::to_string(pivotRow + 1) + "}";
                } else {
                    Fraction posFactor = -factor;
                    elimStep.notation = "R_{" + std::to_string(r + 1) + "} \\to R_{" + std::to_string(r + 1) + "} + (" + posFactor.toString() + ") R_{" + std::to_string(pivotRow + 1) + "}";
                }

                elimStep.reason = "Eliminate non-zero entry (" + targetVal.toString() + ") below pivot at Row " + 
                                  std::to_string(r + 1) + ", Column " + std::to_string(col + 1) + ".";
                elimStep.explanation = "Multiply pivot Row " + std::to_string(pivotRow + 1) + " by factor (" + factorStr + 
                                       ") and subtract from Row " + std::to_string(r + 1) + " to introduce a zero.";
                elimStep.matrixBefore = mat;
                elimStep.pivotRow = static_cast<int>(pivotRow);
                elimStep.pivotCol = static_cast<int>(col);
                elimStep.activeRow = static_cast<int>(r);
                elimStep.sourceRow = static_cast<int>(pivotRow);
                elimStep.eliminatedRow = static_cast<int>(r);
                elimStep.eliminatedCol = static_cast<int>(col);
                elimStep.scalarFactor = factor;

                mat.addRowMultiple(r, pivotRow, -factor);
                elimStep.matrixAfter = mat;
                steps.push_back(elimStep);
            }
        }

        pivotRow++;
    }
}

void GaussianEliminator::solveRREF(Matrix& mat, std::vector<Step>& steps, 
                                  std::vector<std::pair<size_t, size_t>>& pivots, int& stepCounter) {
    // Phase 1: Forward elimination to Row Echelon Form (REF)
    solveREF(mat, steps, pivots, stepCounter);

    // Phase 2: Backward pass (Gauss-Jordan)
    // For each pivot from bottom-right up to top-left:
    for (int p = static_cast<int>(pivots.size()) - 1; p >= 0; --p) {
        size_t pRow = pivots[p].first;
        size_t pCol = pivots[p].second;
        Fraction currentPivotVal = mat.at(pRow, pCol);

        // Normalize pivot to 1 if it isn't 1 already
        if (!currentPivotVal.isOne()) {
            Fraction scaleFactor = Fraction(1) / currentPivotVal;

            Step scaleStep;
            scaleStep.stepNumber = stepCounter++;
            scaleStep.title = "Normalize Pivot at R" + std::to_string(pRow + 1);
            scaleStep.operationType = "SCALE";
            scaleStep.notation = "R_{" + std::to_string(pRow + 1) + "} \\to (" + scaleFactor.toString() + ") R_{" + std::to_string(pRow + 1) + "}";
            scaleStep.reason = "Gauss-Jordan requirement: Each leading pivot entry must be normalized to 1.";
            scaleStep.explanation = "Multiply Row " + std::to_string(pRow + 1) + " by (" + scaleFactor.toString() + 
                                   ") so the pivot at (" + std::to_string(pRow + 1) + ", " + std::to_string(pCol + 1) + ") becomes 1.";
            scaleStep.matrixBefore = mat;
            scaleStep.pivotRow = static_cast<int>(pRow);
            scaleStep.pivotCol = static_cast<int>(pCol);
            scaleStep.activeRow = static_cast<int>(pRow);
            scaleStep.scalarFactor = scaleFactor;

            mat.scaleRow(pRow, scaleFactor);
            scaleStep.matrixAfter = mat;
            steps.push_back(scaleStep);
        }

        // Eliminate entries above this pivot
        for (int r = static_cast<int>(pRow) - 1; r >= 0; --r) {
            Fraction targetVal = mat.at(r, pCol);
            if (!targetVal.isZero()) {
                Fraction factor = targetVal; // Since pivot entry is already 1

                Step elimStep;
                elimStep.stepNumber = stepCounter++;
                elimStep.title = "Eliminate Above Pivot in R" + std::to_string(r + 1);
                elimStep.operationType = "ELIMINATE";

                if (factor > Fraction(0)) {
                    elimStep.notation = "R_{" + std::to_string(r + 1) + "} \\to R_{" + std::to_string(r + 1) + "} - (" + factor.toString() + ") R_{" + std::to_string(pRow + 1) + "}";
                } else {
                    Fraction pos = -factor;
                    elimStep.notation = "R_{" + std::to_string(r + 1) + "} \\to R_{" + std::to_string(r + 1) + "} + (" + pos.toString() + ") R_{" + std::to_string(pRow + 1) + "}";
                }

                elimStep.reason = "In RREF, a pivot must be the ONLY non-zero entry in its entire column.";
                elimStep.explanation = "Subtract (" + factor.toString() + ") times Row " + std::to_string(pRow + 1) + 
                                       " from Row " + std::to_string(r + 1) + " to eliminate entry at column " + std::to_string(pCol + 1) + ".";
                elimStep.matrixBefore = mat;
                elimStep.pivotRow = static_cast<int>(pRow);
                elimStep.pivotCol = static_cast<int>(pCol);
                elimStep.activeRow = r;
                elimStep.sourceRow = static_cast<int>(pRow);
                elimStep.eliminatedRow = r;
                elimStep.eliminatedCol = static_cast<int>(pCol);
                elimStep.scalarFactor = factor;

                mat.addRowMultiple(r, pRow, -factor);
                elimStep.matrixAfter = mat;
                steps.push_back(elimStep);
            }
        }
    }
}
