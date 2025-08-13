TARGET = main
CXXFLAGS := -w -I./inc 
# 确保在链接时包含 FreeType 库
LDFLAGS := 
# 指定链接器在搜索库文件时应考虑的额外目录
# LDFLAGS += -L
# 运行时搜索动态链接库
# LDFLAGS += -Wl,-rpath,./lib

# 定义编译器
CXX = g++ -std=c++17 -g

CXXFLAGS += $(shell pkg-config --cflags opencv4)
LDFLAGS += $(shell pkg-config --libs opencv4)


# 定义源文件和目标文件
SRC = main.cpp

# 添加其他目录中的源文件
SRC += $(wildcard src/*.cpp)

# 默认目标
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

# 清理生成的文件
clean:
	rm -f $(TARGET)

.PHONY: all clean
