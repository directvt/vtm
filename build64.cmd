@echo off
chcp 65001
if "%VisualStudioVersion%" == "16.0" (
    cmake ./src -DCMAKE_BUILD_TYPE=Release "-GVisual Studio 16 2019"
    cmake --build . --config Release
) else if "%VisualStudioVersion%" == "17.0" (
    cmake ./src -DCMAKE_BUILD_TYPE=Release "-GVisual Studio 17 2022"
    cmake --build . --config Release
) else (
    echo VisualStudioVersion %VisualStudioVersion%
    echo Make sure you are using `Developer Command Prompt`.
)
