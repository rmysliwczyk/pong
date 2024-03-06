# PONG in SDL
#### Video Demo:  https://youtu.be/F04mJ8GfpNE
#### Description:
Hello. This is my final project for CS50 2022.
This is a simple game, which is actually a clone of PONG. I've written this game using C and SDL.
The game is not that complicated. Just as in PONG you have to try to make sure that your opponent doesn't 
hit the ball and that you can get the ball past him. 
The technique used to do that in this game is to hit the ball whilst moving the paddle in the same direction as the ball is moving in
because this adds up to the speed of the ball, making it harder for the opponent to hit it.
In my game each object has variables which contain data about their velocity in either direction. Based on these I update
their position on their screen. Collisons of the ball with the paddles and the walls affect the variables which store the information
about its velocity.

The game contains a menu which allows to change some settings such as: Ball speed, Paddle speed, No. of players. Points to win.
I made sure to write my code in such way so that it is easy for me to expand the menu if I need it. I can always just call the function which creates new "entry"
in the menu and I have all the tools ready to modify it if need be.

This game can be played with 2 players or 1 player against CPU.
The AI in this game simply keeps track of its relative position to the ball and tries to get to it as soon as possible. It is not very sophisticated but it works fine.

I've decided to use SDL because in the past, I've attempted do learn how to program and I wanted to use SDL to create a simple game. 
I have failed then, but this time, thanks to CS50, I was able to use my newfound knowledge to achieve my goal of creating a simple game.
This was quite a challenge for me, because I had to familirize myself with this library, but I though that it would be a good exercsise.
I've relied mostly on the documentation of the library and refrained from simply looking up solutions for my problems, instead I wanted
to come up with as many solutions as possible myself. 
Besides having to look up the basic building blocks of the SDL library I had to think about how to handle collisions
drawing text, handling input, and controlling the game logic.

The project contains two files with code. There is main.c and game_engine.h. I wanted to separate those two in order to achieve
as much clarity of code as possible. My priority was to make the code clean and readable. I created my own structs so that I could "pack up" all of the variables
that are necessary for the game to run, so that I could easily pass them around and not have to rely on global variables. I went through multiple
sessions of "refactoring" the code because I wanted it to be clean and reusable so that I would be able to add additional functionaly more easily. 
I aimed to keep my main function as clean as possible, making sure to include all of the variable declarations in one place, 
so that they are clearly visible and accessible.

The game is controlled by the main loop which keeps track of the time elapsed to make sure that the game runs the same on any computer. In the future
if I create any other games I would like to include an independent framerate, which I believe would help to have more control over the movement of the objects in my games.

Controls:
Left player - W/S
Right player - Up/Down
