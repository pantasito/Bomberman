// ☕ Привет

#pragma once

#include <windows.h>

namespace Bomberman
{
    namespace Object {
        
        struct ConsoleCursorSetter {
            HANDLE consoleHandle;
            CONSOLE_CURSOR_INFO info;
            
            void Set() {
                consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
                info.bVisible = false;
                info.dwSize = 100;
                SetConsoleCursorInfo(consoleHandle, &info);
            }

            void x() {
                SetConsoleCursorPosition(_handle.consoleHandle, { 0, 0 });
            }
        };
    }
}