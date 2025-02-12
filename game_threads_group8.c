//Hidayet Aktürk 20050111030
//İsmail Alper Koyuncu 21050111056
//Ahmad Zubair Rahimi 21050141006
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <stdint.h>
#include <limits.h>

#define NUM_ROUNDS 3

int gameEnded = 0; // Shared resource among threads
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex declaration
int roundNum = 0;

// Position structure to represent a 2D position
typedef struct {
    int x, y;
} Position;

// Player structure to represent a player in the game
typedef struct {
    Position playerPosition, lastGuess;
    Position* guessMap;
    int lastdistance;
    int guessMapWantedsize;
    int minDistance;
} Player;

// Data_array structure to represent a list of data's for passing to the threads function as a parameter
typedef struct {
    int size;
    int pos;
    int length;
    Player* players;
    long threadId;
} Data_array;

// Function to calculate the Manhattan distance between two positions
int calculate(Position p1, Position p2);
//static int gameEnded = 0;

// Thread's function
void *runner(void *param);

// Function to initialize a player
void createPlayer(Player* player, Position p1, int n) {
    player->playerPosition = p1;
    player->guessMap = (Position*)malloc(n * n * sizeof(Position));
    player->lastGuess.x = -1;
    player->lastGuess.y = -1;
    player->guessMapWantedsize = 0;
    player->minDistance = INT_MAX;
}

// Function for a player to make a guess
void guess(Player players[], int n,int pos, int playerslength, int guessround) {

    if (gameEnded) {
        exit(0);
    }
    Position* newGuessMap = (Position*)malloc(n * n * sizeof(Position));
    int a = 0;
    
    if (players[pos].lastGuess.x == -1) {
        for (int i = 0; i < n; i++) {
            for (int k = 0; k < n; k++) {
                if (!(players[pos].playerPosition.x == i && players[pos].playerPosition.y == k)) {
                    newGuessMap[a].x = i;
                    newGuessMap[a].y = k;
                    a++;
                }
            }
        }
    } else {
        for (int i = 0; i < players[pos].guessMapWantedsize; i++) {
            if (players[pos].guessMap[i].x != -1 && calculate(players[pos].guessMap[i], players[pos].lastGuess) == players[pos].lastdistance) {
               if(!(players[pos].guessMap[i].x == players[pos].lastGuess.x && players[pos].guessMap[i].y ==players[pos].lastGuess.y)){
                  newGuessMap[a] = players[pos].guessMap[i];
                  a++;
               }
            }
        }
    }
	
    if(a == 0){ // Actually it is for dumb results
       for (int i = 0; i < n; i++) {
            for (int k = 0; k < n; k++) {
                if (!(players[pos].playerPosition.x == i && players[pos].playerPosition.y == k)) {
                    newGuessMap[a].x = i;
                    newGuessMap[a].y = k;
                    a++;
                }
            }
        } 
    }
	
    int randomIndex = rand() % a;
    players[pos].lastGuess = newGuessMap[randomIndex];
    int minanswerforlastguess = INT_MAX; 
    
    for(int k=0;k<playerslength;k++){       
        if(k!=pos){
            int answer = calculate(players[pos].lastGuess, players[k].playerPosition);
            if(minanswerforlastguess>answer)
		minanswerforlastguess=answer; 
            if(players[pos].minDistance>answer)
		players[pos].minDistance=answer; 
        }       
    }
    
    players[pos].lastdistance = minanswerforlastguess;
    players[pos].guessMapWantedsize = a;
    
    for (int i = 0; i < a; i++) {
        players[pos].guessMap[i] = newGuessMap[i];
    }
    
    free(newGuessMap);
    
    printf("%d.Guess of player%d: [%d, %d]\n",guessround+1, pos+1, players[pos].lastGuess.x, players[pos].lastGuess.y);
            
    for(int printnum=0;printnum<playerslength;printnum++){
        if(printnum!=pos)
            printf("the distance with player%d is %d\n",printnum+1,calculate(players[pos].lastGuess, players[printnum].playerPosition));
    }
            
    if(pos==playerslength-1)
        printf("\n");  
         
    if(players[pos].lastdistance == 0){
        gameEnded = 1;
        printf("******************************************\n");
	printf("player%d won the game!!!\n",pos+1);
	printf("******************************************\n");
	exit(0);
    }
        
}

// Function to calculate the Manhattan distance between two positions
int calculate(Position p1, Position p2) {
    return abs(p1.x - p2.x) + abs(p1.y - p2.y);
}

void buildMap(int length, Player players[],int playerslength) {
    char arr[length + 2][length + 2];
    printf("Coordinates of the players are chosen randomly\n");
    
    for(int k=0;k<playerslength-1;k++)
       printf("player%d: [%d,%d] , ",k+1, players[k].playerPosition.x, players[k].playerPosition.y);
     
    printf("player%d: [%d,%d]\n",playerslength, players[playerslength-1].playerPosition.x, players[playerslength-1].playerPosition.y);
     
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
   
   for(int i=0;i<playerslength;i++){
       arr[(players[i].playerPosition.y) + 1][(players[i].playerPosition.x) + 1] = (i+1)+'0';
   }
    
   for (int x = 0; x < length + 2; x++) {
        for (int y = 0; y < length + 2; y++) {
            printf("%c ", arr[x][y]);
        }
        printf("\n");
   }
}


int main(int argc, char *argv[]) {
    
    srand(time(NULL));
    int maxGuesses=3;
    
    // Command line arguments conditions for checking if there is enough arguments which is required
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <size_of_area>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if(argc < 3) {
        fprintf(stderr, "Usage: %s <number of players>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if(atoi(argv[1]) < 0){
        fprintf(stderr, "%d must be >= 0\n", atoi(argv[1]));
        return -1;
    }
    if(atoi(argv[1])*atoi(argv[1]) < atoi(argv[2])){
        fprintf(stderr, "There is not enough space for %d threads!!\n", atoi(argv[2]));
        return -1;
    }
    
    pthread_t threads[atoi(argv[2])];
    intptr_t threadIDs[atoi(argv[2])]; // Use intptr_t for threadIDs

    // Initialize mutex
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        perror("Error initializing mutex");
        exit(EXIT_FAILURE);
    }
    
    // Declare players array to store them
    Player players[atoi(argv[2])];
    Position positions[atoi(argv[2])];// Position array is for storing all the players positions
    
    positions[0].x=rand() % atoi(argv[1]);
    positions[0].y=rand() % atoi(argv[1]);
    createPlayer(&players[0], positions[0], atoi(argv[1])+2);// Create first player by its positions and length of its guess map
    
    // Create all other players
    for(int i=1; i<atoi(argv[2]); i++){
        int sameflag=0;
        do {
            sameflag=0;
            positions[i].x=rand() % atoi(argv[1]);
            positions[i].y=rand() % atoi(argv[1]);
            createPlayer(&players[i], positions[i], atoi(argv[1])+2);
            for(int x=0;x<atoi(argv[2]);x++){
                if(x!=i){
       	          if(players[x].playerPosition.x==players[i].playerPosition.x && players[x].playerPosition.y==players[i].playerPosition.y)
       	             sameflag=1;
                }   
            } 
        }while (sameflag==1);
    }
    
    // Print the size of map and threads
    printf("%dx%d map is created\n",atoi(argv[1]),atoi(argv[1]));
    printf("%d threads are created\n",atoi(argv[2]));
    
    // Invoke this function to build game map
    buildMap(atoi(argv[1]), players,atoi(argv[2]));
    
    printf("Game launches -->\n");
    
    Data_array data[atoi(argv[2])];// Declaring a data structure which stores all the data which is required for making guesses
    
    // Store all required data's to its structure elements
    for(int i=0; i<atoi(argv[2]); ++i){
        data[i].players = (Player*)malloc(atoi(argv[2]) * sizeof(Player));
        
        for(int k=0;k<atoi(argv[2]); ++k){
        data[i].players[k] = players[k];
        }
        
        data[i].length = atoi(argv[2]);
        data[i].size = atoi(argv[1]);
        data[i].pos = i;
    }
    
    // Create threads 
    for(long i=0; i<atoi(argv[2]); i++){
        data[i].threadId = i;
        int result = pthread_create(&threads[i], NULL, runner, (void*)&data[i]);
        if (result) {
            fprintf(stderr, "Error joining thread %ld: %d\n", i, result);
            return 1;
        }
    }
    
    // Join threads
    for (long i = 0; i < atoi(argv[2]); i++) {
        int result = pthread_join(threads[i], NULL);
        if (result) {
            fprintf(stderr, "Error joining thread %ld: %d\n", i, result);
            return 1;
        }
    }
    
   
    int minpos[atoi(argv[2])];
    int a=0;
    int minlength= INT_MAX;
    int *pNum = (int*)malloc(atoi(argv[2]) * sizeof(int));
    int nu = 0;
    
    // Find the least distance between to players
    for(int i=0; i<atoi(argv[2]); i++){
        for(int k=0; k<atoi(argv[2]); k++){
            if(!(data[i].players[k].minDistance<0)&&data[i].players[k].minDistance<minlength){
                minlength = data[i].players[k].minDistance;
                pNum[nu]=k+1;
            }
        }
    }
    
    // Check if there is equality between players least distance and store it to print them as well
    for(int i=0; i<atoi(argv[2]); i++){
        for(int k=0; k<atoi(argv[2]); k++){
            if(k==pNum[nu]-1)
              continue;
            if(data[i].players[k].minDistance == minlength){
              nu++;
              pNum[nu]=k+1;
            }
        }
   }
    
    printf("\nThe game ends!\n");
    printf("The winner with the closest guess of %d-distance:\n",minlength);
    
    int r=0;
    // Print the players if they have least distance
    while(pNum[r] > 0){
        if(pNum[r+1] != 0)
           printf("player%d, ",pNum[r++]);
        else
           printf("player%d\n",pNum[r++]);
    }
    printf("\n");
    
    // Free the data struct
    for(int k=0;k<atoi(argv[2]); k++){
        free(data[k].players);
    }
    
    // Destroy the mutex
    pthread_mutex_destroy(&mutex);
    
    return 0;
}

void *runner(void *param){
    Data_array* mydata = (Data_array*)param;
     
    long tid = mydata->threadId;
    int sizeOfArea = mydata->size;
    
    for(int round = 0; round < NUM_ROUNDS; ++round){
    
      // Acquire the mutex before accessing the shared variable
        pthread_mutex_lock(&mutex);
        
        // Wait for the turn
        while (roundNum != tid) {
            pthread_mutex_unlock(&mutex);
            pthread_mutex_lock(&mutex);
        }
        
        if (tid == 0) {
            printf("---------- Round-%d ----------\n",round+1);
        }
        
        guess(mydata->players, sizeOfArea,mydata->pos,mydata->length,round);
        
        // Update the turn for the next thread
        roundNum = (roundNum + 1) % mydata->length;

        // Release the mutex after finishing the critical section
        pthread_mutex_unlock(&mutex);
    
    }
    
    
    pthread_exit(NULL);    
  }
