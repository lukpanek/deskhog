#pragma once

#include <lvgl.h>
#include <Arduino.h>
#include <memory>
#include <functional>
#include <vector>
#include <string>
#include "ui/InputHandler.h"

/**
 * @class LukasCard
 * @brief UI component for displaying rotating Czech phrases about Lukáš Pánek
 * 
 * Features:
 * - Rotating phrases loaded from JSON file
 * - Center button interaction for cycling phrases
 * - Custom styling and animations similar to QuestionCard
 * - Auto-scrolling for long text
 */
class LukasCard : public InputHandler {
public:
    /**
     * @brief Constructor
     * @param parent LVGL parent object
     */
    LukasCard(lv_obj_t* parent);
    
    /**
     * @brief Destructor - safely cleans up UI resources
     */
    ~LukasCard();
    
    /**
     * @brief Get the underlying LVGL card object
     * @return LVGL object pointer or nullptr if not created
     */
    lv_obj_t* getCard();
    
    /**
     * @brief Add a phrase to the rotation
     * @param phrase The phrase to add
     */
    void addPhrase(const char* phrase);
    
    /**
     * @brief Cycle to the next phrase
     */
    void cycleNextPhrase();
    
    /**
     * @brief Handle button press events
     * @param button Button number (1 = center button)
     * @return true if button was handled, false otherwise
     */
    bool handleButtonPress(uint8_t button) override;

private:
    lv_obj_t* _card;
    lv_obj_t* _background;
    lv_obj_t* _header_label;
    lv_obj_t* _label;
    lv_obj_t* _label_shadow;
    lv_obj_t* _cont;
    lv_obj_t* _shadow_cont;
    
    std::vector<std::string> _phrases;
    size_t _current_phrase_index;
    
    /**
     * @brief Create a styled label with shadow effect
     * @param parent Parent object
     * @param color Text color
     * @param x_offset X offset for shadow effect
     * @param y_offset Y offset for shadow effect
     * @return Created label object
     */
    lv_obj_t* createStyledLabel(lv_obj_t* parent, lv_color_t color, int16_t x_offset, int16_t y_offset);
    
    /**
     * @brief Set text content and start scrolling animation
     * @param txt Text to display
     */
    void setText(const char* txt);
    
    /**
     * @brief Start scrolling animation for long text
     */
    void startScrolling();
    
    /**
     * @brief Check if LVGL object is valid
     * @param obj Object to check
     * @return true if valid, false otherwise
     */
    bool isValidObject(lv_obj_t* obj) const;
};