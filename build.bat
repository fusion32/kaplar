@SETLOCAL

@SET CFLAGS=-W3 -WX -MTd -Zi -D_CRT_SECURE_NO_WARNINGS=1 -DARCH_X64=1 -DOS_WINDOWS=1 -DBUILD_DEBUG=1
@SET LFLAGS=-subsystem:console -incremental:no -opt:ref -dynamicbase
@SET LLIBS=
@SET SRC= "../src/main.cc"

del /q .\build\*
mkdir .\build
pushd .\build
cl %1 -Fe:"k.exe" %CFLAGS%  %SRC% /link %LFLAGS% %LLIBS%
popd

@ENDLOCAL
