// ☕ Привет

#pragma once

#include <windows.h>

namespace Bomberman
{
    namespace Helpers {
        
        class ConsoleCursorSetter {
            HANDLE consoleHandle;
            CONSOLE_CURSOR_INFO info;
            
            ConsoleCursorSetter() {
                consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
                info.bVisible = false;
                info.dwSize = 100;
                SetConsoleCursorInfo(consoleHandle, &info);
            }

        public:
            ConsoleCursorSetter(const ConsoleCursorSetter&) = delete;

            static ConsoleCursorSetter& Get()
            {
                static ConsoleCursorSetter object;
                return object;
            }

            void SetToStart() {
                SetConsoleCursorPosition(consoleHandle, {0, 0});
            }
        };
    }
}