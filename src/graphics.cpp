#include <iostream>
#include <string>
#include "graphics.hpp"

Graphics graphics_initialize()
{
    Graphics g;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Init Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    g.window = SDL_CreateWindow("WAVE Media Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (g.window == nullptr)
    {
        std::cerr << "Create Window ERROR: " << SDL_GetError() << std::endl;
        exit(1);
    }

    g.renderer = SDL_CreateRenderer(g.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (g.renderer == nullptr)
    {
        std::cerr << "Create Renderer ERROR: " << SDL_GetError() << std::endl;
        exit(1);
    }

    if (TTF_Init() < 0)
    {
        std::cerr << "Error Initializing TTF: " << TTF_GetError() << std::endl;
        exit(1);
    }

    g.large_font = TTF_OpenFont("../assets/Roboto-Regular.ttf", 24);
    if (!g.large_font)
    {
        std::cerr << "Error Loading Font: " << TTF_GetError() << std::endl;
        exit(1);
    }
    g.small_font = TTF_OpenFont("../assets/Roboto-Regular.ttf", 12);
    if (!g.small_font)
    {
        std::cerr << "Error Loading Font: " << TTF_GetError() << std::endl;
        exit(1);
    }

    SDL_ShowCursor(SDL_ENABLE);
    SDL_StartTextInput();

    return g;
}

void graphics_draw_rect(SDL_Rect* rect, SDL_Color* outline, SDL_Color* fill, SDL_Renderer* rend)
{
    if (fill != nullptr)
    {
        SDL_SetRenderDrawColor(rend, fill->r, fill->g, fill->b, fill->a); 
        SDL_RenderFillRect(rend, rect);
    }
    if (outline != nullptr)
    {
        SDL_SetRenderDrawColor(rend, outline->r, outline->g, outline->b, outline->a); 
        SDL_RenderDrawRect(rend, rect);
    }
}

void graphics_draw_triangle(Triangle* tri, SDL_Renderer* rend)
{
    SDL_RenderDrawLine(rend, tri->p1.x, tri->p1.y, tri->p2.x, tri->p2.y);
    SDL_RenderDrawLine(rend, tri->p2.x, tri->p2.y, tri->p3.x, tri->p3.y);
    SDL_RenderDrawLine(rend, tri->p3.x, tri->p3.y, tri->p1.x, tri->p1.y);
}

void graphics_draw_button(SDL_Rect* rect, const Position* mouse, 
                    SDL_Color outline, SDL_Color fill, SDL_Color hover_fill, 
                    SDL_Renderer* rend)
{
    if (graphics_on_button(rect, mouse))
    {
        SDL_SetRenderDrawColor(rend, hover_fill.r, hover_fill.g, hover_fill.b, fill.a); 
        SDL_RenderFillRect(rend, rect);
    }
    else
    {
        SDL_SetRenderDrawColor(rend, fill.r, fill.g, fill.b, fill.a); 
        SDL_RenderFillRect(rend, rect);
    }

    SDL_SetRenderDrawColor(rend, outline.r, outline.g, outline.b, outline.a); 
    SDL_RenderDrawRect(rend, rect);
}

bool graphics_on_button(const SDL_Rect* rect, const Position* mouse)
{
    if (mouse->x > rect->x && mouse->x < rect->x + rect->w)
    {
        if (mouse->y > rect->y && mouse->y < rect->y + rect->h) 
        { 
            return true; 
        }
    }
    return false;
}

SDL_Texture* graphics_create_texture(SDL_Rect* rect, std::string text, SDL_Color color, 
                                        TTF_Font* font, SDL_Renderer* rend)
{
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);

    if (surface != nullptr) 
    {
        rect->w = surface->w;
        rect->h = surface->h;
    }
    else 
    {
        rect->w = 0;
        rect->h = 0;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(rend, surface);
    SDL_FreeSurface(surface);

    return texture;
}
