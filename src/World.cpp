#include "World.h"
#include "game.h"
#include <bass.h>

void World::loadWorld() {
	Game* game = Game::instance;

	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

	player = new EntityPlayer();

	//bullets.reserve(numBullets);

	for (size_t i = 0; i < numBullets; i++)
	{
		bullets[i].ttl = 0.0f;
		bullets[i].model = Matrix44();
		bullets[i].last_position = Vector3();
		bullets[i].mesh = Mesh::Get("data/box.obj");
		bullets[i].texture = Texture::Get("data/PolygonMinis_Texture_01_A.png");
		bullets[i].shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
		bullets[i].power = 1;
		bullets[i].velocity = Vector3(20.0f, 20.0f, 20.0f);
	}
	

	currentStage = STAGE_ID::INTRO;
	titleOption = TITLE_OPTIONS::PLAY_GAME;
	menuOption = MENU_OPTIONS::RETURN;

	stages.reserve(8);
	stages.push_back(new introStage());
	stages.push_back(new titleStage());
	stages.push_back(new tutorialStage());
	stages.push_back(new playStage());
	stages.push_back(new transitionStage());
	stages.push_back(new editorStage());
	stages.push_back(new menuStage());
	stages.push_back(new endStage());

	currentSlide = 0;

	skyModel = Matrix44();
	skyMesh = Mesh::Get("data/cielo.ASE");
	skyTex = Texture::Get("data/sky.jpg");
	sky = new EntityMesh(GL_TRIANGLES, skyModel, skyMesh, skyTex, shader);

	bow = new EntityMesh(GL_TRIANGLES, Matrix44(), Mesh::Get("data/bow.obj"), Texture::Get("data/color-atlas.png"), Shader::Get("data/shaders/skinning.vs", "data/shaders/texture.fs"));
	sword = new EntityMesh(GL_TRIANGLES, Matrix44(), Mesh::Get("data/sword.obj"), Texture::Get("data/color-atlas.png"), Shader::Get("data/shaders/skinning.vs", "data/shaders/texture.fs"));

	exit_open = Mesh::Get("data/exit_open.obj");

	Matrix44 groundModel;
	Mesh* groundMesh = new Mesh();
	groundMesh->createPlane(500);
	ground = new EntityMesh(GL_TRIANGLES, groundModel, groundMesh, Texture::Get("data/ground.jpg"), shader, Vector4(1,1,1,1), 500.0f);
	cameraLocked = true;

	viewDatas[1].mesh = Mesh::Get("data/wall.obj");
	viewDatas[1].texture = Texture::Get("data/color-atlas.png");

	viewDatas[2].mesh = Mesh::Get("data/exit.obj");
	viewDatas[2].texture = Texture::Get("data/color-atlas.png");

	viewDatas[3].mesh = Mesh::Get("data/enemy_warrior.mesh");
	viewDatas[3].texture = Texture::Get("data/PolygonMinis_Texture_01_A.png");

	viewDatas[4].mesh = Mesh::Get("data/enemy_archer.mesh");
	viewDatas[4].texture = Texture::Get("data/PolygonMinis_Texture_01_A.png");

	viewDatas[5].mesh = Mesh::Get("data/graveyard.obj");
	viewDatas[5].texture = Texture::Get("data/color-atlas.png");

	level_info.level = 0;
	level_info.level = 0;

	gamemap = new GameMap();

	level_info.tag = ACTION_ID::NO_ACTION;
	level_info.space_pressed = 2.0f;

	if (BASS_Init(-1, 44100, 0, 0, NULL) == false) //-1 significa usar el por defecto del sistema operativo
	{
		std::cout << "Error init BASS" << std::endl;
	}

	audio.LoadSample("data/sounds/intro.wav");
	audio.LoadSample("data/sounds/lvl1.wav");
	audio.LoadSample("data/sounds/lvl2.wav");
	audio.LoadSample("data/sounds/lvl3.mp3");
	audio.LoadSample("data/sounds/lvl4.wav");
	audio.LoadSample("data/sounds/lvl5.wav");
	audio.LoadSample("data/sounds/sword.wav");
	audio.LoadSample("data/sounds/arrow.wav");
	audio.LoadSample("data/sounds/hit.wav");
	audio.LoadSample("data/sounds/hit_enemy.mp3");
	audio.LoadSample("data/sounds/dead_enemy.mp3");
	audio.LoadSample("data/sounds/dead_enemy.mp3");
	audio.LoadSample("data/sounds/jump.wav");
	audio.LoadSample("data/sounds/touch_ground.wav");
	audio.LoadSample("data/sounds/chest.wav");

}

Vector3 CellToWorld(Vector2 cellPos, int cellsize) { //esquina superior izquierda
	Game* game = Game::instance;
	Vector2 result = cellPos * cellsize;
	return Vector3(cellPos.x * game->world.tileWidth, 0.0f, cellPos.y * game->world.tileWidth);
}

Vector3 CellToWorldCenter(Vector2 cellPos, int cellsize) { //center
	Vector2 result = cellPos * cellsize - Vector2(cellsize / 2, cellsize / 2);
	return Vector3(result.x, 0.0f, result.y);
}

void World::importMap(std::vector<EntityMesh*>& entities) {
	for (size_t i = 0; i < gamemap->width; i++)
	{
		for (size_t j = 0; j < gamemap->height; j++)
		{
			sCell& cell = gamemap->getCell(i, j);
			int index = (int)cell.type;
			sPropViewData& prop = viewDatas[0];
			if (index == 0) continue;
			if (index == 3) {
				player->pos = CellToWorldCenter(Vector2(i, j), tileWidth);
				spawnPos = player->pos;
			}else{
				Matrix44 cellModel;
				cellModel.translate(i * tileWidth, 0.0f, j * tileHeight);
				cellModel.scale(3, 3, 3);
				//renderMesh(GL_TRIANGLES, cellModel, prop.mesh, prop.texture, world.shader, game->camera);
				if (index == 1) {
					prop = viewDatas[1];
					EntityMesh* entity = new EntityMesh(GL_TRIANGLES, cellModel, prop.mesh, prop.texture, shader);
					entities.push_back(entity);
				}
				else if (index == 2) {
					prop = viewDatas[1];
					cellModel.rotate(90 * DEG2RAD, Vector3(0, 1, 0));
					EntityMesh* entity = new EntityMesh(GL_TRIANGLES, cellModel, prop.mesh, prop.texture, shader);
					entities.push_back(entity);
				}
				else if (index == 4) {
					prop = viewDatas[2];
					EntityMesh* entity = new EntityMesh(GL_TRIANGLES, cellModel, prop.mesh, prop.texture, shader);
					entity->id = ENTITY_ID::ENTITY_MESH;
					finish = entity;
					//entities.push_back(entity);
				}else if (index == 5) {
					prop = viewDatas[2];
					cellModel.rotate(90 * DEG2RAD, Vector3(0, 1, 0));
					EntityMesh* entity = new EntityMesh(GL_TRIANGLES, cellModel, prop.mesh, prop.texture, shader);
					entity->id = ENTITY_ID::ENTITY_MESH;
					finish = entity;
					//entities.push_back(entity);
				}
				else if (index == 6) {
					prop = viewDatas[3];
					//cellModel.rotate(90 * DEG2RAD, Vector3(1, 0, 0));
					EntityEnemy* enemy = new EntityEnemy(cellModel, prop.mesh, prop.texture, level_info.level, ENEMY_ID::WARRIOR);
					enemy->pos = CellToWorldCenter(Vector2(i, j), tileWidth);
					enemy->spawnPos = enemy->pos;
					enemies.push_back(enemy);
					EntityEnemy* l_enemy = new EntityEnemy(cellModel, prop.mesh, prop.texture, level_info.level, ENEMY_ID::WARRIOR);
					l_enemy->pos = CellToWorldCenter(Vector2(i, j), tileWidth);
					l_enemy->spawnPos = enemy->pos;
					last_enemy = l_enemy;

				}
				//arquero
				else if (index == 7) {
					prop = viewDatas[4];
					//cellModel.rotate(90 * DEG2RAD, Vector3(1, 0, 0));
					EntityEnemy* enenmy = new EntityEnemy(cellModel, prop.mesh, prop.texture, level_info.level, ENEMY_ID::ARCHER);
					enenmy->pos = CellToWorldCenter(Vector2(i, j), tileWidth);
					enenmy->spawnPos = enenmy->pos;
					enemies.push_back(enenmy);

				}
				else if (index == 8) {
					//prop = viewDatas[4];
					cellModel.rotate(90 * DEG2RAD, Vector3(0, 1, 0));
					EntityChest* entity = new EntityChest(cellModel, level_info.level, CHEST_ID::CHEST_SWORD);

					chests.push_back(entity);
				}
				else if (index == 9) {
					//prop = viewDatas[4];
					cellModel.rotate(90 * DEG2RAD, Vector3(0, 1, 0));
					EntityChest* entity = new EntityChest(cellModel, level_info.level, CHEST_ID::CHEST_HEART);
					chests.push_back(entity);
				}
				else if (index == 10) {
					//prop = viewDatas[4];
					cellModel.rotate(90 * DEG2RAD, Vector3(0, 1, 0));
					EntityChest* entity = new EntityChest(cellModel, level_info.level, CHEST_ID::CHEST_STRENGTH);
					chests.push_back(entity);
				}
				else if (index == 11) {
					//prop = viewDatas[4];
					cellModel.rotate(90 * DEG2RAD, Vector3(0, 1, 0));
					EntityChest* entity = new EntityChest(cellModel, level_info.level, CHEST_ID::CHEST_VELOCITY);
					chests.push_back(entity);
				}
				else if (index == 12) {
					prop = viewDatas[5];
					EntityMesh* entity = new EntityMesh(GL_TRIANGLES, cellModel, prop.mesh, prop.texture, shader);
					entities.push_back(entity);
				}
				else if (index == 13) {
					prop = viewDatas[5];
					cellModel.rotate(90 * DEG2RAD, Vector3(0, 1, 0));
					EntityMesh* entity = new EntityMesh(GL_TRIANGLES, cellModel, prop.mesh, prop.texture, shader);
					entities.push_back(entity);
				}
				else if (index == 14) {
					prop = viewDatas[3];
					//cellModel.rotate(90 * DEG2RAD, Vector3(1, 0, 0));
					EntityEnemy* enenmy = new EntityEnemy(cellModel, prop.mesh, prop.texture, level_info.level, ENEMY_ID::BOSS);
					enenmy->pos = CellToWorldCenter(Vector2(i, j), tileWidth);
					enenmy->spawnPos = enenmy->pos;
					enemies.push_back(enenmy);
				}
			}
		}
	}
}

void  World::unifyCollidableEntities() {
	if (!collidable_entities.empty())
	{
		collidable_entities.clear();
	}
	for (size_t i = 0; i < static_entities.size(); i++)
	{
		EntityMesh* entity = static_entities[i];
		//collidable_entities.emplace_back(entity);
		//collidable_entities.push_back(std::make_shared<Entity*>(entity));
		collidable_entities.push_back(entity);
	}
	for (size_t i = 0; i < chests.size(); i++)
	{
		EntityMesh* entity = chests[i]->mesh;
		entity->id = ENTITY_ID::ENTITY_CHEST;
		//collidable_entities.push_back(std::make_shared<Entity*>(entity));
		entity->chest_id = i;
		collidable_entities.push_back(entity);

		chests[i]->collidable_id = i;
	}
	for (size_t i = 0; i < enemies.size(); i++)
	{
		//EntityMesh* entity = enemies[i]->mesh;
		//collidable_entities.push_back(entity);
	}
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

void World::loadLevel() {
	audio.ResetAudio();
	if (level_info.tag == ACTION_ID::WIN) {
		level_info.level += 1;
		if (level_info.level == 6) {
			currentStage = STAGE_ID::END;
			return;
		}
	}

	static_entities.clear();
	collidable_entities.clear();
	chests.clear();
	enemies.clear();
	std::string s = std::to_string(level_info.level);
	char const* level = s.c_str();
	char i_path[100] = "data/lvl";
	char const* path = strcat(i_path, level);
	path = strcat(i_path, ".map");
	std::cout << path << std::endl;

	gamemap = loadGameMap(path);
	importMap(static_entities);
	unifyCollidableEntities();
	
	level_info.tag == ACTION_ID::NO_ACTION;
	
	if (level_info.level == 0)
	{
		audio.PlayGameSound(AUDIO_ID::BACKGROUND_SOUND1);
		player->currentItem = ITEM_ID::SWORD;
		player->hearts = 8;
		player->strength = 1;
	}
	else if (level_info.level == 1) {
		audio.PlayGameSound(AUDIO_ID::BACKGROUND_SOUND1);
		player->currentItem = ITEM_ID::NONE;
		player->hearts = 3;
		player->strength = 1;
	}
	else if (level_info.level == 2) {
		audio.PlayGameSound(AUDIO_ID::BACKGROUND_SOUND2);
		player->currentItem = ITEM_ID::SWORD;
		
	}
	else if (level_info.level == 3) {
		audio.PlayGameSound(AUDIO_ID::BACKGROUND_SOUND3);
		player->currentItem = ITEM_ID::SWORD;

	}
	else if (level_info.level == 4) {
		audio.PlayGameSound(AUDIO_ID::BACKGROUND_SOUND4);
		player->currentItem = ITEM_ID::SWORD;

	}
	else if (level_info.level == 5) {
		audio.PlayGameSound(AUDIO_ID::BACKGROUND_SOUND5);
		player->currentItem = ITEM_ID::SWORD;

	}
	if (level_info.level > 1)
	{
		if (level_info.last_player_hearts < 3) {
			player->hearts = 3;
		}
		else {
			player->hearts = level_info.last_player_hearts;
		}
		player->strength = level_info.last_player_strength;
		player->runSpeed = level_info.last_player_run_speed;
	}

	player->hitTimer = 0.0f;
	player->jaw = 0;

	//camera_inverse = false;

	currentStage = STAGE_ID::PLAY;
}

HSAMPLE loadSample(const char* fileName) {
	//El handler para un sample
	HSAMPLE hSample;
	//use BASS_SAMPLE_LOOP in the last param to have a looped sound
	hSample = BASS_SampleLoad(false, fileName, 0, 0, 3, 0);
	if (hSample == 0)
	{
		std::cout << "ERROR load " << fileName << std::endl;
	}
	std::cout << "+ AUDIO load " << fileName << std::endl;
	return hSample;
}



