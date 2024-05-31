#include <stdint.h>
#include <stdio.h>

#include "SDL2/SDL.h"

typedef float f32;
typedef double f64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#define PI 3.14159

#define ASSERT(_e, ...)               \
    if (!(_e)) {                      \
        fprintf(stderr, __VA_ARGS__); \
        exit(1);                      \
    }

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 512

#define MAP_SIZE 8
#define gridSize 64
static u8 MAPDATA[MAP_SIZE * MAP_SIZE] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 1, 0, 0, 0, 0, 1,
    1, 0, 0, 4, 0, 0, 0, 1,
    1, 0, 3, 0, 0, 2, 0, 1,
    1, 0, 0, 0, 2, 2, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1};

struct {
    SDL_Window *window;
    SDL_Texture *texture;
    SDL_Renderer *renderer;
    u32 pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
    bool quit;
} state;

struct {
    float x;
    float y;
    float dx;
    float dy;
    float angle;

    int size;
    int sight;
} player;

void drawLine(int x0, int y0, int x1, int y1) {
    double x = x1 - x0;
    double y = y1 - y0;
    double length = sqrt(x * x + y * y);

    double addx = x / length;
    double addy = y / length;

    x = x0;
    y = y0;

    for (double i = 0; i < length; i += 1) {
        state.pixels[(int)y * SCREEN_WIDTH + (int)x] = 0xFFFF00FF;
        x += addx;
        y += addy;
    }
}

/**
 * Draws a block with specified color determined by two vertices:
 * x0,y0: Bottom left corner of the block,
 * width,height: width and height of the block.
 * color: The color of the block.
 */
void drawBlock(int x0, int y0, int width, int height, u32 color) {
    for (int j = 0; j < height - 1; j++) {
        for (int i = 0; i < width - 1; i++) {
            state.pixels[((int)y0 + j) * SCREEN_WIDTH + (int)x0 + i] = color;
        }
    }
}

void drawMap() {
    int x, y, x0, y0;
    u32 color;
    for (y = 0; y < MAP_SIZE; y++) {
        for (x = 0; x < MAP_SIZE; x++) {
            int print = y * MAP_SIZE + x;
            if (MAPDATA[y * MAP_SIZE + x] >= 1) {
                color = 0x005C4033;
            } else {
                color = 0xFFFFFFFF;
            }
            x0 = x * gridSize;
            y0 = y * gridSize;
            drawBlock(x0, y0, gridSize, gridSize, color);
        }
    }
}
void drawPlayer(int x0, int y0, int x1, int y1) {
    for (int j = y0; j < y1; j++) {
        for (int i = x0; i < x1; i++) {
            state.pixels[j * SCREEN_WIDTH + i] = 0xFFFF00FF;
        }
    }
    drawLine(player.x + player.size/2, player.y + player.size/2,
             player.x + player.size/2 + player.dx*10,
             player.dy * 10 + player.y + player.size/2);
}

int main(int argc, char *argv[]) {
    ASSERT(!SDL_Init(SDL_INIT_VIDEO), "SDL Failed to iniitalize: %s\n",
           SDL_GetError());

    state.window = SDL_CreateWindow("DEMO", SDL_WINDOWPOS_CENTERED_DISPLAY(1),
                                    SDL_WINDOWPOS_CENTERED_DISPLAY(1), SCREEN_WIDTH, SCREEN_HEIGHT,
                                    SDL_WINDOW_ALLOW_HIGHDPI);

    ASSERT(state.window, "Failed to create SDL Window: %s\n", SDL_GetError());

    state.renderer =
        SDL_CreateRenderer(state.window, -1, SDL_RENDERER_PRESENTVSYNC);
    ASSERT(state.renderer, "Failed to Create SDL Renderer: %s\n", SDL_GetError());
    state.texture = SDL_CreateTexture(state.renderer, SDL_PIXELFORMAT_ABGR8888,
                                      SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH,
                                      SCREEN_HEIGHT);
    ASSERT(state.texture, "Failed to Create SDL Renderer: %s\n", SDL_GetError());

    player = {000.0f, 000.0f, 0.0f, 0.0f, 0.0f, 10, 10};

    while (!state.quit) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                state.quit = true;
                break;
            }
        }

        const uint8_t *keystate = SDL_GetKeyboardState(NULL);

        if (keystate[SDL_SCANCODE_UP]) {
            player.x += player.dx;
            player.y += player.dy;
        }
        if (keystate[SDL_SCANCODE_DOWN]) {
            player.x -= player.dx;
            player.y -= player.dy;
        }
        if (keystate[SDL_SCANCODE_RIGHT]) {
            player.angle -= 0.1;
            if (player.angle < 0) {
                player.angle += 2 * PI;
            }
            player.dx = cos(player.angle) * 2;
            player.dy = sin(player.angle) * 2;
        }
        if (keystate[SDL_SCANCODE_LEFT]) {
            player.angle += 0.1;
            if (player.angle > 2 * PI) {
                player.angle -= 2 * PI;
            }
            player.dx = cos(player.angle) * 2;
            player.dy = sin(player.angle) * 2;
        }
        memset(state.pixels, 0, sizeof(state.pixels));
        // drawBlock(10,10,10,10,0xFFFFFFFF);
        drawMap();
        drawPlayer(player.x, player.y, player.x + player.size, player.y + player.size);

        SDL_UpdateTexture(state.texture, NULL, state.pixels, SCREEN_WIDTH * 4);
        SDL_RenderCopyEx(state.renderer, state.texture, NULL, NULL, 0.0, NULL,
                         SDL_FLIP_VERTICAL);
        SDL_RenderPresent(state.renderer);
    }

    SDL_DestroyTexture(state.texture);
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    return 0;
}