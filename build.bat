@echo off
REM Run from "x64 Native Tools Command Prompt for VS 2022" (needs CMake + Ninja, both ship with VS Build Tools).
setlocal
if exist build rd /s /q build

cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release || goto :err
cmake --build build --target flatc || goto :err

for %%f in (build\_deps\supercell-texture-src\supercell-texture\sctx_schemas\*.fbs) do (
  build\_deps\flatbuffers-build\flatc.exe --cpp --cpp-std=c++17 -o build\_deps\supercell-texture-src\supercell-texture\source\texture\SCTX "%%f" || goto :err
)
for %%f in (build\_deps\supercellflash-src\supercell-flash\sc2_schemas\*.fbs) do (
  build\_deps\flatbuffers-build\flatc.exe --cpp --cpp-std=c++17 -o build\_deps\supercellflash-src\supercell-flash\source\flash\SC2 "%%f" || goto :err
)

cmake --build build || goto :err

echo.
echo Building done. Converting input\ -^> output\
build\sc1tosc2.exe input -o output || goto :err
echo.
echo Optional KTX variant:
build\sc1tosc2.exe input -o output_ktx --ktx
goto :eof

:err
echo.
echo BUILD FAILED - copy the text above and send it to me.
exit /b 1
