#include "RadioArticleCard.h"
#include "Style.h"
#include "hardware/Input.h"
#include <WiFi.h>

const char* RadioArticleCard::API_URL = "https://cue.radio-r.cz/api/public/articles?limit=1";

RadioArticleCard::RadioArticleCard(lv_obj_t* parent, EventQueue& eventQueue, uint16_t width, uint16_t height)
    : _card(nullptr)
    , _title_label(nullptr)
    , _perex_label(nullptr)
    , _date_label(nullptr)
    , _category_label(nullptr)
    , _status_label(nullptr)
    , _event_queue(eventQueue)
    , _last_fetch_time(0)
    , _is_fetching(false) {
    
    // Configure secure client
    _client.setInsecure(); // For simplicity, skip certificate verification
    
    setupUI(width, height);
    
    // Try to fetch initial data
    if (WiFi.status() == WL_CONNECTED) {
        refreshArticleData();
    } else {
        showError("No WiFi connection");
    }
}

RadioArticleCard::~RadioArticleCard() {
    if (_card) {
        lv_obj_del_async(_card);
        _card = nullptr;
    }
}

void RadioArticleCard::setupUI(uint16_t width, uint16_t height) {
    // Create main card container
    _card = lv_obj_create(nullptr);
    if (!_card) {
        Serial.println("[RadioArticleCard] CRITICAL: Failed to create card base object!");
        return;
    }
    
    lv_obj_set_size(_card, width, height);
    lv_obj_set_style_bg_color(_card, Style::backgroundColor(), 0);
    lv_obj_set_style_pad_all(_card, 8, 0);
    lv_obj_set_style_border_width(_card, 0, 0);
    lv_obj_set_style_radius(_card, 0, 0);
    
    // Create scrollable container for content
    lv_obj_t* scroll_container = lv_obj_create(_card);
    lv_obj_set_size(scroll_container, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(scroll_container, LV_OPA_0, 0);
    lv_obj_set_style_border_width(scroll_container, 0, 0);
    lv_obj_set_style_pad_all(scroll_container, 4, 0);
    lv_obj_set_style_pad_row(scroll_container, 4, 0);
    lv_obj_set_flex_flow(scroll_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(scroll_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    
    // Category and date container
    lv_obj_t* meta_container = lv_obj_create(scroll_container);
    lv_obj_set_size(meta_container, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(meta_container, LV_OPA_0, 0);
    lv_obj_set_style_border_width(meta_container, 0, 0);
    lv_obj_set_style_pad_all(meta_container, 0, 0);
    lv_obj_set_flex_flow(meta_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(meta_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Category label
    _category_label = lv_label_create(meta_container);
    lv_obj_set_style_text_font(_category_label, Style::labelFont(), 0);
    lv_obj_set_style_text_color(_category_label, Style::accentColor(), 0);
    lv_label_set_text(_category_label, "RADIO");
    
    // Date label
    _date_label = lv_label_create(meta_container);
    lv_obj_set_style_text_font(_date_label, Style::labelFont(), 0);
    lv_obj_set_style_text_color(_date_label, Style::labelColor(), 0);
    lv_label_set_text(_date_label, "Loading...");
    
    // Title label
    _title_label = lv_label_create(scroll_container);
    lv_obj_set_style_text_font(_title_label, Style::valueFont(), 0);
    lv_obj_set_style_text_color(_title_label, Style::valueColor(), 0);
    lv_obj_set_width(_title_label, lv_pct(100));
    lv_label_set_long_mode(_title_label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(_title_label, "Loading article...");
    
    // Perex (summary) label
    _perex_label = lv_label_create(scroll_container);
    lv_obj_set_style_text_font(_perex_label, Style::labelFont(), 0);
    lv_obj_set_style_text_color(_perex_label, Style::labelColor(), 0);
    lv_obj_set_width(_perex_label, lv_pct(100));
    lv_label_set_long_mode(_perex_label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(_perex_label, "");
    
    // Status label for errors/loading
    _status_label = lv_label_create(scroll_container);
    lv_obj_set_style_text_font(_status_label, Style::labelFont(), 0);
    lv_obj_set_style_text_color(_status_label, Style::labelColor(), 0);
    lv_obj_set_width(_status_label, lv_pct(100));
    lv_label_set_text(_status_label, "");
    lv_obj_add_flag(_status_label, LV_OBJ_FLAG_HIDDEN);
}

bool RadioArticleCard::handleButtonPress(uint8_t button_index) {
    // Center button refreshes the article data
    if (button_index == Input::BUTTON_CENTER) {
        if (!_is_fetching && WiFi.status() == WL_CONNECTED) {
            refreshArticleData();
            return true;
        }
    }
    
    return false; // Let CardNavigationStack handle other buttons
}

void RadioArticleCard::refreshArticleData() {
    if (_is_fetching) {
        return; // Already fetching
    }
    
    // Check if we need to fetch (first time or enough time has passed)
    unsigned long now = millis();
    if (_last_fetch_time > 0 && (now - _last_fetch_time) < FETCH_INTERVAL) {
        Serial.println("[RadioArticleCard] Skipping fetch, too soon since last update");
        return;
    }
    
    showLoading();
    _is_fetching = true;
    
    if (fetchLatestArticle()) {
        _last_fetch_time = now;
        updateDisplay();
    } else {
        showError("Failed to fetch article");
    }
    
    _is_fetching = false;
}

bool RadioArticleCard::fetchLatestArticle() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[RadioArticleCard] WiFi not connected");
        return false;
    }
    
    Serial.printf("[RadioArticleCard] Fetching from: %s\n", API_URL);
    
    _http.begin(_client, API_URL);
    _http.addHeader("Accept", "application/json");
    _http.addHeader("User-Agent", "DeskHog/1.0 ESP32");
    
    int httpCode = _http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        String response = _http.getString();
        Serial.printf("[RadioArticleCard] Response received, length: %d\n", response.length());
        
        parseArticleData(response);
        _http.end();
        return _current_article.valid;
    } else {
        Serial.printf("[RadioArticleCard] HTTP GET failed, error: %d\n", httpCode);
        _http.end();
        return false;
    }
}

void RadioArticleCard::parseArticleData(const String& response) {
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
        Serial.printf("[RadioArticleCard] JSON parsing failed: %s\n", error.c_str());
        _current_article.valid = false;
        return;
    }
    
    JsonArray data = doc["data"];
    if (data.size() == 0) {
        Serial.println("[RadioArticleCard] No articles in response");
        _current_article.valid = false;
        return;
    }
    
    JsonObject article = data[0];
    
    _current_article.title = article["title"].as<String>();
    _current_article.perex = article["perex"].as<String>();
    _current_article.published_date = article["published"].as<String>();
    _current_article.category = article["category"].as<String>();
    _current_article.slug = article["slug"].as<String>();
    _current_article.valid = true;
    
    Serial.printf("[RadioArticleCard] Parsed article: %s\n", _current_article.title.c_str());
}

String RadioArticleCard::formatDate(const String& iso_date) {
    // Simple date formatting: "2025-07-27 12:00:00.000Z" -> "27.7.2025"
    if (iso_date.length() < 10) {
        return iso_date;
    }
    
    String year = iso_date.substring(0, 4);
    String month = iso_date.substring(5, 7);
    String day = iso_date.substring(8, 10);
    
    // Remove leading zeros
    int monthInt = month.toInt();
    int dayInt = day.toInt();
    
    return String(dayInt) + "." + String(monthInt) + "." + year;
}

void RadioArticleCard::updateDisplay() {
    if (!_current_article.valid) {
        showError("No article data");
        return;
    }
    
    // Hide status label and show content
    lv_obj_add_flag(_status_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(_title_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(_perex_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(_date_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(_category_label, LV_OBJ_FLAG_HIDDEN);
    
    // Update content
    lv_label_set_text(_title_label, _current_article.title.c_str());
    lv_label_set_text(_perex_label, _current_article.perex.c_str());
    lv_label_set_text(_date_label, formatDate(_current_article.published_date).c_str());
    
    // Format category
    String category = _current_article.category;
    category.toUpperCase();
    lv_label_set_text(_category_label, category.c_str());
    
    Serial.printf("[RadioArticleCard] Display updated with: %s\n", _current_article.title.c_str());
}

void RadioArticleCard::showError(const String& message) {
    // Hide content labels and show status
    lv_obj_add_flag(_title_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(_perex_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(_status_label, LV_OBJ_FLAG_HIDDEN);
    
    lv_obj_set_style_text_color(_status_label, lv_color_hex(0xFF5555), 0);
    lv_label_set_text(_status_label, ("Error: " + message).c_str());
    
    Serial.printf("[RadioArticleCard] Error: %s\n", message.c_str());
}

void RadioArticleCard::showLoading() {
    // Show loading state
    lv_obj_clear_flag(_status_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_text_color(_status_label, Style::labelColor(), 0);
    lv_label_set_text(_status_label, "Refreshing...");
    lv_label_set_text(_date_label, "Loading...");
}