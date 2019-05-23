// ☕ Привет

#pragma once

#include <windows.h>

namespace Bomberman
{
    namespace Object {
        
        struct ConsoleCursorSetter {
            HANDLE consoleHandle;
            CONSOLE_CURSOR_INFO info;
            
            ConsoleCursorSetter() {
                consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
                info.bVisible = false;
                info.dwSize = 100;
                SetConsoleCursorInfo(consoleHandle, &info);
            }

            void Set() {
                SetConsoleCursorPosition(consoleHandle, {0, 0});
            }
        };
    }
}