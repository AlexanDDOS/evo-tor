#include <SFML/Graphics.hpp>
#include <beings.hpp>
#include <string>
#include <fstream>

using namespace std;

const unsigned int BOARD_SIZE = 500;
Being* board[BOARD_SIZE * BOARD_SIZE];
unsigned int boardOcc = 0;

extern void setBeing(int, int, Being*);
extern Being* getBeing(int, int);
extern int torus(int&, int&);
sf::Font renderFont;

sf::Clock sfClock;
int beingNums[3] = {0, 0, 0}; //Количество клеток разного типа
ofstream logFile; //Журнал популяции существ

unsigned long long frameN = 0; //Номер текущего кадра
void recordLog() { //Создать новую запись в журнале популяции существ
    logFile << frameN;
    for (int i = 0; i < 3; i++)
        logFile << '\t' << beingNums[i];
    logFile << endl;
}
void updateAll(){
    for (int i = 0; i < 3; i++) beingNums[i] = 0;
    for (int pr = 0; pr < 5; pr++) {
        for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
            if (board[i] == nullptr || board[i]->lastFrame == frameN) continue;
            //Define type
            sf::Color bc = board[i]->getColor();
            if (bc == Weed::color) beingNums[0]++;
            else if (bc == Fish::color || bc == PlayerFish::color) beingNums[1]++;
            else beingNums[2]++;
            if (pr == board[i]->getPriority()) board[i]->update(frameN);
        }
        if (pr == 0 && frameN % 120 == 0) {
            int x = rand() % BOARD_SIZE;
            int y = rand() % BOARD_SIZE;
            srand(sfClock.getElapsedTime().asMicroseconds());
            if (getBeing(x, y) == nullptr) {
                setBeing(x, y, new Weed); //Создать новую водоросль в случайной позиции
            }
        }
    }
    recordLog();
    frameN++;
}

PlayerFish* plf = new PlayerFish;
Player* pl = plf; //Указатель на игрока
Being* plb = plf; //Указатель на существо игрока
//Параметры окончания игры (устанавливаются дестуруктором игрока)
bool gameOver = false; //Игра окончена? (клетка игрока метрва)
unsigned int finalScore = 0; //Окончательный счёт

sf::Vector2u cameraPos = {0, 0};
sf::Vector2u cameraViewport = {100, 100}; //Количество ячейк, которые захватывает камера
void cameraCenter(int x, int y) { //Центрировать камеру на определённой ячейке
    x -= cameraViewport.x / 2;
    y -= cameraViewport.y / 2;
    torus(x, y); //Нормировать координаты по тору
    cameraPos = sf::Vector2u(x, y);
}
void renderAll(sf::RenderWindow& win, int pixSize = 2) { //Рендерить всё поле
    sf::RectangleShape r;
    r.setSize(sf::Vector2f(pixSize, pixSize));
    for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
        if (board[i] == nullptr) continue;
        r.setFillColor(board[i]->getColor());
        int x = board[i]->x;
        int y = board[i]->y;
        r.setPosition(sf::Vector2f(x, y) * (float)pixSize);
        win.draw(r);
    }
}
void renderCamera(sf::RenderWindow& win, int pixSize = 10) { //Рендерить область, видимую камерой
    sf::RectangleShape r;
    r.setSize(sf::Vector2f(pixSize, pixSize));
    sf::Vector2u cameraEnd = cameraPos + cameraViewport;
    for (int x = cameraPos.x; x < cameraEnd.x; x++) {
        for (int y = cameraPos.y; y < cameraEnd.y; y++) {
            Being* b = getBeing(x, y);
            if (b == nullptr) continue;
            r.setFillColor(b->getColor());
            r.setPosition(sf::Vector2f(x - cameraPos.x, y  - cameraPos.y)  * (float)pixSize);
            win.draw(r);
        }
    }
}

void renderHugeText(sf::RenderWindow& win, sf::Text text, int h, int y) { //Отобразить огромный красный текст по центру поля
    text.setColor(sf::Color::Red);
    text.setOutlineColor(sf::Color::Black);
    text.setOutlineThickness(max(h / 20, 1));
    text.setCharacterSize(h);
    float textWidth = 0;
    for (sf::Uint32 ch: text.getString().toUtf32())
        textWidth += renderFont.getGlyph(ch, h, h / 20).advance;
    text.setPosition(sf::Vector2f(1000 - textWidth, 2 * y - h) / 2.f);
    win.draw(text);
}

int main()
{
    for (int x = 0; x < BOARD_SIZE; x++) {
        for (int y = 0; y < BOARD_SIZE; y++) {
            int chance = rand() % 10000;
            if (chance < 1400)
                setBeing(x + 2, y + 2, new Weed);
            else if (chance < 1650)
                setBeing(x, y, new Fish);
            else if (chance < 1700)
                setBeing(x + 3, y + 3, new Shark);
        }
        srand(sfClock.getElapsedTime().asMicroseconds() + time(NULL));
    }
    sf::RenderWindow window(sf::VideoMode(1200, 1000), "Evo-Tor");
    window.setFramerateLimit(60);
    renderFont.loadFromFile("C:\\Windows\\fonts\\Arial.ttf");
    logFile.open("population.log");
    bool fullMode = false; //Отображать ли всё поле или только его часть?
    setBeing(50, 50, plb);
    pl->updateWays();

    while (window.isOpen())
    {
        bool playerMoved = false;
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                sf::Event::KeyEvent key = event.key;

                if (key.code == sf::Keyboard::Space)
                    fullMode = !fullMode;
                if (!gameOver && pl != nullptr) {
                    if (key.code == sf::Keyboard::A)
                        playerMoved = pl->moveAt(0);
                    if (key.code == sf::Keyboard::D)
                        playerMoved = pl->moveAt(1);
                    if (key.code == sf::Keyboard::W)
                        playerMoved = pl->moveAt(2);
                    if (key.code == sf::Keyboard::S)
                        playerMoved = pl->moveAt(3);
                }
                if (key.code == sf::Keyboard::LShift) //Пропуск хода
                    playerMoved = true;
            }
        }
        if (playerMoved) {
            updateAll();
            if (pl != nullptr) {
                pl->updateWays();
                cameraCenter(plb->x, plb->y);
            }
        }
        window.clear(sf::Color::Black);
        if (fullMode) renderAll(window);
        else renderCamera(window);
        //Отобразить хар-ки игрока
        unsigned int frame = frameN, hunger = 0, score = finalScore, breedsIn = 0;
        if (pl != nullptr) {
            hunger = plb->getHunger();
            score = pl->getScore();
            breedsIn = plb->getBreeding();
        }
        sf::RectangleShape sideLine({5.f, 1000.f});
        sideLine.setPosition(1000, 0);
        window.draw(sideLine);
        sf::Text frameText(L"Кадр: " + std::to_wstring(frame), renderFont);
        frameText.setPosition(1010, 32);
        sf::Text hungerText(L"Голод: " + std::to_wstring(hunger), renderFont);
        hungerText.setPosition(1010, 64);
        sf::Text breedText(L"Размн.: " + std::to_wstring(breedsIn), renderFont);
        breedText.setPosition(1010, 96);
        sf::Text scoreText(L"Счёт: " + std::to_wstring(score), renderFont);
        scoreText.setPosition(1010, 128);
        window.draw(frameText);
        window.draw(hungerText);
        window.draw(breedText);
        window.draw(scoreText);
        if (gameOver) {
            sf::Text goText(L"Игра окончана!", renderFont);
            renderHugeText(window, goText, 72, 500 - 72  * 3 / 4);
            renderHugeText(window, scoreText, 36, 500 + 36 / 4);
        }
        /*for (int i = 0; i < 3; i++) {
            sf::Text bCnt(std::to_string(beingNums[i]), renderFont);
            bCnt.setPosition(1000, 32 * i);
            window.draw(bCnt);
        }*/
        window.display();
    }
    logFile.close();
    return 0;
}
