chcp 866
echo off
set /A COUNTER=0
set /A TESTOK=0
for /r %~dp0test_compiller %%i in (*.pas) do (
	echo %%i
	%~dp0pascal.exe -a %%i > %~dp0ASM.asm
	%~dp0NASM\nasm.exe -g -f win64 %~dp0ASM.asm -o %~dp0ASM.obj
	"C:\Program Files\mingw-w64\x86_64-4.9.3-win32-seh-rt_v4-rev1\mingw64\bin\gcc.exe" %~dp0ASM.obj -m64 -o %~dp0ASM.exe
	
	%~dp0ASM.exe > %~dp0ASM.res
	
	fc /a /w %~dp0ASM.res %%i.out
	if errorlevel 1 (
		echo [ERROR]
	) else (
		echo [OK]
		set /A TESTOK=TESTOK+1
	)
	set /A COUNTER=COUNTER+1
)
echo OK TESTS: %TESTOK% / %COUNTER%
