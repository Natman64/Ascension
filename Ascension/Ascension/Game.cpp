#include "Game.h"

#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>

#include <SDL.h>

#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"

#include "Surface.h"
#include "FlagCondition.h"
#include "Flags.h"

const char* kWindowTitle = "...";
const char* kTitle = "ASCENSION";

const int kFPS = 60;
const int kMaxFrameTime = 5 * 1000 / 60;

Game::Game()
	: mRunning(false)
{
	SDL_Init(SDL_INIT_EVERYTHING);

	LoadContent();
}

Game::~Game()
{
	SDL_Quit();
}

void Game::Run()
{
	ascii::Graphics graphics(kWindowTitle);
	ascii::Input input;

	mRunning = true;

	int lastUpdateTime = SDL_GetTicks();

	while (mRunning)
	{
		const int initialTime = SDL_GetTicks();

		input.beginNewFrame();

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					Quit();
					return;
				case SDL_KEYDOWN:
					input.keyDownEvent(event);
					break;
				case SDL_KEYUP:
					input.keyUpEvent(event);
					break;
			}
		}

		HandleInput(input);

		const int currentTime = SDL_GetTicks();
		const int elapsedTime = currentTime - lastUpdateTime;

		Update(std::min(elapsedTime, kMaxFrameTime));
		lastUpdateTime = currentTime;

		Draw(graphics);

		const int msPerFrame = 1000 / kFPS;
		const int elapsedTimeMS = SDL_GetTicks() - initialTime;

		if (elapsedTimeMS < msPerFrame)
			SDL_Delay(msPerFrame - elapsedTimeMS);
	}
}

void Game::Quit()
{
	mRunning = false;
}

void Game::LoadContent()
{
	flags::init();

	rapidxml::file<> file("data.xml");
	rapidxml::xml_document<> doc;
	doc.parse<0>(const_cast<char *>(file.data()));

	rapidxml::xml_node<>* screenNode = NULL;
	int i = 0;
	while (screenNode = doc.first_node("Screen"))
	{
		Screen* screen = new Screen();
		rapidxml::xml_node<>* textNode = NULL;
		while (textNode = screenNode->first_node("Text"))
		{
			rapidxml::xml_attribute<>* conditionAttribute = textNode->first_attribute("condition");
			
			FlagCondition condition(-1, -1, 0);

			if (conditionAttribute)
			{
				char* condcstr = conditionAttribute->value();

				char* word = "";

				word = strtok(condcstr, " "); //flag
				word = strtok(NULL, " "); //row

				int row = atoi(word);

				word = strtok(NULL, " "); //col

				int col = atoi(word);

				word = strtok(NULL, " "); //value

				int val = atoi(word);

				condition = FlagCondition(row, col, val);
			}

			const char* text = textNode->value();

			screen->add(Text(text, condition));

			screenNode->remove_first_node();

		}

		rapidxml::xml_node<>* actionNode = NULL;
		while (actionNode = screenNode->first_node("Action"))
		{
			rapidxml::xml_attribute<>* conditionAttribute = actionNode->first_attribute("condition");
			
			FlagCondition condition(-1, -1, 0);

			if (conditionAttribute)
			{
				char* condcstr = conditionAttribute->value();

				char* word = "";

				word = strtok(condcstr, " "); //flag
				word = strtok(NULL, " "); //row

				int row = atoi(word);

				word = strtok(NULL, " "); //col

				int col = atoi(word);

				word = strtok(NULL, " "); //value

				int val = atoi(word);

				condition = FlagCondition(row, col, val);
			}

			const char* text = actionNode->first_attribute("text")->value();

			rapidxml::xml_attribute<>* flagSetAttribute = actionNode->first_attribute("flagset");

			FlagSet flagSet(-1, -1, 0);

			if (flagSetAttribute)
			{
				char* setcstr = flagSetAttribute->value();

				char* word = "";

				word = strtok(setcstr, " "); //row

				int row = atoi(word);

				word = strtok(NULL, " "); //col

				int col = atoi(word);

				word = strtok(NULL, " "); //value

				int val = atoi(word);

				flagSet = FlagSet(row, col, val);
			}

			screen->addAction(Action(text, condition, flagSet, actionNode->value()));

			screenNode->remove_first_node();
		}

		if (i == 0)
		{
			mScreen = screen;
			++i;
		}

		mScreens[screenNode->first_attribute("name")->value()] = screen;

		doc.remove_first_node();
	}

	
}

void Game::Update(int deltaMS)
{
	if (strcmp(flags::getNextRoom(), ""))
	{
		mScreen = mScreens[flags::getNextRoom()];
		flags::setNextRoom("");
	}

	mScreen->update(deltaMS);
}

void Game::HandleInput(ascii::Input& input)
{
	mScreen->handleInput(input);
}

void Game::Draw(ascii::Graphics& graphics)
{
	graphics.clear();

	graphics.drawBorder(' ', ascii::Color::Green, ascii::Color::White);
	graphics.blitString(kTitle, ascii::Color::Black, ascii::Graphics::kBufferWidth / 2 - strlen(kTitle) / 2, 0);

	mScreen->draw(graphics);

	graphics.update();
}