#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

int main(void) {
    SDL_Event event;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;
    SDL_Window *window = NULL;

    SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(
        500, 500,
        0, &window, &renderer
    );
    IMG_Init(IMG_INIT_PNG);
    texture = IMG_LoadTexture(renderer, "flower.png");
    while (1) {
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;
    }
    SDL_DestroyTexture(texture);
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
