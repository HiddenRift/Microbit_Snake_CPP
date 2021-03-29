/**
 *
 * File: main.cpp
 * Author: Robert Fry
 * Date: 01-02-2018
 * Desc: Snake game for the BBC micro bit
 */
#include "MicroBit.h"

#define SNAKEMAXLEN 25
#define SNAKEHEAD 0
#define GRIDMAX_X 5
#define GRIDMAX_Y 5
#define MOVEUP 0
#define MOVERIGHT 1
#define MOVEDOWN 2
#define MOVELEFT 3
#define TRUE 1
#define FALSE 0
#define HIGHSCOREFILE "highscore"

struct segment {
    int8_t x;
    int8_t y;
};

/** Prototypes **/
int8_t generate_apple(const struct segment snake[SNAKEMAXLEN], uint8_t snake_len, struct segment &apple);
int8_t head_is_in_bounds(const struct segment &snake_head);
int8_t head_not_in_snake(const struct segment snake[SNAKEMAXLEN], const uint8_t snake_len);
void render_items(const struct segment snake[SNAKEMAXLEN], const uint8_t snake_len, const struct segment apple);
void move_snake(struct segment snake[SNAKEMAXLEN], const uint8_t snake_len, const uint8_t dir);
int8_t has_ate_apple(const struct segment &snake_head, const struct segment &apple);
void extend_snake(struct segment snake[SNAKEMAXLEN], uint8_t &snake_len);
int direction_sensitive_getter(uint8_t current_dir);

/** Microbit Variables **/
MicroBit uBit;
MicroBitI2C i2c = MicroBitI2C(I2C_SDA0, I2C_SCL0);
MicroBitAccelerometer accelerometer = MicroBitAccelerometer(i2c);
MicroBitButton button_a(MICROBIT_PIN_BUTTON_A, MICROBIT_ID_BUTTON_A);

int main()
{
    // Initialise the micro:bit runtime.
    uBit.init();
    uBit.display.setDisplayMode(DISPLAY_MODE_GREYSCALE);
    while(1)
    {
        // wait for user to start
        if(!button_a.isPressed())
        {
            uBit.display.scrollAsync("<");
            uBit.sleep(20);
            // if button isnt pressed restart loop
            continue;
        }
        //when button is pressed stop animation scrolling
        uBit.display.stopAnimation();

        uBit.seedRandom();
        // set up variables
        struct segment snake[SNAKEMAXLEN];
        // set initial snake head location
        snake[SNAKEHEAD].x = 2;
        snake[SNAKEHEAD].y = 3;
        // snake len is 0 indexed and is also the score
        uint8_t snake_len = 0;

        // initialise direction variables to default(up)
        uint8_t dir = MOVEUP;
        uint8_t dir_temp = MOVEUP;

        // set up apple (this is preset so player doesnt accidentally score)
        struct segment apple;
        apple.x = 3;
        apple.y = 3;

        // set game in progress
        int game_in_progress = 1;
        while (game_in_progress)
        {
            // retrieve and update direction
            dir_temp = direction_sensitive_getter(dir);
            if (dir_temp != 4) {
                dir = dir_temp;
            }
            // move snake in direction
            move_snake(snake, snake_len, dir);

            //test if new position is valid
            if(!(head_is_in_bounds(snake[SNAKEHEAD]) && head_not_in_snake(snake, snake_len)))
            {
                // game over if it is not
                game_in_progress = 0;
            }

            // test if player has scored
            if(has_ate_apple(snake[SNAKEHEAD], apple) == TRUE)
            {
                // if they have increment the snake length and regenerate apple
                snake_len++;
                // if apple generation fails end game
                if(generate_apple(snake, snake_len, apple) == FALSE)
                {
                    //end game you win
                    game_in_progress = 0;
                }
                render_items(snake, snake_len - 1, apple);
            } else
            {
                // only render items, no need to update length
                render_items(snake, snake_len, apple);
            }
            // sleep
            uBit.sleep(900);
        }

        // if player hasn't scored then reset
        if(snake_len == 0)
        {
            uBit.display.clear();
            continue;
        }

        // attempt retrieval of high score from storage
        KeyValuePair* high_score = uBit.storage.get(HIGHSCOREFILE);
        if(high_score == NULL)
        {
            // if NULL ptr recieved then no high score exists, set one
            uBit.storage.put(HIGHSCOREFILE, (uint8_t *)&snake_len, sizeof(uint8_t));
            uBit.display.scroll("Game Over");
            uBit.display.scroll("Score: ");
            uBit.display.scroll(snake_len);
        }
        else
        {
            // there is an old score, retrieve + comapare
            uint8_t old_score = 0;
            memcpy(&old_score, high_score->value, sizeof(uint8_t));
            if(snake_len>old_score)
            {
                // store new high score
                uBit.storage.put(HIGHSCOREFILE, (uint8_t *)&snake_len, sizeof(uint8_t));
                uBit.display.scroll("High Score");
                uBit.display.scroll("Score: ");
                uBit.display.scroll(snake_len);
            }
            else
            {
                // better luck next time, no new high score
                uBit.display.scroll("Game Over");
                uBit.display.scroll("Score: ");
                uBit.display.scroll(snake_len);
            }
        }
    }


    release_fiber();
}

/**
 *Description: This function generates a new location for the apple and returns
 *it via the segment apple whilst returning TRUE.
 *If there is no space left in the grid for the apple then the function exits
 *immediately returning false.
 *This function checks to ensure that the apple is not generated within
 *the snakes body
 */
int8_t generate_apple(const struct segment snake[SNAKEMAXLEN], const uint8_t snake_len, struct segment &apple)
{
    if(snake_len < SNAKEMAXLEN-1)
    {
        //snake still has space
        uint8_t is_not_finished = 1;
        while(is_not_finished) {
            //generate new apple_pos
            apple.x = uBit.random(GRIDMAX_X);
            apple.y = uBit.random(GRIDMAX_Y);
            // validate apple not in snake
            // set isnot finished to 0 in case validate passes
            is_not_finished = 0;
            for (size_t i = 0; i <= snake_len; i++)
            {
                if (snake[i].x == apple.x && snake[i].y == apple.y)
                {
                    //if this is true the apple is in the snake so stop and regenerate apple
                    is_not_finished = 1;
                    break;
                }
            }
        }
        return TRUE;
    }
    return FALSE;
}

/**
 *Description: This function validates that the head position is still on
 *the grid.
 */
int8_t head_is_in_bounds(const struct segment &snake_head)
{
    //check head x and y are both in grid
    if (snake_head.x < 0 || snake_head.x >= GRIDMAX_X)
    {
        return FALSE;
    }
    if (snake_head.y < 0 || snake_head.y >= GRIDMAX_Y)
    {
        return FALSE;
    }
    // if neither statement triggered had is valid
    return TRUE;
}

/**
 *Description: This function iterates over the snake and ensures that the
 *snakes head has not collided with its body on detecting a collision
 *returns FALSE. If no collision detected returns TRUE
 */
int8_t head_not_in_snake(const struct segment snake[SNAKEMAXLEN], const uint8_t snake_len)
{
    // loop through snake
    for (size_t i = 1; i <= snake_len; i++) {
        // check if each segment shares coordinates with the head
        if(snake[SNAKEHEAD].x == snake[i].x)
        {
            if(snake[SNAKEHEAD].y == snake[i].y)
            {
                // return false on a successful match
                return FALSE;
            }
        }
    }
    // if loop finishes then no match found
    return TRUE;
}

/**
 *Description: This function is called whenever the display requires an
 *update due to contents to variables changing
 */
void render_items(const struct segment snake[SNAKEMAXLEN], const uint8_t snake_len, const struct segment apple)
{
    // clear display
    uBit.display.clear();
    // display snake
    for (uint8_t i = 0; i <= snake_len; i++) {
        uBit.display.image.setPixelValue(snake[i].x, snake[i].y, 255);
    }
    // display apple
    uBit.display.image.setPixelValue(apple.x, apple.y, 50);
}

/**
 *Description: Updates the coordinates of the snake segments.
 *Also moves head in the direction specified by dir
 */
void move_snake(struct segment snake[SNAKEMAXLEN], const uint8_t snake_len, const uint8_t dir)
{
    /*iterates through snake from tail end moving segments coordinates to
     *the position of the one ahead.
     */
    for (int8_t i = snake_len; i > 0; i--)
    {
        snake[i].x = snake[i-1].x;
        snake[i].y = snake[i-1].y;
    }
    // updates the coordinates of the head in the direction specified
    switch (dir)
    {
    case MOVEUP:
        snake[SNAKEHEAD].y--;
        break;
    case MOVERIGHT:
        snake[SNAKEHEAD].x++;
        break;
    case MOVEDOWN:
        snake[SNAKEHEAD].y++;
        break;
    case MOVELEFT:
        snake[SNAKEHEAD].x--;
        break;
    default:
        // this should not happen
        uBit.panic(0);
        break;
    }
}

/**
 *Description: Tests whether the head of the snake overlaps with the apple
 *object. If so the snake length should be increased and the apples location
 *regenerated.
 *On detecting an overlap the function returns TRUE, In all other
 *circumstances it returns false
 */
int8_t has_ate_apple(const struct segment &snake_head, const struct segment &apple)
{
    // test if head matches apple coordinates
    if(snake_head.x == apple.x)
    {
        if (snake_head.y == apple.y)
        {
            // return TRUE on successful match
            return TRUE;
        }
    }
    // otherwise return false
    return FALSE;
}

/**
 *Description: increments the snakes length by copying the tail end of the
 *snake to the position behind it This function will fail silently if
 *snake_len is more than or equal to SNAKEMAXLEN
 */
void extend_snake(struct segment snake[SNAKEMAXLEN], uint8_t &snake_len)
{
    snake_len++;

    // Test if extending snake would cause seg fault
    if(snake_len < SNAKEMAXLEN)
    {
        snake[snake_len].x = snake[snake_len-1].x;
        snake[snake_len].y = snake[snake_len-1].y;
    }
}

/**
 *Description: Retreives values from the accelerometer and converts
 *them to a value that can be used by the move_snake function which is returned.
 *this function uses the current_dir to test whether the direction it returns
 *is valid (the snake is unable to immediately turn back on itself). In the
 *case of no direction being valid (int)4 is returned
 */
int direction_sensitive_getter(uint8_t current_dir)
{
    // get accelerometer values
    int32_t x = accelerometer.getX();
    int32_t y = accelerometer.getY();

    // test x and y values for new direction
    if(x<-350)
    {
        // if detected then check if its a valid direction
        if(current_dir != MOVERIGHT)
            return MOVELEFT;
    }
    if(x>350)
    {
        if (current_dir != MOVELEFT)
            return MOVERIGHT;
    }
    if(y<-350)
    {
        if(current_dir != MOVEDOWN)
            return MOVEUP;
    }
    if(y>350)
    {
        if(current_dir != MOVEUP)
            return MOVEDOWN;
    }
    return 4;// stay default
}
