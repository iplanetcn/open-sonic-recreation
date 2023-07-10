#include "Level.h"

#include <cstdio>


void Level::create(std::string fZone, std::string fAct, int act) {
	sZone = fZone;
	sAct  = fAct;
	this->act = act;

	verHeights = nullptr;
	horHeights = nullptr;
	angles = nullptr;

	tilesLayout = nullptr;
	tiles16 = nullptr;
	tilesBig = nullptr;

	loadTerrainTiles("assets/levels/collide/Collision Array (Normal).bin",
					 "assets/levels/collide/Collision Array (Rotated).bin",
					 "assets/levels/collide/Angle Map.bin");

	std::string sTex       = "assets/levels/textures/"  + sZone + ".png";
	std::string sCollide   = "assets/levels/collide/"   + sZone + ".bin";
	std::string sMap256    = "assets/levels/map256/"    + sZone + ".bin";
	std::string sLayout    = "assets/levels/layout/"    + sAct  + ".bin";
	std::string sStartPos  = "assets/levels/startpos/"  + sAct  + ".bin";
	std::string sObjPos    = "assets/levels/objpos/" 	 + sAct  + ".bin";

	scr.loadTextureFromFile(sTex.c_str(), 255);

	loadTerrainZone(sCollide.c_str(), sMap256.c_str());
	loadTerrainAct(sLayout.c_str(), sStartPos.c_str());
	loadObjects(sObjPos.c_str());

	trn.create(verHeights, horHeights, angles,
			   tiles16, tilesBig, tilesLayout, 255);
	trn.createLayeringObjs(entities);

	// Bg
	bg = new Bg();

	const char* zoneName = sZone.c_str();
	if      (sZone == "GHZ") zoneName = "GREEN HILL";
	else if (sZone == "LZ")  zoneName = "LABYRINTH";
	else if (sZone == "MZ")  zoneName = "MARBLE";
	else if (sZone == "SBZ") zoneName = "SCRAP BRAIN";
	else if (sZone == "SYZ") zoneName = "SPRING YARD";
	else if (sZone == "SLZ") zoneName = "STAR LIGHT";

	lvInformer = new  LevelInformer(zoneName, act, scr, audio, LevelInformer::T_TITLE_CARD);

	if (sZone == "GHZ") {
		scr.setBgColor(0x08E0);
		bg->create(0x3C, 3);
		bg->addLayer(0,   32, 0.001, 0.5);
		bg->addLayer(32,  16, 0.002, 0.25);
		bg->addLayer(48,  16, 0.003, 0.125);
		bg->addLayer(64,  48, 0.005);
		bg->addLayer(112, 40, 0.060);
		const int waterDiv = 8; 
		for (int i = 0; i < 104/waterDiv; i++)
			bg->addLayer(152+i*waterDiv, waterDiv, 0.060+(float)i/25.0);
	} else if (sZone == "LZ") {
		scr.setBgColor(0x0000);
		bg->create(0x32, 7);
		bg->addLayer(0, 256, 0.005);
	} else if (sZone == "MZ") {
		scr.setBgColor(0x00a0);
		bg->create(0x36, 2);
		bg->addLayer(0, 256, 0.005);
	} else if (sZone == "SBZ") {
		scr.setBgColor(0x6400);
		bg->create(0x34, 4);
		bg->addLayer(0, 256, 0.005);
	} else if (sZone == "SLZ") {
		scr.setBgColor(0x0000);
		bg->create(0x30, 1);
		bg->addLayer(0, 256, 0.005);
	} else if (sZone == "SYZ") {
		scr.setBgColor(0x6260);
		bg->create(0x3D, 2);
		bg->addLayer(0, 256, 0.005);
	}

	// Create player
	Player* pl = new Player(plStartPos, trn, input, audio, rings, score);
	entities.push_back(pl);	

    cam.create(Vector2f(plStartPos.x - scr.getSize().width / 2, 
						plStartPos.y - scr.getSize().height / 2), 
			   trn.getSize());

	// S Tubes
	if (sZone == "GHZ") {
		audio.playMusic(2);
		for (int i = 0; i < trn.getSize().height; i++)
			for (int j = 0; j < trn.getSize().width; j++) {
				int x = j * 256;
				int y = i * 256;
				uint8_t chunk = trn.getChunk(Vector2i(x, y));

				if (chunk == 0x1F) 		
					entities.push_back(new GimGHZ_STube(Vector2f(x+8+8, y+112), 0));
				else if (chunk == 0x20) 
					entities.push_back(new GimGHZ_STube(Vector2f(x+248-8, y+112), 1));
			}
	}
	
	for (it = entities.begin(); it != entities.end(); it++)
		(*it)->create();

	// Level Variables
	ringFrame = 80.0; // Using for same animation to all rings

	tick = 0;
	rings = 0;

	end = false;
	isTimeStopped = false;
	isFadeOut = false;
	isFadeDeath = false;
	fade = 0;

}

void Level::update() {
	if (lvInformer) {
		if (lvInformer->getType() == LevelInformer::T_ROUND_CLEAR &&
			lvInformer->getTick() == 0)
				audio.playMusic(1);

		if (lvInformer->isDone()) {
			if (isTimeStopped)
				isFadeOut = true;
			else {
				delete lvInformer;
				lvInformer = nullptr;
			}
		}
	}

	if (!isFadeDeath) {
		if (!isFadeOut) {
			if (fade < 0xFF)
				fade++;
		} else {
			if (fade > 0)
				fade--;
			else
				end = true;
		}
	}

	if (isFadeDeath) {
		if (fade > 0)
			fade--;
		else {
			score = 0;
			restart();
		}
	}

	if (sAct == "ghz1") {
		if (cam.getPos().x < 0x16D0) 
			cam.setBottomBorder(1024);
		else 
			cam.setBottomBorder(1280);
	} else if (sAct == "ghz2") {
		if (cam.getPos().x > 0x0EE0 && cam.getPos().x < 0x1580) 
			cam.setBottomBorder(0x02E0);
		else 
			cam.setBottomBorder(1536);
	} else if (sAct == "ghz3") {
		if (cam.getPos().x > 0x1680)
			cam.setBottomBorder(0x0400);
		else if (cam.getPos().x < 0x0400)
			cam.setBottomBorder(0x03F0);
		else
			cam.setBottomBorder(1536);
	}

	for (it = entities.begin(); it != entities.end();) {
		Entity* ent = *it;
		if (!ent->isLiving()) {
			it = entities.erase(it);
			delete ent;
		} else {
			it++;
		}
	}

	for (it = entities.begin(); it != entities.end(); it++) {
		if ((*it)->isInCamera(cam) || (*it)->getType() == TYPE_PLAYER) {
			(*it)->update();
			(*it)->reactingToOthers(entities);
			if ((*it)->getType() == TYPE_PLAYER) {
				Player* pl = (Player*)(*it);
				pl->terrainCollision(cam);
				pl->entitiesCollision(entities, cam);
				pl->moveCam(cam);	

				if (pl->isEndLv() && !lvInformer && !isTimeStopped) {
					isTimeStopped = true;
					lvInformer = new LevelInformer("err", act, scr, audio, 
						LevelInformer::T_ROUND_CLEAR, &score, rings, time );
				}

				if (pl->isDied())
					isFadeDeath = true;
			} else if ((*it)->getType() == TYPE_ENEMY) {
				Enemy* en = (Enemy*)(*it);
				en->trnCollision(trn);
			} else if ((*it)->getType() == TYPE_RING) {
				// Ring animation
				Ring* ring = (Ring*)(*it);
				ring->animate(float(ringFrame));
			} 
			
		} else if ((*it)->isLiving()) {
			if ((*it)->getType() == TYPE_PARTICLE) {
				(*it)->destroy();
			}

			(*it)->goToStartPos();
			if ((*it)->getType() == TYPE_PLATFORM) {
				GimGHZ_Platform* plt = (GimGHZ_Platform*)(*it);
				plt->create();
			}
			if ((*it)->getType() == TYPE_RING) {
				Ring* ring = (Ring*)*it;
				if (ring->isBounce())
					ring->destroy();
			}
			if ((*it)->getType() == TYPE_BULLET) {
				(*it)->destroy();
			}
		}
	}

	// Ring animation
	ringFrame += 0.125;
	if (ringFrame >= 84)
		ringFrame = 80;

	// Time count
	tick++;

	if (!isTimeStopped)
		time = tick / 60;

}

void Level::draw() {
	cam.update();

	bg->draw(cam, trn);

	// Terrain
	trn.draw(cam);

	// Entities
	for (it = entities.begin(); it != entities.end(); it++) {
		if ((*it)->isInCamera(cam))
			(*it)->draw(cam);
	}

	drawHud();

	if (lvInformer && isTimeStopped) lvInformer->draw();

	if (fade != 0xFF) {
		sf::RenderWindow& wnd = scr.getSfmlWindow();
		sf::RectangleShape rs(sf::Vector2f(scr.getSize().width, scr.getSize().height));
		rs.setFillColor(sf::Color(0, 0, 0, 0xFF - fade));
		wnd.draw(rs);
	}

	if (lvInformer && !isTimeStopped) lvInformer->draw();
}

void Level::drawHud() {
	scr.drawTextureRect(2, IntRect(1, 60, 40, 16), Vector2f(16, 8));
	scr.drawTextureRect(2, IntRect(42, 60, 32, 16), Vector2f(16, 24));
	scr.drawTextureRect(2, IntRect(75, 60, 40, 16), Vector2f(16, 40));

	char sScore[7];
	char sTime[5];
	char sRings[4];

	snprintf(sScore, 7, "%6d", score);
	snprintf(sTime, 5, "%d:%02d", time / 60, time % 60);
	snprintf(sRings, 4, "%3d", rings);

	scr.drawText(3, sScore, Vector2f(64, 8));
	scr.drawText(3, sTime, Vector2f(56, 24));
	scr.drawText(3, sRings, Vector2f(64, 40));
}

void Level::loadObjects(const char* filename) {
	FILE* f = fopen(filename, "rb");
	if (!f) {
		printf("File load error\n");
		return;
	}

	uint8_t buff[6];
	while (fread(buff, 1, 6, f)) {
		uint16_t x = ((uint16_t)buff[0] << 8) | ((uint16_t)buff[1]);
		uint16_t y = (((uint16_t)buff[2] << 8) & 0x0FFF) | ((uint16_t)buff[3]);

		uint8_t vf = buff[2] >> 7;
		uint8_t hf = (buff[2] & 0b01000000) >> 6 ;

		uint8_t type 	= buff[4] & 0b01111111;
		uint8_t respawn = buff[4] >> 7;

		uint8_t args = buff[5];

		Vector2f oPos = Vector2f((float)x, (float)y);
		Entity* ent = nullptr;
		switch(type) {
			case 0x0D:
				ent = new SignPost(oPos);
				break;
			case 0x11: 
				ent = new GimGHZ_BridgeController(oPos, args, entities);
				break;
			case 0x18:
				switch (args & 0x0F) {
					case 0x00: ent = new GimGHZ_Platform(oPos, false, 0); break;
					case 0x01: ent = new GimGHZ_Platform(oPos, GimGHZ_Platform::DIR_LEFT, true); break;
					case 0x02: ent = new GimGHZ_Platform(oPos, GimGHZ_Platform::DIR_UP, true); break;
					case 0x03: ent = new GimGHZ_Platform(oPos, false, 0, true); break; // Falls when stood on
					//case 0x04: break; // Falls immediately
					case 0x05: ent = new GimGHZ_Platform(oPos, GimGHZ_Platform::DIR_RIGHT, true); break;
					case 0x06: ent = new GimGHZ_Platform(oPos, GimGHZ_Platform::DIR_DOWN, true); break;
					//case 0x07: break; // x7: Rises when switch in high nybble is pressed
					//case 0x08: break; // x8: Rises immediately
					case 0x09: ent = new GimGHZ_Platform(oPos, false, false); break; // x9: Doesn't move
					//case 0x0A: break; // xA: Large - moves up and down
					//case 0x0B: break; // xB: Moves up and down slowly
					//case 0x0C: break; // xC: Moves up and down slowly
					default: ent = new GimGHZ_Platform(oPos, false, false); break;
				}
				break;
			case 0x1A:
				ent = new GimGHZ_SlpPlatform(oPos, entities, (bool)args);
				break;
			case 0x1C:
				ent = new GimGhz_BridgeColumn(oPos, (bool)hf);
				break;
			case 0x1F:
				ent = new EnCrab(oPos, entities);
				break;
			case 0x22:
				ent = new EnBuzz(oPos);
				break;
			case 0x25:
				ent = new Ring(oPos, (args & 0x0F) + 1, ((args & 0xF0) >> 4) - 1, entities);
				break;
			case 0x26:
				switch (args) {
					case 0x02: ent = new Monitor(oPos, Monitor::M_LIVE); break;
					case 0x03: ent = new Monitor(oPos, Monitor::M_SPEED); break;
					case 0x04: ent = new Monitor(oPos, Monitor::M_SHIELD); break;
					case 0x05: ent = new Monitor(oPos, Monitor::M_INVINCIBILITY); break;
					case 0x06: ent = new Monitor(oPos, Monitor::M_RINGS); break;
					default:   ent = new Monitor(oPos, Monitor::M_RINGS); break;
				}
				break;
			case 0x2B:
				ent = new EnChopper(oPos);
				break;
			case 0x36:
				if ((args & 0b00010000) == 0)
					ent = new Spikes(oPos, args, entities);
				break;
			case 0x3B:
				ent = new GimGHZ_Stone(oPos);
				break;
			case 0x3E:
				if (!args) ent = new SignPost(oPos);
				break;
			case 0x40:
				ent = new EnMotobug(oPos);
				break;
			case 0x41: 
				if (((args & 0xF0) >> 4) == 0x01) 
					ent = new Spring(oPos, !(args & 0x0F), 
									 hf ? Spring::R_LEFT : Spring::R_RIGHT);
				else 
					ent = new Spring(oPos, !(args & 0x0F), 
									 vf ? Spring::R_DOWN : Spring::R_UP);
				break;
			case 0x44:
				ent = new GimGHZ_Wall(oPos, args & 0x0F, args & 0xF0);
				break;
			default:
				ent = new Ring(oPos);
				break;
		}

		if (ent)
			entities.push_back(ent);
	}

	fclose(f);
}

bool Level::loadTerrainTiles(const char* fnVer, const char* fnHor, const char* fnAngles) {
	FILE* fVerHeights = fopen(fnVer, "rb");
	FILE* fHorHeights = fopen(fnHor, "rb");
	FILE* fAngles	  = fopen(fnAngles, "rb");
	
	if (!fVerHeights || !fHorHeights || !fAngles) {
		printf("Error loading terrain data\n"
			   "Vertical heights: %s %s\n"
			   "Horizontal heights: %s %s\n"
			   "Angles: %s %s\n",
			   fnVer,    fVerHeights ? "OK" : "FAIL",
			   fnHor,    fHorHeights ? "OK" : "FAIL",
			   fnAngles, fAngles     ? "OK" : "FAIL");
		return false;
	}

	verHeights = new uint8_t[TILE_HEIGHTS_BUFF_SIZE];
	if (!fread(verHeights, 1, TILE_HEIGHTS_BUFF_SIZE, fVerHeights), fclose(fVerHeights)) {
		printf("Error. Ver heights file corrupted\n");
		return false;
	}

	horHeights = new uint8_t[TILE_HEIGHTS_BUFF_SIZE];
	if (!fread(horHeights, 1, TILE_HEIGHTS_BUFF_SIZE, fHorHeights), fclose(fHorHeights)) {
		printf("Error. Hor heights file corrupted\n");
		return false;
	}

	angles = new uint8_t[TILE_ANGLES_BUFF_SIZE];
	if (!fread(angles, 1, TILE_ANGLES_BUFF_SIZE, fAngles), fclose(fAngles)) {
		printf("Error. Angles file corrupted\n");
		return false;
	}

	return true;
}	

bool Level::loadTerrainZone(const char* fn16, const char* fnBig) {
	FILE* f16  = fopen(fn16, "rb");
	FILE* fBig = fopen(fnBig, "rb");

	if (!f16 || !fBig) {
		printf("Error loading Zone data\n"
			   "16x16 mappings: %s %s\n"
			   "256x256 mappings: %s %s\n",
			   fn16,  f16  ? "OK" : "FAIL",
			   fnBig, fBig ? "OK" : "FAIL");
		return false;
	}

	uint32_t numbytes;
	uint8_t* buffer;

	// F16
	fseek(f16, 0L, SEEK_END);
	numbytes = ftell(f16);
	fseek(f16, 0L, SEEK_SET);	
	
	buffer = new uint8_t[numbytes];	
	fread(buffer, 1, numbytes, f16);
	fclose(f16);

	tiles16 = buffer;

	// F256
	fseek(fBig, 0L, SEEK_END);
	numbytes = ftell(fBig);
	fseek(fBig, 0L, SEEK_SET);	
	
	buffer = new uint8_t[numbytes];	
	fread(buffer, 1, numbytes, fBig);
	fclose(fBig);

	tilesBig = new uint16_t[numbytes / 2];
	for (int i = 0; i < numbytes / 2; i++) {
		tilesBig[i] = buffer[i*2];
		tilesBig[i] = (tilesBig[i] << 8) | buffer[i*2+1];
	}
	delete buffer;

	return true;
}

bool Level::loadTerrainAct(const char* fnLayout, const char* fnStartPos) {
	FILE* fLayout   = fopen(fnLayout, "rb");
	FILE* fStartPos = fopen(fnStartPos, "rb");

	if (!fLayout || !fStartPos) {
		printf("Error loading Act data\n"
			   "Layout: %s %s\n"
			   "Start position: %s %s\n",
			   fnLayout,  fLayout  ? "OK" : "FAIL",
			   fnStartPos, fStartPos ? "OK" : "FAIL");
		return false;
	}

	uint32_t numbytes;
	uint8_t* buffer;

	// Layout
	fseek(fLayout, 0L, SEEK_END);
	numbytes = ftell(fLayout);
	fseek(fLayout, 0L, SEEK_SET);	
	
	buffer = new uint8_t[numbytes];	
	fread(buffer, 1, numbytes, fLayout);
	fclose(fLayout);

	tilesLayout = buffer;

	// StartPos
	numbytes = 4;
	buffer = new uint8_t[numbytes];	
	if (!fread(buffer, 1, numbytes, fStartPos)) {
		return false;
	}

	uint16_t x = buffer[0];
	x = (x << 8) | buffer[1];
	uint16_t y = buffer[2];
	y = (y << 8) | buffer[3];

	plStartPos.x = (float)x;
	plStartPos.y = (float)y;

	delete buffer;

	return true;
}

void Level::free() {
	for (it = entities.begin(); it != entities.end();) {
		it = entities.erase(it);
		delete *it;
	}

	delete bg;
	delete lvInformer;
	delete [] verHeights;
	delete [] horHeights;
	delete [] angles;
	delete [] tilesLayout;
	delete [] tiles16;
	delete [] tilesBig;
	scr.freeTexture(255);
}