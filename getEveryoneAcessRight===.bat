@echo off
color 9f
title �ļ�Ȩ�޲���ѡ��
mode con cols=36 lines=10
if exist "%windir%\System32\choice.exe" goto a
cls
:c
echo      [1]���everyoneȨ��
echo      [2]���everyoneȨ��
echo      [3]�˳�
echo.
set /p d=������:
if %d%==1 goto 2
if %d%==2 goto 1
if %d%==x exit
clsnul&cls&goto c
:a
echo      [1]���everyoneȨ��
echo      [2]���everyoneȨ��
echo      [3]�˳�
echo.
choice /c 123 /m ��ѡ��
if errorlevel 1 if not errorlevel 2 goto 2
if errorlevel 2 if not errorlevel 3 goto 1
if errorlevel 3 exit
clsnul&cls&goto a
:2
cls
title ���Ȩ��
echo (���Ϸ��ļ����ļ���)
set /p p=���룺
echo Y|cacls %p% /T /C /p everyone:n>nul 2>nul
exit
:1
cls
title ���Ȩ��
echo (���Ϸ��ļ����ļ���)
set /p p=���룺
echo Y|cacls %p% /T /C /P everyone:f>nul 2>nul
exit