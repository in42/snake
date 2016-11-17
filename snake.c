#include "SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

// 4:3 aspect ratio is assumed
#define FIELD_CELL_WIDTH 64
#define CELL_SIZE ((SCREEN_WIDTH) / (FIELD_CELL_WIDTH))
#define FIELD_CELL_HEIGHT 48

#define FIELD_START_CELL_X 0
#define FIELD_START_CELL_Y 0


void draw_field(SDL_Renderer *renderer)
{
	Uint8 r, g, b, a;
	SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);

	SDL_SetRenderDrawColor(renderer, 0, 0, 100, 255);
	SDL_Rect field = {
		FIELD_START_CELL_X * CELL_SIZE,
		FIELD_START_CELL_Y * CELL_SIZE,
		FIELD_CELL_WIDTH * CELL_SIZE,
		FIELD_CELL_HEIGHT * CELL_SIZE
	};

	SDL_RenderFillRect(renderer, &field);
	SDL_SetRenderDrawColor(renderer, 0, 0, 150, 255);
	for (int i = 1; i < FIELD_CELL_WIDTH; i++) {
		SDL_RenderDrawLine(renderer, i * CELL_SIZE, 0,
				i * CELL_SIZE, FIELD_CELL_HEIGHT * CELL_SIZE);
	}
	for (int i = 1; i < FIELD_CELL_HEIGHT; i++) {
		SDL_RenderDrawLine(renderer, 0, i * CELL_SIZE,
				FIELD_CELL_WIDTH * CELL_SIZE, i * CELL_SIZE);
	}

	SDL_SetRenderDrawColor(renderer, r, g, b, a);
}


typedef enum dir {
	UP,
	DOWN,
	LEFT,
	RIGHT,
	NONE
} Dir;

// change in dx, dy when one moves in that direction by one cell
void get_dx_dy(int *dx, int *dy, Dir dir)
{
	switch (dir) {
	case UP:
		*dx = 0;
		*dy = -CELL_SIZE;
		break;
	case DOWN:
		*dx = 0;
		*dy = CELL_SIZE;
		break;
	case RIGHT:
		*dx = CELL_SIZE;
		*dy = 0;
		break;
	case LEFT:
		*dx = -CELL_SIZE;
		*dy = 0;
		break;
	default:
		*dx = 0;
		*dy = 0;
		break;
	}
}

typedef struct snake_segment {
	int x;
	int y;
	Dir dir;

	// struct snake_segment *next;
	// struct snake_segment *prev;
} SnakeSegment;

#define SNAKE_MAX_SIZE 10000

typedef struct snake {
	SnakeSegment segments[SNAKE_MAX_SIZE];
	int size;
} Snake;

Snake snake;

void init_snake_segment(int i)
{
	snake.segments[i] = (SnakeSegment){ 0, 0, RIGHT };
}

void snake_init(int size, int x, int y, Dir dir)
{
	snake.size = size;
	for (int i = 0; i < size; i++) {
		if (i == 0) {
			snake.segments[i].x = x;
			snake.segments[i].y = y;
		} else {
			int dx, dy;
			get_dx_dy(&dx, &dy, snake.segments[i - 1].dir);
			snake.segments[i].x = snake.segments[i - 1].x - dx;
			snake.segments[i].y = snake.segments[i - 1].y - dy;
		}
		snake.segments[i].dir = dir;
	}
}

void snake_move()
{
	for (int i = 0; i < snake.size; i++) {
		int dx, dy;
		get_dx_dy(&dx, &dy, snake.segments[i].dir);
		snake.segments[i].x += dx;
		snake.segments[i].y += dy;

	}

	for (int i = snake.size - 1; i >= 1; i--) {
		snake.segments[i].dir = snake.segments[i - 1].dir;
	}
}

void snake_draw(SDL_Renderer *renderer)
{
	Uint8 r, g, b, a;
	SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
	SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
	for (int i = 0; i < snake.size; i++) {
		SDL_Rect cell = { snake.segments[i].x, snake.segments[i].y,
			CELL_SIZE, CELL_SIZE };
		SDL_RenderFillRect(renderer, &cell);

		SDL_SetRenderDrawColor(renderer, 0, 0, 180, 255);
		SDL_RenderDrawRect(renderer, &cell);
		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
	}

	SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

Dir snake_get_dir()
{
	if (snake.size == 0) {
		return 0;
	}

	return snake.segments[0].dir;
}

void snake_set_dir(Dir dir)
{
	if (snake.size != 0) {
		snake.segments[0].dir = dir;
	}
}

void draw_screen(SDL_Renderer *renderer)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderDrawLine(renderer, SCREEN_WIDTH - 1, 0, SCREEN_WIDTH - 1,
			SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	draw_field(renderer);
	snake_draw(renderer);
}

int main()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "\nUnable to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	atexit(SDL_Quit);

	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_CreateWindowAndRenderer(SCREEN_HEIGHT, SCREEN_WIDTH,
			SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &renderer);
	// make the scaled rendering look smoother.
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

	// int w, h;
	// SDL_GetRendererOutputSize(renderer, &w, &h);
	// printf("%d %d\n", w, h);
	// SDL_GetWindowSize(window, &w, &h);
	// printf("%d %d\n", w, h);

	int quit = 0;
	snake_init(5, 100, 100, RIGHT);
	draw_screen(renderer);
	bool last_move_drawn = true;
	while (!quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				quit = 1;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					quit = 1;
					break;
				case SDLK_UP:
					if (snake_get_dir() != DOWN && last_move_drawn) {
						snake_set_dir(UP);
						last_move_drawn = false;
					}
					break;
				case SDLK_DOWN:
					if (snake_get_dir() != UP && last_move_drawn) {
						snake_set_dir(DOWN);
						last_move_drawn = false;
					}
					break;
				case SDLK_LEFT:
					if (snake_get_dir() != RIGHT && last_move_drawn) {
						snake_set_dir(LEFT);
						last_move_drawn = false;
					}
					break;
				case SDLK_RIGHT:
					if (snake_get_dir() != LEFT && last_move_drawn) {
						snake_set_dir(RIGHT);
						last_move_drawn = false;
					}
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
		}

		snake_move(snake);
		last_move_drawn = true;

		SDL_Delay(100);
		draw_screen(renderer);
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	return 0;
}
