#pragma once

#include <opencv2/opencv.hpp>

/**
 * @brief Abstract base class for lab-style objects that do update() and draw().
 */
class CBase4618
{
public:
    virtual ~CBase4618() {}

    /**
     * @brief Called each loop to update logic.
     */
    virtual void update() = 0;

    /**
     * @brief Called each loop to draw UI, show camera frames, etc.
     */
    virtual void draw() = 0;

    /**
     * @brief Default run loop that repeatedly calls update() and draw().
     *        Press 'q' or ESC to break out.
     */
    virtual void run();

    bool _exit = false;
};
