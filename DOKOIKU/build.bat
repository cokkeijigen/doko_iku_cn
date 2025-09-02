set "OUT_DIR=.\\_output"
if not exist "%OUT_DIR%" ( mkdir %OUT_DIR% )
cd %OUT_DIR%
cmake -A Win32 -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release