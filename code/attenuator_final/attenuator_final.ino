#include <Arduino.h>
#include <math.h>

/*
 * By: Ryan Forelli
 * M3 Learning group
 * Current setup has ~3700 steps per degree (with MS 1, 2, and 3 set high)
 * Default speed is 5 deg/sec
*/

unsigned long steps_rotate_per_rev = 1332000;              //Steps required for one revolution
unsigned long max_angle = 90;                              //Maximum allowed angle for attenuator
int steps_per_deg = 3700;                                  //Steps per degree
unsigned long steps_max_angle = max_angle * steps_per_deg; //Steps to maximum angle

float curr_angle = 0;                                      //Current angle
float dest_angle = -1;                                     //Destination angle
float angle_rotate = -1;                                   //Degrees to rotate to reach destination angle
unsigned long steps_rotate;                                //Steps to rotate to reach destination

char command;                                              //Command from user

unsigned long delay_micros;                                //Delay in microseconds to achieve desired angular velocity

//Macros
#define CW HIGH                                            //Clockwise
#define CCW LOW                                            //Counter Clockwise
#define DIR_PIN 7                                          //Dir pin on A4988
#define STEP_PIN 6                                         //Step pin on A4988
#define LASER_BLOCK 11                                     //Rotary solenoid pin
#define HOME_SWITCH 8                                      //Limit switch pin
#define MIN_SPEED 1                                        //Minimum speed in deg/sec
#define MAX_SPEED 8                                        //Maximum speed in deg/sec
#define STEP                      \
    digitalWrite(STEP_PIN, HIGH); \
    digitalWrite(STEP_PIN, LOW);  \
    delayMicroseconds(delay_micros)
#define setRotationSpeed(new_rotation_speed) ((1 / (double)(steps_per_deg * new_rotation_speed)) * 1000000) //Set rotation speed
#define setDirection(dir) (digitalWrite(DIR_PIN, dir))                                                      //Set direction
#define BLOCK_LASER digitalWrite(LASER_BLOCK, LOW)                                                          //Block laser path
#define CLEAR_LASER digitalWrite(LASER_BLOCK, HIGH)                                                         //Clear laser path

void setup(){
    Serial.begin(9600);

    //Set inputs and outputs
    pinMode(LASER_BLOCK, OUTPUT);
    pinMode(HOME_SWITCH, INPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);
    delay_micros = setRotationSpeed(5);
}

void loop(){
    //Check for available characters to read from serial port
    if (Serial.available() > 0){ Serial.readBytes(&command, 1); }

    switch (command){
        case ',':{ //Rotate to the desired angle
            dest_angle = Serial.parseFloat(SKIP_NONE, ',');
            if ((dest_angle != curr_angle) && (dest_angle <= max_angle) && (dest_angle >= 0)){
                angle_rotate = abs(curr_angle - dest_angle);
                setDirection(((curr_angle - dest_angle) > 0));
                curr_angle = dest_angle;
                steps_rotate = map(angle_rotate, 0, max_angle, 0, steps_max_angle);
                Serial.println('\0');
                for(int i=0;i<steps_rotate;i++){
                    STEP;
                }
            }
            break;
        }case '#':{ //Set rotation speed
            int new_speed = Serial.parseFloat(SKIP_NONE, '#');
            delay_micros = (new_speed <= MAX_SPEED && new_speed >= MIN_SPEED) ? setRotationSpeed(new_speed) : delay_micros;
            break;
        }case 'o':{ //Home the attenuator
            setDirection(CW);
            while (1){
                if (digitalRead(HOME_SWITCH)){
                    curr_angle = 0;
                    break;
                }
                STEP;
            }
            break;
        }case 'f':{ //Clear laser
            CLEAR_LASER;
            break;
        }case 'g':{ //Block laser
            BLOCK_LASER;
            break;
        }default:
            break;
    }
    command = '\0';
}
