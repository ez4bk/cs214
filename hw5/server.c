

// x  x  x  x  x
// x  x  x  x  x
// x  x  x  x  x
// x  x  x  x  x
// x  x  x  x  x

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


#include <stdlib.h>

#include <errno.h>

#ifndef _SCAFFOLDING
#define _SCAFFOLDING

typedef struct _Node {
    void *ptr;
    struct _Node *next;
} Node;


Node *create_node(void *ptr);
void append_node(Node *head, void *ptr);
void delete_node(Node *head, Node *to_delete);


Node *create_node(void *ptr) {
    Node *node = malloc(sizeof(Node));
    node->ptr = ptr;
    node->next = NULL;
    return node;
}

void append_node(Node *head, void *ptr) {
    if (head == NULL) { return ; }
    Node *curr = head;

    while (curr->next != NULL) {
        curr = curr->next;
    }

    if(curr->ptr == NULL){
        curr->ptr = ptr;
    } else {
        Node *new_node = (Node *) malloc(sizeof(Node));
        new_node->ptr = ptr;
        curr->next = new_node;
    }

}

void delete_node(Node *head, Node *to_delete) {
    if (head == NULL) { return ; }
    Node *curr = head;

    while (curr->next != NULL) {
        if(curr->next == to_delete) {
            curr->next = curr->next->next;
            free(to_delete);
            return;
        }
    }

}




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












void assert(bool condition, char* message);
bool warning(bool condition, char* message);
void on_new_connection_srv(int connection_fd);
void accept_loop(int server_fd);
int init_network(int port);
Game init_game();
Player *find_player_by_fd(int fd);
void broadcast(Command command, Player *exclude_player);
Node *find_node_by_fd(int fd);

Game game;
bool flag_exit = false;


void* sakura_main(int port){
    game = init_game();
    int socket_fd = init_network(port);
    accept_loop(socket_fd);
    return NULL;
}

void* sakura_main_8877(void *_){
    game = init_game();
    int socket_fd = init_network(8877);
    accept_loop(socket_fd);
    return NULL;
}

void sakure_main_8877_threaded(){
    pthread_t ptid;
    pthread_create(&ptid, NULL, sakura_main_8877, NULL);
    pthread_detach(ptid);
}


void signal_handler(int signo){
    printf("Signal %d received.\n", signo);
    _exit(0);
}

#include <signal.h>

#ifndef _LIBSERVER

int main(int argc, char* argv[]) {

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGTSTP, signal_handler);


    if(argc < 2) {
        printf("Usage: server <port>\n");
        return 1;
    }

    printf("main: Server started\n");

    sakura_main(atoi(argv[1]));

}

#endif




void accept_loop(int server_fd){

    struct sockaddr_in client_address;

    socklen_t length = sizeof(client_address);

    while(!flag_exit){
        int connection_fd = accept(server_fd, (struct sockaddr*) &client_address, &length);
        
        if(warning(connection_fd == 0, "accept_loop: Remote host closed connection.")){
            flag_exit = true;
            continue;
        }
        if(warning(connection_fd < 0, "accept_loop: Socket accept failed.")){
            continue;
        }

        printf("accept_loop: New socket accepted.\n");

        on_new_connection_srv(connection_fd);

    }
    close(server_fd);
}

int init_network(int port) {
    int socket_fd;

    struct sockaddr_in server_address;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(socket_fd == -1, "init_network: Socket creation failed.");
    
    printf("init_network: Socket created.\n");

    bzero(&server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;

    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if(
        warning(
            bind(socket_fd, (struct sockaddr*) &server_address, sizeof(server_address)) != 0,
            "init_network: Socket bind failed.\n")
    ){
        printf("strerror: %s\n", strerror(errno));
        exit(errno);
    }
    
    printf("init_network: Socket binded.\n");

    assert(
        listen(socket_fd, 5) != 0,
        "init_network: Socket listen failed."
    );

    printf("init_network: Socket listening.\n");
    return socket_fd;
}

Game init_game() {
    Game game;
    bzero(&game, sizeof(game));
    game.players = create_node(NULL);
    for(int i = 0; i < GRIDSIZE; i++){
        for(int j = 0; j < GRIDSIZE; j++){
            game.grid[i][j] = rand() % 2;
        }
    }
    game.grid[0][0] = 0;
    return game;
}

int calaulate_order(int x, int y) {
    return x * GRIDSIZE + y;
}

void hex_print(void *ptr, ssize_t size){
    return ;
    printf("\n%%");
    for(size_t i = 0; i < size; i++){
        printf("%02X ", ((char*)ptr)[i]);
    }
    printf("\n");
    printf("%%\n");
}

Command build_sync_map_command() {
    Command command;
    bzero(&command, sizeof(command));
    command.command_id = COMMAND_SYNC_MAP;

    int order = 0;
    for(int i = 0; i < GRIDSIZE; i++){
        for(int j = 0; j < GRIDSIZE; j++){
            command.argument_str[order++] = game.grid[i][j];
            // printf("%d ", order);
        }
    }
    Node *current = game.players;
    while(current != NULL){
        Player *player = current->ptr;
        int order = calaulate_order(player->position.x, player->position.y);
        command.argument_str[order] = 3;
        current = current->next;
    }

    hex_print(&command, sizeof(Command));

    return command;
}

int legitimate(int num){
    if(num < 0){
        return 0;
    }
    if(num >= GRIDSIZE){
        return GRIDSIZE -1;
    }
    return num;
}

bool check_if_no_tomato_left(){
    int count = 0;
    for(int i = 0; i < GRIDSIZE; i++){
        for(int j = 0; j < GRIDSIZE; j++){
            if(game.grid[i][j] == TILE_TOMATO){
                return false;
            }
        }
    }
    return count == 0;
}

void handle_no_tomato_left(){
    if(!check_if_no_tomato_left()){
        return ;
    }
    printf("handle_no_tomato_left: Called!\n");
    game.level++;
    for(int i = 0; i < GRIDSIZE; i++){
        for(int j = 0; j < GRIDSIZE; j++){
            game.grid[i][j] = rand() % 2;
        }
    }
    game.grid[0][0] = 0;
    Command command;
    bzero(&command, sizeof(command));
    command.command_id = COMMAND_SYNC_LEVEL;
    command.argument_int_0 = game.level;
    broadcast(command, NULL);
    broadcast(build_sync_map_command(), NULL);
}


void command_handler_srv(Command command, int connection_fd) {
    switch(command.command_id) {
        case COMMAND_MOVE_PLAYER:{
            printf("command_handler_srv: Move player\n");
            Player *player = find_player_by_fd(connection_fd);
            switch(command.argument_str[0]){
                case 'w':{
                    player->position.y = legitimate(player->position.y - 1);
                    break;
                }
                case 's':{
                    player->position.y = legitimate(player->position.y + 1);
                    break;
                }
                case 'a':{
                    player->position.x = legitimate(player->position.x - 1);
                    break;
                }
                case 'd':{
                    player->position.x = legitimate(player->position.x + 1);
                    break;
                }
            }
            if(game.grid[player->position.x][player->position.y] == 1){
                player->score++;
                game.grid[player->position.x][player->position.y] = 0;
                Command command_sync_score;
                bzero(&command_sync_score, sizeof(command_sync_score));
                command_sync_score.command_id = COMMAND_SYNC_SCORE;
                command_sync_score.argument_int_0 = player->score;
                send(connection_fd, &command_sync_score, sizeof(Command), 0);
            }

            handle_no_tomato_left();
            broadcast(build_sync_map_command(), NULL);
            break;
        }
        case COMMAND_END_GAME:{
            printf("command_handler_srv: End game\n");
            Node *node = find_node_by_fd(connection_fd);
            delete_node(game.players, node);
            close(connection_fd);
            broadcast(build_sync_map_command(), NULL);
            break;
        }
        case COMMAND_JOIN_GAME:{
            printf("command_handler_srv: Join game\n");
            Player *player = malloc(sizeof(Player));
            player->player_fd = connection_fd;
            player->position.x = 0;
            player->position.y = 0;
            append_node(game.players, player);
            broadcast(build_sync_map_command(), NULL);

            Command command;
            bzero(&command, sizeof(command));
            command.command_id = COMMAND_SYNC_LEVEL;
            command.argument_int_0 = game.level;
            send(connection_fd, &command, sizeof(Command), 0);
            break;
        }
        default:{
            printf("command_handler_srv: Unknown command[%d]\n", command.command_id);
            break;
        }
    }
}

Node *find_node_by_fd(int fd) {
    Node *node = game.players;
    while(node != NULL) {
        Player *player = node->ptr;
        if(player->player_fd == fd) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

Player *find_player_by_fd(int fd) {
    Node *node = find_node_by_fd(fd);
    if(node == NULL) {
        return NULL;
    }
    return node->ptr;
}

void broadcast(Command command, Player *exclude_player) {
    // printf("**\n");
    hex_print(&command, sizeof(Command));
    // printf("**\n");
    Node *node = game.players;
    while(node != NULL) {
        Player *player = node->ptr;
        if(player != NULL && player != exclude_player) {
            send(player->player_fd, &command, sizeof(command), 0);
        }
        node = node->next;
    }
}

void *client_handler(void *ptr_connection_fd){
    int connection_fd = *(int *)ptr_connection_fd;
    bool flag_thread_exit = false;
    Command command;
    while(!flag_thread_exit) {
        bzero(&command, sizeof(Command));
        ssize_t bytes_read = read(connection_fd, &command, sizeof(Command));
        if(bytes_read == (ssize_t)-1) {
            continue;
        }
        printf("client_handler: read %zd bytes\n", bytes_read);
        if(bytes_read != sizeof(Command)){
            if(bytes_read == 0){
                printf("client_handler: Remote client closed connection\n");
                Node *node = find_node_by_fd(connection_fd);
                delete_node(game.players, node);
                close(connection_fd);
                break;
            }
            printf("client_handler: Error reading from socket: package size(%zd) != %zd error. \n", bytes_read, sizeof(Command));
            continue;
        }
        command_handler_srv(command, connection_fd);
    }

    return NULL;
}

void on_new_connection_srv(int connection_fd){
    pthread_t ptid;
    pthread_create(&ptid, NULL, client_handler, (void *)&connection_fd);
    pthread_detach(ptid);
    printf("on_new_connection_srv: New thread for connection %d created.\n", connection_fd);
}

void assert(bool condition, char* message) {
    if (condition) {
        printf("assert: %s\n", message);
        exit(1);
    }
}

bool warning(bool condition, char* message) {
    if (condition) {
        printf("warning: %s\n", message);
    }
    return condition;
}
