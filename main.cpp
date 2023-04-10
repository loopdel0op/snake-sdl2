#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

// Ses efekti
Mix_Chunk* eatSound = nullptr;

// Yem yendiğinde ses efekti çal
void eatFood() {
    if (Mix_PlayChannel(-1, eatSound, 0) == -1) {
        std::cerr << "Mix_PlayChannel Error: " << Mix_GetError() << std::endl;
    }
}


// Background music
Mix_Music* backgroundMusic = nullptr;


// Ekran boyutları
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// Yılanın boyutları
const int SNAKE_WIDTH = 20;
const int SNAKE_HEIGHT = 20;

// Yılanın hareket hızı
const int SNAKE_SPEED = 20;

// Yem boyutları
const int FOOD_WIDTH = 20;
const int FOOD_HEIGHT = 20;

// Oyun penceresi
SDL_Window* window = nullptr;

// Oyun ekranı
SDL_Surface* screenSurface = nullptr;

// Yılanın rengi
SDL_Color snakeColor = {0, 255, 0, 255};

// Yılanın segmentleri
std::vector<SDL_Rect> snake;

// Yem
SDL_Rect food;

// Yönler
enum class Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

// Yılanın yönü
Direction direction = Direction::RIGHT;

// Yem oluşturma
void createFood() {
    int x = rand() % (SCREEN_WIDTH / FOOD_WIDTH);
    int y = rand() % (SCREEN_HEIGHT / FOOD_HEIGHT);

    food.x = x * FOOD_WIDTH;
    food.y = y * FOOD_HEIGHT;

    eatFood();
}

// Oyunun başlatılması
bool init() {

	// Initialize SDL Mixer library
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
	    std::cerr << "Mix_OpenAudio Error: " << Mix_GetError() << std::endl;
	    return false;
	}

    // Load the background music file
    backgroundMusic = Mix_LoadMUS("./background/background.wav");
    if (backgroundMusic == nullptr) {
    std::cerr << "Mix_LoadMUS Error: " << Mix_GetError() << std::endl;
    	return false;
    }




    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    screenSurface = SDL_GetWindowSurface(window);

    return true;
}

// Ekranı güncelleme
void updateScreen() {
    SDL_FillRect(screenSurface, nullptr, SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0x00));

    // Yılanı çiz
    for (auto segment : snake) {
        SDL_Rect rect = {segment.x, segment.y, SNAKE_WIDTH, SNAKE_HEIGHT};
        SDL_FillRect(screenSurface, &rect, SDL_MapRGB(screenSurface->format, snakeColor.r, snakeColor.g, snakeColor.b));
    }

    // Yemi çiz
    SDL_Rect rect = {food.x, food.y, FOOD_WIDTH, FOOD_HEIGHT};
    SDL_FillRect(screenSurface, &rect, SDL_MapRGB(screenSurface->format, 255, 0, 0));

    SDL_UpdateWindowSurface(window);
}

// Yılanı hareket ettirme
void moveSnake() {
    // Yılanın kuyruğunu güncelle
    SDL_Rect tail = snake.back();
    for (int i = snake.size() - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }

    // Yılanın başını hareket ettir
        SDL_Rect& head = snake.front();

    switch (direction) {
        case Direction::UP:
            head.y -= SNAKE_SPEED;
            break;
        case Direction::DOWN:
            head.y += SNAKE_SPEED;
            break;
        case Direction::LEFT:
            head.x -= SNAKE_SPEED;
            break;
        case Direction::RIGHT:
            head.x += SNAKE_SPEED;
            break;
        default:
            break;
    }

    // Yılanın pencerenin dışına çıkmasını engelleme
    if (head.x < 0 || head.y < 0 || head.x + SNAKE_WIDTH > SCREEN_WIDTH || head.y + SNAKE_HEIGHT > SCREEN_HEIGHT) {
        std::cout << "Game over!" << std::endl;
        exit(0);
    }

    // Yem yendi mi?
    if (head.x == food.x && head.y == food.y) {
        snake.push_back(tail);
        createFood();
    }

    // Yılanın kendi kendine çarpışması
    for (int i = 1; i < snake.size(); i++) {
        if (head.x == snake[i].x && head.y == snake[i].y) {
            std::cout << "Game over!" << std::endl;
            exit(0);
        }
    }
}

// Yönetici girdileri işleme
void handleInput() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                exit(0);
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        if (direction != Direction::DOWN) {
                            direction = Direction::UP;
                        }
                        break;
                    case SDLK_DOWN:
                        if (direction != Direction::UP) {
                            direction = Direction::DOWN;
                        }
                        break;
                    case SDLK_LEFT:
                        if (direction != Direction::RIGHT) {
                            direction = Direction::LEFT;
                        }
                        break;
                    case SDLK_RIGHT:
                        if (direction != Direction::LEFT) {
                            direction = Direction::RIGHT;
                        }
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
}

// Oyun döngüsü
void gameLoop() {

	// Start playing the background music
	if (Mix_PlayMusic(backgroundMusic, -1) < 0) {
	    std::cerr << "Mix_PlayMusic Error: " << Mix_GetError() << std::endl;
	}

    srand(time(nullptr));

    eatSound = Mix_LoadWAV("eat.wav");
    if (eatSound == nullptr) {
	std::cerr << "Mix_LoadWAV Error: " << Mix_GetError() << std::endl;
    exit(1);
    }

    // Başlangıçta 3 segmentli yılan oluştur
    SDL_Rect segment = {SCREEN_WIDTH / 2 - SNAKE_WIDTH, SCREEN_HEIGHT / 2, SNAKE_WIDTH, SNAKE_HEIGHT};
    snake.push_back(segment);

    segment = {SCREEN_WIDTH / 2 - SNAKE_WIDTH * 2, SCREEN_HEIGHT / 2, SNAKE_WIDTH, SNAKE_HEIGHT};
    snake.push_back(segment);

    segment = {SCREEN_WIDTH / 2 - SNAKE_WIDTH * 3, SCREEN_HEIGHT / 2, SNAKE_WIDTH, SNAKE_HEIGHT};
    snake.push_back(segment);

    createFood();

    while (true) {
        handleInput();

        moveSnake();

        updateScreen();

        SDL_Delay(100);
    }
}

// Oyunun sonlandırılması
void quit() {
    Mix_FreeMusic(backgroundMusic);
    Mix_CloseAudio();	
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// Ana fonksiyon
int main(int argc, char* argv[]) {
    if (!init()) {
        std::cerr << "Initialization failed" << std::endl;
        return 1;
    }


    // Ses özelliklerini ayarla
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "Mix_OpenAudio Error: " << Mix_GetError() << std::endl;
        return 1;
    }

    gameLoop();

    quit();

    return 0;
}


