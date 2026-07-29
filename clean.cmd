@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0clean.ps1" %*
exit /b %ERRORLEVEL%
