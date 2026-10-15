#include "../include/Fraction.h"
#include "../include/Matrix.h"
#include "../include/GaussianEliminator.h"
#include "../include/SystemClassifier.h"
#include "../include/InputValidator.h"
#include <iostream>
#include <cassert>
#include <cmath>

static int totalTests = 0;
static int passedTests = 0;

#define TEST_ASSERT(cond, msg) do { \
    totalTests++; \
    if (cond) { \
        passedTests++; \
        std::cout << "  [PASS] " << msg << std::endl; \
    } else { \
        std::cerr << "  [FAIL] " << msg << " (line " << __LINE__ << ")" << std::endl; \
    } \
} while(0)

void testUniqueSolution() {
    std::cout << "\n--- Test 1: Unique Solution (3x3 system) ---" << std::endl;
    // System:
    //  2x +  y -  z =  8
    // -3x -  y + 2z = -11
    // -2x +  y + 2z = -3
    // Expected solution: x = 2, y = 3, z = -1
    std::vector<std::vector<Fraction>> data = {
        {Fraction(2),  Fraction(1),  Fraction(-1), Fraction(8)},
        {Fraction(-3), Fraction(-1), Fraction(2),  Fraction(-11)},
        {Fraction(-2), Fraction(1),  Fraction(2),  Fraction(-3)}
    };
    Matrix mat(3, 4, data);

    auto res = GaussianEliminator::solve(mat, SolveMode::RREF);
    TEST_ASSERT(res.finalMatrix.isAlreadyRREF(), "Final matrix satisfies RREF criteria");

    auto classification = SystemClassifier::classify(res.finalMatrix);
    TEST_ASSERT(classification.type == SystemType::UNIQUE_SOLUTION, "Classified as UNIQUE_SOLUTION");
    TEST_ASSERT(classification.uniqueSolutionValues.size() == 3, "Solution vector has 3 components");
    TEST_ASSERT(classification.uniqueSolutionValues[0] == Fraction(2), "x_1 == 2");
    TEST_ASSERT(classification.uniqueSolutionValues[1] == Fraction(3), "x_2 == 3");
    TEST_ASSERT(classification.uniqueSolutionValues[2] == Fraction(-1), "x_3 == -1");

    // Verify solution satisfies original system
    Fraction eq1 = Fraction(2)*classification.uniqueSolutionValues[0] + 
                   Fraction(1)*classification.uniqueSolutionValues[1] - 
                   Fraction(1)*classification.uniqueSolutionValues[2];
    TEST_ASSERT(eq1 == Fraction(8), "Equation 1 verified with solution values: 2(2) + 3 - (-1) = 8");
}

void testInfinitelyManySolutions() {
    std::cout << "\n--- Test 2: Infinitely Many Solutions (Underdetermined / Dependent) ---" << std::endl;
    // System:
    //  x + 2y - z = 4
    // 2x + 4y - 2z = 8
    // 3x + 6y - 3z = 12
    // Dependent rows -> 1 pivot, 2 free variables
    std::vector<std::vector<Fraction>> data = {
        {Fraction(1), Fraction(2), Fraction(-1), Fraction(4)},
        {Fraction(2), Fraction(4), Fraction(-2), Fraction(8)},
        {Fraction(3), Fraction(6), Fraction(-3), Fraction(12)}
    };
    Matrix mat(3, 4, data);

    auto res = GaussianEliminator::solve(mat, SolveMode::RREF);
    TEST_ASSERT(res.finalMatrix.isAlreadyRREF(), "Final matrix satisfies RREF");

    auto classification = SystemClassifier::classify(res.finalMatrix);
    TEST_ASSERT(classification.type == SystemType::INFINITELY_MANY, "Classified as INFINITELY_MANY");
    TEST_ASSERT(classification.numFreeVars == 2, "Identified exactly 2 free variables");
    TEST_ASSERT(classification.rankA == 1, "Rank of coefficient matrix is 1");
}

void testInconsistentSystem() {
    std::cout << "\n--- Test 3: Inconsistent System (Parallel planes / No Solution) ---" << std::endl;
    // System:
    //  x + y + z = 2
    //  x + y + z = 5
    // 2x + 2y + 2z = 9
    std::vector<std::vector<Fraction>> data = {
        {Fraction(1), Fraction(1), Fraction(1), Fraction(2)},
        {Fraction(1), Fraction(1), Fraction(1), Fraction(5)},
        {Fraction(2), Fraction(2), Fraction(2), Fraction(9)}
    };
    Matrix mat(3, 4, data);

    auto res = GaussianEliminator::solve(mat, SolveMode::RREF);
    auto classification = SystemClassifier::classify(res.finalMatrix);
    TEST_ASSERT(classification.type == SystemType::INCONSISTENT, "Classified as INCONSISTENT (No solution)");
    TEST_ASSERT(classification.rankA < classification.rankAb, "rank(A) < rank([A|b]) detected");
}

void testInitialPivotZeroRowSwap() {
    std::cout << "\n--- Test 4: Matrix Requiring Row Swap (Initial Pivot is Zero) ---" << std::endl;
    // System:
    //  0x + 2y = 4
    //  3x + 1y = 5
    std::vector<std::vector<Fraction>> data = {
        {Fraction(0), Fraction(2), Fraction(4)},
        {Fraction(3), Fraction(1), Fraction(5)}
    };
    Matrix mat(2, 3, data);

    auto res = GaussianEliminator::solve(mat, SolveMode::RREF);
    TEST_ASSERT(res.finalMatrix.isAlreadyRREF(), "Final matrix is in RREF");

    bool hasSwap = false;
    for (const auto& step : res.steps) {
        if (step.operationType == "SWAP") {
            hasSwap = true;
            break;
        }
    }
    TEST_ASSERT(hasSwap, "Recorded elementary SWAP step to replace zero pivot");

    auto classification = SystemClassifier::classify(res.finalMatrix);
    TEST_ASSERT(classification.type == SystemType::UNIQUE_SOLUTION, "Classified as UNIQUE_SOLUTION");
    TEST_ASSERT(classification.uniqueSolutionValues[0] == Fraction(1), "x_1 == 1");
    TEST_ASSERT(classification.uniqueSolutionValues[1] == Fraction(2), "x_2 == 2");
}

void testRectangularMatrices() {
    std::cout << "\n--- Test 5: Rectangular Matrix (2 equations, 3 variables) ---" << std::endl;
    // System:
    //  x + y + z = 6
    //  2x - y + z = 3
    std::vector<std::vector<Fraction>> data = {
        {Fraction(1),  Fraction(1),  Fraction(1), Fraction(6)},
        {Fraction(2), Fraction(-1), Fraction(1), Fraction(3)}
    };
    Matrix mat(2, 4, data);

    auto res = GaussianEliminator::solve(mat, SolveMode::RREF);
    TEST_ASSERT(res.finalMatrix.isAlreadyRREF(), "Final rectangular matrix is in RREF");

    auto classification = SystemClassifier::classify(res.finalMatrix);
    TEST_ASSERT(classification.type == SystemType::INFINITELY_MANY, "Rectangular underdetermined classified as INFINITELY_MANY");
    TEST_ASSERT(classification.numFreeVars == 1, "Exactly 1 free variable");
}

void testZeroRowHandling() {
    std::cout << "\n--- Test 6: Matrix with an Entirely Zero Row ---" << std::endl;
    // System:
    //  x + 2y = 3
    //  0x + 0y = 0
    //  2x + 4y = 6
    std::vector<std::vector<Fraction>> data = {
        {Fraction(1), Fraction(2), Fraction(3)},
        {Fraction(0), Fraction(0), Fraction(0)},
        {Fraction(2), Fraction(4), Fraction(6)}
    };
    Matrix mat(3, 3, data);

    auto res = GaussianEliminator::solve(mat, SolveMode::REF);
    TEST_ASSERT(res.finalMatrix.isAlreadyREF(), "Matrix with zero rows properly sorted to bottom in REF");
    TEST_ASSERT(res.finalMatrix.isRowAllZeros(1, true) || res.finalMatrix.isRowAllZeros(2, true), "Zero row placed at bottom");
}

void testAlreadyInREFOrRREF() {
    std::cout << "\n--- Test 7: Matrix Already in REF / RREF ---" << std::endl;
    // System already in RREF:
    //  1  0  2
    //  0  1  3
    std::vector<std::vector<Fraction>> data = {
        {Fraction(1), Fraction(0), Fraction(2)},
        {Fraction(0), Fraction(1), Fraction(3)}
    };
    Matrix mat(2, 3, data);

    auto res = GaussianEliminator::solve(mat, SolveMode::RREF);
    TEST_ASSERT(res.wasAlreadyInForm, "Identified that matrix is already in RREF");
    TEST_ASSERT(res.steps.size() <= 2, "Did not perform unnecessary row operations");
}

void testInputValidationAndDimensionLimits() {
    std::cout << "\n--- Test 8: Input Validation & Safe Rejection ---" << std::endl;
    std::string err;
    TEST_ASSERT(!InputValidator::validateDimensions(0, 3, err), "Rejected 0 rows");
    TEST_ASSERT(!InputValidator::validateDimensions(15, 3, err), "Rejected >10 rows");
    TEST_ASSERT(!InputValidator::validateDimensions(3, 0, err), "Rejected 0 variables");

    // Test fraction parsing
    Fraction f;
    TEST_ASSERT(Fraction::tryParse("3/4", f, err) && f == Fraction(3, 4), "Parsed fraction '3/4'");
    TEST_ASSERT(Fraction::tryParse("-5/2", f, err) && f == Fraction(-5, 2), "Parsed fraction '-5/2'");
    TEST_ASSERT(Fraction::tryParse("2.5", f, err) && f == Fraction(5, 2), "Parsed decimal '2.5' as 5/2");
    TEST_ASSERT(!Fraction::tryParse("5/0", f, err), "Rejected division by zero fraction '5/0'");
    TEST_ASSERT(!Fraction::tryParse("abc", f, err), "Rejected invalid string 'abc'");
}

int main() {
    std::cout << "=================================================" << std::endl;
    std::cout << "   MatrixFlow - C++ Mathematical Test Suite      " << std::endl;
    std::cout << "=================================================" << std::endl;

    testUniqueSolution();
    testInfinitelyManySolutions();
    testInconsistentSystem();
    testInitialPivotZeroRowSwap();
    testRectangularMatrices();
    testZeroRowHandling();
    testAlreadyInREFOrRREF();
    testInputValidationAndDimensionLimits();

    std::cout << "\n=================================================" << std::endl;
    std::cout << "Test Summary: " << passedTests << " / " << totalTests << " passed." << std::endl;
    std::cout << "=================================================" << std::endl;

    return (passedTests == totalTests) ? 0 : 1;
}
