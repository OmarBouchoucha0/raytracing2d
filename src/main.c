#include <math.h>
#include <raylib.h>
#include <stdio.h>

#define S 30.0f
#define GAME_WIDTH (16.0f * S)
#define GAME_HEIGHT (9.0f * S)
#define MAX_ENTITIES 1000
#define TILE_WIDTH 32
#define TILE_HEIGHT 32
#define PLAYER_ID 1
#define PLAYER_SPEED 100.0f
#define LIGHT_SPRITE_SIZE 128
#define LIGHT_DIAMETER 128.0f

typedef enum EntityType EntityType;
enum EntityType {
  PLAYER,
  BACKGROUND,
};

typedef struct Entity Entity;
struct Entity {
  Vector2 pos;
  Vector2 vecloity;
  float speed;
  Texture2D texture;
  EntityType type;
};

static int LAST_ENTITY_ID = PLAYER_ID;
static bool DEBUG = true;

float getScale(void) {
  float scale = floorf(fminf((float)GetScreenWidth() / GAME_WIDTH,
                             (float)GetScreenHeight() / GAME_HEIGHT));
  return scale < 1.0f ? 1.0f : scale;
}

Rectangle getScreenRect(void) {
  float scale = getScale();
  float w = GAME_WIDTH * scale;
  float h = GAME_HEIGHT * scale;
  return (Rectangle){floorf((GetScreenWidth() - w) / 2.0f),
                     floorf((GetScreenHeight() - h) / 2.0f), w, h};
}

Vector2 getMousePosition(void) {
  Vector2 mouse_pos = GetMousePosition();
  Rectangle screen = getScreenRect();
  float scale = getScale();
  mouse_pos.x = (mouse_pos.x - screen.x) / scale;
  mouse_pos.y = (mouse_pos.y - screen.y) / scale;
  return mouse_pos;
}

Texture2D makeLightSprite(int size, float power) {
  Image img = GenImageColor(size, size, BLANK);
  float center = (size - 1) / 2.0f;
  float radius = size / 2.0f;

  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      float dx = x - center;
      float dy = y - center;
      float d = sqrtf(dx * dx + dy * dy) / radius;
      float a = 0.0f;
      if (d < 1.0f)
        a = powf(1.0f - d, power);
      ImageDrawPixel(&img, x, y,
                     (Color){255, 255, 255, (unsigned char)(a * 255.0f)});
    }
  }

  Texture2D tex = LoadTextureFromImage(img);
  UnloadImage(img);
  return tex;
}

void addEntity(Entity *entities, Entity entity) {
  entities[LAST_ENTITY_ID] = entity;
  LAST_ENTITY_ID++;
}

void input(Entity *entities, float dt) {
  (void)dt;
  entities[PLAYER_ID].vecloity.x = 0;
  entities[PLAYER_ID].vecloity.y = 0;
  if (IsKeyDown(KEY_A)) {
    entities[PLAYER_ID].vecloity.x -= entities[PLAYER_ID].speed;
  }
  if (IsKeyDown(KEY_D)) {
    entities[PLAYER_ID].vecloity.x += entities[PLAYER_ID].speed;
  }
  if (IsKeyDown(KEY_W)) {
    entities[PLAYER_ID].vecloity.y -= entities[PLAYER_ID].speed;
  }
  if (IsKeyDown(KEY_S)) {
    entities[PLAYER_ID].vecloity.y += entities[PLAYER_ID].speed;
  }

  if (IsKeyPressed(KEY_F3)) {
    DEBUG = !DEBUG;
  }
}

void ai(Entity *entities) {}

void updatePos(Entity *entities, float dt) {
  for (int i = 1; i < LAST_ENTITY_ID; i++) {
    entities[i].pos.x += entities[i].vecloity.x * dt;
    entities[i].pos.y += entities[i].vecloity.y * dt;
  }
}

void physics(Entity *entities, float dt) { updatePos(entities, dt); }

void updateCamera(Camera2D *camera, Entity *entities) {
  camera->target.x = floorf(entities[PLAYER_ID].pos.x);
  camera->target.y = floorf(entities[PLAYER_ID].pos.y);
}

void drawGrid(void) {
  int cols = GAME_WIDTH / TILE_WIDTH;
  int rows = GAME_HEIGHT / TILE_HEIGHT;
  for (int i = 0; i < cols; i++) {
    DrawLine(i * TILE_WIDTH, 0, i * TILE_WIDTH, GAME_HEIGHT, RED);
  }
  for (int j = 0; j < rows; j++) {
    DrawLine(0, j * TILE_HEIGHT, GAME_WIDTH, j * TILE_HEIGHT, RED);
  }
  DrawRectangleLines(0, 0, GAME_WIDTH, GAME_HEIGHT, RED);
}

void drawDebugInfo(Entity *entities) {
  char buffer[100];
  int font_size = 8;
  snprintf(buffer, sizeof(buffer), "PLAYER POS : {%.2f|%.2f}",
           entities[PLAYER_ID].pos.x, entities[PLAYER_ID].pos.y);
  DrawText(buffer, 2, 2, font_size, WHITE);

  Vector2 mouse_pos = getMousePosition();
  snprintf(buffer, sizeof(buffer), "MOUSE POS : {%.2f|%.2f}", mouse_pos.x,
           mouse_pos.y);
  DrawText(buffer, 2, 12, font_size, WHITE);
}

void drawDebug(Entity *entities) {
  drawGrid();
  drawDebugInfo(entities);
}

void drawPlayer(Entity player) {
  Rectangle src = {0, 0, 96, 96};
  Vector2 pos = {floorf(player.pos.x), floorf(player.pos.y)};
  DrawTextureRec(player.texture, src, pos, WHITE);
}

void drawBackground(Entity background) {
  Rectangle src = {0, 0, 384, 216};
  Rectangle dst = {0, 0, GAME_WIDTH, GAME_HEIGHT};
  DrawTexturePro(background.texture, src, dst, (Vector2){0, 0}, 0.0f, WHITE);
}

void drawEntities(Entity *entities) {
  for (int i = 0; i < LAST_ENTITY_ID; i++) {
    if (entities[i].type == BACKGROUND) {
      drawBackground(entities[i]);
    }
  }
  drawPlayer(entities[PLAYER_ID]);
}

void drawLights(Texture2D light_sprite, Camera2D camera) {
  BeginBlendMode(BLEND_ADDITIVE);
  float d = LIGHT_DIAMETER;
  Vector2 mouse_world = GetScreenToWorld2D(getMousePosition(), camera);
  Rectangle src = {0, 0, (float)light_sprite.width, (float)light_sprite.height};
  Rectangle dst = {floorf(mouse_world.x), floorf(mouse_world.y), d, d};
  Vector2 origin = {d / 2.0f, d / 2.0f};
  DrawTexturePro(light_sprite, src, dst, origin, 0.0f, RED);

  d -= 30.0f;
  dst = (Rectangle){floorf(mouse_world.x), floorf(mouse_world.y), d, d};
  origin = (Vector2){d / 2.0f, d / 2.0f};
  DrawTexturePro(light_sprite, src, dst, origin, 0.0f, ORANGE);

  d -= 30.0f;
  dst = (Rectangle){floorf(mouse_world.x), floorf(mouse_world.y), d, d};
  origin = (Vector2){d / 2.0f, d / 2.0f};
  DrawTexturePro(light_sprite, src, dst, origin, 0.0f, YELLOW);

  EndBlendMode();
}

void renderer(Entity *entities, RenderTexture2D scene_texture,
              RenderTexture2D light_buffer, RenderTexture2D composite,
              Texture2D light_sprite, Camera2D camera) {
  BeginTextureMode(light_buffer);
  ClearBackground((Color){25, 25, 40, 255});
  BeginMode2D(camera);
  drawLights(light_sprite, camera);
  EndMode2D();
  EndTextureMode();

  BeginTextureMode(scene_texture);
  ClearBackground(BLACK);
  BeginMode2D(camera);
  drawEntities(entities);
  EndMode2D();
  EndTextureMode();

  Rectangle src = {0, 0, GAME_WIDTH, -GAME_HEIGHT};
  Rectangle full = {0, 0, GAME_WIDTH, GAME_HEIGHT};
  BeginTextureMode(composite);
  ClearBackground(BLACK);
  DrawTexturePro(scene_texture.texture, src, full, (Vector2){0, 0}, 0.0f,
                 WHITE);
  BeginBlendMode(BLEND_MULTIPLIED);
  DrawTexturePro(light_buffer.texture, src, full, (Vector2){0, 0}, 0.0f, WHITE);
  EndBlendMode();
  if (DEBUG) {
    drawDebug(entities);
  }
  EndTextureMode();

  BeginDrawing();
  ClearBackground(BLACK);
  DrawTexturePro(composite.texture, src, getScreenRect(), (Vector2){0, 0}, 0.0f,
                 WHITE);
  EndDrawing();
}

int main(void) {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(GAME_WIDTH, GAME_HEIGHT, "RAYTRACING");
  SetTargetFPS(60);
  SetMouseCursor(MOUSE_CURSOR_ARROW);

  RenderTexture2D scene_texture = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
  RenderTexture2D light_buffer = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
  RenderTexture2D composite = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
  SetTextureFilter(scene_texture.texture, TEXTURE_FILTER_POINT);
  SetTextureFilter(light_buffer.texture, TEXTURE_FILTER_POINT);
  SetTextureFilter(composite.texture, TEXTURE_FILTER_POINT);

  Entity entities[MAX_ENTITIES] = {0};

  Texture2D char_texture = LoadTexture(
      "/home/omar/Work/raytracing2D/assets/samurai/Sprites/IDLE.png");
  Texture2D background_texture =
      LoadTexture("./assets/background/background_glacial_mountains.png");

  // Image gradient = GenImageGradientRadial(LIGHT_SPRITE_SIZE,
  // LIGHT_SPRITE_SIZE,
  //                                         0.0f, WHITE, BLANK);
  // Texture2D light_sprite = LoadTextureFromImage(gradient);
  // UnloadImage(gradient);

  Texture2D light_sprite = makeLightSprite(LIGHT_SPRITE_SIZE, 2.0f);

  Entity player = {0};
  player.pos.x = GAME_WIDTH / 2.0f;
  player.pos.y = GAME_HEIGHT / 2.0f;
  player.speed = PLAYER_SPEED;
  player.texture = char_texture;
  player.type = PLAYER;
  addEntity(entities, player);

  Entity background = {0};
  background.texture = background_texture;
  background.type = BACKGROUND;
  addEntity(entities, background);

  Camera2D camera = {0};
  camera.target = player.pos;
  camera.offset =
      (Vector2){floorf(GAME_WIDTH / 2.0f), floorf(GAME_HEIGHT / 2.0f)};
  camera.zoom = 1.0f;

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();
    input(entities, dt);
    ai(entities);
    physics(entities, dt);
    updateCamera(&camera, entities);
    renderer(entities, scene_texture, light_buffer, composite, light_sprite,
             camera);
  }

  UnloadTexture(light_sprite);
  UnloadTexture(char_texture);
  UnloadTexture(background_texture);
  UnloadRenderTexture(scene_texture);
  UnloadRenderTexture(light_buffer);
  UnloadRenderTexture(composite);
  CloseWindow();
  return 0;
}
