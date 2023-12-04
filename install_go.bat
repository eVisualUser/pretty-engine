@echo off
setlocal

set GOLANG_DOWNLOAD_URL=https://golang.org/dl/go1.21.4.windows-amd64.msi

echo Downloading from %GOLANG_DOWNLOAD_URL%
powershell -Command "Invoke-WebRequest %GOLANG_DOWNLOAD_URL% -OutFile goinstaller.msi"

echo Installing GoLang
start /wait msiexec /i goinstaller.msi /quiet /qn /norestart

:: Add Go to the system path because GoLang needs it.
setx path "%path%;C:\Go\bin"

echo Removing setup file...
del goinstaller.msi

endlocal
echo Msi installer finished, setup file removed, Go added to path.

pause