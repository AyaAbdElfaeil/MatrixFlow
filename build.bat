@echo off
echo ===============================================================
echo     MatrixFlow - Compilation Script (MinGW g++)
echo ===============================================================

echo Compiling C++ Mathematical Core and Desktop Server...
g++ -O2 -std=c++14 src/Fraction.cpp src/Matrix.cpp src/GaussianEliminator.cpp src/SystemClassifier.cpp src/InputValidator.cpp src/JsonHelper.cpp src/HttpServer.cpp src/main.cpp -o matrixflow.exe -lws2_32

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] matrixflow.exe compilation failed.
    exit /b %ERRORLEVEL%
)

echo Compiling C++ Mathematical Test Suite...
g++ -O2 -std=c++14 src/Fraction.cpp src/Matrix.cpp src/GaussianEliminator.cpp src/SystemClassifier.cpp src/InputValidator.cpp tests/test_main.cpp -o test_runner.exe

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] test_runner.exe compilation failed.
    exit /b %ERRORLEVEL%
)

echo.
echo [SUCCESS] Build complete!
echo Generated executables:
echo   - matrixflow.exe  (Main Interactive Visualizer)
echo   - test_runner.exe (Automated C++ Unit Test Suite)
echo.
echo Run "run.bat" to start the application.
echo Run "test.bat" to execute tests.
