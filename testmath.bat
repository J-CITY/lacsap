chcp 866
echo off
set /A COUNTER=0
set /A TESTOK=0
for /r %~dp0testmath %%i in (*.in) do (
	echo %%i
	%~dp0pascal.exe -p %%i > %~dp0test.test
	fc /a /w %%i.out %~dp0test.test > nul
	if errorlevel 1 (
		echo [ERROR]
	) else (
		echo [OK]
		set /A TESTOK=TESTOK+1
	)
	set /A COUNTER=COUNTER+1
)
echo OK TESTS: %TESTOK% / %COUNTER%
