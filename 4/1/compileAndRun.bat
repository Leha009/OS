g++ reader.cpp -o reader
g++ writer.cpp -o writer
g++ main.cpp
del *.log
del readers\*.log
del writers\*.log
pause
cls