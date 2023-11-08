# 编译器和编译选项
CXX = g++
CXXFLAGS = -std=c++11 -Wall
LDFLAGS =

# 源文件和目标文件
SRCS = main.cpp file1.cpp file2.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = my_program

# pixel.cpp 目标
PIXEL_SRC = pixel.cpp
PIXEL_OBJ = $(PIXEL_SRC:.cpp=.o)

# 默认目标（构建可执行文件）
all: $(TARGET)

pixel: $(PIXEL_OBJ) 
	$(CXX) $(LDFLAGS) -o $@ $(PIXEL_OBJ)

# 生成可执行文件
$(TARGET): $(OBJS) 
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) 
	
# 生成 pixel.cpp 的目标文件
$(PIXEL_OBJ): $(PIXEL_SRC)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 生成目标文件
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清除生成的文件
clean:
	rm -f $(OBJS) $(PIXEL_OBJ) $(TARGET)

.PHONY: all clean
