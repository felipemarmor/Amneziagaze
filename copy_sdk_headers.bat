@echo off
echo ===================================================
echo Copying VST3 SDK Headers to Include Directory
echo ===================================================
echo.

REM Create directories in include folder
mkdir include\pluginterfaces\base
mkdir include\pluginterfaces\vst
mkdir include\pluginterfaces\gui
mkdir include\public.sdk\source\vst
mkdir include\public.sdk\source\vst\utility
mkdir include\public.sdk\source\main
mkdir include\public.sdk\source\common
mkdir include\base\source

REM Copy base interface headers
echo Copying base interface headers...
copy lib\vst3sdk\vst3sdk\pluginterfaces\base\*.h include\pluginterfaces\base\

REM Copy VST interface headers
echo Copying VST interface headers...
copy lib\vst3sdk\vst3sdk\pluginterfaces\vst\*.h include\pluginterfaces\vst\

REM Copy GUI interface headers
echo Copying GUI interface headers...
copy lib\vst3sdk\vst3sdk\pluginterfaces\gui\*.h include\pluginterfaces\gui\

REM Copy VST implementation headers
echo Copying VST implementation headers...
copy lib\vst3sdk\vst3sdk\public.sdk\source\vst\*.h include\public.sdk\source\vst\

REM Copy VST utility headers
echo Copying VST utility headers...
copy lib\vst3sdk\vst3sdk\public.sdk\source\vst\utility\*.h include\public.sdk\source\vst\utility\

REM Copy plugin factory headers
echo Copying plugin factory headers...
copy lib\vst3sdk\vst3sdk\public.sdk\source\main\*.h include\public.sdk\source\main\

REM Copy common headers
echo Copying common headers...
copy lib\vst3sdk\vst3sdk\public.sdk\source\common\*.h include\public.sdk\source\common\

REM Copy base implementation headers
echo Copying base implementation headers...
copy lib\vst3sdk\vst3sdk\base\source\*.h include\base\source\

echo.
echo VST3 SDK headers have been copied to the include directory.
echo.

pause