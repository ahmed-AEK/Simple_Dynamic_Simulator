#pragma once

#include <SDL2/SDL.h>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define alloc_stack _malloca
#else
#define alloc_stack alloca
#endif

inline void draw_circle(const SDL_Point& center, int radius, SDL_Renderer* renderer)
{
    const int& centreX = center.x;
    const int& centreY = center.y;
    const int32_t diameter = (radius * 2);

   int32_t x = (radius - 1);
   int32_t y = 0;
   int32_t dx = 1;
   int32_t dy = 1;
   int32_t error = (dx - diameter);

   while (x >= y)
   {
      //  Each of the following renders an octant of the circle
      SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
      SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
      SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
      SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
      SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
      SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
      SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
      SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);

      if (error <= 0)
      {
         ++y;
         error += dy;
         dy += 2;
      }

      if (error > 0)
      {
         --x;
         dx += 2;
         error += (dx - diameter);
      }
   }
}

inline void draw_circle2(const SDL_Point& center, int radius, SDL_Renderer* renderer)
{
    const int& centreX = center.x;
    const int& centreY = center.y;
    const int32_t diameter = (radius * 2);

   int32_t x = (radius - 1);
   int32_t y = 0;
   int32_t dx = 1;
   int32_t dy = 1;
   int32_t error = (dx - diameter);
   int num_points_to_draw = 0;
   while (x >= y)
   {

      num_points_to_draw += 8;
      if (error <= 0)
      {
         ++y;
         error += dy;
         dy += 2;
      }

      if (error > 0)
      {
         --x;
         dx += 2;
         error += (dx - diameter);
      }
   }
   x = (radius - 1);
   y = 0;
   dx = 1;
   dy = 1;
   error = (dx - diameter);
   void* vec_ptr = alloc_stack(num_points_to_draw * sizeof(SDL_Point));
   SDL_assert(vec_ptr);
   SDL_Point* vec = new(vec_ptr) SDL_Point[num_points_to_draw];
   int current_point_num = 0;
   while (x >= y)
   {
      vec[current_point_num]     = {centreX + x, centreY - y};
      vec[current_point_num + 1] = {centreX + x, centreY + y};
      vec[current_point_num + 2] = {centreX - x, centreY - y};
      vec[current_point_num + 3] = {centreX - x, centreY + y};
      vec[current_point_num + 4] = {centreX + y, centreY - x};
      vec[current_point_num + 5] = {centreX + y, centreY + x};
      vec[current_point_num + 6] = {centreX - y, centreY - x};
      vec[current_point_num + 7] = {centreX - y, centreY + x};
      current_point_num += 8;

      if (error <= 0)
      {
         ++y;
         error += dy;
         dy += 2;
      }

      if (error > 0)
      {
         --x;
         dx += 2;
         error += (dx - diameter);
      }
   }
    SDL_RenderDrawPoints(renderer, vec, num_points_to_draw);
}

inline void draw_circle_filled(const SDL_Point& center, int radius, SDL_Renderer* renderer)
{
    const int& centreX = center.x;
    const int& centreY = center.y;
    const int32_t diameter = (radius * 2);

    int32_t x = (radius - 1);
    int32_t y = 0;
    int32_t dx = 1;
    int32_t dy = 1;
    int32_t error = (dx - diameter);
    int num_points_to_draw = 0;
    while (x >= y)
    {

        num_points_to_draw += 8;
        if (error <= 0)
        {
            ++y;
            error += dy;
            dy += 2;
        }

        if (error > 0)
        {
            --x;
            dx += 2;
            error += (dx - diameter);
        }
    }
    x = (radius - 1);
    y = 0;
    dx = 1;
    dy = 1;
    error = (dx - diameter);
    void* vec_ptr = alloc_stack(num_points_to_draw * sizeof(SDL_Point));
    SDL_assert(vec_ptr);
    SDL_Point* vec = new(vec_ptr) SDL_Point[num_points_to_draw];
    int current_point_num = 0;
    while (x >= y)
    {
        vec[current_point_num]     = {centreX + x, centreY - y};
        vec[current_point_num + 1] = {centreX + x, centreY + y};
        vec[current_point_num + 2] = {centreX - x, centreY - y};
        vec[current_point_num + 3] = {centreX - x, centreY + y};
        vec[current_point_num + 4] = {centreX + y, centreY - x};
        vec[current_point_num + 5] = {centreX + y, centreY + x};
        vec[current_point_num + 6] = {centreX - y, centreY - x};
        vec[current_point_num + 7] = {centreX - y, centreY + x};
        current_point_num += 8;

        if (error <= 0)
        {
            ++y;
            error += dy;
            dy += 2;
        }

        if (error > 0)
        {
            --x;
            dx += 2;
            error += (dx - diameter);
        }
    }
    SDL_RenderDrawLines(renderer, vec, num_points_to_draw);
}

inline SDL_Texture* draw_circle_filled_tex(int radius, SDL_Color color, SDL_Renderer* renderer)
{
    const int32_t diameter = (radius * 2);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
    SDL_TEXTUREACCESS_TARGET, diameter, diameter);
    if (int fail = SDL_SetRenderTarget(renderer, texture))
    {
        SDL_Log("%d\n",fail);
        SDL_Log("%s",SDL_GetError());
    }
    SDL_SetRenderDrawColor(renderer,255,255,255,0);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer,color.r, color.g, color.b, color.a);
    const int& centreX = radius;
    const int& centreY = radius;

    int32_t x = (radius - 1);
    int32_t y = 0;
    int32_t dx = 1;
    int32_t dy = 1;
    int32_t error = (dx - diameter);
    int num_points_to_draw = 0;
    while (x >= y)
    {

        num_points_to_draw += 8;
        if (error <= 0)
        {
            ++y;
            error += dy;
            dy += 2;
        }

        if (error > 0)
        {
            --x;
            dx += 2;
            error += (dx - diameter);
        }
    }
    x = (radius - 1);
    y = 0;
    dx = 1;
    dy = 1;
    error = (dx - diameter);
    void* vec_ptr = alloc_stack(num_points_to_draw * sizeof(SDL_Point));
    SDL_assert(vec_ptr);
    SDL_Point* vec = new(vec_ptr) SDL_Point[num_points_to_draw];
    int current_point_num = 0;
    while (x >= y)
    {
        vec[current_point_num]     = {centreX + x, centreY - y};
        vec[current_point_num + 1] = {centreX + x, centreY + y};
        vec[current_point_num + 2] = {centreX - x, centreY - y};
        vec[current_point_num + 3] = {centreX - x, centreY + y};
        vec[current_point_num + 4] = {centreX + y, centreY - x};
        vec[current_point_num + 5] = {centreX + y, centreY + x};
        vec[current_point_num + 6] = {centreX - y, centreY - x};
        vec[current_point_num + 7] = {centreX - y, centreY + x};
        current_point_num += 8;

        if (error <= 0)
        {
            ++y;
            error += dy;
            dy += 2;
        }

        if (error > 0)
        {
            --x;
            dx += 2;
            error += (dx - diameter);
        }
    }
    SDL_RenderDrawLines(renderer, vec, num_points_to_draw);
    SDL_SetRenderTarget(renderer, NULL);
    return texture;
}
inline void draw_circle_filled2(const SDL_Point& center, int radius, SDL_Renderer* renderer)
{
    const int& centreX = center.x;
    const int& centreY = center.y;
    const int32_t diameter = (radius * 2);

    int32_t x = 0;
    int32_t y = 1 - radius;
    int32_t dx = 1;
    int32_t dy = 1;
    int32_t error = (dy - diameter);
    int num_points_to_draw = 0;
    while (y < 0)
    {

        num_points_to_draw += 4;
        if (error <= 0)
        {
            ++x;
            error += dx;
            dx += 2;
        }

        if (error > 0)
        {
            ++y;
            dy += 2;
            error += (dy - diameter);
        }
    }
    x = 0;
    y = 1 - radius;
    dx = 1;
    dy = 1;
    error = (dy - diameter);
    bool flip = false;
    void* vec_ptr = alloc_stack(num_points_to_draw * sizeof(SDL_Point));
    SDL_assert(vec_ptr);
    SDL_Point* vec = new(vec_ptr) SDL_Point[num_points_to_draw];
    int current_point_num = 0;
    while (y < 0)
    {
        if (flip)
        {
            vec[current_point_num] = {centreX - x, centreY + y};
            vec[current_point_num + 1] = {centreX + x, centreY + y};
            flip = ! flip;
        }
        else
        {
            vec[current_point_num] = {centreX + x, centreY + y};
            vec[current_point_num + 1] = {centreX - x, centreY + y};
            flip = ! flip;
        }
        current_point_num += 2;

        if (error <= 0)
        {
            ++x;
            error += dx;
            dx += 2;
        }

        if (error > 0)
        {
            ++y;
            dy += 2;
            error += (dy - diameter);
        }
    }

    x = radius;
    y = 0;
    dx = 1;
    dy = 1;
    error = (dx - diameter);
    while (x > 0)
    {
        if (flip)
        {
            vec[current_point_num] = {centreX - x, centreY + y};
            vec[current_point_num + 1] = {centreX + x, centreY + y};
            flip = ! flip;
        }
        else
        {
            vec[current_point_num] = {centreX + x, centreY + y};
            vec[current_point_num + 1] = {centreX - x, centreY + y};
            flip = ! flip;
        }
        current_point_num += 2;

        if (error <= 0)
        {
            ++y;
            error += dy;
            dy += 2;
        }

        if (error > 0)
        {
            --x;
            dx += 2;
            error += (dx - diameter);
        }
    }
    SDL_RenderDrawLines(renderer, vec, num_points_to_draw);
}


inline void draw_circle_filled3(const SDL_Point& center, int radius, SDL_Renderer* renderer)
{
    const int32_t diameter = (radius * 2);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
    SDL_TEXTUREACCESS_TARGET, diameter, diameter);
    if (int fail = SDL_SetRenderTarget(renderer, texture))
    {
        SDL_Log("%d\n",fail);
        SDL_Log("%s",SDL_GetError());
    }
    SDL_SetRenderDrawColor(renderer,255,255,255,0);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer,0,0,255,255);

    const int top = center.y - radius;
    const int left = center.x - radius;
    const int centreX = radius;
    const int centreY = radius;

    int32_t x = 0;
    int32_t y = 1 - radius;
    int32_t dx = 1;
    int32_t dy = 1;
    int32_t error = (dy - diameter);
    int num_points_to_draw = 0;
    while (y < 0)
    {

        num_points_to_draw += 4;
        if (error <= 0)
        {
            ++x;
            error += dx;
            dx += 2;
        }

        if (error > 0)
        {
            ++y;
            dy += 2;
            error += (dy - diameter);
        }
    }
    x = 0;
    y = 1 - radius;
    dx = 1;
    dy = 1;
    error = (dy - diameter);
    bool flip = false;
    void* vec_ptr = alloc_stack(num_points_to_draw * sizeof(SDL_Point));
    SDL_assert(vec_ptr);
    SDL_Point* vec = new(vec_ptr) SDL_Point[num_points_to_draw];
    int current_point_num = 0;
    while (y < 0)
    {
        if (flip)
        {
            vec[current_point_num] = {centreX - x, centreY + y};
            vec[current_point_num + 1] = {centreX + x, centreY + y};
            flip = ! flip;
        }
        else
        {
            vec[current_point_num] = {centreX + x, centreY + y};
            vec[current_point_num + 1] = {centreX - x, centreY + y};
            flip = ! flip;
        }
        current_point_num += 2;

        if (error <= 0)
        {
            ++x;
            error += dx;
            dx += 2;
        }

        if (error > 0)
        {
            ++y;
            dy += 2;
            error += (dy - diameter);
        }
    }

    x = radius;
    y = 0;
    dx = 1;
    dy = 1;
    error = (dx - diameter);
    while (x > 0)
    {
        if (flip)
        {
            vec[current_point_num] = {centreX - x, centreY + y};
            vec[current_point_num + 1] = {centreX + x, centreY + y};
            flip = ! flip;
        }
        else
        {
            vec[current_point_num] = {centreX + x, centreY + y};
            vec[current_point_num + 1] = {centreX - x, centreY + y};
            flip = ! flip;
        }
        current_point_num += 2;

        if (error <= 0)
        {
            ++y;
            error += dy;
            dy += 2;
        }

        if (error > 0)
        {
            --x;
            dx += 2;
            error += (dx - diameter);
        }
    }
    SDL_RenderDrawLines(renderer, vec, num_points_to_draw);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_Rect target_rect{left, top, diameter, diameter};
    SDL_RenderCopy(renderer, texture, NULL, &target_rect);
    SDL_DestroyTexture(texture);
}

inline void draw_curved_rectangle(const SDL_Rect& rect, const int radius, SDL_Renderer* renderer)
{

    if (radius*2 > rect.w || radius*2 > rect.h)
    {
        SDL_Log("radius too big.\n");
    }

    SDL_RenderDrawLine(renderer, rect.x+radius, rect.y, rect.x + rect.w - radius, rect.y);
    SDL_RenderDrawLine(renderer, rect.x, rect.y + radius, rect.x, rect.y + rect.h - radius);
    SDL_RenderDrawLine(renderer, rect.x+rect.w, rect.y + radius, rect.x + rect.w, rect.y + rect.h - radius);
    SDL_RenderDrawLine(renderer, rect.x+radius, rect.y + rect.h, rect.x + rect.w - radius, rect.y + rect.h);
    SDL_Point top_left = {rect.x + radius, rect.y + radius};
    SDL_Point top_right = {rect.x + rect.w - radius, rect.y + radius};
    SDL_Point bottom_left = {rect.x + radius, rect.y + rect.h - radius};
    SDL_Point bottom_right = {rect.x + rect.w - radius, rect.y + rect.h - radius};

    const int32_t diameter = (radius * 2);

    int32_t x = (radius - 1);
    int32_t y = 0;
    int32_t dx = 1;
    int32_t dy = 1;
    int32_t error = (dx - diameter);
    while (x >= y)
    {
        //  Each of the following renders an octant of the circle
        SDL_RenderDrawPoint(renderer, top_right.x + x, top_right.y - y); // 1
        SDL_RenderDrawPoint(renderer, bottom_right.x + x, bottom_right.y + y); // 8
        SDL_RenderDrawPoint(renderer, top_left.x - x, top_left.y - y); // 4
        SDL_RenderDrawPoint(renderer, bottom_left.x - x, bottom_left.y + y); // 5
        SDL_RenderDrawPoint(renderer, top_right.x + y, top_right.y - x); // 7
        SDL_RenderDrawPoint(renderer, bottom_right.x + y, bottom_right.y + x); // 2
        SDL_RenderDrawPoint(renderer, top_left.x - y, top_left.y - x); // 6
        SDL_RenderDrawPoint(renderer, bottom_left.x - y, bottom_left.y + x); // 3

        if (error <= 0)
        {
            ++y;
            error += dy;
            dy += 2;
        }

        if (error > 0)
        {
            --x;
            dx += 2;
            error += (dx - diameter);
        }
    }

}

inline void draw_curved_rectangle2(const SDL_Rect& rect, int radius, SDL_Renderer* renderer)
{

    radius = std::min(rect.w/2, radius);
    radius = std::min(rect.h/2, radius);
    

    SDL_RenderDrawLine(renderer, rect.x+radius, rect.y, rect.x + rect.w - radius, rect.y);
    SDL_RenderDrawLine(renderer, rect.x, rect.y + radius, rect.x, rect.y + rect.h - radius);
    SDL_RenderDrawLine(renderer, rect.x+rect.w, rect.y + radius, rect.x + rect.w, rect.y + rect.h - radius);
    SDL_RenderDrawLine(renderer, rect.x+radius, rect.y + rect.h, rect.x + rect.w - radius, rect.y + rect.h);
    SDL_Point top_left = {rect.x + radius, rect.y + radius};
    SDL_Point top_right = {rect.x + rect.w - radius, rect.y + radius};
    SDL_Point bottom_left = {rect.x + radius, rect.y + rect.h - radius};
    SDL_Point bottom_right = {rect.x + rect.w - radius, rect.y + rect.h - radius};

    const int32_t diameter = (radius * 2);

    int32_t x = (radius - 1);
    int32_t y = 0;
    int32_t dx = 1;
    int32_t dy = 1;
    int32_t error = (dx - diameter);
    int num_points_to_draw = 0;
    while (x >= y)
    {

        num_points_to_draw += 8;
        if (error <= 0)
        {
            ++y;
            error += dy;
            dy += 2;
        }

        if (error > 0)
        {
            --x;
            dx += 2;
            error += (dx - diameter);
        }
    }

    x = (radius - 1);
    y = 0;
    dx = 1;
    dy = 1;
    error = (dx - diameter);
    void* vec_ptr = alloc_stack(num_points_to_draw * sizeof(SDL_Point));
    SDL_assert(vec_ptr);
    SDL_Point* vec = new(vec_ptr) SDL_Point[num_points_to_draw];
       int current_point_num = 0;
    while (x >= y)
    {        
        //  Each of the following renders an octant of the circle
        vec[current_point_num]     = {top_right.x + x, top_right.y - y};
        vec[current_point_num + 1] = {bottom_right.x + x, bottom_right.y + y};
        vec[current_point_num + 2] = {top_left.x - x, top_left.y - y};
        vec[current_point_num + 3] = {bottom_left.x - x, bottom_left.y + y};
        vec[current_point_num + 4] = {top_right.x + y, top_right.y - x};
        vec[current_point_num + 5] = {bottom_right.x + y, bottom_right.y + x};
        vec[current_point_num + 6] = {top_left.x - y, top_left.y - x};
        vec[current_point_num + 7] = {bottom_left.x - y, bottom_left.y + x};
        current_point_num += 8;

        if (error <= 0)
        {
            ++y;
            error += dy;
            dy += 2;
        }

        if (error > 0)
        {
            --x;
            dx += 2;
            error += (dx - diameter);
        }
    }
    SDL_RenderDrawPoints(renderer, vec, num_points_to_draw);

}