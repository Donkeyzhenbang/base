# Toybox: C/C++ 初学者的第一个游戏 & 动画引擎

请参考 toybox.h 头部的注释和 hello.cpp 的例子。

C/C++ 都可以从以下模板开始，只需实现 “TODO” 中更新屏幕和响应按键逻辑 (可以不提供响应按键的 keypress) 即可：

```c
#include "toybox.h"

// toybox_run(fps, update, keypress)
// - 进入游戏/动画主循环
// - 每秒 fps 次调用 update(w, h, draw)
// - 当任何时候有按键时，调用 keypress(key)

void update(int w, int h, draw_function draw) {
    // 当前屏幕大小为 w x h (此时屏幕为空)
    // 可以使用 draw(x, y, ch) 可以在第 x 列第 y 行绘制字符 h

    // TODO
}

void keypress(int key) {
    // 获得一个按键，例如 W, A, S, D

    // TODO
}

int main() {
    toybox_run(20, update, keypress);
}
```

```c++
#include "toybox.h"

int main() {
    toybox_run(1, [](int w, int h, auto draw) {
        static int t = 0;
        t++;
        for (int x = 0; x < w; x++) {
            for (int y = 0; y < h; y++) {
                draw(x, y, '0' + t % 10);
            }
        }
    }, nullptr);
}
```
