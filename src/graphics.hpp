#ifdef __linux__
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_ttf.h>
#elif _WIN32
    #include <SDL.h>
    #include <SDL_ttf.h>
#endif

#define SCREEN_WIDTH (720)
#define SCREEN_HEIGHT (576)

typedef struct
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* large_font;
    TTF_Font* small_font;
} Graphics;

typedef struct 
{
    int x, y; 
} Position;

typedef struct
{
    Position p1;
    Position p2;
    int length;
} Line;

typedef struct 
{
    Position p1;
    Position p2;
    Position p3;
    int base;
    int height;
} Triangle;

Graphics graphics_initialize();

void graphics_draw_rect(SDL_Rect* rect, SDL_Color* outline, SDL_Color* fill, SDL_Renderer* rend);

void graphics_draw_triangle(Triangle* tri, SDL_Renderer* rend);

void graphics_draw_button(SDL_Rect* rect, const Position* mouse, 
                            SDL_Color outline, SDL_Color fill, SDL_Color hover_fill, 
                            SDL_Renderer* rend);

bool graphics_on_button(const SDL_Rect* rect, const Position* mouse);

SDL_Texture* graphics_create_texture(SDL_Rect* rect, std::string text, SDL_Color color, TTF_Font* font, 
                                    SDL_Renderer* rend);

bool graphics_blink();

