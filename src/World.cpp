#include "World.h"
#include "game.h"

void World::loadWorld() {
	Game* game = Game::instance;

	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

	player = new EntityPlayer();
	play = new playStage();
	current_stage = play;

	skyModel = Matrix44();
	skyMesh = Mesh::Get("data/cielo.ASE");
	skyTex = Texture::Get("data/cielo.tga");
	sky = new EntityMesh(GL_TRIANGLES, skyModel, skyMesh, skyTex, shader);


	Matrix44 groundModel;
	Mesh* groundMesh = new Mesh();
	groundMesh->createPlane(1000);
	ground = new EntityMesh(GL_TRIANGLES, groundModel, groundMesh, Texture::Get("data/ground.jpg"), shader);
	ground->tiling = 500.0f;
	cameraLocked = true;

	viewDatas[1].mesh = Mesh::Get("data/wall.obj");
	viewDatas[1].texture = Texture::Get("data/color-atlas.png");

	viewDatas[2].mesh = Mesh::Get("data/box.obj");
	viewDatas[2].texture = Texture::Get("data/color-atlas.png");

	gamemap = new GameMap();
	gamemap = loadGameMap("data/mymap.map");
	//std::cout << "static_entities: " << static_entities.size() << std::endl;
}

GameMap* loadGameMap(const char* filename)
{
	FILE* file = fopen(filename, "rb");
	if (file == NULL) //file not found
		return NULL;

	sMapHeader header; //read header and store it in the struct
	fread(&header, sizeof(sMapHeader), 1, file);
	assert(header.bytes == 1); //always control bad cases!!


	//allocate memory for the cells data and read it
	unsigned char* cells = new unsigned char[header.w * header.h];
	fread(cells, header.bytes, header.w * header.h, file);
	fclose(file); //always close open files
	//create the map where we will store it
	GameMap* map = new GameMap(header.w, header.h);

	for (int x = 0; x < map->width; x++)
		for (int y = 0; y < map->height; y++)
			map->getCell(x, y).type = (eCellType)cells[x + y * map->width];

	delete[] cells; //always free any memory allocated!

	return map;
}
