@echo off
color 9f
title 文件权限操作选择
mode con cols=36 lines=10
if exist "%windir%\System32\choice.exe" goto a
cls
:c
echo      [1]添加everyone权限
echo      [2]解除everyone权限
echo      [3]退出
echo.
set /p d=请输入:
if %d%==1 goto 2
if %d%==2 goto 1
if %d%==x exit
clsnul&cls&goto c
:a
echo      [1]添加everyone权限
echo      [2]解除everyone权限
echo      [3]退出
echo.
choice /c 123 /m 请选择
if errorlevel 1 if not errorlevel 2 goto 2
if errorlevel 2 if not errorlevel 3 goto 1
if errorlevel 3 exit
clsnul&cls&goto a
:2
cls
title 添加权限
echo (可拖放文件或文件夹)
set /p p=输入：
echo Y|cacls %p% /T /C /p everyone:n>nul 2>nul
exit
:1
cls
title 解除权限
echo (可拖放文件或文件夹)
set /p p=输入：
echo Y|cacls %p% /T /C /P everyone:f>nul 2>nul
exit