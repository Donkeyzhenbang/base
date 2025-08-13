TARGET = main

# 确保在链接时包含 FreeType 库
LDFLAGS := -lpthread

# 定义编译器
CXX = g++ -std=c++17 -g

# 使用 pkg-config 获取 OpenCV 编译和链接选项
CXXFLAGS = $(shell pkg-config --cflags opencv4)
LDFLAGS += $(shell pkg-config --libs opencv4)

# 定义源文件和目标文件
SRC = main.cpp

# 默认目标
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

# 清理生成的文件
clean:
	rm -f $(TARGET)

.PHONY: all clean
