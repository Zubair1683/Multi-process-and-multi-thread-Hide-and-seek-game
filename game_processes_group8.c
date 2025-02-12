//Hidayet Aktürk 20050111030
//İsmail Alper Koyuncu 21050111056
//Ahmad Zubair Rahimi 21050141006
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>

#define SHM_SIZE sizeof(int)

// Structure to represent a 2D position
typedef struct {
    int x, y;
} Position;

// Structure to represent a player in the game
typedef struct {
    Position playerPosition, lastGuess;
    Position* guessMap;
    int lastdistance;
    int guessMapWantedsize;
} Player;

int* gameEnded; // Shared variable to decide if the game has ended

// Function to calculate the Manhattan distance between two positions
int calculate(Position p1, Position p2);

// Function to initialize a player
void createPlayer(Player* player, Position p1, int n) {
    player->playerPosition = p1;
    player->guessMap = (Position*)malloc(n * n * sizeof(Position));
    player->lastGuess.x = -1;
    player->lastGuess.y = -1;
    player->guessMapWantedsize = 0;
}


// Function for a player to make a guess
void guess(Player* p1, Player* p2, int n, int roundnum,char turnchar) {

        if (*gameEnded) {
            exit(0);
        }

        Position* newGuessMap = (Position*)malloc(n * n * sizeof(Position));
        int a = 0;
		// Initial guess
        if (p1->lastGuess.x == -1) {
            for (int i = 0; i < n; i++) {
                for (int k = 0; k < n; k++) {
                    if (!(p1->playerPosition.x == i && p1->playerPosition.y == k)) {
                        newGuessMap[a].x = i;
                        newGuessMap[a].y = k;
                        a++;
                    }
                }
            }
        } else {
        	// Next player's guess
            for (int i = 0; i < p1->guessMapWantedsize; i++) {
                if (p1->guessMap[i].x != -1 && calculate(p1->guessMap[i], p1->lastGuess) == p1->lastdistance) {
                  if(!(p1->guessMap[i].x == p1->lastGuess.x && p1->guessMap[i].y == p1->lastGuess.y)){
                      newGuessMap[a] = p1->guessMap[i];
                      a++;
                     }
                }
            }
        }

        int randomIndex = rand() % a;
        p1->lastGuess = newGuessMap[randomIndex];
        int answer = calculate(p1->lastGuess, p2->playerPosition);
        p1->lastdistance = answer;
        p1->guessMapWantedsize = a;
        for (int i = 0; i < a; i++) {
            p1->guessMap[i] = newGuessMap[i];
        }
        free(newGuessMap);
	if(turnchar=='a'){
            printf("%d.Guess of player1: [%d, %d]\n",roundnum, p1->lastGuess.x, p1->lastGuess.y);
            printf("the distance with player2 is %d\n",answer);
            
            if(answer == 0)
            *gameEnded = 1;
        }
        else if(turnchar=='b'){
            printf("%d.Guess of player2: [%d, %d]\n",roundnum, p1->lastGuess.x, p1->lastGuess.y);
            printf("the distance with player1 is %d\n\n",answer);
            
            if(answer == 0)
            *gameEnded = 1;
        }
    
}

// Calculating the manhattan distance between the players
int calculate(Position p1, Position p2) {
    return abs(p1.x - p2.x) + abs(p1.y - p2.y);
}

// Function to build and display the game map
void buildMap(int length, Player* p1, Player* p2) {
    char arr[length + 2][length + 2];
    
    printf("Coordinates of the players are chosen randomly\n");
    printf("player1: [%d,%d] , player2: [%d,%d]\n", p1->playerPosition.x, p1->playerPosition.y, p2->playerPosition.x, p2->playerPosition.y);
   
   // Creating an empty char array to build map then placing the position of the players
    for (int x = 0; x < length + 2; x++) {
        for (int y = 0; y < length + 2; y++) {
            arr[x][y] = ' ';
        }

    }
    for (int i = 0; i < length + 2; i++) {
        arr[0][i] = '-';
        if (i < length) {
            arr[i + 1][0] = '|';
            arr[i + 1][length + 1] = '|';
        }

        arr[length + 1][i] = '-';
    }
    arr[(p1->playerPosition.y) + 1][(p1->playerPosition.x) + 1] = '1';
    arr[(p2->playerPosition.y) + 1][(p2->playerPosition.x) + 1] = '2';

    for (int x = 0; x < length + 2; x++) {
        for (int y = 0; y < length + 2; y++) {
            printf("%c ", arr[x][y]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) { // Command line argument check
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <size_of_area>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
 
    if(atoi(argv[1]) <= 0){ 
      fprintf(stderr, "Size must be >= 0\n");
      return -1;
    }
    
    // Check if there is enough space for two players in the map
    if(atoi(argv[1]) == 1){
      fprintf(stderr, "In %d*%d map there is no avaiable place for two players\n", atoi(argv[1]), atoi(argv[1]));
      return -1;
    }
    
    int sizeOfArea = atoi(argv[1]);

    int maxGuesses = 3;

    srand(time(NULL));

	// Randoming the initial position of the player1
    Position p1 = { rand() % sizeOfArea, rand() % sizeOfArea };
    Player player1;
    createPlayer(&player1, p1, sizeOfArea);

	// Randoming the initial position of the player2
    Player player2;
    do {
        Position p2 = { rand() % sizeOfArea, rand() % sizeOfArea };
        createPlayer(&player2, p2, sizeOfArea);
    } while (player1.playerPosition.x == player2.playerPosition.x && player1.playerPosition.y == player2.playerPosition.y);

	 // Shared memory setup
    int shmid = shmget(IPC_PRIVATE, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    gameEnded = (int*)shmat(shmid, NULL, 0);
    *gameEnded = 0;
    
     
    //pid_t childPID1, childPID2;

    printf("%dx%d map is created\n",sizeOfArea,sizeOfArea);
    printf("A child process created\n");

    buildMap(sizeOfArea, &player1, &player2);
    printf("Game launches -->\n");

       for (int roundd= 1; roundd<= 3; roundd++) {
        printf("---------- Round-%d ---------- \n",roundd);
        int flag = 0;
        pid_t childPID1 = fork();
        if (childPID1 == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (childPID1 == 0) {
        	 // Child process for player2
            char turnchar='b';
            guess(&player2, &player1, sizeOfArea, roundd,turnchar);
            exit(0);
        }
        else{
        	 // Parent process for player1
            char turnchar = 'a';
            guess(&player1, &player2, sizeOfArea, roundd,turnchar);
            // Adding flags if any player won the game
            if(*gameEnded == 1){
              flag = 1; 
            }
            wait(NULL);
            if(*gameEnded == 1 && flag != 1){
              flag = 2;   
            }
        }
        if(flag == 1){
            printf("******************************************\n");
	    printf("player%d won the game!!!\n",flag);
	    printf("******************************************\n");
            exit(0);    
            }
        if(flag == 2){
            printf("******************************************\n");
	    printf("player%d won the game!!!\n",flag);
	    printf("******************************************\n");
            exit(0);    
            }
        
	}
	

    shmdt(gameEnded);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
