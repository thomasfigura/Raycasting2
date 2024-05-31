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

#define PI 3.1415926535
#define P2 PI / 2
#define P3 3 * PI / 2
#define DR 0.0174533 // 1 Degree in radians

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
    1, 0, 0, 1, 0, 0, 0, 1,
    1, 0, 1, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 1, 1, 0, 1,
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

void drawLine(int x0, int y0, int x1, int y1, u32 color) {
    if (x1 >= 0 && x1 < SCREEN_WIDTH && x0 >= 0 && x0 < SCREEN_WIDTH && y1 >= 0 && y1 < SCREEN_HEIGHT && y0 >= 0 && y0 < SCREEN_HEIGHT) {

        double x = x1 - x0;
        double y = y1 - y0;
        double length = sqrt(x * x + y * y);

        double addx = x / length;
        double addy = y / length;

        x = x0;
        y = y0;

        for (double i = 0; i < length; i += 1) {
            if ((int)y * SCREEN_WIDTH + (int)x < SCREEN_HEIGHT * SCREEN_WIDTH) {
                state.pixels[(int)y * SCREEN_WIDTH + (int)x] = color;
                x += addx;
                y += addy;
            }
        }
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
            if (((int)y0 + j) * SCREEN_WIDTH + (int)x0 + i) {
                state.pixels[((int)y0 + j) * SCREEN_WIDTH + (int)x0 + i] = color;
            }
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

float dist(float ax, float ay, float bx, float by, float ang) {
    return (sqrt(((bx - ax) * (bx - ax)) + ((by - ay) * (by - ay))));
}

void drawRays3D() {
    int r, mx, my, mp, dof;
    float rx, ry, ra, xo, yo, distT;
    ra = player.angle - DR * 30;
    if (ra < 0) {
        ra += 2 * PI;
    }
    if (ra > 2 * PI) {
        ra -= 2 * PI;
    }
    for (r = 0; r < 60; r++) { // Number of rays casted
        // Checking for horizontal Lines
        dof = 0;
        float distH = 100000, hx = player.x, hy = player.y;
        float aTan = -1 / tan(ra);
        if (ra > PI) { // looking up or down // Looking Up
            ry = (((int)player.y >> 6) << 6) - 0.0001;
            rx = (player.y - ry) * aTan + player.x;
            yo = -64;
            xo = -yo * aTan;
        }
        if (ra < PI) { // looking up or down // Looking Down
            ry = (((int)player.y >> 6) << 6) + 64;
            rx = (player.y - ry) * aTan + player.x;
            yo = 64;
            xo = -yo * aTan;
        }
        if (ra == 0 || ra == PI) { // Looking left or right
            rx = player.x;
            ry = player.y;
            dof = 8;
        }
        while (dof < 8) {
            mx = (int)(rx) >> 6;
            my = (int)(ry) >> 6;
            mp = my * MAP_SIZE + mx;
            if (mp > 0 && mp < MAP_SIZE * MAP_SIZE && MAPDATA[mp] >= 1) { // Wall hit
                hx = rx;
                hy = ry;
                distH = dist(player.x, player.y, hx, hy, ra);
                dof = 8;
            } else {
                rx += xo;
                ry += yo;
                dof++;
            }
        }

        // Check Verticle Wall
        dof = 0;
        float distV = 1000000, vx = player.x, vy = player.y;
        float nTan = -tan(ra);
        if (ra > P2 && ra < P3) {
            rx = (((int)player.x >> 6) << 6) - 0.0001;
            ry = (player.x - rx) * nTan + player.y;
            xo = -64;
            yo = -xo * nTan;
        }
        if (ra < P2 || ra > P3) {
            rx = (((int)player.x >> 6) << 6) + 64;
            ry = (player.x - rx) * nTan + player.y;
            xo = 64;
            yo = -xo * nTan;
        }
        if (ra == 0 || ra == PI) {
            rx = player.x;
            ry = player.y;
            dof = 8;
        }

        while (dof < 8) {
            mx = (int)(rx) >> 6;
            my = (int)(ry) >> 6;
            mp = my * MAP_SIZE + mx;
            if (mp > 0 && mp < MAP_SIZE * MAP_SIZE && MAPDATA[mp] >= 1) { // Wall hit
                vx = rx;
                vy = ry;
                distV = dist(player.x, player.y, vx, vy, ra);
                dof = 8;
            } else {
                rx += xo;
                ry += yo;
                dof++;
            }
        }
        if (distV < distH) {
            rx = vx;
            ry = vy;
            distT = distV;
        }
        if (distH > distV) {
            rx = hx;
            ry = hy;
            distT = distH;
        }

        //        printf("distV: %f, distH: %f\n", distV, distH);

        drawLine(player.x,
                 player.y,
                 rx, ry, 0x32cd32);
        ra += DR;
        if (ra < 0) {
            ra += 2 * PI;
        }
        if (ra > 2 * PI) {
            ra -= 2 * PI;
        }

        // draw 3d walls

        float lineH = (gridSize * 320) / distT;
        if (lineH > 320) {
            lineH = 320;
        }
        for (int j = 0; j < r; j++) {
            for (int i = 0; i < 8; i++) {
                drawLine(j * i + 320, 0, j * i * 320, lineH, 0x800020);
            }
        }
    }
}

void drawPlayer(int x0, int y0, int x1, int y1) {
    for (int j = y0; j < y1; j++) {
        for (int i = x0; i < x1; i++) {
            state.pixels[j * SCREEN_WIDTH + i] = 0xFFFF00FF;
        }
    }
    drawLine(player.x, player.y,
             player.x + player.dx * 10,
             player.dy * 10 + player.y, 0xFFFF00FF);
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

    player = {300.0f, 300.0f, 0.0f, 0.0f, 0.0f, 10, 10};

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
        if (keystate[SDL_SCANCODE_LEFT]) {
            player.angle -= 0.1;
            if (player.angle < 0) {
                player.angle += 2 * PI;
            }
            player.dx = cos(player.angle) * 5;
            player.dy = sin(player.angle) * 5;
        }
        if (keystate[SDL_SCANCODE_RIGHT]) {
            player.angle += 0.1;
            if (player.angle > 2 * PI) {
                player.angle -= 2 * PI;
            }
            player.dx = cos(player.angle) * 5;
            player.dy = sin(player.angle) * 5;
        }
        memset(state.pixels, 0, sizeof(state.pixels));
        drawBlock(10, 10, 10, 10, 0xFFFFFFFF);
        // drawMap();
        // drawPlayer(player.x, player.y, player.x + player.size, player.y + player.size);
        // drawRays3D();
        drawLine(10, 10, 300, 250, 0xFFFFFFFF);
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