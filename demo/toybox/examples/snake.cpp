// Author: GPT-4-turbo

#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <string>
#include "../toybox.h"
using namespace std;

// 定义蛇的方向
enum Direction { UP, DOWN, LEFT, RIGHT };

// 蛇的初始方向
Direction dir = RIGHT;

// 蛇的身体，用一系列的 x,y 坐标表示
std::vector<std::pair<int, int>> snake = {{5, 5}, {5, 4}, {5, 3}};

// 食物的位置
std::pair<int, int> food = {7, 7};

// 游戏是否结束
bool gameOver = false;

// 生成食物
void generateFood(int w, int h) {
    srand(time(0));
    food.first = rand() % w;
    food.second = rand() % h;
}

void update();

// 渲染游戏
// void render(int w, int h, void(*draw)(int, int, char)) {
void render(int w, int h, draw_function draw) {
    update();
    if (gameOver) {
        return;
    }

    // 清屏
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            draw(x, y, ' ');
        }
    }

    // 绘制蛇
    for (auto &part : snake) {
        draw(part.first, part.second, '*');
    }

    // 绘制食物
    draw(food.first, food.second, '#');
}

// 处理按键
void keypress(int key) {
    switch (key) {
        case 'w': dir = UP; break;
        case 's': dir = DOWN; break;
        case 'a': dir = LEFT; break;
        case 'd': dir = RIGHT; break;
    }
}

// 更新游戏状态
void update() {
    if (gameOver) {
        return;
    }

    // 计算蛇头的新位置
    std::pair<int, int> head = snake.front();
    switch (dir) {
        case UP:    head.second--; break;
        case DOWN:  head.second++; break;
        case LEFT:  head.first--; break;
        case RIGHT: head.first++; break;
    }

    // 检查蛇是否撞墙或撞到自己
    if (head.first < 0 || head.second < 0 || head.first >= 80 || head.second >= 25 || std::find(snake.begin(), snake.end(), head) != snake.end()) {
        gameOver = true;
        return;
    }

    // 将新头部添加到蛇的身体中
    snake.insert(snake.begin(), head);

    // 检查是否吃到食物
    if (head == food) {
        generateFood(80, 25); // 假设屏幕大小为 80x25
    } else {
        // 移除蛇尾
        snake.pop_back();
    }
}

// 主函数
int main() {
    toybox_run(20, render, keypress); // 假设 toybox_run 函数接受一个更新游戏状态的函数作为参数
}
