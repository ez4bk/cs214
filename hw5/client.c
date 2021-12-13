
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>


#ifndef _SCAFFOLDING
#define _SCAFFOLDING

typedef struct _Node {
    void *ptr;
    struct _Node *next;
} Node;


Node *create_node(void *ptr);
void append_node(Node *head, void *ptr);
void delete_node(Node *head, Node *to_delete);

void assert(bool condition, char* message);
bool warning(bool condition, char* message);


#define GRIDSIZE 10

typedef struct _Position {
    int x;
    int y;
} Position;

typedef enum _TILETYPE {
    TILE_GRASS = 0,
    TILE_TOMATO,
    DUMMY,
    PLAYER
} TILETYPE;

typedef struct _Game {
    int level;
    TILETYPE grid[GRIDSIZE][GRIDSIZE];
    Node *players;
} Game;

typedef struct _Player {
    int player_fd;
    Position position;
    int score;
} Player;

typedef struct _Command {
    int uid;
    int client_id;
    int command_id;
    int is_ack;
    char argument_str[128];
    int argument_int_0;
    int argument_int_1;
} Command;

// Commands 
#define COMMAND_SYNC_MAP 0          // argument_str: map [0, 1]
#define COMMAND_MOVE_PLAYER 1       // argument_str: direction
#define COMMAND_SYNC_LEVEL 2        // argument_int_0: level
#define COMMAND_SYNC_SCORE 3        // argument_int_0: score
#define COMMAND_END_GAME 4          // 
#define COMMAND_JOIN_GAME 5         // 

// #define COMMAND_ADD_PLAYER 1     // argument_int_0: player_id
// #define COMMAND_REMOVE_PLAYER 2  // argument_int_0: player_id
// #define COMMAND_REMOVE_TOMATO 4  // argument_int_0: x, argument_int_1: y
// #define COMMAND_CHANGE_LEVEL 6   // argument_int_0: new level

/*
    <- JOIN_GAME
    -> INIT_MAP
    -> ADD_PLAYER
    -> ...
    
    <- END_GAME
    -> END_GAME
*/

#endif

















// Dimensions for the drawn grid (should be GRIDSIZE * texture dimensions)
#define GRID_DRAW_WIDTH 640
#define GRID_DRAW_HEIGHT 640

#define WINDOW_WIDTH GRID_DRAW_WIDTH
#define WINDOW_HEIGHT (HEADER_HEIGHT + GRID_DRAW_HEIGHT)

// Header displays current score
#define HEADER_HEIGHT 50

// Number of cells vertically/horizontally in the grid
#define GRIDSIZE 10

SDL_Renderer* renderer;

TILETYPE grid[GRIDSIZE][GRIDSIZE];

Position playerPosition;
int score = 0;
int level = 0;
int numTomatoes;

bool shouldExit = false;

bool flag_grid_setuped = false;

TTF_Font* font;

int connection_fd;


// get a random value in the range [0, 1]
double rand01()
{
    return (double) rand() / (double) RAND_MAX;
}

void _initGrid()
{
    for (int i = 0; i < GRIDSIZE; i++) {
        for (int j = 0; j < GRIDSIZE; j++) {
            double r = rand01();
            if (r < 0.1) {
                grid[i][j] = TILE_TOMATO;
                numTomatoes++;
            }
            else
                grid[i][j] = TILE_GRASS;
        }
    }

    // force player's position to be grass
    if (grid[playerPosition.x][playerPosition.y] == TILE_TOMATO) {
        grid[playerPosition.x][playerPosition.y] = TILE_GRASS;
        numTomatoes--;
    }

    // ensure grid isn't empty
    while (numTomatoes == 0)
        _initGrid();
}

void initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "initSDL: Error initializing SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    int rv = IMG_Init(IMG_INIT_PNG);
    if ((rv & IMG_INIT_PNG) != IMG_INIT_PNG) {
        fprintf(stderr, "initSDL: Error initializing IMG: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }

    if (TTF_Init() == -1) {
        fprintf(stderr, "initSDL: Error initializing TTF: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
}

void moveTo(int x, int y)
{
    // Prevent falling off the grid
    if (x < 0 || x >= GRIDSIZE || y < 0 || y >= GRIDSIZE)
        return;

    // Sanity check: player can only move to 4 adjacent squares
    if (!(abs(playerPosition.x - x) == 1 && abs(playerPosition.y - y) == 0) &&
        !(abs(playerPosition.x - x) == 0 && abs(playerPosition.y - y) == 1)) {
        fprintf(stderr, "moveTo: Invalid move attempted from (%d, %d) to (%d, %d)\n", playerPosition.x, playerPosition.y, x, y);
        return;
    }

    playerPosition.x = x;
    playerPosition.y = y;

    if (grid[x][y] == TILE_TOMATO) {
        grid[x][y] = TILE_GRASS;
        score++;
        numTomatoes--;
        if (numTomatoes == 0) {
            level++;
            _initGrid();
        }
    }
}

void send_move_command(char direction){
    Command command;
    bzero(&command, sizeof(Command));
    command.command_id = COMMAND_MOVE_PLAYER;
    command.argument_str[0] = direction;
    send(connection_fd, &command, sizeof(Command), 0);
}

void handleKeyDown(SDL_KeyboardEvent* event)
{
    // ignore repeat events if key is held down
    if (event->repeat)
        return;

    if (event->keysym.scancode == SDL_SCANCODE_Q || event->keysym.scancode == SDL_SCANCODE_ESCAPE)
        shouldExit = true;

    if (event->keysym.scancode == SDL_SCANCODE_UP || event->keysym.scancode == SDL_SCANCODE_W)
        //moveTo(playerPosition.x, playerPosition.y - 1);
        send_move_command('w');

    if (event->keysym.scancode == SDL_SCANCODE_DOWN || event->keysym.scancode == SDL_SCANCODE_S)
        //moveTo(playerPosition.x, playerPosition.y + 1);
        send_move_command('s');

    if (event->keysym.scancode == SDL_SCANCODE_LEFT || event->keysym.scancode == SDL_SCANCODE_A)
        //moveTo(playerPosition.x - 1, playerPosition.y);
        send_move_command('a');

    if (event->keysym.scancode == SDL_SCANCODE_RIGHT || event->keysym.scancode == SDL_SCANCODE_D)
        //moveTo(playerPosition.x + 1, playerPosition.y);
        send_move_command('d');
}

void processInputs()
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				shouldExit = true;
				break;

            case SDL_KEYDOWN:
                handleKeyDown(&event.key);
				break;

			default:
				break;
		}
	}
}

void drawGrid(SDL_Renderer* renderer, SDL_Texture* grassTexture, SDL_Texture* tomatoTexture, SDL_Texture* playerTexture)
{
    SDL_Rect dest;
    for (int i = 0; i < GRIDSIZE; i++) {
        for (int j = 0; j < GRIDSIZE; j++) {
            dest.x = 64 * i;
            dest.y = 64 * j + HEADER_HEIGHT;
            //SDL_Texture* texture = (grid[i][j] == TILE_GRASS) ? grassTexture : tomatoTexture;
            SDL_Texture* texture;
            
            switch(grid[i][j]) {
                case TILE_GRASS: texture = grassTexture; break;
                case TILE_TOMATO: texture = tomatoTexture; break;
                case PLAYER: texture = playerTexture; break;
                default:
                    fprintf(stderr, "drawGrid: Invalid tile type: %d\n", grid[i][j]);
                    texture = grassTexture; break;
            }

            SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);
            SDL_RenderCopy(renderer, texture, NULL, &dest);
        }
    }

    // dest.x = 64 * playerPosition.x;
    // dest.y = 64 * playerPosition.y + HEADER_HEIGHT;
    // SDL_QueryTexture(playerTexture, NULL, NULL, &dest.w, &dest.h);
    // SDL_RenderCopy(renderer, playerTexture, NULL, &dest);
}

void drawUI(SDL_Renderer* renderer)
{
    // largest score/level supported is 2147483647
    char scoreStr[18];
    char levelStr[18];
    sprintf(scoreStr, "Score: %d", score);
    sprintf(levelStr, "Level: %d", level);

    SDL_Color white = {255, 255, 255};
    SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreStr, white);
    SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);

    SDL_Surface* levelSurface = TTF_RenderText_Solid(font, levelStr, white);
    SDL_Texture* levelTexture = SDL_CreateTextureFromSurface(renderer, levelSurface);

    SDL_Rect scoreDest;
    TTF_SizeText(font, scoreStr, &scoreDest.w, &scoreDest.h);
    scoreDest.x = 0;
    scoreDest.y = 0;

    SDL_Rect levelDest;
    TTF_SizeText(font, levelStr, &levelDest.w, &levelDest.h);
    levelDest.x = GRID_DRAW_WIDTH - levelDest.w;
    levelDest.y = 0;

    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreDest);
    SDL_RenderCopy(renderer, levelTexture, NULL, &levelDest);

    SDL_FreeSurface(scoreSurface);
    SDL_DestroyTexture(scoreTexture);

    SDL_FreeSurface(levelSurface);
    SDL_DestroyTexture(levelTexture);
}



void command_handler_cli(Command command, int connection_fd) {
    switch(command.command_id) {
        case COMMAND_SYNC_MAP:{
            printf("command_handler_cli: Sync map\n");
            int order = 0;
            for (size_t i = 0; i < GRIDSIZE; i++) {
                for (size_t j = 0; j < GRIDSIZE; j++) {
                    grid[i][j] = (int)command.argument_str[order++];
                }
            }
            flag_grid_setuped = true;
            break;
        }
        case COMMAND_SYNC_LEVEL:{
            printf("command_handler_cli: Sync level\n");
            level = command.argument_int_0;
            break;
        }
        case COMMAND_SYNC_SCORE:{
            printf("command_handler_cli: Sync score\n");
            score = command.argument_int_0;
            break;
        }
        default:{
            printf("command_handler_cli: Unknown command[%d]\n", command.command_id);
            break;
        }
    }
}

void *server_handler(){
    //int connection_fd = *(int *)ptr_connection_fd;
    //char buffer[256];
    bool flag_thread_exit = false;
    Command command;
    while(!flag_thread_exit) {
        bzero(&command, 152);
        ssize_t bytes_read = read(connection_fd, &command, sizeof(Command));
        if(bytes_read == (ssize_t)-1) {
            continue;
        }
        printf("server_handler: server_handler read %zd bytes\n", bytes_read);
        if(bytes_read != sizeof(Command)){
            if(bytes_read == 0){
                flag_thread_exit = true;
            }
            printf("server_handler: Error reading from socket: package size(%zd) != %zd error. \n", bytes_read, sizeof(Command));
            exit(0);
            continue;
        }
        command_handler_cli(command, connection_fd);
    }

    return NULL;
}


void on_new_connection_cli(int connection_fd){
    pthread_t ptid;
    pthread_create(&ptid, NULL, server_handler, (void *)&connection_fd);
    pthread_detach(ptid);
    printf("on_new_connection_cli: New thread for connection %d created.\n", connection_fd);
}


int connect_to_server(char *address, int port) {
    int socket_fd;

    struct sockaddr_in client_address;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(socket_fd == -1, "on_new_connection_cli: Socket creation failed.");
    
    printf("on_new_connection_cli: Socket created.\n");

    bzero(&client_address, sizeof(client_address));

    client_address.sin_family = AF_INET;

    client_address.sin_addr.s_addr = inet_addr(address);
    client_address.sin_port = htons(port);

    int connect_result = connect(socket_fd, (struct sockaddr *) &client_address, sizeof(client_address));

    assert(connect_result != 0, "on_new_connection_cli: Socket connect failed.");
    
    printf("on_new_connection_cli: Socket connected.\n");

    return socket_fd;
}

void sakure_main_8877_threaded();

void signal_handler(int signo);

#include <signal.h>

int main(int argc, char* argv[])
{
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGTSTP, signal_handler);


    char *address_ = "127.0.0.1";
    int port_ = 8877;

    if(argc <= 1){
        sakure_main_8877_threaded();
        printf("Running in local mode.");
    }
    else{
        address_= argv[1];
        port_ = atoi(argv[2]);
    }


    connection_fd = connect_to_server(address_, port_);

    Command command_join;
    bzero(&command_join, sizeof(command_join));
    command_join.command_id = COMMAND_JOIN_GAME;
    send(connection_fd, &command_join, sizeof(Command), 0);
    on_new_connection_cli(connection_fd);

    printf("main: Waiting for map...\n");

    while(!flag_grid_setuped) {
        usleep(100);
    }

    printf("main: Map received.\n");

    srand(time(NULL));

    // level = 1;

    initSDL();

    font = TTF_OpenFont("resources/Burbank-Big-Condensed-Bold-Font.otf", HEADER_HEIGHT);
    if (font == NULL) {
        fprintf(stderr, "main: Error loading font: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }

    // playerPosition.x = playerPosition.y = GRIDSIZE / 2;
    // initGrid();

    SDL_Window* window = SDL_CreateWindow("Client", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);

    if (window == NULL) {
        fprintf(stderr, "main: Error creating app window: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    renderer = SDL_CreateRenderer(window, -1, 0);

	if (renderer == NULL)
	{
		fprintf(stderr, "main: Error creating renderer: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
	}

    SDL_Texture *grassTexture = IMG_LoadTexture(renderer, "resources/grass.png");
    SDL_Texture *tomatoTexture = IMG_LoadTexture(renderer, "resources/tomato.png");
    SDL_Texture *playerTexture = IMG_LoadTexture(renderer, "resources/player.png");

    // main game loop
    while (!shouldExit) {
        SDL_SetRenderDrawColor(renderer, 0, 105, 6, 255);
        SDL_RenderClear(renderer);

        processInputs();

        drawGrid(renderer, grassTexture, tomatoTexture, playerTexture);
        drawUI(renderer);

        SDL_RenderPresent(renderer);

        SDL_Delay(16); // 16 ms delay to limit display to 60 fps
    }

    // clean up everything
    SDL_DestroyTexture(grassTexture);
    SDL_DestroyTexture(tomatoTexture);
    SDL_DestroyTexture(playerTexture);

    TTF_CloseFont(font);
    TTF_Quit();

    IMG_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
