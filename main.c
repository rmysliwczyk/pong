#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "game_engine.h"

void CheckEvents(mySDLvars *libVars, myGameVars *gameVars, gameObject gameObjects[]);
void CheckMenuEvents(mySDLvars *libVars, myGameVars *gameVars, gameObject gameObjects[]);


int main(int argc, char **argv)
{
    // Random seed
    srand(time(0));

    // VARIABLES AND INITIAL/DEFAULT SETTINGS 
    mySDLvars *libVars = malloc(sizeof(mySDLvars)); // This struct holds all SDL specific variables, like textures, renderers, windows etc..
    
    libVars->uiTextColor[TEXT_DEFAULT].r = 150; // Greyish white?
    libVars->uiTextColor[TEXT_DEFAULT].g = 150; // --
    libVars->uiTextColor[TEXT_DEFAULT].b = 150; // --
    libVars->uiTextColor[TEXT_DEFAULT].a = 255; // --

    libVars->uiTextColor[TEXT_SELECTED].r = 255; // White white
    libVars->uiTextColor[TEXT_SELECTED].g = 255; // --
    libVars->uiTextColor[TEXT_SELECTED].b = 255; // --
    libVars->uiTextColor[TEXT_SELECTED].a = 255; // --

    myGameVars *gameVars = malloc(sizeof(myGameVars)); // This struct holds all game specific variables that hold settings and variables necessary for the game.
    gameVars->exitGame = false;
    gameVars->twoPlayers = false;
    gameVars->showMenu = true;
    gameVars->playerPoints[LEFT_PLAYER] = 0;
    gameVars->playerPoints[RIGHT_PLAYER] = 0;
    gameVars->aiDetectionThresholdOnX = DEFAULT_DETECTION_THRESHOLD;
    gameVars->selectedMenuOption = 0;
    gameVars->roundsToWin = 10;

    gameObject gameObjects[N_OBJECTS];
    InitializeGameObject(&gameObjects[OBJ_LEFT_PADDLE], 10, WINDOW_HEIGHT/2, 5, 60);
    InitializeGameObject(&gameObjects[OBJ_RIGHT_PADDLE] ,WINDOW_WIDTH - 15, WINDOW_HEIGHT/2, 5, 60);
    InitializeGameObject(&gameObjects[OBJ_BALL], WINDOW_WIDTH/2, WINDOW_HEIGHT/2, 10, 10);

    // Randomly decide on which direction the ball goes at first and the initial position on Y axis
    switch (rand() % 2)
    {
        case 0:
            gameObjects[OBJ_BALL].velocity.up = 1.0;
            break;
        case 1:
            gameObjects[OBJ_BALL].velocity.down = 1.0;
            break;
    }

    switch (rand() % 2)
    {
        case 0:
            gameObjects[OBJ_BALL].velocity.left = 1.0;
            break;
        case 1:
            gameObjects[OBJ_BALL].velocity.right = 1.0;
            break;
    }

    gameObjects[OBJ_BALL].rect.y = rand() % WINDOW_HEIGHT;

    gameObjects[OBJ_BALL].speed_modifier = BALL_DEFAULT_SPEED;
    gameObjects[OBJ_LEFT_PADDLE].speed_modifier = PADDLES_DEFAULT_SPEED;
    gameObjects[OBJ_RIGHT_PADDLE].speed_modifier = PADDLES_DEFAULT_SPEED;

    Uint32 game_time = SDL_GetTicks(); // These variables store the information about elapsed time
    Uint32 current_time = SDL_GetTicks(); // they are necessary to control the flow of the main loop

    // -END OF VARIABLES AND INITIAL/DEFAULT SETTINGS-

    // INITIALIZATION OF LIBRARIES AND ITS COMPONENTS
    InitializeSDLLibrary();
    
    InitializeFontLibrary(gameVars);
    TTF_Font *font = LoadFont(gameVars);

    // Create the window for the program and the renderer that is going to draw things inside the window
    CreateWindowAndRenderer(libVars, WINDOW_WIDTH, WINDOW_HEIGHT); 


    // -END OF INITIALIZATION-
    
    // Main loop. To keep the program running.
    while (!gameVars->exitGame)
    {
        
        // Getting the current time value;
        current_time = SDL_GetTicks();
        // Main loop for all the elements of the game, executes every 20ms
        while (current_time - game_time >= 20)
        {
            if (gameVars->showMenu == true)
            {
                // Renders all of the menu elements
                RenderMenu(libVars, font, gameObjects, gameVars);
                
                // Checks for options selected by the player and updates relevant options
                CheckMenuEvents(libVars, gameVars, gameObjects);
                
                game_time = current_time; 
                continue;
            }
            // A loop used to go through all of the events that can be polled.
            CheckEvents(libVars, gameVars, gameObjects);
            
            // Updates points for the ball if it reaches the edge od the screen. Resets the ball
            UpdatePoints(gameObjects, gameVars);
            
            // Detect collisions
            DetectCollision(gameObjects);
            
            // CPU player, if the game is not two player
            if (gameVars->twoPlayers == false)
            {
                AiMove(gameObjects, gameVars);
            }
            
            // Move all of the game objects. We give the list head to this function
            MoveObjects(gameObjects);
            
            // Clear the renderer in order to draw a new frame.
            SDL_RenderClear(libVars->renderer);
            
            // Draw score on the screen
            RenderScore(libVars, font, gameVars);
            
            // Draw all of the objects (paddles, ball)
            RenderObjects(libVars, gameObjects);
            
            CheckForWin(libVars, gameVars, font);
            // Storing the previous time value to keep track of time
            game_time = current_time;  
        }
    }
    
    
    //Freeing memory
    SDL_FreeSurface(libVars->textSurface);
    SDL_DestroyTexture(libVars->textTexture);
    TTF_CloseFont(font);
    
    SDL_DestroyWindow(libVars->window);
    SDL_DestroyRenderer(libVars->renderer);
    SDL_Quit();
    TTF_Quit();
    
}

void CheckEvents(mySDLvars *libVars, myGameVars *gameVars, gameObject gameObjects[])
{
    while (SDL_PollEvent(&libVars->event))
        {
            // If there is an event of user pressing the X on the game window. Set the exit variable to true
            if (libVars->event.type == SDL_WINDOWEVENT)
            {
                if (libVars->event.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    gameVars->exitGame = true;
                }
                    
            }
            
            switch(libVars->event.type)
            {
                case SDL_KEYDOWN:
                     switch (libVars->event.key.keysym.sym)
                     {
                         case SDLK_w:
                             gameObjects[OBJ_LEFT_PADDLE].velocity.up = 1.0;
                             break;
                         case SDLK_s:
                             gameObjects[OBJ_LEFT_PADDLE].velocity.down = 1.0;
                             break;
                         case SDLK_UP:
                             if (gameVars->twoPlayers == true)
                             {
                                gameObjects[OBJ_RIGHT_PADDLE].velocity.up = 1.0;
                             }
                             break;
                         case SDLK_DOWN:
                             if (gameVars->twoPlayers == true)
                             {
                                gameObjects[OBJ_RIGHT_PADDLE].velocity.down = 1.0;
                             }
                             break;
                         case SDLK_ESCAPE:
                             gameVars->showMenu = true;
                             break;
                     }
                    break;
                case SDL_KEYUP:
                    switch (libVars->event.key.keysym.sym)
                     {
                         case SDLK_w:
                             gameObjects[OBJ_LEFT_PADDLE].velocity.up = 0.0;
                             break;
                         case SDLK_s:
                             gameObjects[OBJ_LEFT_PADDLE].velocity.down = 0.0;
                             break;
                         case SDLK_UP:
                             if (gameVars->twoPlayers == true)
                             {
                                gameObjects[OBJ_RIGHT_PADDLE].velocity.up = 0.0;
                             }
                             break;
                         case SDLK_DOWN:
                             if (gameVars->twoPlayers == true)
                             {
                                gameObjects[OBJ_RIGHT_PADDLE].velocity.down = 0.0;
                             }
                             break;
                     }
                    break;
            }
        }
}

void CheckMenuEvents(mySDLvars *libVars, myGameVars *gameVars, gameObject gameObjects[])
{
    //CHECK KEY INPUTS AND MODIFY VARIABLES BASED ON SELECTION
    
    while (SDL_PollEvent(&libVars->event))
    {
        if (libVars->event.type == SDL_WINDOWEVENT)
        {
            if (libVars->event.window.event == SDL_WINDOWEVENT_CLOSE)
            {
                gameVars->exitGame = true;
            }
        }
    
        switch (libVars->event.type)
        {
            case SDL_KEYDOWN:
                switch(libVars->event.key.keysym.sym)
                {
                    case SDLK_DOWN:
                        if (gameVars->selectedMenuOption < NUMBER_OF_MENU_OPTIONS)
                        {
                            gameVars->selectedMenuOption = gameVars->selectedMenuOption + 1;
                        }
                        break;
                    case SDLK_UP:
                        if (gameVars->selectedMenuOption > 0)
                        {
                            gameVars->selectedMenuOption = gameVars->selectedMenuOption - 1;
                        }
                        break;
                    case SDLK_RIGHT:
                        switch (gameVars->selectedMenuOption)
                        {
                            case 0:
                                gameObjects[OBJ_BALL].speed_modifier += 1.0;
                                break;
                            case 1:
                                gameObjects[OBJ_LEFT_PADDLE].speed_modifier += 1.0;
                                gameObjects[OBJ_RIGHT_PADDLE].speed_modifier += 1.0;
                                break;
                            case 2:
                                gameVars->twoPlayers = true;
                                break;
                            case 3:
                                gameVars->roundsToWin += 1;
                                break;
                        }
                        break;
                    case SDLK_LEFT:
                        switch (gameVars->selectedMenuOption)
                        {
                            case 0:
                                if (gameObjects[OBJ_BALL].speed_modifier > 1.0)
                                {
                                    gameObjects[OBJ_BALL].speed_modifier -= 1.0;
                                }
                                break;
                            case 1:
                                if (gameObjects[OBJ_LEFT_PADDLE].speed_modifier > 1.0 && gameObjects[OBJ_RIGHT_PADDLE].speed_modifier > 1.0)
                                {
                                    gameObjects[OBJ_LEFT_PADDLE].speed_modifier -= 1.0;
                                    gameObjects[OBJ_RIGHT_PADDLE].speed_modifier -= 1.0;
                                }
                                break;
                            case 2:
                                gameVars->twoPlayers = false;
                                break;
                            case 3:
                                if (gameVars->roundsToWin > 1)
                                {
                                    gameVars->roundsToWin -= 1;
                                }
                                break;
                        }
                        break;
                    case SDLK_RETURN:
                        gameVars->showMenu = false;
                        break;
                                
                }
                break;
        }
    }
}
