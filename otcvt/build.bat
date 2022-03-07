@SETLOCAL

@SET CFLAGS=-W3 -WX -MTd -Zi -D_CRT_SECURE_NO_WARNINGS=1
@SET LFLAGS=-subsystem:console -incremental:no -opt:ref -dynamicbase
@SET LLIBS=
@SET SRC="../common.cc" "../xml.cc" "../items_stats.cc"

pushd %~dp0
del /q .\build\*
mkdir .\build
pushd .\build
cl %1 -Fe:"o.exe" %CFLAGS%  %SRC% /link %LFLAGS% %LLIBS%
popd
popd

@ENDLOCAL
