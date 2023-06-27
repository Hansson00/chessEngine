
# Compiler
cc = /mnt/c/msys64/mingw64/bin/g++.exe


# Source map
src = ../src/

# Source files
file = $(src)*.cpp

# Includes
inc = -I
inc += ../inc/
# $(inc)

# Compile command
main:
	$(cc) $(file) -Wall -o main.out

debug:
	$(cc) $(file) -Wall -g -o main.out

clean:
	rm *.out



