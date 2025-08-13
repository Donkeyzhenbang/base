// Author: GPT-4-turbo
// Prompt: 帮我填写模板 (README.md)，实现一个函数 f(x) 的绘制和导航。
//         给 f(x) = x, f(x) = sin(x), f(x) = sqrt(x)，
//         绘制出它们在 [-1,1] [-1,1] 上的图像。
//         w/a/s/d 按键可以移动坐标；z/x可以放大/缩小。 
//         不同的函数用不同的mark；绘制出横竖坐标轴。

#include "../toybox.h"
#include <math.h>
#include <stdio.h>

// 视图状态
double centerX = 0.0;  // 视图中心X
double centerY = 0.0;  // 视图中心Y
double scale = 10.0;   // 缩放级别，每个单位长度代表的像素点数

// 函数原型
void drawAxes(int w, int h, void (*draw)(int, int, char));
void drawFunction(int w, int h, void (*draw)(int, int, char), double (*func)(double), char mark);
double identityFunction(double x);
double sinFunction(double x);
double squareRootFunction(double x);

void update(int w, int h, void (*draw)(int, int, char)) {
    // 清屏
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            draw(x, y, ' ');
        }
    }

    // 绘制坐标轴
    drawAxes(w, h, draw);

    // 绘制函数
    drawFunction(w, h, draw, identityFunction, '*');
    drawFunction(w, h, draw, sinFunction, '.');
    drawFunction(w, h, draw, squareRootFunction, 'o');
}

void keypress(int key) {
    switch (key) {
        case 's': centerY -= 1.0 / scale; break;
        case 'w': centerY += 1.0 / scale; break;
        case 'a': centerX -= 1.0 / scale; break;
        case 'd': centerX += 1.0 / scale; break;
        case 'z': scale *= 1.1; break;  // 放大
        case 'x': scale *= 0.9; break;  // 缩小
    }
}

void drawAxes(int w, int h, void (*draw)(int, int, char)) {
    int originX = (int)(w / 2 + centerX * scale);
    int originY = (int)(h / 2 - centerY * scale);

    // 绘制X轴
    for (int x = 0; x < w; x++) {
        draw(x, originY, '-');
    }

    // 绘制Y轴
    for (int y = 0; y < h; y++) {
        draw(originX, y, '|');
    }

    // 绘制原点
    draw(originX, originY, '+');
}

void drawFunction(int w, int h, void (*draw)(int, int, char), double (*func)(double), char mark) {
    for (int x = 0; x < w; x++) {
        double worldX = (x - w / 2) / scale - centerX;
        double worldY = func(worldX);
        int screenY = h / 2 - (int)(worldY * scale + centerY * scale);

        if (screenY >= 0 && screenY < h) {
            draw(x, screenY, mark);
        }
    }
}


double identityFunction(double x) {
    return x;
}

double sinFunction(double x) {
    return sin(x);
}

double squareRootFunction(double x) {
    return sqrt(x);
}

int main() {
    toybox_run(20, update, keypress);
    return 0;
}
