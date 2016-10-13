#include <string.h>
#include <SDL2/SDL.h>
#include <pthread.h>
#include <stdio.h>
#include "graphics.h"
#include "gb_cpu.h"

#define WINDOW_TITLE_FORMAT_STRING "GameboyEmulator running %s"

struct gb_graphics_subsystem_options {
	gb_cpu* processor;
	uint8_t* ram;
	char* title;
};

static void*
gb_graphics_subsystem(void* arg)
{
	SDL_Window* window = NULL;
	struct gb_graphics_subsystem_options options = * ((struct gb_graphics_subsystem_options*) arg);
	gb_cpu* processor = options.processor;
	uint8_t* ram = options.ram;

	window = SDL_CreateWindow(
		options.title,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		160,
		144,
		SDL_WINDOW_SHOWN
	);

	if(window == NULL)
	{
		printf("Error creating window: %s\n", SDL_GetError());
	}
	else
	{
		// don't print from this thread you twat
		//printf("Window should be open now...\n");
	}

	while(processor->halted == 0)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			// handle events
		}

		// drawing etc.
	}

	SDL_DestroyWindow(window);

	return NULL;
}

void gb_start_graphics_subsystem(gb_cpu* processor, uint8_t* ram, char* title)
{
	pthread_t graphics_thread;
	struct gb_graphics_subsystem_options* options = malloc(sizeof (struct gb_graphics_subsystem_options));
	char* window_title = malloc(14 + strlen(WINDOW_TITLE_FORMAT_STRING));
	options->processor = processor;
	options->ram = ram;
	options->title = window_title;

	sprintf(window_title, WINDOW_TITLE_FORMAT_STRING, title);

	SDL_Init(SDL_INIT_VIDEO);

	pthread_create(&graphics_thread, NULL, &gb_graphics_subsystem, options);

}

void gb_shutdown_graphics_subsystem(void)
{
	SDL_Quit();	
}