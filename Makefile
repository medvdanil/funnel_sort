gcc = D:/QtSDK/mingw/bin/gcc

all:
        $(gcc) -W -Wall -Werror sort.c main.c -o sort.exe
test:
        ./sort
