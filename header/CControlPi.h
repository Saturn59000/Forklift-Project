#pragma once

#include <string>
#include <opencv2/opencv.hpp>
#include "pigpio.h"

#define BAUD_RATE 115200
#define TX_BUF 2
#define BUFFLEN 1
#define TIMEOUT 1.0
#define SUBSTR_INDEX 1
#define ADC_MAX 1024
#define ADC_OFFSET 128
#define PERCENT 100
#define JOYX 0
#define JOYY 1
#define BUTTON1 19
#define BUTTON2 13
#define DEBOUNCE 0.150
#define INIT 0x1234567
#define SERVO_MAX 180
#define SERVO_MIN 1
#define SERVO_1 0
#define TEXT_READ_DELAY 1500
#define BLUE_LED_CH 37
#define RED_LED_CH 39
#define GREEN_LED_CH 38
#undef ERR
#define ERR -1

#undef DIGITAL
#undef ANALOG
#undef SERVO

#define DIGITAL 0
#define ANALOG 1
#define SERVO 2


/**
*
* @brief Communicates with the microcontroller over com port
*
* This class is meant to provide an interface between the program and
* the microcontroller
*
* @author Derek Wilson
*/

class CControlPi
{
private:
    bool _program_init; ///< Is true if it is the very first time init_com is called
    bool _comm_failed;

    float _last_button1_state;
    float _last_button2_state;
    float _start_time1;
    float _start_time2;
    /**
    *
    * @brief parses open com port and times out if no data
    *
    `    * @return string
    *
    */
    std::string _read_port();

public:

    /**
    *
    * @brief CClass Constructor
    *
    * @return void
    *
    */
    CControlPi();

    /**
    *
    * @brief CClass Deconstructor
    *
    * @return void
    *
    */
    ~CControlPi();

    /**
    *
    * @brief Searches for com port and initializes
    *
    *
    * @return true if initialization was succesful, false if not no user prompt on first call), _init_flag is made true if user wishes to stop searching for device
    *
    */
    bool init_com();

    /**
    *
    * @brief Gets data from microcontroller
    *
    * @param type the desired data type (0: Digital, 1: Analog, 2: Servo)
    * @param channel the channel to read data from (Digital: 0-40 Analog: 0-11 Servo: 0-3)
    * @param result value sent from microcontroller
    *
    * @return true if get data was succesful
    */
    bool get_data(int type, int channel, int& result);

    /**
    *
    * @brief Sets data on microcontroller
    *
    * @param type the desired data type (0: Digital, 1: Analog, 2: Servo)
    * @param channel the channel to write data to (Digital: 0-40 Analog: 0-11 Servo 0-3) Note: no analog write
    * @param val value sent to microcontroller
    *
    * @return true if set data was succesful
    */
    bool set_data(int type, int channel, int val);

    /**
    *
    * @brief Gets % full scale value of analog reading from microcontroller
    *
    * @return % of full scale analog value
    */
    float get_analog(int channel);

    /**
    *
    * @brief Reads digital input and debounces it
    *
    * @return true if valid, false if not
    */
    bool get_button(int channel);

    /**
    *
    * @brief getter for _init_flag
    *
    * @return state of _init_flag
    */
    bool get_comm_failed();

};
