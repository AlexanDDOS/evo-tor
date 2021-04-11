#include "beings.hpp"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <SFML/System/Clock.hpp>
#include <ctime>

using namespace std;

Being::Being() {
    x = Being::UNTIED_POS;
    y = Being::UNTIED_POS;
    lastFrame = -1;
}
Being::~Being() {
    breedTime = 0;
}
const unsigned int BOARD_SIZE = 500;
extern Being* board[BOARD_SIZE * BOARD_SIZE]; //Игровое поле
extern unsigned int boardOcc;

extern sf::Clock sfClock;

int torus(int& x, int& y) {
    //Искажение пространства по тору и возврат номера ячейки поля
    while (x < 0) x = BOARD_SIZE + x;
    while (y < 0) y = BOARD_SIZE + y;
    x %= BOARD_SIZE;
    y %= BOARD_SIZE;
    return y * BOARD_SIZE + x;
}

Being* getBeing(int x, int y) {
    //Вернуть указатель на сущность в ячейке (x, y)
    int n = torus(x, y);
    return board[n];
}

void delBeing(int x, int y, bool destroy = false) {
    //Удалить указатель на сущность в ячейке (x, y) и вызвать её деструктор при destory = true
    int n = torus(x, y);
    Being* b = board[n];
    if (destroy) {
        delete b;
        board[n] = nullptr;
    }
    board[n] = nullptr;
    boardOcc--;
}

void setBeing(int x, int y, Being* b) {
    //Привязать сущность к ячейке (x, y)
    int n = torus(x, y);
    board[n] = b;
    boardOcc++;
    if (b->x != Being::UNTIED_POS)
        delBeing(b->x, b->y);
    b->x = x;
    b->y = y;
}

void dirToDxDy(int dir, int& dx, int& dy) { //Преобразовать номер направления в вектор направления
    dx = (dir < 2 ? -1 + 2 * (dir & 1): 0);
    dy = (dir > 1 ? -1 + 2 * (dir & 1): 0);
}

template <class T>
T chooseElement(vector<T> v) { //Аналог random.choice() из Python
    int r = rand() % v.size();
    srand(sfClock.getElapsedTime().asMicroseconds() + time(NULL));
    return v[r];
}

void Being::genTrajectories(vector<int>& waysFree, vector<int>& waysWithFood,
                            std::vector<sf::Color> victimColors, int visionDist) {
    //Общий алгоритм просчёта траекторий для рыб и акул
    for (int dir = 0; dir < 4; dir++) {
        bool eatsWeed = false;
        for (int dist = 1; dist <= visionDist; dist++) {
            int dx, dy;
            dirToDxDy(dir, dx, dy);
            dx *= dist;
            dy *= dist;
            Being* b = getBeing(x + dx, y + dy);
            if (b == nullptr) waysFree.push_back(dir);
            else {
                sf::Color bc = b->getColor();
                bool edible = false;
                for (sf::Color c: victimColors) {
                    if (c == Weed::color) eatsWeed = true;
                    if (c == bc) edible = true;
                }
                if (edible) waysWithFood.push_back(dir);
                else if (bc == Weed::color && !eatsWeed)
                    waysFree.push_back(dir);
                else goto stopSeeing;
            }
        }
        stopSeeing: continue;
    }
}

void Being::chooseTrajectory(int& dx, int& dy, std::vector<sf::Color> victimColors, int visionDist) {
    vector<int> waysWithFood, waysFree;
    Being::genTrajectories(waysFree, waysWithFood, victimColors, visionDist);
    if (!waysWithFood.empty())
        dirToDxDy(chooseElement(waysWithFood), dx, dy);
    else if (waysFree.empty()) {
        dx = 0;
        dy = 0;
    } else
        dirToDxDy(chooseElement(waysFree), dx, dy);
}

unsigned int Being::getHunger(){
    return hunger;
}

unsigned int Being::getBreeding(){
    return breedTime;
}

Weed::Weed() : Being::Being() {
    breedTime = maxBreedTime;
    hunger = maxHunger;
}

sf::Color Weed::color = sf::Color::Green;
sf::Color Weed::getColor() {
    return color;
}
int Weed::getPriority() {
    return PRIORITY;
}

void Weed::update(unsigned long long frame) {
    if (hunger > 0) hunger--;
    else {
        delBeing(x, y, true); //Die
        return;
    }
    if (breedTime > 0) breedTime--;
    else {
        for (int dir = 0; dir < 4; dir++) {
            int dx, dy;
            dirToDxDy(dir, dx, dy);
            if (getBeing(x + dx, y + dy) != nullptr)
                continue;
            bool chance = rand() % 4;
            srand(sfClock.getElapsedTime().asMicroseconds() + time(NULL));
            if (chance != 0)
                continue;
            Weed* child = new Weed();
            child->lastFrame = frame;
            setBeing(x + dx, y + dy, child);
        }
    }
    lastFrame = frame;
}

Fish::Fish() : Being::Being() {
    hunger = maxHunger;
    breedTime = maxBreedTime;
}
sf::Color Fish::color = sf::Color::Cyan;
sf::Color Fish::getColor() {
    return color;
}
int Fish::getPriority() {
    return PRIORITY;
}

void Fish::chooseTrajectory(int &dx, int &dy) {
    Being::chooseTrajectory(dx, dy, {Weed::color}, 3);
}

void Fish::update(unsigned long long frame) {
    if (breedTime > 0) breedTime--;
    if (hunger > 0) hunger--;
    else {
        delBeing(x, y, true); //Die
        return;
    }
    int dx, dy;
    chooseTrajectory(dx, dy);
    bool canMove = (dx != 0 || dy != 0);
    if (canMove) {
        int ox = x, oy = y;
        Being* toCell = getBeing(x + dx, y + dy);
        if (toCell != nullptr && toCell->getColor() == Weed::color) {
            delBeing(x + dx, y + dy, true);
            hunger = maxHunger;
        }
        setBeing(x + dx, y + dy, this);
        if (breedTime <= 0) { //Breeding
            Fish* child = new Fish();
            child->lastFrame = frame;
            setBeing(ox, oy, child);
            breedTime = maxBreedTime;
        }
    }
    lastFrame = frame;
}

Shark::Shark() : Being::Being() {
    hunger = maxHunger;
    breedTime = maxBreedTime;
}

sf::Color Shark::color = sf::Color::Red;
sf::Color Shark::getColor() {
    return color;
}
int Shark::getPriority() {
    return PRIORITY;
}

void Shark::chooseTrajectory(int &dx, int &dy) {
    Being::chooseTrajectory(dx, dy, {Fish::color, PlayerFish::color}, 10);
}

void Shark::update(unsigned long long frame) {
    if (breedTime > 0) breedTime--;
    if (hunger > 0) hunger--;
    else {
        delBeing(x, y, true); //Die
        return;
    }
    int dx, dy;
    chooseTrajectory(dx, dy);
    bool canMove = (dx != 0 || dy != 0);
    if (canMove) {
        int ox = x, oy = y;
        Being* toCell = getBeing(x + dx, y + dy);
        if (toCell != nullptr) {
            sf::Color tcc = toCell->getColor();
            if (tcc == Fish::color || tcc == PlayerFish::color) {
                delBeing(x + dx, y + dy, true);
                hunger = maxHunger;
            }
        }
        setBeing(x + dx, y + dy, this);
        if (breedTime <= 0) { //Breeding
            Shark* child = new Shark();
            child->lastFrame = frame;
            setBeing(ox, oy, child);
            breedTime = maxBreedTime;
        }
    }
    lastFrame = frame;
}

extern bool gameOver;
extern Player* pl;
extern Being* plb;
extern unsigned int finalScore;
Player::~Player(){
    gameOver = true;
    finalScore = score;
    pl = nullptr;
    plb = nullptr;
}

unsigned int Player::getScore(){
    return score;
}

PlayerFish::PlayerFish() : Fish::Fish() {
    score = 0;
}

sf::Color PlayerFish::color = sf::Color::Blue;
sf::Color PlayerFish::getColor() {
    return color;
}
int PlayerFish::getPriority() {
    return PRIORITY;
}

bool PlayerFish::moveAt(int dir) {
    for (int adir: availableWays) {
        if (adir != dir) continue;
        int ox = x, oy = y, dx, dy;
        dirToDxDy(dir, dx, dy);
        Being* toCell = getBeing(x + dx, y + dy);
        if (toCell != nullptr && toCell->getColor() == Weed::color) {
            delBeing(x + dx, y + dy, true);
            hunger = maxHunger;
            score++;
        }
        setBeing(x + dx, y + dy, this);
        if (breedTime <= 0) { //Breeding
            Fish* child = new Fish();
            child->lastFrame = lastFrame + 1;
            setBeing(ox, oy, child);
            breedTime = maxBreedTime;
            score += 10;
        }
        return true;
    }
    return false;
}

void PlayerFish::update(unsigned long long frame) {
    if (breedTime > 0) breedTime--;
    if (hunger > 0) hunger--;
    else {
        delBeing(x, y, true); //Die
        return;
    }
    lastFrame = frame;
}

void PlayerFish::updateWays(){
    vector<int> w1, w2;
    genTrajectories(w1, w2, {Weed::color}, 1);
    for (int i: w2) w1.push_back(i);
    availableWays = w1;
}
