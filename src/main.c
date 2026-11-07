#include <raylib.h>

int main() {
  InitWindow(800, 450, "RAYTRACING");

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RED);
    EndDrawing();
  }
  CloseWindow();
  return 0;
}
