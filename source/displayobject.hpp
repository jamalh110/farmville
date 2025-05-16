#include <iostream>
#include <list>
#include <unordered_map>

#pragma once
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
	DisplayObject(const DisplayObject&, const int);
	~DisplayObject();

	static void redisplay();

	static const int WIDTH = 800;
	static const int HEIGHT = 600;
	static const int NLAYERS = 4;

	//TODO: should store pointers or copies in theFarm?
	static std::array<std::unordered_map<int, DisplayObject>,NLAYERS> theFarm;
	static std::shared_ptr<std::array<std::unordered_map<int, DisplayObject>,NLAYERS>> buffedFarmPointer;
	
private:
	void updateFarm();
};
