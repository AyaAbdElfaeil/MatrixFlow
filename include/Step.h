#ifndef STEP_H
#define STEP_H

#include "Matrix.h"
#include <string>

/**
 * @struct Step
 * @brief Records a single discrete step in the Gaussian Elimination process.
 * 
 * Captures the matrix before and after, the exact row operation notation,
 * the pedagogical rationale, and the specific coordinates for visual highlighting.
 */
struct Step {
    int stepNumber;
    std::string title;
    std::string operationType; // "INITIAL", "SWAP", "SCALE", "ELIMINATE", "COMPLETE"
    std::string notation;      // e.g. "R_2 <-> R_1", "R_2 -> R_2 - 3 R_1", "R_1 -> (1/2) R_1"
    std::string reason;        // High-level mathematical purpose
    std::string explanation;   // Detailed arithmetic and educational commentary
    
    Matrix matrixBefore;
    Matrix matrixAfter;

    // Visual highlighting coordinates (-1 if not applicable)
    int pivotRow;
    int pivotCol;
    int activeRow;
    int sourceRow;
    int eliminatedRow;
    int eliminatedCol;
    Fraction scalarFactor;

    Step()
        : stepNumber(0), title("Initial Matrix"), operationType("INITIAL"),
          notation("None"), reason("Starting augmented matrix [A | b]"),
          explanation("Original linear system represented as an augmented matrix."),
          pivotRow(-1), pivotCol(-1), activeRow(-1), sourceRow(-1),
          eliminatedRow(-1), eliminatedCol(-1), scalarFactor(1) {}

    std::string toJson() const {
        std::ostringstream oss;
        oss << "{"
            << "\"stepNumber\":" << stepNumber << ","
            << "\"title\":\"" << escapeJson(title) << "\","
            << "\"operationType\":\"" << escapeJson(operationType) << "\","
            << "\"notation\":\"" << escapeJson(notation) << "\","
            << "\"reason\":\"" << escapeJson(reason) << "\","
            << "\"explanation\":\"" << escapeJson(explanation) << "\","
            << "\"pivotRow\":" << pivotRow << ","
            << "\"pivotCol\":" << pivotCol << ","
            << "\"activeRow\":" << activeRow << ","
            << "\"sourceRow\":" << sourceRow << ","
            << "\"eliminatedRow\":" << eliminatedRow << ","
            << "\"eliminatedCol\":" << eliminatedCol << ","
            << "\"scalarFactor\":\"" << scalarFactor.toString() << "\","
            << "\"matrixBefore\":" << matrixBefore.toJson() << ","
            << "\"matrixAfter\":" << matrixAfter.toJson()
            << "}";
        return oss.str();
    }

private:
    static std::string escapeJson(const std::string& s) {
        std::ostringstream o;
        for (char c : s) {
            if (c == '"') o << "\\\"";
            else if (c == '\\') o << "\\\\";
            else if (c == '\b') o << "\\b";
            else if (c == '\f') o << "\\f";
            else if (c == '\n') o << "\\n";
            else if (c == '\r') o << "\\r";
            else if (c == '\t') o << "\\t";
            else if (c >= 0 && c <= 0x1f) {
                // Control character
                o << " ";
            } else {
                o << c;
            }
        }
        return o.str();
    }
};

#endif // STEP_H
