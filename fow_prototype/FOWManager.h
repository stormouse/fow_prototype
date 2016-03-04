#pragma once

#include <list>
using namespace std;

struct Entity
{
	int lbx;
	int lby;
	float w;
	float h;
};

struct V4
{
	float r, g, b;
	// r = 1 : revealed last frame
	// g = 1 : will be revealed this frame
	// b = 1 : nothing
};

class FOWManager
{
public:
	FOWManager(int mapWidth, int mapHeight);
	void Update();
	void SetHeroPosition(int x, int y);
	void SetSight(int r);
	void AddEntity(Entity e);
	V4** Map() const;
private:
	//float distance(int x1, int y1, int x2, int y2);
	float sqrDistance(float x1, float y1, float x2, float y2);
	bool intersect(float x11, float y11, float x12, float y12, float x21, float y21, float x22, float y22);

	list<Entity> _entityList;
	V4 **_map;
	int _mapWidth;
	int _mapHeight;
	int _sight;
	int _heroX, _heroY;
};

FOWManager::FOWManager(int mapWidth, int mapHeight)
{
	_map = new V4*[mapHeight];
	for (int i = 0; i < mapHeight; i++)
	{
		_map[i] = new V4[mapWidth];
		for (int j = 0; j < mapWidth; j++)
			_map[i][j].r = _map[i][j].g = _map[i][j].b = 0;
	}

}

void FOWManager::SetHeroPosition(int x, int y)
{
	_heroX = x;
	_heroY = y;
}

void FOWManager::SetSight(int r)
{
	_sight = r;
}

void FOWManager::AddEntity(Entity e)
{
	_entityList.push_back(e);
}

void FOWManager::Update()
{
	list<Entity>::iterator it;
	
	for (int i = 0; i < _mapHeight; i++)
	{
		for (int j = 0; j < _mapWidth; j++)
		{
			_map[i][j].r = _map[i][j].g;
			_map[i][j].g = 0.0f;
		}
	}

	// first version, use plain search
	for (int i = _heroY - _sight; i < _heroY + _sight; i++)
	{
		if (i < 0) continue;
		if (i >= _mapHeight) break;
		for (int j = _heroX - _sight; j < _heroX + _sight; j++)
		{
			if (j < 0) continue;
			if (j >= _mapWidth) break;

			float dis = sqrDistance(j, i, _heroX, _heroY);
			for (it = _entityList.begin(); it != _entityList.end(); it++)
			{
				float obsdis = sqrDistance(it->lbx + it->w / 2, it->lby + it->h / 2, _heroX, _heroY);
				if (obsdis > dis)continue;

				float x11 = it->lbx,
					y11 = it->lby,
					x12 = it->lbx + it->w,
					y12 = it->lby + it->h;

				bool its1 = intersect(x11, y11, x12, y12, it->lbx + it->w / 2, it->lby + it->h / 2, _heroX, _heroY);
				bool its2 = intersect(x12, y11, x11, y12, it->lbx + it->w / 2, it->lby + it->h / 2, _heroX, _heroY);

				if (its1 || its2)
				{
					// remain what it was.
				}
				else
				{
					_map[i][j].g = 1.0f;
				}
			}
		}

	}
}

V4** FOWManager::Map() const
{
	return _map;
}

float FOWManager::sqrDistance(float x1, float y1, float x2, float y2)
{
	return (x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1);
}

bool FOWManager::intersect(float x11, float y11, float x12, float y12, float x21, float y21, float x22, float y22)
{
	// cross product : a.x * b.y - a.y * b.x;
	float ab_x = x12 - x11;
	float ab_y = y12 - y11;
	float cd_x = x22 - x21;
	float cd_y = y22 - y21;
	float ca_x = x11 - x21;
	float ca_y = y11 - y21;
	float ac_x = x21 - x11;
	float ac_y = y21 - y11;
	float cb_x = x12 - x21;
	float cb_y = y12 - y21;
	float ad_x = x22 - x11;
	float ad_y = y22 - y11;

	if (
		(cd_x * ca_y - cd_y * ca_x)*(cd_x * cb_y - cd_y * cb_x) <= 0 &&
		(ac_x * ab_y - ac_y * ab_x)*(ad_x * ab_y - ad_y * ab_x) <= 0
		)
	{
		return true;
	}

	return false;
}