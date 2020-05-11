@echo off

git clone https://github.com/Microsoft/vcpkg.git external\vcpkg\
cd external\vcpkg
call .\bootstrap-vcpkg.bat
call .\vcpkg integrate install
call .\vcpkg install nlohmann-json --triplet x86-windows
call .\vcpkg install nlohmann-json --triplet x64-windows
cd ..


echo "building ta-lib"
for /f "usebackq delims=#" %%a in (`"%programfiles(x86)%\Microsoft Visual Studio\Installer\vswhere" -latest -property installationPath`) do call "%%a\Common7\Tools\VsDevCmd.bat" 
for /f "usebackq delims=#" %%a in (`"%programfiles(x86)%\Microsoft Visual Studio\Installer\vswhere" -latest -property installationPath`) do call "%%a\VC\Auxiliary\Build\vcvarsall.bat" amd64_x86
call msbuild ta-lib\0.4.0\c\ide\vs2019\lib_proj\ta_lib.sln /t:clean,build,ta_libc /property:Configuration=cdd /property:Platform=x64
call msbuild ta-lib\0.4.0\c\ide\vs2019\lib_proj\ta_lib.sln /t:clean,build,ta_libc /property:Configuration=cdr /property:Platform=x64
call msbuild ta-lib\0.4.0\c\ide\vs2019\lib_proj\ta_lib.sln /t:clean,build,ta_libc /property:Configuration=cmd /property:Platform=x64
call msbuild ta-lib\0.4.0\c\ide\vs2019\lib_proj\ta_lib.sln /t:clean,build,ta_libc /property:Configuration=cmr /property:Platform=x64
call msbuild ta-lib\0.4.0\c\ide\vs2019\lib_proj\ta_lib.sln /t:clean,build,ta_libc /property:Configuration=cdd /property:Platform=win32
call msbuild ta-lib\0.4.0\c\ide\vs2019\lib_proj\ta_lib.sln /t:clean,build,ta_libc /property:Configuration=cdr /property:Platform=win32
call msbuild ta-lib\0.4.0\c\ide\vs2019\lib_proj\ta_lib.sln /t:clean,build,ta_libc /property:Configuration=cmd /property:Platform=win32
call msbuild ta-lib\0.4.0\c\ide\vs2019\lib_proj\ta_lib.sln /t:clean,build,ta_libc /property:Configuration=cmr /property:Platform=win32

cd .. 


rem call msbuild tradery.sln /t:clean,build,traderyconsole,tests /property:Configuration=debug /property:Platform=x64
rem call msbuild tradery.sln /t:clean,build,traderyconsole,tests /property:Configuration=release /property:Platform=x64
rem call msbuild tradery.sln /t:clean,build,traderyconsole,tests /property:Configuration=debug /property:Platform=win32
rem call msbuild tradery.sln /t:clean,build,traderyconsole,tests /property:Configuration=release /property:Platform=win32

