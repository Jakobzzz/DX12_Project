#pragma once
#define SCREEN_WIDTH 800U
#define SCREEN_HEIGHT 600U
#define FRAME_BUFFERS 2U
#define SAFE_RELEASE(arg) if(*arg != nullptr) { (*arg)->Release(); (*arg) = nullptr; } 