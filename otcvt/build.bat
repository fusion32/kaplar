@SETLOCAL

@SET CFLAGS=%1 -W3 -WX -MTd -Zi -D_CRT_SECURE_NO_WARNINGS=1
@SET LFLAGS=-subsystem:console -incremental:no -opt:ref -dynamicbase

pushd %~dp0
del /q .\build\*
mkdir .\build
pushd .\build
cl -Fe:"is.exe" %CFLAGS% "../common.cc" "../xml.cc" "../items_stats.cc" /link %LFLAGS%
cl -Fe:"os.exe" %CFLAGS% "../common.cc" "../otb_stats.cc" /link %LFLAGS%
cl -Fe:"ol.exe" %CFLAGS% "../common.cc" "../otb_list.cc" /link %LFLAGS%
cl -Fe:"ml.exe" %CFLAGS% "../common.cc" "../otbm_list.cc" /link %LFLAGS%
popd
popd

@ENDLOCAL
