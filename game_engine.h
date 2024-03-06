#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define DEFAULT_DETECTION_THRESHOLD WINDOW_WIDTH / 2
#define NUMBER_OF_MENU_OPTIONS 4 
#define PADDLES_DEFAULT_SPEED 5
#define BALL_DEFAULT_SPEED 2

typedef enum {false, true} bool_enum;
typedef enum {TEXT_DEFAULT, TEXT_SELECTED, N_TEXT_STATES} text_state_enum;
typedef enum {LEFT_PLAYER, RIGHT_PLAYER, MAX_N_PLAYERS} player_enum;
typedef enum {OBJ_LEFT_PADDLE, OBJ_RIGHT_PADDLE, OBJ_BALL, N_OBJECTS} objects_enum;

typedef struct velocity_data {
    
    double up;
    double down;
    double left;
    double right;
    
} velocity_data;

typedef struct gameObject {
    SDL_Rect rect; //Dimension and position information
    velocity_data velocity;
    double speed_modifier;
} gameObject;

// A struct with all SDL specific variables. This is done so that I can have a single variable that I can pass around.
typedef struct mySDLvars {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;
    SDL_Surface *textSurface;
    SDL_Texture *textTexture;
    SDL_Color uiTextColor[N_TEXT_STATES];
} mySDLvars;

typedef struct myGameVars {
    bool_enum exitGame;
    bool_enum twoPlayers;
    bool_enum showMenu;
    player_enum playerPoints[MAX_N_PLAYERS];
    int aiDetectionThresholdOnX;
    int selectedMenuOption;
    int roundsToWin;
} myGameVars;

// Function to initialize the SDL library
void InitializeSDLLibrary()
{
    // Initialiaze the SDL library
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS))
    {
        printf("Error %s \n", SDL_GetError());
    }
}

void InitializeFontLibrary(myGameVars *gameVars)
{
    if (TTF_Init() != 0)
    {
        printf("TTF Library error: %s", TTF_GetError());
        gameVars->exitGame = true;
    }
    

}

TTF_Font *LoadFont(myGameVars *gameVars)
{
    TTF_Font *font = TTF_OpenFont("FreeMono.ttf", 16);
    
    if (font == NULL)
    {
        printf("TTF Library error: Couldn't load a font: %s", TTF_GetError());
        gameVars->exitGame = true;
    }
    
    return font;
}

// Function to prepare the window and renderer associated with it.
void CreateWindowAndRenderer(mySDLvars *libVars, int width, int height)
{
    libVars->window = SDL_CreateWindow("PONG IN SDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);

    if(libVars->window == NULL)
    {
        printf("Error %s \n", SDL_GetError());
    }
    
    libVars->renderer = SDL_CreateRenderer(libVars->window, -1, SDL_RENDERER_ACCELERATED);
    if(libVars->renderer == NULL)
    {
        printf("Error %s \n", SDL_GetError());
    }
    
    SDL_SetRenderDrawColor(libVars->renderer, 100, 100, 100, SDL_ALPHA_OPAQUE);
}


void RenderObjects(mySDLvars *libVars, gameObject gameObjects[])
{    
    
    SDL_SetRenderDrawColor(libVars->renderer, 200, 200, 200, SDL_ALPHA_OPAQUE);
    for (int i = 0; i < N_OBJECTS; i++)
    {
        SDL_RenderDrawRect(libVars->renderer, &gameObjects[i].rect);     
    }
    SDL_SetRenderDrawColor(libVars->renderer, 100, 100, 100, SDL_ALPHA_OPAQUE);
    SDL_RenderPresent(libVars->renderer);
}

void RenderMenuOption(mySDLvars *libVars, SDL_Rect *text_position, char *text_input, TTF_Font *font, text_state_enum textState)
{
    libVars->textSurface = TTF_RenderText_Blended(font, text_input, libVars->uiTextColor[textState]);
    libVars->textTexture = SDL_CreateTextureFromSurface(libVars->renderer, libVars->textSurface);
    TTF_SizeText(font, text_input, &text_position->w, &text_position->h);
    text_position->x = WINDOW_WIDTH/2 - text_position->w / 2;
    text_position->y = text_position->y + 20;
    SDL_RenderCopy(libVars->renderer, libVars->textTexture, NULL, text_position);
}

// Shows all menu options
void RenderMenu(mySDLvars *libVars, TTF_Font *font, gameObject gameObjects[], myGameVars *gameVars)
{
    // Clear the renderer to make sure there is nothing else on the screen
    SDL_RenderClear(libVars->renderer);

    // VARIABLES
    char menuOptions[NUMBER_OF_MENU_OPTIONS][50]; // Array which holds the menu options text
    SDL_Rect text_position; // Text position data on screen

    // Default text position. Will be modified in RenderMenuOptions:
    // X based on length of the line of text and Y advanced with each new line of text (i.e. RenderMenuOptions function call).
    text_position.x = WINDOW_WIDTH/2;
    text_position.y = WINDOW_HEIGHT/4;

    RenderMenuOption(libVars, &text_position, "PONG IN SDL | CS50 FINAL PROJECT | RAFAL MYSLIWCZYK\0", font, TEXT_DEFAULT);
    RenderMenuOption(libVars, &text_position, "Use keyboard arrows to navigate\0", font, TEXT_DEFAULT);

  
    sprintf(menuOptions[0],"BALL SPEED: %i", (int) gameObjects[OBJ_BALL].speed_modifier);
    sprintf(menuOptions[1],"PLAYER SPEED: %i", (int) gameObjects[OBJ_LEFT_PADDLE].speed_modifier);
    sprintf(menuOptions[2],"NO. OF PLAYERS: %i", (int) gameVars->twoPlayers + 1);
    sprintf(menuOptions[3],"POINTS TO WIN: %i", (int) gameVars->roundsToWin);
   
    for (int i = 0; i < NUMBER_OF_MENU_OPTIONS; i++)
    {
        if (gameVars->selectedMenuOption == i)
        {
            RenderMenuOption(libVars, &text_position, menuOptions[i], font, TEXT_SELECTED);
        }
        else
        {
            RenderMenuOption(libVars, &text_position, menuOptions[i], font, TEXT_DEFAULT);
        }
    }
    
    RenderMenuOption(libVars, &text_position, "Press Enter to start.\0", font, TEXT_DEFAULT);
    
    // Show the menu on screen
    SDL_RenderCopy(libVars->renderer, libVars->textTexture, NULL, &text_position);
    SDL_RenderPresent(libVars->renderer);
}

void RenderScore(mySDLvars *libVars, TTF_Font *font, myGameVars *gameVars)
{
    SDL_Rect left_player_points_position;
    left_player_points_position.x = WINDOW_WIDTH/4;
    left_player_points_position.y = 20;
    SDL_Rect right_player_points_position;
    right_player_points_position.x = WINDOW_WIDTH/4 * 3;
    right_player_points_position.y = 20;
    SDL_Color text_color = {255,255,255,255};
    
    // Char array to store the points converted to string. size of 5 will most likely by enough.
    char points[5];
    //Render points 
        sprintf(points,"%i", (int)gameVars->playerPoints[LEFT_PLAYER]);
        libVars->textSurface = TTF_RenderText_Blended(font, points, text_color);
        libVars->textTexture = SDL_CreateTextureFromSurface(libVars->renderer, libVars->textSurface);
        TTF_SizeText(font, points, &left_player_points_position.w, &left_player_points_position.h);
        
        
        SDL_RenderCopy(libVars->renderer, libVars->textTexture, NULL, &left_player_points_position);

        sprintf(points,"%i", (int)gameVars->playerPoints[RIGHT_PLAYER]);
        libVars->textSurface = TTF_RenderText_Blended(font, points, text_color);
        libVars->textTexture = SDL_CreateTextureFromSurface(libVars->renderer, libVars->textSurface);
        TTF_SizeText(font, points, &right_player_points_position.w, &right_player_points_position.h);
        
        
        SDL_RenderCopy(libVars->renderer, libVars->textTexture, NULL, &right_player_points_position);
    
}

void InitializeGameObject(gameObject *newGameObject, int x, int y, int w, int h)
{
    // Allocate memory for a new object and set given and defaul values to all of the variables
    newGameObject->rect.x = x;
    newGameObject->rect.y = y;
    newGameObject->rect.w = w;
    newGameObject->rect.h = h;
    newGameObject->velocity.up = 0.0;
    newGameObject->velocity.down = 0.0;
    newGameObject->velocity.left = 0.0;
    newGameObject->velocity.right = 0.0;
    newGameObject->speed_modifier = 1.0;
    
}

void MoveObjects(gameObject gameObjects[])
{   
    // This loop serves a purpose of iterating over all objects in game in order to move them
    for (int i = 0; i < N_OBJECTS; i++)
    { 
        gameObjects[i].rect.y -= round(gameObjects[i].velocity.up * gameObjects[i].speed_modifier);

        gameObjects[i].rect.y += round(gameObjects[i].velocity.down * gameObjects[i].speed_modifier);
        
        gameObjects[i].rect.x -= round(gameObjects[i].velocity.left * gameObjects[i].speed_modifier);
        
        gameObjects[i].rect.x += round(gameObjects[i].velocity.right * gameObjects[i].speed_modifier);
       
    }
    
}

void DetectCollision(gameObject gameObjects[])
{
    // Collision detection for the paddles ( with the edges of the screen )
    if (gameObjects[OBJ_LEFT_PADDLE].rect.y <= 0)
    {
        gameObjects[OBJ_LEFT_PADDLE].velocity.up = 0;
    }
    
    if (gameObjects[OBJ_LEFT_PADDLE].rect.y >= WINDOW_HEIGHT - gameObjects[OBJ_LEFT_PADDLE].rect.h)
    {
        gameObjects[OBJ_LEFT_PADDLE].velocity.down = 0;
    }
    
    if (gameObjects[OBJ_RIGHT_PADDLE].rect.y <= 0)
    {
        gameObjects[OBJ_RIGHT_PADDLE].velocity.up = 0;
    }
    
    if (gameObjects[OBJ_RIGHT_PADDLE].rect.y >= WINDOW_HEIGHT - gameObjects[OBJ_RIGHT_PADDLE].rect.h)
    {
        gameObjects[OBJ_RIGHT_PADDLE].velocity.down = 0;
    }
    
    // Ball collision detection
    if (gameObjects[OBJ_BALL].rect.y <= 0)
    {
        gameObjects[OBJ_BALL].velocity.down = gameObjects[OBJ_BALL].velocity.up;
        gameObjects[OBJ_BALL].velocity.up = 0;
    }
    if (gameObjects[OBJ_BALL].rect.y >= WINDOW_HEIGHT - gameObjects[OBJ_BALL].rect.h)
    {
        gameObjects[OBJ_BALL].velocity.up = gameObjects[OBJ_BALL].velocity.down;
        gameObjects[OBJ_BALL].velocity.down = 0;
    }
    
    
    if (gameObjects[OBJ_BALL].rect.x > gameObjects[OBJ_LEFT_PADDLE].rect.x && gameObjects[OBJ_BALL].rect.x <= gameObjects[OBJ_LEFT_PADDLE].rect.x + gameObjects[OBJ_LEFT_PADDLE].rect.w
        && gameObjects[OBJ_BALL].rect.y < gameObjects[OBJ_LEFT_PADDLE].rect.y + gameObjects[OBJ_LEFT_PADDLE].rect.h && gameObjects[OBJ_BALL].rect.y + gameObjects[OBJ_BALL].rect.h > gameObjects[OBJ_LEFT_PADDLE].rect.y)
    {
        gameObjects[OBJ_BALL].velocity.right = 1.0;
        gameObjects[OBJ_BALL].velocity.left = 0.0;
        
        if (gameObjects[OBJ_LEFT_PADDLE].velocity.up > 0)
        {
            if(gameObjects[OBJ_BALL].velocity.up == 0)
            {
                gameObjects[OBJ_BALL].velocity.down = 0;
                gameObjects[OBJ_BALL].velocity.up = 1;
            }
            else
            {
                gameObjects[OBJ_BALL].velocity.up = gameObjects[OBJ_BALL].velocity.up + 1.0;
            }
        }
        if (gameObjects[OBJ_LEFT_PADDLE].velocity.down > 0)
        {
            if(gameObjects[OBJ_BALL].velocity.down == 0)
            {
                gameObjects[OBJ_BALL].velocity.down = 1;
                gameObjects[OBJ_BALL].velocity.up = 0;
            }
            else
            {
                gameObjects[OBJ_BALL].velocity.down = gameObjects[OBJ_BALL].velocity.down + 1.0;
            }
        }
    }
    
    if (gameObjects[OBJ_BALL].rect.x + gameObjects[OBJ_BALL].rect.w < gameObjects[OBJ_RIGHT_PADDLE].rect.x + gameObjects[OBJ_RIGHT_PADDLE].rect.w && gameObjects[OBJ_BALL].rect.x + gameObjects[OBJ_BALL].rect.w >= gameObjects[OBJ_RIGHT_PADDLE].rect.x
        && gameObjects[OBJ_BALL].rect.y < gameObjects[OBJ_RIGHT_PADDLE].rect.y + gameObjects[OBJ_RIGHT_PADDLE].rect.h && gameObjects[OBJ_BALL].rect.y + gameObjects[OBJ_BALL].rect.h > gameObjects[OBJ_RIGHT_PADDLE].rect.y)
    {
        gameObjects[OBJ_BALL].velocity.left = 1.0;
        gameObjects[OBJ_BALL].velocity.right = 0.0;
        
        if (gameObjects[OBJ_RIGHT_PADDLE].velocity.up > 0)
        {
            if(gameObjects[OBJ_BALL].velocity.up == 0)
            {
                gameObjects[OBJ_BALL].velocity.down = 0;
                gameObjects[OBJ_BALL].velocity.up = 1;
            }
            else
            {
                gameObjects[OBJ_BALL].velocity.up = gameObjects[OBJ_BALL].velocity.up + 1.0;
            }
        }
        
        if (gameObjects[OBJ_RIGHT_PADDLE].velocity.down > 0)
        {
            if(gameObjects[OBJ_BALL].velocity.down == 0)
            {
                gameObjects[OBJ_BALL].velocity.down = 1;
                gameObjects[OBJ_BALL].velocity.up = 0;
            }
            else
            {
                gameObjects[OBJ_BALL].velocity.down = gameObjects[OBJ_BALL].velocity.down + 1.0;
            }
        }
    }
    
}

void AiMove(gameObject gameObjects[], myGameVars *gameVars)
{
    if (gameObjects[OBJ_BALL].rect.x > gameVars->aiDetectionThresholdOnX)
    {
        if (gameObjects[OBJ_BALL].rect.y < gameObjects[OBJ_RIGHT_PADDLE].rect.y && gameObjects[OBJ_BALL].velocity.right > 0)
        {
            gameObjects[OBJ_RIGHT_PADDLE].velocity.up = 1.0;
            gameObjects[OBJ_RIGHT_PADDLE].velocity.down = 0.0;
        }
        if (gameObjects[OBJ_BALL].rect.y > gameObjects[OBJ_RIGHT_PADDLE].rect.y + gameObjects[OBJ_RIGHT_PADDLE].rect.h && gameObjects[OBJ_BALL].velocity.right > 0)
        {
            gameObjects[OBJ_RIGHT_PADDLE].velocity.up = 0.0;
            gameObjects[OBJ_RIGHT_PADDLE].velocity.down = 1.0;
        }
        
        if(gameObjects[OBJ_BALL].velocity.left > 0)
        {
            gameObjects[OBJ_RIGHT_PADDLE].velocity.up = 0.0;
            gameObjects[OBJ_RIGHT_PADDLE].velocity.down = 0.0;
        }
        
    }
    else
    {
        gameObjects[OBJ_RIGHT_PADDLE].velocity.up = 0.0;
        gameObjects[OBJ_RIGHT_PADDLE].velocity.down = 0.0;
    }
}

void UpdatePoints(gameObject gameObjects[], myGameVars *gameVars)
{

    if (gameObjects[OBJ_BALL].rect.x + gameObjects[OBJ_BALL].rect.w < 0)
    {
        gameObjects[OBJ_BALL].rect.x = WINDOW_WIDTH/2;
        gameObjects[OBJ_BALL].rect.y = rand() % WINDOW_HEIGHT;
        SDL_Delay(1000);
        gameObjects[OBJ_BALL].velocity.right = 1.0;
        gameObjects[OBJ_BALL].velocity.left = 0.0;
        gameObjects[OBJ_BALL].velocity.up = 0.0;
        gameObjects[OBJ_BALL].velocity.down = 0.0;
        /* *right_player_points = *right_player_points + 1; */
        gameVars->playerPoints[RIGHT_PLAYER] += 1; 
    }
    else if (gameObjects[OBJ_BALL].rect.x > WINDOW_WIDTH)
    {
        gameObjects[OBJ_BALL].rect.x = WINDOW_WIDTH/2;
        gameObjects[OBJ_BALL].rect.y = rand() % WINDOW_HEIGHT;
        SDL_Delay(1000);
        gameObjects[OBJ_BALL].velocity.left = 1.0;
        gameObjects[OBJ_BALL].velocity.right = 0.0;
        gameObjects[OBJ_BALL].velocity.up = 0.0;
        gameObjects[OBJ_BALL].velocity.down = 0.0;
        /* *left_player_points = *left_player_points + 1; */
        gameVars->playerPoints[LEFT_PLAYER] += 1;
    }
}

void RenderWinnerScreen(mySDLvars *libVars, myGameVars *gameVars, player_enum winner, TTF_Font *font)
{
    SDL_RenderClear(libVars->renderer);
    SDL_Rect text_position; // Text position data on screen

    // Default text position. Will be modified in RenderMenuOptions:
    // X based on length of the line of text
    text_position.x = WINDOW_WIDTH/2;
    text_position.y = WINDOW_HEIGHT/4;

    switch (winner)
    {
        case LEFT_PLAYER:
            RenderMenuOption(libVars, &text_position, "LEFT PLAYER WON!\0", font, TEXT_SELECTED);
            break;
        case RIGHT_PLAYER:
            RenderMenuOption(libVars, &text_position, "RIGHT PLAYER WON!\0", font, TEXT_SELECTED);
            break;
        default:
            break;

    }

    // Show the text on screen
    SDL_RenderCopy(libVars->renderer, libVars->textTexture, NULL, &text_position);
    SDL_RenderPresent(libVars->renderer);
    SDL_Delay(5000);
}
void CheckForWin(mySDLvars *libVars, myGameVars *gameVars, TTF_Font *font)
{
    for (int i = 0; i < MAX_N_PLAYERS; i++)
    {
        if (gameVars->playerPoints[i] >= gameVars->roundsToWin)
        {
            RenderWinnerScreen(libVars, gameVars, i, font);
            gameVars->exitGame = true;
        }
    }
}

