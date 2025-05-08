#pragma once

#include "pigpio.h"
#include <opencv2/opencv.hpp>
#include <sstream>
#include <string>
#include <iostream>
#include <unistd.h>

/**
*
* @brief The C++ object used for communication with the embedded system
*
* This class sends given values on a selected com port
*
* @author Kaveh A.
*
*/
class CControlPi
   {
   public:
      CControlPi();
      ~CControlPi();

      /** @brief It storts received data from comport in &result
      *
      * @param type Whether the type of received data is analog or digital
      * @param channel The channel for receiving data
      * @param &result Is the address of where the recevied data should be stored
      *
      * @return rturns whether the communication was successful
      */
      bool get_data(int channel, int &result);

      /** @brief Writes/Sets data, val, on a channel
      *
      * @param type Whether the type of received data is analog, digital, or servo
      * @param channel The channel for writing the data on
      * @param val The value being sent to the selected channel
      *
      * @return returns 1 if setting was successful, otherwise 0
      */
      bool set_data(int channel, int val);

      /** @brief Calls the get_data function for receving analog data and converting it through ADC
      * @param channel The channel for receiving data
      *
      * @return Returns the analog input as a % of the full scale (i.e. a 12 bit ADC would return X / 4096)
      */
      double get_analog(int channel);

      /** @brief Reads a digital input and debounces it (using a 1 second timeout)
      *
      * @param channel The channel for receiving data
      *
      * @return returns the data from the debounced button
      */
      int get_button(int channel);


      /** @brief Reads servo value
      *
      * @param channel The channel for reading data
      * @param position of the servo
      *
      * @return returns 1
      */
      int get_servo(int channel, int& position);

      /** @brief Writes a value to servo
      *
      * @param channel The channel for writing data
      *
      * @return returns 1
      */
      bool set_servo(int channel, int val);

   private:

   };
