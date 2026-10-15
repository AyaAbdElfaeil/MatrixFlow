@echo off
if not exist matrixflow.exe (
    echo matrixflow.exe not found. Building first...
    call build.bat
)
echo Launching MatrixFlow Desktop Visualizer...
matrixflow.exe
