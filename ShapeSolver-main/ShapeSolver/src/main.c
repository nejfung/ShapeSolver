#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "ece198.h"

#define LED_GPIO_PORT GPIOA
#define LED_PIN GPIO_PIN_5

#define BUTTON_GPIO_PORT GPIOC
#define BUTTON_PIN GPIO_PIN_13

#define MORSE_LIGHTS
//#define RANDOM

//standard time for morse code dots and dashes
const int standard_time = 250;

//morse alphabet for converting text to morse code
const char morse_alphabet[26][2][5] = {
  {".-", 'a'},
  {"-...", 'b'},
  {"-.-.", 'c'},
  {"-..", 'd'},
  {".", 'e'},
  {"..-.", 'f'},
  {"--.", 'g'},
  {"....", 'h'},
  {"..", 'i'},
  {".---", 'j'},
  {"-.-", 'k'},
  {".-..", 'l'},
  {"--", 'm'},
  {"-.", 'n'},
  {"---", 'o'},
  {".--.", 'p'},
  {"--.-", 'q'},
  {".-.", 'r'},
  {"...", 's'},
  {"-", 't'},
  {"..-", 'u'},
  {"...-", 'v'},
  {".--", 'w'},
  {"-..-", 'x'},
  {"-.--", 'y'},
  {"--..", 'z'}
};

//array for difficulty level 1
char shapes_level1[7][2][10] = {{"square",4}, {"triangle", 3}, {"rectangle", 4}, {"hexagon", 6}, {"octagon", 8}, 
{"pentagon", 5}, {"diamond", 4}};

//array for difficulty level 2
char shapes_level2[5][2][10] = {{"heart", 2}, {"star", 10}, {"arrow", 7}, {"crescent", 2}, {"cross", 12}};

//array for difficulty level 3
char shapes_level3[8][2][30] = {{"clock", 1}, {"window", 4}, {"stopsign", 8}, {"flagofnepal", 5}, {"paper", 4}, 
{"tire", 1}, {"protractor", 2}, {"sliceofpizza", 3}};

//functions
void print_morse(char *string);
void dot();
void dash();
void correct();
void wrong();
void next_level();
void win();
int count_clicks();
void delay(int time);
int get_shape (int level);

int main(void)
{
    
    HAL_Init();

    SerialSetup(9600);

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;

    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_PIN, GPIO_PIN_RESET);

    //wait until it's clicked
    while(HAL_GPIO_ReadPin(GPIOC, BUTTON_PIN));
    //wait until it's clicked
    while(HAL_GPIO_ReadPin(GPIOC, BUTTON_PIN));

    

    //variable to store the number of clicks
    int clicks;
    char string_to_pass;
    int current_difficulty = 1;
    int current_right = 0;
    //seed a random number based on the number of items in the list per difficulty level
    int random = get_shape(current_difficulty);


    while(1){

        //use the print_morse function to blink the first value of string_to_pass in morse code
        //chose which list to pull the data from based on difficulty_level

        
        if(current_difficulty == 1)
            print_morse(&shapes_level1[random][0]);
        else if(current_difficulty == 2)
            print_morse(&shapes_level2[random][0]);
        else if(current_difficulty == 3)
            print_morse(&shapes_level3[random][0]);

        //standard_time*7 is the standard delay between words
        delay(standard_time*7);

        //use count_clicks function to set the number of clicks
        clicks = count_clicks();

        //if the number of clicks is equal to the second value of the item in the list
        if((current_difficulty == 1 && clicks == shapes_level1[random][1][0]) || (current_difficulty == 2 && clicks == shapes_level2[random][1][0]) || (current_difficulty == 3 && clicks == shapes_level3[random][1][0])){

            //get a new random numer
            random = get_shape(current_difficulty);

            ++current_right;
            //if it's time to move to the next difficulty
            if(current_right ==3 ){
                //reset the current number of correct in a row
                current_right = 0;
                //if all levels are passed
                if(current_difficulty == 3){
                    //display win blinking lights
                    win();
                    //end program
                    break;
                }
                //add one to the current difficulty
                ++current_difficulty;
                //run lights to show moving to the next level
                next_level();
            }
            else{
                correct();
            }
        }
        else if(clicks != 0){
            //run the wrong display
            wrong();
            //get a new random number
            random = get_shape(current_difficulty);
            current_right = 0;
        }
        
    }
}

void delay(int time){
    //function to delay (used for on/off for morse code)
    //use instead of normal delay function so it can be broken if the button is clicksd
    int counter = 0;
    
    //repeat until the delay is finished (counter = time) or the button is clicked
    while(counter < time && HAL_GPIO_ReadPin(GPIOC, BUTTON_PIN)){
        //increase counter by one and delay 1 ms
        counter+=1;
        HAL_Delay(1);
    }
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}

void print_morse(char *string){

  //run for each letter in the string
  for(int j = 0; j < strlen(string); j++){
    char character = string[j];

    //run for each letter in the alphabet
    for(int i = 0; i < 26; i++){
      //if the letter in the string equals the letter in the alphabet
      if(morse_alphabet[i][1][0] == character){

        int index = 0;

        while(1){
            //if it's the null character (end of string), break
            if(morse_alphabet[i][0][index] == '\0')
                break;
            else if(morse_alphabet[i][0][index] == '.')
                dot();
            else if(morse_alphabet[i][0][index] == '-')
                dash();

            ++index;
        }
      }
    }

    //use delay function for standard time between character (two)
    delay(standard_time*3);

  }

}

//function to flash a dot (one unit on, one unit off)
void dot(){
    //on
    HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
    delay(standard_time);

    //off
    HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
    delay(standard_time);
}

//function to flash a dash (three units on, one unit off)
void dash(){
    //on
    HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
    delay(standard_time*3);

    //off
    HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
    delay(standard_time);
}

//function to flash a wrong (two quick blinks)
void wrong(){
    //on
    for(int i = 0 ; i < 4; i++){
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
        HAL_Delay(120); 
    }
    delay(1000); 
}

//function to flash a correct (three quick blinks)
void correct(){
    //on
    for(int i = 0 ; i < 6; i++){
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
        HAL_Delay(120); 
    }
    delay(1000); 
}

//function to flash next level (four quick blinks)
void next_level(){
    //on
    for(int i = 0 ; i < 8; i++){
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
        HAL_Delay(120); 
    }
    delay(1000); 
}

//function to flash a win (twenty quick blinks)
void win(){
    //on
    for(int i = 0 ; i < 40; i++){
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
        HAL_Delay(50); 
    }
}


//function to count clicks when the button is clicked
int count_clicks(){
    //start the amount of clicks at 0
    int counter = 0;
    //store the current time
    int time_click = HAL_GetTick();

    while (true){
        //while the button is not clicked
        while(HAL_GPIO_ReadPin(GPIOC, BUTTON_PIN)){
            //check if the time since the last click is over 1.5s
            if((HAL_GetTick() - time_click) > 1500){
                //return the amount of clicks
                return counter;
            }
        }
        //once the button is clicked, increase the amount of clicks
        counter++;
        //set the time that the button is clicked
        time_click = HAL_GetTick();
        //wait until the button is not clicked
        while(!HAL_GPIO_ReadPin(GPIOC, BUTTON_PIN));
        
    }

}


int get_shape (int level) {
    srand(HAL_GetTick());

    //the random value corresponds to the index of the array.
    //must be smaller than capacity - 1
    int max = level;

    if (level == 1) {
        max = 7;
    } else if (level == 2) {
        max = 5;
    } else {
        max = 8;
    }

    int random_number = (rand() % max);

    return random_number;
  
}
