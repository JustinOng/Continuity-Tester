::cd to the directory that this bat file is in
::necessary because Run as Admin might reset cwd
cd /D "%~dp0"

mklink /D common "..\master_tester\common"
pause