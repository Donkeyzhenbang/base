// Author: GPT-4-turbo

#include "../toybox.h"
#include <stdlib.h>
#include <time.h>

#define MAX_LETTERS 26

typedef struct {
    char ch; // 掉落的字母
    int x, y; // 字母的位置
    bool active; // 字母是否处于活动状态
} Letter;

Letter letters[MAX_LETTERS]; // 存储屏幕上的字母

void init_letters() {
    for (int i = 0; i < MAX_LETTERS; ++i) {
        letters[i].active = false;
    }
}

void spawn_letter(int w) {
    for (int i = 0; i < MAX_LETTERS; ++i) {
        if (!letters[i].active) {
            letters[i].ch = 'a' + rand() % 26; // 随机选择一个字母
            letters[i].x = rand() % w; // 随机选择一个横坐标
            letters[i].y = 0; // 从顶部开始掉落
            letters[i].active = true;
            break;
        }
    }
}

void update(int w, int h, draw_function draw) {
    static int tick = 0;
    if (tick++ % 10 == 0) { // 每隔一段时间生成一个新字母
        spawn_letter(w);
    }

    for (int i = 0; i < MAX_LETTERS; ++i) {
        if (letters[i].active) {
            draw(letters[i].x, letters[i].y, letters[i].ch); // 绘制字母
            letters[i].y += 1; // 字母向下移动
            if (letters[i].y >= h) { // 如果字母到达底部，重置
                letters[i].active = false;
            }
        }
    }
}

void keypress(int key) {
    for (int i = 0; i < MAX_LETTERS; ++i) {
        if (letters[i].active && letters[i].ch == key) {
            letters[i].active = false; // 按键匹配，移除字母
            break;
        }
    }
}

int main() {
    srand(time(NULL)); // 初始化随机数生成器
    init_letters(); // 初始化字母数组
    toybox_run(10, update, keypress); // 运行游戏
}
