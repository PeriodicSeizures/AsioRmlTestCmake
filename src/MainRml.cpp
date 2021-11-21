/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2008-2010 Nuno Silva
 * Copyright (c) 2019 The RmlUi Team, and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <RmlUi/Core.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Debugger/Debugger.h>
#include <string.h>

#include "FileInterface.h"
#include "SystemInterfaceSDL2.h"
#include "RenderInterfaceSDL2.h"
#include <sol/sol.hpp>
#include <RmlUi/Lua.h>

#ifdef RMLUI_PLATFORM_WIN32
#include <windows.h>
#endif

#include <SDL.h>
#include <iostream>

#include <GL/glew.h>

int main(int argc, char** argv)
{

	if (argc != 2) {
		std::cout << "Must provide working path";
		return 1;
	}

#ifdef RMLUI_PLATFORM_WIN32
	AllocConsole();
#endif

	int window_width = 1024;
	int window_height = 768;

	SDL_Init(SDL_INIT_VIDEO);
	
	SDL_Window* screen = SDL_CreateWindow("LibRmlUi SDL2 test", 
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		window_width, window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	SDL_GLContext glcontext = SDL_GL_CreateContext(screen);
	int oglIdx = -1;
	int nRD = SDL_GetNumRenderDrivers();
	for (int i = 0; i < nRD; i++)
	{
		SDL_RendererInfo info;
		if (!SDL_GetRenderDriverInfo(i, &info))
		{
			if (!strcmp(info.name, "opengl"))
			{
				oglIdx = i;
			}
		}
	}
	SDL_Renderer* renderer = SDL_CreateRenderer(screen, oglIdx, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	GLenum err = glewInit();

	if (err != GLEW_OK)
		fprintf(stderr, "GLEW ERROR: %s\n", glewGetErrorString(err));

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	glMatrixMode(GL_PROJECTION | GL_MODELVIEW);
	glLoadIdentity();
	glOrtho(0, window_width, window_height, 0, 0, 1);

	RmlUiSDL2Renderer Renderer(renderer, screen);
	RmlUiSDL2SystemInterface SystemInterface;

	//Rml::String root = Shell::FindSamplesRoot();
	//FileInterface FileInterface("C:/Users/Rico/Documents/VisualStudio2019/Projects/RMLCMake/cwd/");
	FileInterface FileInterface(argv[1]);

	Rml::SetFileInterface(&FileInterface);
	Rml::SetRenderInterface(&Renderer);
	Rml::SetSystemInterface(&SystemInterface);

	if (!Rml::Initialise())
		return 1;

	sol::state lua; // = sol::state();
	lua.open_libraries();
	Rml::Lua::Initialise(lua.lua_state());

	//struct FontFace {
	//	Rml::String filename;
	//	bool fallback_face;
	//};
	//FontFace font_faces[] = {
	//	{ "LatoLatin-Regular.ttf",    false },
	//	{ "LatoLatin-Italic.ttf",     false },
	//	{ "LatoLatin-Bold.ttf",       false },
	//	{ "LatoLatin-BoldItalic.ttf", false },
	//	{ "NotoEmoji-Regular.ttf",    true  },
	//};
	//for (const FontFace& face : font_faces)
	//{
	//	Rml::LoadFontFace("assets/" + face.filename, face.fallback_face);
	//}
	
	Rml::LoadFontFace("assets/fonts/OpenSans-Regular.ttf", false);

	Rml::Context* Context = Rml::CreateContext("default",
		Rml::Vector2i(window_width, window_height));

#ifndef NDEBUG
	Rml::Debugger::Initialise(Context);
#endif
	Rml::ElementDocument* doc = Context->LoadDocument("assets/main-menu.rml");

	if (!doc) {
		std::cout << "Failed to load document\n";

		Rml::Shutdown();

		SDL_DestroyRenderer(renderer);
		SDL_GL_DeleteContext(glcontext);
		SDL_DestroyWindow(screen);
		SDL_Quit();
		return 1;
	}

	std::cout << std::endl;

	doc->Show();

	bool done = false;
	while (!done)
	{
		SDL_Event event;

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderClear(renderer);

		Context->Render();
		SDL_RenderPresent(renderer);

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				done = true;
				break;

			case SDL_MOUSEMOTION:
				Context->ProcessMouseMove(event.motion.x, event.motion.y, SystemInterface.GetKeyModifiers());
				break;
			case SDL_MOUSEBUTTONDOWN:
				Context->ProcessMouseButtonDown(SystemInterface.TranslateMouseButton(event.button.button), SystemInterface.GetKeyModifiers());
				break;

			case SDL_MOUSEBUTTONUP:
				Context->ProcessMouseButtonUp(SystemInterface.TranslateMouseButton(event.button.button), SystemInterface.GetKeyModifiers());
				break;

			case SDL_MOUSEWHEEL:
				Context->ProcessMouseWheel(float(event.wheel.y), SystemInterface.GetKeyModifiers());
				break;

			case SDL_KEYDOWN:
			{
				// Intercept F8 key stroke to toggle RmlUi's visual debugger tool
				if (event.key.keysym.sym == SDLK_F8)
				{
					Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
					break;
				}

				Context->ProcessKeyDown(SystemInterface.TranslateKey(event.key.keysym.sym), SystemInterface.GetKeyModifiers());
				break;
			}

			case SDL_WINDOWEVENT: {
				switch (event.window.event) {
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					auto w = event.window.data1;
					auto h = event.window.data2;
					Context->SetDimensions(Rml::Vector2i(w, h));
					break;
				}
			}

			default:
				break;
			}
		}
		Context->Update();
	}

	Rml::Shutdown();

	SDL_DestroyRenderer(renderer);
	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(screen);
	SDL_Quit();

	return 0;
}