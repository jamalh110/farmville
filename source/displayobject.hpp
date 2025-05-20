#include <iostream>
#include <list>
#include <unordered_map>

#pragma once


struct BakeryStats {
    int eggs_laid       = 0;
    int eggs_used       = 0;
    int butter_produced = 0;
    int butter_used     = 0;
    int sugar_produced  = 0;
    int sugar_used      = 0;
    int flour_produced  = 0;
    int flour_used      = 0;
    int cakes_produced  = 0;
    int cakes_sold      = 0;
};

class DisplayObject {
public:
	int  width;
	int  height;
	int  layer;
	int  x;
	int  y;
	int  id;
	bool isUpdating = false;
	std::string texture;

	void setPos(int, int);
	void setTexture(const std::string&);

	DisplayObject(const std::string&, const int, const int, const int, const int);
	~DisplayObject();
	void updateFarm();
	void erase();

	static void redisplay(BakeryStats& stats);

	static const int WIDTH = 800;
	static const int HEIGHT = 600;
	static const int NLAYERS = 2;

	//TODO: should store pointers or copies in theFarm?
	static std::unordered_map<int, DisplayObject> theFarm;
	static std::shared_ptr<std::unordered_map<int, DisplayObject>> buffedFarmPointer;
	static BakeryStats stats;
private:
	
};
