CC = clang++
src_dir = src
obj_dir = obj
header_dir = include
flags = -std=c++20 -D _CRT_SECURE_NO_WARNINGS -D _USE_MATH_DEFINES -Wall -Werror -Wextra -O2 -I $(header_dir)

headers = $(wildcard $(header_dir)/*.hpp)
sources = $(wildcard $(src_dir)/*.cpp)
objects = $(patsubst $(src_dir)/%.cpp,$(obj_dir)/%.o,$(sources))
product_name = rv3

$(product_name): obj $(objects)
	@echo Compiling of $(sources) finished
	@$(CC) $(flags) -o $(product_name) $(objects)

all: $(product_name)

obj: 
	@echo Directory obj created
	@mkdir obj

$(obj_dir)/%.o: $(src_dir)/%.cpp $(headers)
	@echo Compiling $@ from $<
	@$(CC) $(flags) -o $@ -c $<

clean:
	@rm -rf obj $(product_name)

run: all
	@./rv3 test_elf output.dump


.PHONY: clean build
