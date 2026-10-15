# MatrixFlow — Interactive Gaussian Elimination Visualizer

> **A modern, educational application and C++ mathematical engine for exploring Gaussian and Gauss-Jordan Elimination step by step.**

Designed with a calm student aesthetic, MatrixFlow turns abstract matrix algebra into an interactive visual journey. It features:
- A warm pastel palette (`#e1dbd3`, `#86c6d4`, `#75889c`, `#baa49f`, `#f2d1c8`).
- Clean 3-tab layout: **Input**, **Solve**, and **Results**.
- The student anime character studying at her desk anchored at the bottom-right corner.
- Dynamic equation table with instant cell-level validation.
- Step-by-step Gaussian and Gauss-Jordan elimination with detailed mathematical rationales.

---

## 🌟 Live Demo (GitHub Pages)

You can run MatrixFlow directly in your browser without installing anything:
👉 **`https://<your-username>.github.io/MatrixFlow/`** *(Replace `<your-username>` with your GitHub username once published!)*

---

## 🚀 Key Features

- **Exact Rational Arithmetic (`Fraction`)**: Custom 64-bit rational number engine ($n/d$) with Euclidean GCD simplification. Keeps all numbers as clean textbook fractions (e.g., `2/3`, `-5/4`, `7`) with zero floating-point drift.
- **Elementary Row Operations (EROs)**:
  - Row Swap: $R_i \leftrightarrow R_j$
  - Row Scaling: $R_i \to k R_i$ ($k \ne 0$)
  - Row Addition: $R_i \to R_i + k R_j$
- **Step-by-Step Mathematical Transparency**: Every single row operation records:
  - Formula notation (e.g. $R_2 \to R_2 - \frac{2}{3} R_1$)
  - High-level mathematical reason
  - Detailed arithmetic explanation
  - Matrix snapshot before and after the operation
  - Visual coordinate tags for pivot, active row, eliminated cell, and source row.
- **REF & RREF Modes**:
  - **REF Mode**: Forward pass with partial pivoting (selects largest pivot for numerical stability, handles zero pivots, and skips missing pivot columns).
  - **RREF Mode**: Full Gauss-Jordan elimination (normalizes pivots to 1 and performs upward elimination to make each pivot the sole non-zero entry in its column).
- **System Classification & Parametric Solver**:
  - **Unique Solution**: Full rank verification ($r = m$), computes exact solution vector $x = [x_1, \dots, x_m]^T$.
  - **Infinitely Many Solutions**: Identifies $f = m - r$ free variables ($t_1, t_2, \dots$) and constructs explicit parametric vector equations (e.g. $x_1 = 4 - 2t_1 + t_2$).
  - **Inconsistent System**: Detects contradictory rows $[0\ 0\ \dots\ 0 \mid c]$ ($c \ne 0$) and explains why $\text{rank}(A) < \text{rank}([A \mid b])$.
- **Dual Runtime Support**:
  - **Desktop Native**: Powered by high-speed C++ executable (`matrixflow.exe`) with local Winsock2 server.
  - **Static Web / GitHub Pages**: Includes a pure client-side fallback in JavaScript with identical exact rational arithmetic, allowing instant hosting on GitHub Pages.

---

## 📁 Project Structure

```
MatrixFlow/
├── include/                   # C++ Header Files
│   ├── Fraction.h             # 64-bit exact rational arithmetic
│   ├── Matrix.h               # Augmented matrix [A | b] & EROs
│   ├── Step.h                 # Step data model with snapshots & highlight tags
│   ├── GaussianEliminator.h   # Step-by-step REF and RREF algorithms
│   ├── SystemClassifier.h     # Rank analysis & parametric solver
│   ├── InputValidator.h       # Dimension checks & cell validation
│   ├── JsonHelper.h           # Lightweight JSON serializer/parser
│   └── HttpServer.h           # Multi-threaded WinSock2 local server
├── src/                       # C++ Implementation Files
│   ├── Fraction.cpp           # Continued fractions, GCD, operators
│   ├── Matrix.cpp             # Row swaps, scale, linear combinations
│   ├── GaussianEliminator.cpp # Forward elimination & backward pass
│   ├── SystemClassifier.cpp   # Rouché-Capelli rank & free variables
│   ├── InputValidator.cpp     # Granular equation validation
│   ├── JsonHelper.cpp         # API payload parsing
│   ├── HttpServer.cpp         # Native Windows socket daemon & routing
│   └── main.cpp               # Desktop window launcher & lifecycle
├── tests/
│   └── test_main.cpp          # Automated C++ test suite (33 assertions)
├── web/                       # Application Frontend (HTML5 / CSS3 / ES6)
│   ├── index.html             # Semantic 3-tab layout
│   ├── style.css              # Soft pastel palette & responsive grid
│   ├── app.js                 # UI controller & dual C++/JS solver
│   ├── anime_girl.png         # Studying student illustration cutout
│   └── decor_bl.png           # Pastel wave corner element
├── docs/                      # Ready-to-deploy GitHub Pages folder
├── build.bat                  # One-click Windows build script
├── run.bat                    # One-click desktop launch script
├── test.bat                   # One-click test runner script
├── CMakeLists.txt             # Standard CMake build configuration
├── .gitignore                 # Clean repository filter
└── README.md                  # Project documentation
```

---

## 💻 How to Run Locally on Your Computer

### Prerequisites
- Windows OS
- MinGW GCC (`g++`) with C++14 support (or any standard C++14 compiler)

### Step 1: Run the Desktop Application
Double-click `run.bat` (or run in Command Prompt):
```cmd
run.bat
```
*This starts the C++ engine and automatically opens the MatrixFlow interface in your browser.*

### Step 2: Build from Source (Optional)
To recompile the binaries:
```cmd
build.bat
```
This generates:
- `matrixflow.exe` — The main visualizer application
- `test_runner.exe` — The automated C++ unit test runner

### Step 3: Run the Test Suite
```cmd
test.bat
```
*Validates 33 test cases verifying unique solutions, free variables, inconsistent systems, zero pivot swaps, rectangular systems, and input edge cases.*

---

## 🌐 How to Host on GitHub Pages (For Classmates)

To let your classmates access the app online without downloading anything:

1. Push this repository to GitHub.
2. In your GitHub repository, click **Settings** (top tab).
3. In the left sidebar, click **Pages**.
4. Under **Build and deployment** > **Branch**:
   - Select branch: **`main`**
   - Select folder: **`/docs`**
   - Click **Save**.
5. Wait 1-2 minutes. GitHub will provide your live URL:
   `https://<your-username>.github.io/MatrixFlow/`
6. Share this link with your classmates and instructor!

---

## 🎓 University Presentation Walkthrough

When presenting this project to your instructor:

1. **Architecture (Separation of Concerns)**:
   Explain that the mathematical core (`Fraction`, `Matrix`, `GaussianEliminator`, `SystemClassifier`) is implemented independently in C++, with zero floating-point rounding errors.
2. **Equation Input & Validation**:
   Show how dimension steppers dynamically adjust the table, accepting integers, decimals (`2.5`), and fractions (`3/4`). Point out that invalid entries highlight the exact cell without erasing the rest of the equations.
3. **Step-by-Step Visualization**:
   Demonstrate row swapping, pivot normalization, and row elimination. Point out the mathematical formulas ($R_i \to R_i - k R_j$), color highlights (cyan pivot, ice blue active row, soft peach eliminated cell), and the auto-play playback feature.
4. **Rank Analysis & Parametric Form**:
   Show how the solver classifies systems:
   - Full rank $\to$ Unique solution.
   - Dependent rows $\to$ Infinitely many solutions with explicit parametric forms ($x_1 = 4 - 2t_1 + t_2$).
   - Contradiction row ($0 = c$) $\to$ Inconsistent system.
5. **Automated Verification**:
   Run `test_runner.exe` in the console to demonstrate that all 33 unit tests pass.

---

## 📄 License
Open source for educational purposes. Developed for University Linear Algebra coursework.
