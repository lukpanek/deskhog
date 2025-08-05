#include "ui/ClockCard.h"
#include "Style.h"
#include "hardware/Input.h"
#include <time.h>
#include <sys/time.h>
#include <Arduino.h>
// Removed WiFi.h include to avoid early boot issues

ClockCard::ClockCard(lv_obj_t* parent) 
    : _card(nullptr)
    , _background(nullptr)
    , _time_label(nullptr) 
    , _date_label(nullptr)
    , _is_24_hour_format(true)
    , _show_seconds(true)
    , _last_update_time(0)
    , _sync_attempted(false)
    , _timer_state(CLOCK_MODE)
    , _timer_start_millis(0)
    , _timer_elapsed_millis(0)
    , _last_timer_update_millis(0) {
    
    setupUI(parent);
}

ClockCard::~ClockCard() {
    if (_card) {
        lv_obj_del_async(_card);
        _card = nullptr;
    }
}

void ClockCard::setupUI(lv_obj_t* parent) {
    // Create the main card container
    _card = lv_obj_create(parent);
    lv_obj_set_size(_card, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(_card, lv_color_black(), 0);
    lv_obj_set_style_pad_all(_card, 5, 0);
    lv_obj_set_style_border_width(_card, 0, 0);
    lv_obj_set_style_radius(_card, 0, 0);

    // Create background panel with rounded corners (like QuestionCard)
    _background = lv_obj_create(_card);
    lv_obj_set_size(_background, lv_pct(100), lv_pct(100));
    lv_obj_set_style_radius(_background, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(_background, getTimeBasedColor(), 0);
    lv_obj_set_style_border_width(_background, 0, 0);
    lv_obj_set_style_pad_all(_background, 10, 0);

    // Create a flex container for vertical layout
    lv_obj_t* flex_col = lv_obj_create(_background);
    lv_obj_set_size(flex_col, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(flex_col, 15, 0);
    lv_obj_set_style_pad_row(flex_col, 8, 0);
    lv_obj_set_flex_flow(flex_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(flex_col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(flex_col, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(flex_col, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(flex_col, 0, 0);
    
    // Create time label with LARGE loud noises font
    _time_label = lv_label_create(flex_col);
    lv_label_set_text(_time_label, "--:--:--");
    lv_obj_set_style_text_color(_time_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(_time_label, Style::loudNoisesFontLarge(), 0);
    lv_obj_set_style_text_align(_time_label, LV_TEXT_ALIGN_CENTER, 0);
    
    // Create date label with smaller font
    _date_label = lv_label_create(flex_col);
    lv_label_set_text(_date_label, "Time not synced");
    lv_obj_set_style_text_color(_date_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(_date_label, Style::labelFont(), 0);
    lv_obj_set_style_text_align(_date_label, LV_TEXT_ALIGN_CENTER, 0);
}

bool ClockCard::handleButtonPress(uint8_t button_index) {
    // Center button cycles through timer states
    if (button_index == Input::BUTTON_CENTER) {
        switch (_timer_state) {
            case CLOCK_MODE:
                // Start timer
                _timer_state = TIMER_RUNNING;
                _timer_start_millis = millis();
                break;
                
            case TIMER_RUNNING:
                // Stop timer and record elapsed time
                _timer_state = TIMER_STOPPED;
                _timer_elapsed_millis = millis() - _timer_start_millis;
                break;
                
            case TIMER_STOPPED:
                // Go back to clock mode
                _timer_state = CLOCK_MODE;
                _timer_start_millis = 0;
                _timer_elapsed_millis = 0;
                break;
        }
        updateTimeDisplay();
        return true;
    }
    
    return false;
}

bool ClockCard::update() {
    bool should_update = false;
    
    if (_timer_state == TIMER_RUNNING) {
        // Update timer display every 50ms for smooth millisecond display
        unsigned long current_millis = millis();
        if (current_millis - _last_timer_update_millis >= 50) {
            should_update = true;
            _last_timer_update_millis = current_millis;
        }
    } else {
        // Update clock display every second for normal time
        time_t current_time = time(nullptr);
        if (current_time != _last_update_time) {
            should_update = true;
            _last_update_time = current_time;
        }
    }
    
    if (should_update) {
        updateTimeDisplay();
    }
    
    return true; // Keep receiving updates
}

void ClockCard::updateTimeDisplay() {
    char time_buffer[32];
    char date_buffer[32];
    
    // Handle different display modes
    switch (_timer_state) {
        case CLOCK_MODE:
            // Normal clock display
            // Try to sync time once if not attempted yet
            if (!_sync_attempted && !isTimeValid()) {
                initializeTimeSync();
                _sync_attempted = true;
            }
            
            if (isTimeValid()) {
                formatTime(time_buffer, sizeof(time_buffer));
                formatDate(date_buffer, sizeof(date_buffer));
                updateBackgroundColor(); // Update background based on time of day
            } else {
                // Show that time is not synced yet
                snprintf(time_buffer, sizeof(time_buffer), "--:--:%s", _show_seconds ? "--" : "");
                snprintf(date_buffer, sizeof(date_buffer), "Syncing time...");
            }
            break;
            
        case TIMER_RUNNING:
            // Show running timer
            {
                unsigned long current_elapsed_millis = millis() - _timer_start_millis;
                formatTimer(time_buffer, sizeof(time_buffer), current_elapsed_millis);
                snprintf(date_buffer, sizeof(date_buffer), "Timer running");
            }
            break;
            
        case TIMER_STOPPED:
            // Show stopped timer
            formatTimer(time_buffer, sizeof(time_buffer), _timer_elapsed_millis);
            snprintf(date_buffer, sizeof(date_buffer), "Timer stopped");
            break;
    }
    
    if (_time_label) {
        lv_label_set_text(_time_label, time_buffer);
    }
    
    if (_date_label) {
        lv_label_set_text(_date_label, date_buffer);
    }
}

void ClockCard::formatTime(char* buffer, size_t buffer_size) {
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    
    if (_is_24_hour_format) {
        if (_show_seconds) {
            strftime(buffer, buffer_size, "%H:%M:%S", &timeinfo);
        } else {
            strftime(buffer, buffer_size, "%H:%M", &timeinfo);
        }
    } else {
        if (_show_seconds) {
            strftime(buffer, buffer_size, "%I:%M:%S %p", &timeinfo);
        } else {
            strftime(buffer, buffer_size, "%I:%M %p", &timeinfo);
        }
    }
}

void ClockCard::formatDate(char* buffer, size_t buffer_size) {
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    
    strftime(buffer, buffer_size, "%d.%m.%Y", &timeinfo);
}

void ClockCard::formatTimer(char* buffer, size_t buffer_size, unsigned long elapsed_millis) {
    unsigned long total_seconds = elapsed_millis / 1000;
    unsigned long milliseconds = elapsed_millis % 1000;
    
    int hours = total_seconds / 3600;
    int minutes = (total_seconds % 3600) / 60;
    int seconds = total_seconds % 60;
    
    if (hours > 0) {
        // Over 1 hour: show HH:MM:SS format without milliseconds
        snprintf(buffer, buffer_size, "%02d:%02d:%02d", hours, minutes, seconds);
    } else {
        // Under 1 hour: show MM:SS.mmm format with milliseconds
        snprintf(buffer, buffer_size, "%02d:%02d.%03lu", minutes, seconds, milliseconds);
    }
}

void ClockCard::initializeTimeSync() {
    // Set timezone to Prague (Central European Time)
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();
    
    // Configure NTP time sync
    configTime(0, 0, "time.fi.muni.cz", "time.nist.gov");
}

void ClockCard::updateBackgroundColor() {
    if (_background) {
        lv_obj_set_style_bg_color(_background, getTimeBasedColor(), 0);
    }
}

lv_color_t ClockCard::getTimeBasedColor() {
    if (!isTimeValid()) {
        return lv_color_hex(0x363636); // Gray for unsynced time
    }
    
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    int hour = timeinfo.tm_hour;
    
    // Time-based color scheme inspired by Prague sky colors
    if (hour >= 6 && hour < 9) {
        // Dawn - soft orange/pink
        return lv_color_hex(0xC2410C); 
    } else if (hour >= 9 && hour < 12) {
        // Morning - bright blue
        return lv_color_hex(0x2563EB); 
    } else if (hour >= 12 && hour < 17) {
        // Afternoon - warm yellow/gold
        return lv_color_hex(0xCA8A04); 
    } else if (hour >= 17 && hour < 20) {
        // Evening - purple/violet
        return lv_color_hex(0x059669); 
    } else if (hour >= 20 && hour < 22) {
        // Dusk - deep purple
        return lv_color_hex(0x9333EA); 
    } else {
        // Night - dark blue
        return lv_color_hex(0x4F46E5); 
    }
}

bool ClockCard::isTimeValid() {
    time_t now = time(nullptr);
    // Simplified validation without static variables or debug output
    return now > (8 * 3600 * 2); // 57600 seconds after epoch
}