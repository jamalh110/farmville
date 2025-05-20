#include "FarmLogic.h"
#include "displayobject.hpp"
#include <unistd.h>
#include <thread>
#include <cstdlib>
#include <ctime>

void FarmLogic::run() {
    BakeryStats stats;

    std::srand(std::time(0));
    DisplayObject chicken("chicken", 80, 80, 1, 0);
    DisplayObject chicken2("chicken", 40, 40, 1, 1);
    chicken.setPos(400, 300);
    chicken2.setPos(400, 300);
    chicken.updateFarm();
    chicken2.updateFarm();
    DisplayObject::redisplay(stats);
    
    int frame = 0;
    int randomNumberX = (std::rand() % 11) - 5;
    int randomNumberY = (std::rand() % 11) - 5;
    while (true) {
        frame++;
        if(frame % 5 == 0) {
            randomNumberX = (std::rand() % 11) - 5; // Generate a random number between -5 and 5
            randomNumberY = (std::rand() % 11) - 5; // Generate a random number between -5 and 5
        }

        // move our local copies:
        chicken.setPos(chicken.x + randomNumberX, chicken.y  + randomNumberY);
        chicken2.setPos(chicken2.x + randomNumberX, chicken2.y + randomNumberY);
        chicken.updateFarm();
        chicken2.updateFarm();
        //DisplayObject::redisplay(stats);
        // sleep for 1 second
        usleep(1000000);
    }
}


void FarmLogic::start() {
    std::thread([]() {
       FarmLogic::run();
    })
    .detach();
}