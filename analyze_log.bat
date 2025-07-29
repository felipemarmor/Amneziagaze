@echo off
echo AMNEZIAGAZE VST Log Analyzer
echo ============================
echo.

REM Check if Python is available
python --version >nul 2>&1
if errorlevel 1 (
    echo Error: Python is not installed or not in PATH
    echo Please install Python 3.x from https://python.org
    pause
    exit /b 1
)

REM Check if log file exists
if not exist "C:\temp\amneziagaze_realtime_log.txt" (
    echo Log file not found: C:\temp\amneziagaze_realtime_log.txt
    echo.
    echo To generate a log file:
    echo 1. Load the AMNEZIAGAZE plugin in your DAW
    echo 2. Play some audio through it
    echo 3. Run this analyzer again
    echo.
    pause
    exit /b 1
)

REM Install required packages if needed
echo Installing required Python packages...
pip install pandas matplotlib numpy >nul 2>&1

REM Run the analyzer
echo.
echo Running log analysis...
python analyze_vst_log.py --plots

echo.
echo Analysis complete! Check the vst_analysis_plots folder for visualizations.
pause