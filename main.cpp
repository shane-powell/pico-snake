#include "pico_display.hpp"
#include <string>
#include "pico/multicore.h"
#include "pico/stdlib.h"

using namespace pimoroni;

uint16_t buffer[PicoDisplay::WIDTH * PicoDisplay::HEIGHT];
PicoDisplay picoDisplay(buffer);

uint16_t maxX = PicoDisplay::WIDTH;
uint16_t minX = 0;
uint16_t maxY = PicoDisplay::HEIGHT;
uint16_t minY = 14;
char gameState = 'T';
bool sound = false;

float buttonTimeout = -1;
float buttonTimeoutOnPress = 1.0;

float movementCounter = 0;

void count()
{
  while (true)
  {
    if (buttonTimeout > -1)
    {
      buttonTimeout -= 0.25;
    }

    if (gameState == 'G')
    {
      movementCounter += 1;
    }
    else
    {
      movementCounter = 0;
    }

    sleep_ms(100);
  }
}

struct Coords
{
public:
  uint16_t X = 0;
  uint16_t Y = 0;
  bool Active = false;
};

struct Player
{
  uint16_t X = 0;
  uint16_t Y = 0;
  char dir;
  int Len = 1;
  static const int MaxLength = 100;
  Coords Moves[MaxLength];
};

Player p1;
Coords foodLocation;
Coords previousPosition;

int score = 0;
int arrayPosition = 0;

void GenerateFood()
{
  foodLocation.X = (random() % maxX - 1) + 1;
  foodLocation.Y = (random() % maxY - 1) + 1 + 12;
}

void SetButtonTimeout()
{
  buttonTimeout = buttonTimeoutOnPress;
}

void StartGame()
{

  p1.X = maxX / 2;
  p1.Y = (maxY / 2) + 14;
  p1.dir = 'U';

  GenerateFood();
  score = 0;
  arrayPosition = 0;
  p1.Len = 1;
  for (int i = 0; i < p1.MaxLength; i++)
  {
    p1.Moves[i].Active = false;
  }

  gameState = 'G';
}

void LogMove()
{
  p1.Moves[arrayPosition].Y = p1.Y;

  if (p1.Moves[arrayPosition].Active == false)
  {
    p1.Moves[arrayPosition].Active = true;
  }
  p1.Moves[arrayPosition].X = p1.X;
  p1.Moves[arrayPosition].Y = p1.Y;

  if (arrayPosition < p1.Len - 1)
  {
    arrayPosition += 1;
  }
  else
  {
    arrayPosition = 0;
  }
}

void EndGame()
{
  if (sound == true)
  {
    // No sound output :(
  }

  gameState = 'E';
}

void CollisionDetection()
{

  //Check if hit wall

  if (p1.X >= maxX || p1.X <= minX || p1.Y >= maxY || p1.Y <= minY)
  {
    printf("\nHit wall\n");
    printf(std::to_string(p1.X).c_str());
    printf("\n");
    printf(std::to_string(p1.Y).c_str());
    printf("\n");

    EndGame();
  }

  //Check if hit tail
  for (int i = 0; i < p1.MaxLength; i++)
  {
    if (p1.Moves[i].Active == true)
    {
      if (p1.Moves[i].X == p1.X && p1.Moves[i].Y == p1.Y)
      {
        printf("\nHit tail\n");
        printf(std::to_string(p1.X).c_str());
        printf("\n");
        printf(std::to_string(p1.Y).c_str());
        printf("\n");
        EndGame();
      }
    }
  }

  // Check if on food
  if (p1.X == foodLocation.X && p1.Y == foodLocation.Y)
  {
    if (p1.Len < p1.MaxLength)
    {
      p1.Len += 1;
    }
    if (sound == true)
    {
      // No sound output :(
    }

    score += 1;
    LogMove();
    GenerateFood();
  }
}

void DrawTail()
{
  int tailPosition = arrayPosition;
  for (int i = 0; i < p1.Len; i++)
  {
    if (p1.Moves[tailPosition].Active == true)
    {
      picoDisplay.pixel(Point(p1.Moves[tailPosition].X, p1.Moves[tailPosition].Y));
    }
    else
    {
      return;
    }

    if (tailPosition > 0)
    {
      tailPosition -= 1;
    }
    else
    {
      tailPosition = p1.Len - 1;
    }
  }
}

void DrawFrame()
{
  picoDisplay.set_pen(255, 255, 255);
  picoDisplay.rectangle(Rect(minX, minY, maxX, (maxY - minY)));
  picoDisplay.set_pen(0, 0, 0);
  picoDisplay.rectangle(Rect(minX + 1, minY + 1, maxX - 2, (maxY - minY) - 2));
  picoDisplay.set_pen(255, 255, 255);
}

int main()
{

  stdio_init_all();
  multicore_launch_core1(count);

  picoDisplay.init();

  // set the backlight to a value between 0 and 255
  // the backlight is driven via PWM and is gamma corrected by our
  // library to give a gorgeous linear brightness range.
  picoDisplay.set_backlight(100);

  while (true)
  {

    // first we clear our screen to black
    picoDisplay.set_pen(0, 0, 0);
    picoDisplay.clear();
    picoDisplay.set_pen(255, 255, 255);

    if (gameState == 'G')
    {
      DrawFrame();

      if (picoDisplay.is_pressed(picoDisplay.B) && buttonTimeout <= 0)
      {
        switch (p1.dir)
        {
        case 'U':
          p1.dir = 'L';
          break;
        case 'L':
          p1.dir = 'D';
          break;
        case 'D':
          p1.dir = 'R';
          break;
        case 'R':
          p1.dir = 'U';
          break;
        }

        //p1.dir = p1.dir->left;
        SetButtonTimeout();
      }

      if (picoDisplay.is_pressed(picoDisplay.Y) && buttonTimeout <= 0)
      {
        switch (p1.dir)
        {
        case 'U':
          p1.dir = 'R';
          break;
        case 'L':
          p1.dir = 'U';
          break;
        case 'D':
          p1.dir = 'L';
          break;
        case 'R':
          p1.dir = 'D';
          break;
        }

        //p1.dir = p1.dir->right;
        //picoDisplay.text("Y", Point(20, 20), 0);
        //printf(std::to_string(p1.dir).c_str());
        SetButtonTimeout();
      }

      picoDisplay.text(std::string(1, p1.dir).c_str(), Point(20, 20), 0);

      if (movementCounter >= 1)
      {
        switch (p1.dir)
        {
        case 'U':
          if (p1.Y > minY)
          {
            p1.Y -= 1;
          }
          break;
        case 'D':
          if (p1.Y < maxY)
          {
            p1.Y += 1;
          }
          break;
        case 'L':
          if (p1.X > minX)
          {
            p1.X -= 1;
          }
          break;
        case 'R':
          if (p1.X < maxX)
          {
            p1.X += 1;
          }
          break;
        }
        movementCounter = 0;
        CollisionDetection();
        printf("\nDraw\n");
        printf(std::to_string(p1.X).c_str());
        printf("\n");
        printf(std::to_string(p1.Y).c_str());
        printf("\n");
      }

      picoDisplay.pixel(Point(p1.X, p1.Y));

      picoDisplay.set_pen(0, 255, 0);
      picoDisplay.pixel(Point(foodLocation.X, foodLocation.Y));
      picoDisplay.set_pen(255, 255, 255);

      if (p1.X != previousPosition.X || p1.Y != previousPosition.Y)
      {
        previousPosition.X = p1.X;
        previousPosition.Y = p1.Y;
        LogMove();
      }
      DrawTail();

      std::string scoreString = "Score ";

      picoDisplay.text(scoreString.append(std::to_string(score).c_str()), Point(2, 0), picoDisplay.WIDTH);

      picoDisplay.text(std::to_string(p1.X), Point(50, 50), 0);
      picoDisplay.text(std::to_string(p1.Y), Point(50, 70), 0);
    }
    else if (gameState == 'E')
    {
      picoDisplay.text("Game Over", Point(20, 20), picoDisplay.WIDTH);

      std::string scoreString = "Score ";

      picoDisplay.text(scoreString.append(std::to_string(score).c_str()), Point(20, 40), picoDisplay.WIDTH);

      if ((picoDisplay.is_pressed(picoDisplay.B) && buttonTimeout == 0) || (picoDisplay.is_pressed(picoDisplay.Y) && buttonTimeout <= 0))
      {
        gameState = 'T';
        SetButtonTimeout();
      }
    }
    else if (gameState == 'T')
    {
      picoDisplay.text("Pico-Snake", Point(50, 1), picoDisplay.WIDTH);

      picoDisplay.text("By Shane Powell", Point(20, 15), picoDisplay.WIDTH);

      if ((picoDisplay.is_pressed(picoDisplay.B) && buttonTimeout == 0) || (picoDisplay.is_pressed(picoDisplay.Y) && buttonTimeout <= 0))
      {
        StartGame();
        SetButtonTimeout();
      }
    }
    // now we've done our drawing let's update the screen
    picoDisplay.update();
  }
}
