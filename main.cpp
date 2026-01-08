#include <SDL2/SDL.h>
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <iostream>
#include <mm_malloc.h>
struct Position
{
    int x, y;
};
struct Dimensions
{
    int w, h;
};
inline bool intersectCircleY_int(int cx, int cy, int r, int Y, int& x1, int& x2)
{
    int dy = Y - cy;
    int d = r * r - dy * dy;
    if (d < 0)
        return false;
    int dx = static_cast<int>(std::floor(std::sqrt(d)));
    x1 = cx - dx;
    x2 = cx + dx;
    return true;
}
class Circle
{
  private:
    SDL_Renderer* m_renderer;
    Position m_position;
    int m_radius;
    Dimensions m_win;
    SDL_Texture* m_texture;

  public:
    explicit Circle(SDL_Renderer* renderer, Position position, int radius) noexcept
        : m_renderer(renderer), m_position(position), m_radius(radius)
    {
        this->updateWindowGeometry();
        m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888,
                                      SDL_TEXTUREACCESS_TARGET, 2 * radius, 2 * radius);
        SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND);
        SDL_SetRenderTarget(m_renderer, m_texture);
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
        SDL_RenderClear(m_renderer);
        SDL_SetRenderDrawColor(m_renderer, 0xFF, 0xFF, 0x11, 255);
        for (int y = 0; y < 2 * m_radius; y++)
        {
            int x1, x2;
            bool ok = intersectCircleY_int(m_radius, m_radius, m_radius, y, x1, x2);
            if (ok)
                SDL_RenderDrawLine(m_renderer, x1, y, x2, y);
        }
        SDL_SetRenderTarget(m_renderer, nullptr);
    }
    ~Circle()
    {
        SDL_DestroyTexture(m_texture);
    }
    void updateWindowGeometry()
    {
        SDL_GetRendererOutputSize(m_renderer, &m_win.w, &m_win.h);
    }
    [[nodiscard]]
    int getRadius() const noexcept
    {
        return m_radius;
    }
    [[nodiscard]]
    Position getPosition() const noexcept
    {
        return m_position;
    }
    void setX(int x) noexcept
    {
        if (x - m_radius < 0)
        {
            m_position.x = m_radius;
        }
        else if (m_win.w < x + m_radius)
        {
            m_position.x = m_win.w - m_radius;
        }
        else
        {
            m_position.x = x;
        }
    }
    void setY(int y) noexcept
    {
        if (y - m_radius < 0)
        {
            m_position.y = m_radius;
        }
        else if (y + m_radius > m_win.h)
        {
            m_position.y = m_win.h - m_radius;
        }
        else
        {
            m_position.y = y;
        }
    }
    void move(const Position& newPosition) noexcept
    {
        this->updateWindowGeometry();
        this->setX(newPosition.x);
        this->setY(newPosition.y);
    }
    void draw()
    {
        SDL_Rect dst = {m_position.x - m_radius, m_position.y - m_radius, 2 * m_radius,
                        2 * m_radius};
        SDL_RenderCopy(m_renderer, m_texture, nullptr, &dst);
    }
    void erase()
    {
        SDL_Rect eraseRect = {m_position.x - m_radius, m_position.y - m_radius, 2 * m_radius,
                              2 * m_radius};
        SDL_RenderDrawRect(m_renderer, &eraseRect);
    }
};
int main()
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "error" << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_Window* win =
        SDL_CreateWindow("Ray Tracing", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
                         SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    bool running = 1;
    Circle c(renderer, {40, 60}, 30);
    SDL_Event e;
    int step = 5;
    bool redraw = 1;
    while (running)
    {
        if (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = false;
            if (e.type == SDL_MOUSEMOTION && e.motion.state != 0)
            {
                Position pos;
                pos.x = e.motion.x;
                pos.y = e.motion.y;
                c.move(pos);
                redraw = 1;
            }
            if (e.type == SDL_KEYDOWN)
            {
                SDL_Keycode key = e.key.keysym.sym;
                c.erase();
                if (key == SDLK_w)
                {
                    Position pos = c.getPosition();
                    pos.y -= step;
                    c.move(pos);
                    redraw = 1;
                }
                if (key == SDLK_s)
                {
                    Position pos = c.getPosition();
                    pos.y += step;
                    c.move(pos);
                    redraw = 1;
                }
                if (key == SDLK_d)
                {
                    Position pos = c.getPosition();
                    pos.x += step;
                    c.move(pos);
                    redraw = 1;
                }
                if (key == SDLK_a)
                {
                    Position pos = c.getPosition();
                    pos.x -= step;
                    c.move(pos);
                    redraw = 1;
                }
                if (key == SDLK_q)
                    running = 0;
            }
        }
        if (redraw)
        {
            SDL_SetRenderDrawColor(renderer, 0x11, 0x11, 0x11, 255);
            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x11, 255);
            c.draw();
            SDL_RenderPresent(renderer);
        }
        redraw = 0;
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
