#include "utils.h"
#include <stdio.h>
#include "raylib.h"


bool DrawButton(Font font,Rectangle btn, const char* text) {
    DrawRectangleRec(btn, LIGHTGRAY);
    DrawRectangleLinesEx(btn, 2, DARKGRAY);
    Vector2 mouse = GetMousePosition();

    if (CheckCollisionPointRec(mouse, btn)) {
        DrawRectangleRec(btn, GRAY);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            return true;
        }
    }

    DrawTextEx(font, text, (Vector2) { btn.x + 10, btn.y + 10 }, 20, 2, (Color) { 0, 0, 0, 255 });
    return false;
}
