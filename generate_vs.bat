rem Use this batch file to generated Visual Studio project files
rmdir /s /q build
mkdir build
cd build
cmake ..
cd ..
start build/box2d.sln
