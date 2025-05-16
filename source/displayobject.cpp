#include "displayobject.hpp"

//std::unordered_map<int, DisplayObject> DisplayObject::theFarm[NLAYERS];
std::array<std::unordered_map<int, DisplayObject>, DisplayObject::NLAYERS> DisplayObject::theFarm{};
std::shared_ptr<std::array<std::unordered_map<int, DisplayObject>,DisplayObject::NLAYERS>>
    DisplayObject::buffedFarmPointer{std::make_shared<decltype(theFarm)>()};


DisplayObject::DisplayObject(const std::string& str, const int w, const int h, const int l, const int i)
{
	x = 0;
	y = 0;
	texture = str;
	layer = l;
	width = w;
	height = h;
	id = i;
	updateFarm();
	//theFarm[layer][id] = *this;
}

DisplayObject::DisplayObject(const DisplayObject& from, const int i)
{
	id = i;
	width = from.width;
	height = from.height;
	layer = from.layer;
	x = from.y;
	x = from.y;
	texture = from.texture;
	updateFarm();
	//theFarm[layer][id] = *this;
}

DisplayObject::~DisplayObject()
{
	//theFarm[layer].erase(id);
}
void DisplayObject::updateFarm()
{
	//TODO: should store pointers or copies in theFarm?
	auto res = theFarm[layer].insert({id, *this});
	if (!res.second) {
		res.first->second = *this;
	}
}
void DisplayObject::setPos(int x, int y)
{
	
	//isUpdating = true;
	this->x = x;
	this->y = y;
	//isUpdating = false;
	updateFarm();

}
void DisplayObject::setTexture(const std::string& str)
{
	//isUpdating = true;
	texture = str;
	//isUpdating = false;
	updateFarm();
}

void DisplayObject::redisplay()
{
	auto snapshot = std::make_shared<std::array<std::unordered_map<int,DisplayObject>,NLAYERS>>(theFarm);
	std::atomic_store_explicit(
		&buffedFarmPointer,
		snapshot,
		std::memory_order_release);
  
}
