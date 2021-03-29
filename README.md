# Snake Micro Bit
A simple implementation of snake using the accelerometer on the BBC Micro Bit using lancaster C++ library, The game also uses the flash memory to keep track of the highest score on the unit and alert the user when they have set a new best.

To compile the project issue the following commands in the project directory after cloning:

`yt target bbc-microbit-classic-gcc`

`yt build`

Then copy the built file to the BBC micro bit:

`cp build/bbc-microbit-classic-gcc/source/iot-example-combined.hex /media/student/MICROBIT`

To play the game once it has been loaded click button A when prompted by the '<' symbol to begin. You can tilt the micro bit to move the snake around the grid, changing its direction. The goal is to move the snake so that it eats the apples (which are a slightly less vibrant shade of red) that spawn around the map. Going off the grid or colliding with the snakes own body results in the game ending. If the player has scored then a game over screen will be displayed with the score attained in that round. If the user has set a new personal best on the micro bit then a High score message will be displayed and the new score recorded.

After this the game will restart itself. To restart the game without finishing it use the reset button on the back of the micro bit.

If the player has not scored before going off the edge of the screen then the game will reset itself without the game over message.
