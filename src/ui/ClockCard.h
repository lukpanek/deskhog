#pragma once

#include <lvgl.h>
#include "ui/InputHandler.h"
#include <time.h>

class ClockCard : public InputHandler {
public:
    ClockCard(lv_obj_t* parent);
    ~ClockCard();
    
    lv_obj_t* getCard() const { return _card; }
    
    bool handleButtonPress(uint8_t button_index) override;
    bool update() override; // Called regularly to update the time display
    void prepareForRemoval() override { _card = nullptr; }

private:
    lv_obj_t* _card;
    lv_obj_t* _background;
    lv_obj_t* _time_label;
    lv_obj_t* _date_label;
    
    // Time display settings
    bool _is_24_hour_format;
    bool _show_seconds;
    
    // Time tracking
    time_t _last_update_time;
    bool _sync_attempted;
    
    // Timer functionality
    enum TimerState {
        CLOCK_MODE,      // Normal clock display
        TIMER_RUNNING,   // Timer is running
        TIMER_STOPPED    // Timer is stopped, showing elapsed time
    };
    TimerState _timer_state;
    unsigned long _timer_start_millis;
    unsigned long _timer_elapsed_millis;
    unsigned long _last_timer_update_millis;
    
    // Private methods
    void updateTimeDisplay();
    void formatTime(char* buffer, size_t buffer_size);
    void formatDate(char* buffer, size_t buffer_size);
    void formatTimer(char* buffer, size_t buffer_size, unsigned long elapsed_millis);
    bool isTimeValid();
    void setupUI(lv_obj_t* parent);
    void initializeTimeSync();
    void updateBackgroundColor();
    lv_color_t getTimeBasedColor();
};