@echo off
if not exist test_runner.exe (
    echo test_runner.exe not found. Building first...
    call build.bat
)
echo Running C++ Mathematical Core Test Suite...
test_runner.exe
