// compile : gcc -g -o asg2 asg2.c `pkg-config --cflags sdl2` -I"C:/msys64/home/alber/box2d/include" "C:/msys64/home/alber/box2d/build/src/libbox2d.a" `pkg-config --libs sdl2`
#include <memory.h>
#include <SDL2/SDL.h>
#include <box2d/box2d.h>
#include <box2d/collision.h>
#include <box2d/math_functions.h>
#include <math.h>
#include <stdio.h>

#define RECT_W 20
#define RECT_H 20
#define MAX_RECTS 20
#define PIXELS_PER_METER 30.0f
#define METERS_PER_PIXEL (1.0f / PIXELS_PER_METER)
#define CANVAS_HEIGHT 480

// Converts Box2D world coordinates to SDL canvas coordinates
b2Vec2 world_to_canvas(b2Vec2 world) // world coords. are in box2d units
{
    b2Vec2 cc = {0., 0.};
    // Convert Box2D x-coordinate meters to pixels same for y
    cc.x = round(PIXELS_PER_METER * world.x);
    cc.y = CANVAS_HEIGHT - round(PIXELS_PER_METER * world.y);

    return cc;
}

int main(int argc, char **argv)
{
    int quit = 0;
    SDL_Event event;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("SDL + Box2D Example",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_SetRenderDrawColor(renderer, 22, 255, 255, 255);
    SDL_RenderClear(renderer);

    // Create Box2D world
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = (b2Vec2){0.0f, -1.0f};
    b2WorldId worldId = b2CreateWorld(&worldDef);

    // Create ground body
    b2BodyDef groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.position = (b2Vec2){0.0f, (50.0f * METERS_PER_PIXEL) - 10.0f};
    b2BodyId groundId = b2CreateBody(worldId, &groundBodyDef);
    b2Polygon groundBox = b2MakeBox(50.0f, 10.0f);
    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    groundShapeDef.material.restitution = 0.5f;
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

    // SDL rectangle for drawing ground
    SDL_FRect ground = {0, 480 - 50, 640, 50};

    SDL_FRect rects[MAX_RECTS];
    b2BodyId bodies[MAX_RECTS];
    SDL_Color rectColors[MAX_RECTS];

    int rect_count = 0;

    SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255);
    SDL_RenderFillRectF(renderer, &ground);
    SDL_RenderPresent(renderer);

    // Main loop
    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                quit = 1;
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (rect_count < MAX_RECTS)
                {
                    SDL_FRect rect;
                    int mouseX = event.button.x;
                    int mouseY = event.button.y;

                    rect.w = RECT_W;
                    rect.h = RECT_H;
                    rect.x = mouseX - 0.5f * rect.w;
                    rect.y = mouseY - 0.5f * rect.h;
                    rects[rect_count] = rect;

                    // Generate random color for this rectangle
                    rectColors[rect_count].r = rand() % 256;
                    rectColors[rect_count].g = rand() % 256;
                    rectColors[rect_count].b = rand() % 256;
                    rectColors[rect_count].a = 255;

                    // Create Box2D dynamic body
                    b2BodyDef bodyDef = b2DefaultBodyDef();
                    bodyDef.type = b2_dynamicBody;
                    bodyDef.position = (b2Vec2){mouseX * METERS_PER_PIXEL, (CANVAS_HEIGHT - mouseY) * METERS_PER_PIXEL};
                    bodies[rect_count] = b2CreateBody(worldId, &bodyDef);

                    // Create shape for body
                    b2Polygon dynamicBox = b2MakeBox(RECT_W * 0.5f * METERS_PER_PIXEL, RECT_H * 0.5f * METERS_PER_PIXEL);
                    b2ShapeDef shapeDef = b2DefaultShapeDef();
                    shapeDef.density = 1.0f;
                    shapeDef.material.friction = 0.3f;
                    shapeDef.material.restitution = 0.5f;
                    b2CreatePolygonShape(bodies[rect_count], &shapeDef, &dynamicBox);

                    rect_count++;
                }
                break;
            }
        }

        // Step Box2D simulation
        float timeStep = 1.0f / 60.0f;
        int subStepCount = 4;
        b2World_Step(worldId, timeStep, subStepCount);

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 22, 255, 255, 255);
        SDL_RenderClear(renderer);

        // Draw ground
        SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255);
        SDL_RenderFillRectF(renderer, &ground);

        // Draw dynamic rectangles
        for (int i = 0; i < rect_count; i++)
        {

            // Set the random color for the rectangle
            SDL_SetRenderDrawColor(renderer, rectColors[i].r, rectColors[i].g, rectColors[i].b, 255);

            b2Vec2 pos = b2Body_GetPosition(bodies[i]);
            b2Vec2 canvasPos = world_to_canvas(pos);
            rects[i].x = canvasPos.x - 0.5f * rects[i].w;
            rects[i].y = canvasPos.y - 0.5f * rects[i].h;
            SDL_RenderFillRectF(renderer, &rects[i]);
        }

        // Present updated frame
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
