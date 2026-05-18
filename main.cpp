#include <iostream>
#include <vector>
#include <SDL2/SDL.h>

struct LineSegment {
    float x1, y1;
    float x2, y2;
    float pressure;
};

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *render_target = NULL;

static float pressure = 0.0f;
static float previous_touch_x = -1.0f;
static float previous_touch_y = -1.0f;

static std::vector<LineSegment> drawn_lines;

void recreate_render_target(int w, int h) {
    if (render_target) {
        SDL_DestroyTexture(render_target);
    }

    render_target = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        w, h
    );

    SDL_SetRenderTarget(renderer, render_target);
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, NULL);
}

int main(int argc, char *argv[])
{
    int w, h;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow(
        "SDL2 Pen Drawing Lines (with recording)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        640, 480,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);

    if (!renderer) {
        SDL_Log("Couldn't create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Go fullscreen
    //SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

    SDL_GetWindowSize(window, &w, &h);
    recreate_render_target(w, h);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    int running = 1;
    SDL_Event event;

    while (running) {

        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_QUIT) {
                running = 0;
            }

            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {

                int new_w = event.window.data1;
                int new_h = event.window.data2;
                recreate_render_target(new_w, new_h);
            }

            if (event.type == SDL_MOUSEMOTION) {
                if (pressure > 0.0f) {
                    if (previous_touch_x >= 0.0f) {
                        SDL_SetRenderTarget(renderer, render_target);
                        SDL_SetRenderDrawColor(renderer, 0, 0, 0, (Uint8)(pressure * 255));
                        SDL_RenderDrawLine(renderer,
                                           previous_touch_x, previous_touch_y,
                                           event.motion.x, event.motion.y);

                        drawn_lines.push_back({
                            previous_touch_x,
                            previous_touch_y,
                            (float)event.motion.x,
                            (float)event.motion.y,
                            pressure
                        });
                    }
                    previous_touch_x = event.motion.x;
                    previous_touch_y = event.motion.y;
                } else {
                    previous_touch_x = previous_touch_y = -1.0f;
                }
            }

            if (event.type == SDL_MOUSEBUTTONDOWN) {
                pressure = 1.0f;
                previous_touch_x = event.button.x;
                previous_touch_y = event.button.y;
            }

            if (event.type == SDL_MOUSEBUTTONUP) {
                pressure = 0.0f;
                previous_touch_x = previous_touch_y = -1.0f;
            }

            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_p) {
                std::cout << "Recorded line segments:\n";
                for (const auto &line : drawn_lines) {
                    std::cout << "Line: (" << line.x1 << ", " << line.y1
                              << ") -> (" << line.x2 << ", " << line.y2
                              << ") pressure=" << line.pressure << "\n";
                }
                SDL_RenderPresent(renderer);
                
            }
        }

        SDL_SetRenderTarget(renderer, NULL);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, render_target, NULL, NULL);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(render_target);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
