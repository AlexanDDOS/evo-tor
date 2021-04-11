#ifndef BEINGS_HPP
#define BEINGS_HPP
#include <SFML/Graphics/Color.hpp>
#include <vector>

class Being //Абстрактный класс для игровых сущностей
{
protected:
    static const unsigned int maxHunger = 0;
    static const unsigned int maxBreedTime = 0;
    unsigned int hunger;
    unsigned int breedTime;
    void genTrajectories(std::vector<int>& waysFree, std::vector<int>& waysWithFood,
                         std::vector<sf::Color> victimColors, int visionDist);
    void chooseTrajectory(int& dx, int& dy, std::vector<sf::Color> victimColors, int visionDist);
public:
    static const int PRIORITY = 0; //Приоретет в обработке
    static const int UNTIED_POS = -1; //Значение координат для непривязанных к сетке объектов
    int x;
    int y;
    unsigned long long lastFrame; //Номер последнего обработанного кадра (чтобы избежать повторной обработки уже обработанной сущности
    Being();
    virtual ~Being();
    unsigned int getHunger();
    unsigned int getBreeding();
    virtual sf::Color getColor() = 0;
    virtual int getPriority() = 0;
    virtual void update(unsigned long long) = 0;
    //virtual ~Being() = 0;
};

class Weed: public Being { //Класс водорослей
protected:
    static const unsigned int maxHunger = 45;
    static const unsigned int maxBreedTime = 30;
    static sf::Color victimColor;
public:
    Weed();
    static sf::Color color;
    int getPriority();
    sf::Color getColor();
    void update(unsigned long long);
};

class Fish: public Being { //Класс для травоядных рыб
private:
    void chooseTrajectory(int& dx, int& dy);
protected:
    static const unsigned int maxHunger = 45;
    static const unsigned int maxBreedTime = 60;
public:
    static const int PRIORITY = 1;
    Fish();
    static sf::Color color;
    int getPriority();
    sf::Color getColor();
    void update(unsigned long long);
};

class Shark: public Being { //Класс для акул
private:
    void chooseTrajectory(int& dx, int& dy);
protected:
    static const unsigned int maxHunger = 120;
    static const unsigned int maxBreedTime = 150;
public:
    const int PRIORITY = 3;
    Shark();
    static sf::Color color;
    int getPriority();
    sf::Color getColor();
    void update(unsigned long long);
};

class Player { //Класс существа игрока
protected:
    unsigned int score;
    std::vector<int> availableWays;
public:
    virtual ~Player();
    virtual bool moveAt(int dir) = 0;
    virtual void updateWays() = 0;
    void addScore(int points);
    unsigned int getScore();
};

class PlayerFish: public Fish, public Player { //Класс рыбы, управляемой игроком
public:
    static const int PRIORITY = 2;
    PlayerFish();
    static sf::Color color;
    sf::Color getColor();
    int getPriority();
    bool moveAt(int dir);
    void update(unsigned long long);
    void updateWays();
};

/*class PlayerShark: public Shark, public Player { //Класс акулы, управляемой игроком
public:
    static const int PRIORITY = 4;
    PlayerShark();
    static sf::Color color;
    sf::Color getColor();
    int getPriority();
    bool moveAt(int dir);
    void update(unsigned long long);
    void updateWays();
};*/

#endif
