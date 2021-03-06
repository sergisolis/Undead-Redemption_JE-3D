#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "fbo.h"
#include "shader.h"
#include "input.h"
#include "animation.h"

#include <cmath>

#include "World.h"
#include "Stage.h"


float angle = 0;
float mouse_speed = 100.0f;
FBO* fbo = NULL;

Game* Game::instance = NULL;

Game::Game(int window_width, int window_height, SDL_Window* window)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;
	mouse_locked = false;

	//OpenGL flags
	glEnable( GL_CULL_FACE ); //render both sides of every triangle
	glEnable( GL_DEPTH_TEST ); //check the occlusions using the Z buffer

	world.loadWorld();

	//create our camera
	camera = new Camera();
	camera->lookAt(Vector3(0.f,50.f, 100.f),Vector3(0.f,0.f,0.f), Vector3(0.f,1.f,0.f)); //position the camera and point to 0,0,0
	camera->setPerspective(70.f,window_width/(float)window_height,0.1f,100000.f); //set the projection, we want to be perspective

	//create our camera
	camera2 = new Camera();
	camera2->lookAt(Vector3(0.f, 50.f, 100.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)); //position the camera and point to 0,0,0
	camera2->setPerspective(70.f, window_width / (float)window_height, 0.1f, 100000.f); //set the projection, we want to be perspective
	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
	
}

//what to do when the image has to be draw
void Game::render(void)
{

	if (world.currentStage == STAGE_ID::EDITOR) {
		world.stages[STAGE_ID::PLAY]->render();
	}
	else {
		world.stages[world.currentStage]->render();
	}

}

void Game::update(double seconds_elapsed)
{
	if (world.currentStage == STAGE_ID::TUTORIAL) {
		world.stages[STAGE_ID::PLAY]->update(seconds_elapsed);
	}
	else
	{
		world.stages[world.currentStage]->update(seconds_elapsed);
	}

}

//Keyboard event handler (sync input)
void Game::onKeyDown( SDL_KeyboardEvent event )
{
	Game* game = Game::instance;
	World& world = game->world;
	EntityPlayer* player = world.player;

	switch(event.keysym.sym)
	{
		//case SDLK_ESCAPE: must_exit = true; break; //ESC key, kill the app
		case SDLK_F1: Shader::ReloadAll(); break; 
			//#ifdef EDITOR
		case SDLK_2: addEntityInFront(camera,"data/box.obj" , "data/PolygonWestern_Texture_01_A.png"); break;
		case SDLK_3: rayPick(camera); break;
		//case SDLK_4: takeEntity(); break;
		case SDLK_F2: rotateSelected(10.0f); break;
		case SDLK_KP_MINUS: rotateSelected(-10.0f); break;
		case SDLK_0: player->firstPerson = !player->firstPerson; break;
			//#endif // EDITOR

	}
}

void Game::onKeyUp(SDL_KeyboardEvent event)
{
}

void Game::onGamepadButtonDown(SDL_JoyButtonEvent event)
{

}

void Game::onGamepadButtonUp(SDL_JoyButtonEvent event)
{

}

void Game::onMouseButtonDown( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_MIDDLE) //middle mouse
	{
		mouse_locked = !mouse_locked;
		SDL_ShowCursor(!mouse_locked);
	}
}

void Game::onMouseButtonUp(SDL_MouseButtonEvent event)
{
}

void Game::onMouseWheel(SDL_MouseWheelEvent event)
{
	mouse_speed *= event.y > 0 ? 1.1 : 0.9;
}

void Game::onResize(int width, int height)
{
    std::cout << "window resized: " << width << "," << height << std::endl;
	glViewport( 0,0, width, height );
	camera->aspect =  width / (float)height;
	window_width = width;
	window_height = height;
}

