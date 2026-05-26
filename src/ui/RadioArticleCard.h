#pragma once

#include <lvgl.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "ui/InputHandler.h"
#include "../EventQueue.h"

/**
 * @brief Card that displays the latest article from student radio API
 * 
 * This card fetches articles from https://cue.radio-r.cz/api/public/articles?limit=3
 * and displays the most recent article's title, perex (summary), and published date.
 */
class RadioArticleCard : public InputHandler {
public:
    RadioArticleCard(lv_obj_t* parent, EventQueue& eventQueue, uint16_t width, uint16_t height);
    ~RadioArticleCard();
    
    lv_obj_t* getCard() const { return _card; }
    
    // InputHandler interface
    bool handleButtonPress(uint8_t button_index) override;
    void prepareForRemoval() override { _card = nullptr; }
    
    // Public method to refresh article data
    void refreshArticleData();

private:
    // Core objects
    lv_obj_t* _card;
    lv_obj_t* _title_label;
    lv_obj_t* _perex_label;
    lv_obj_t* _date_label;
    lv_obj_t* _category_label;
    lv_obj_t* _status_label;
    
    // Event queue for async operations
    EventQueue& _event_queue;
    
    // HTTP client for API requests
    HTTPClient _http;
    WiFiClientSecure _client;
    
    // Article data
    struct Article {
        String title;
        String perex;
        String published_date;
        String category;
        String slug;
        bool valid;
        
        Article() : valid(false) {}
    };
    
    Article _current_article;
    unsigned long _last_fetch_time;
    bool _is_fetching;
    
    static const unsigned long FETCH_INTERVAL = 300000; // 5 minutes in milliseconds
    static const char* API_URL;
    
    // Private methods
    void setupUI(uint16_t width, uint16_t height);
    void updateDisplay();
    bool fetchLatestArticle();
    void parseArticleData(const String& response);
    String formatDate(const String& iso_date);
    void showError(const String& message);
    void showLoading();
};