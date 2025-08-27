clear
g++ -Ofast -march=native -flto -funroll-loops -fomit-frame-pointer -std=c++23 -o built.cppb main.cpp -lSDL2 -lSDL2_ttf -lpython3.11 -lm -lpthread -I/usr/include/python3.11
if [ $? -ne 0 ]; then
  exit 1
fi
./built.cppb
