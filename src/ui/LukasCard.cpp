#include "ui/LukasCard.h"
#include "Style.h"

/*--------------------------------------------------------------
 *  Helper – animation callback that moves a scrollable object
 *------------------------------------------------------------*/
static void scroll_y_anim_cb(void * obj, int32_t v)
{
    lv_obj_scroll_to_y(static_cast<lv_obj_t *>(obj), v, LV_ANIM_OFF);
}

/*--------------------------------------------------------------
 *  Constructor
 *------------------------------------------------------------*/
LukasCard::LukasCard(lv_obj_t* parent)
    : _card(nullptr)
    , _background(nullptr)
    , _header_label(nullptr)
    , _label(nullptr)
    , _label_shadow(nullptr)
    , _current_phrase_index(0)
    , _phrases()
    , _cont(nullptr)
    , _shadow_cont(nullptr)
{
    /* ---------- main card ----------- */
    _card = lv_obj_create(parent);
    if (!_card) return;

    lv_obj_set_width (_card, lv_pct(100));
    lv_obj_set_height(_card, lv_pct(100));
    lv_obj_set_style_bg_color  (_card, lv_color_black(), 0);
    lv_obj_set_style_border_width(_card, 0, 0);
    lv_obj_set_style_pad_all     (_card, 5, 0);
    lv_obj_set_style_margin_all  (_card, 0, 0);

    /* ---------- background panel ---- */
    _background = lv_obj_create(_card);
    if (!_background) return;

    lv_obj_set_style_radius      (_background, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color    (_background, lv_color_hex(0x4A4A4A), 0);
    lv_obj_set_style_border_width(_background, 0, 0);
    lv_obj_set_style_pad_all     (_background, 5, 0);

    lv_obj_set_width (_background, lv_pct(100));
    lv_obj_set_height(_background, lv_pct(100));

    /* ---------- header label ---------- */
    _header_label = lv_label_create(_background);
    if (!_header_label) {
        Serial.println("LukasCard: ERROR - Failed to create header label");
        return;
    }

    lv_obj_set_style_text_font(_header_label, Style::labelFont(), 0);
    lv_obj_set_style_text_color(_header_label, lv_color_hex(0x888888), 0); // Gray color
    lv_label_set_text(_header_label, "Lukas Panek joke of the day:");
    lv_obj_set_style_text_align(_header_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(_header_label, lv_pct(100));
    lv_obj_align(_header_label, LV_ALIGN_TOP_MID, 0, 8);

    /* ---------- 2 labels (shadow + main) ---------- */
    _label_shadow = createStyledLabel(_background, lv_color_black(),  0,  25);
    _label        = createStyledLabel(_background, lv_color_white(), -1,  24);

    /* ---------- hardcoded Czech phrases ---------- */
    addPhrase("Lukas Panek, chystam se na zimni spanek.");
    addPhrase("Lukas Panek, na porade mam spanek.");
    addPhrase("Lukas Panek, Marťas mi da pesti na spanek.");
    addPhrase("Lukas Panek, citim zimni vanek.");
    addPhrase("Lukas Panek, mejte dobry spanek.");
    addPhrase("Lukas Panek, deti se chystaji naspanek.");
    addPhrase("Lukas Panek, prejeme dobry spanek.");
    addPhrase("Lukas Panek, nedostal jsem svůj 4CAMPS naramek.");
    addPhrase("Lukas Panek, je docela slaby clanek.");
    addPhrase("Lukas Panek, ministr zdravotnictvi je Valek.");
    addPhrase("Lukas Panek, mam dost spatnych znamek.");
    addPhrase("Lukas Panek, nevlastnim zadny zamek.");
    addPhrase("Lukas Panek, Lolgomez rozbil dzbanek.");
    addPhrase("Lukas Panek, Libor ma kavovy dychanek.");
    addPhrase("Lukas Panek, k snidani mame dalamanek.");
    addPhrase("Lukas Panek, mam docela stylovy 4CAMPS naramek.");
    addPhrase("Lukas Panek, mam karton kavenek.");
    addPhrase("Lukas Panek, Libor mel na porade mikrospanek.");
    addPhrase("Lukas Panek, ztratil svůj zupanek.");
    addPhrase("Lukas Panek, nasbiral si hermanek.");
    addPhrase("Lukas Panek, napsal programek.");
    addPhrase("Lukas Panek, napsal na Instagram spatny clanek.");
    addPhrase("Lukas Panek, videl mnoho pornostranek.");
    addPhrase("Lukas Panek, spadl mu na hlavu kastanek.");
    addPhrase("Lukas Panek, precetl v knize mnoho stranek.");
    addPhrase("Lukas Panek, posral to kapanek.");
    addPhrase("Lukas Panek, rozbil altanek.");
    addPhrase("Lukas Panek, chci konec valek.");
    addPhrase("Lukas Panek, Martin ma pika plny nosanek.");
    addPhrase("Lukas Panek, dnes zadny spanek.");
    addPhrase("Lukas Panek, od boha je darek.");
    addPhrase("Lukas Panek, relaxu plny dzbanek.");
    addPhrase("Lukas Panek, nevim kde ma zoo planek.");
    addPhrase("Lukas Panek, můj sef je zamek.");
    addPhrase("Lukas Panek, Goťak ma kramek.");
    addPhrase("Lukas Panek, k veceri mam parek.");
    addPhrase("Lukas Panek, dal bych chcanek.");
    addPhrase("Lukas Panek, na 4CAMPS prislo hodne obalek.");
    addPhrase("Lukas Panek, odhodil nedopalek.");
    addPhrase("Lukas Panek, Libor mu koupil kocarek.");
    addPhrase("Lukas Panek, to je ale vtipalek.");
    addPhrase("Lukas Panek, na Instagramu pisalek.");
    addPhrase("Lukas Panek, mam mnoho mineralek.");
    addPhrase("Lukas Panek, deti si na hristi hraji fotbalek.");
    addPhrase("Lukas Panek, to je mi velky kecalek.");
    addPhrase("Lukas Panek, obetuje se beranek.");
    addPhrase("Lukas Panek, mam ve vete mnoho carek.");
    addPhrase("Lukas Panek, půjdeme spolu na barek.");
    addPhrase("Lukas Panek, ztratil jsem svůj lego dilek.");
    addPhrase("Lukas Panek, dneska mi prislo mnoho zasilek.");
    addPhrase("Lukas Panek, nekdo nam ukradl podsalek.");
    addPhrase("Lukas Panek, ve Dvore jsme nasrali mnoho obyvatelek.");
    addPhrase("Lukas Panek, u Rudolfa meli malo servirek.");
    addPhrase("Lukas Panek, dal jsem si jeden hranolek.");
    addPhrase("Lukas Panek, na 4CAMPS jsme meli svůj kumbalek.");
    addPhrase("Lukas Panek, potrolil me autokorekt.");
    addPhrase("Lukas Panek, mam nakrajeny salamek.");
    addPhrase("Lukas Panek, ke svacine mam bananek.");
    addPhrase("Lukas Panek, minus beranek.");
    addPhrase("Lukas Panek, vladne mi Zaoralek.");
    addPhrase("Lukas Panek, nestiham kalendar splatek.");
    addPhrase("Lukas Panek, misto programu spanek.");
    addPhrase("Lukas Panek, Zygi je nas oblibeny clanek.");
    addPhrase("Lukas Panek, jezisek mi neprinesl darek.");
    addPhrase("Lukas Panek, jdu na spanek.");
    addPhrase("Lukas Panek, nasral nam na parek.");
    addPhrase("Lukas Panek, rozbil nam zamek.");
    addPhrase("Lukas Panek, vykouril sam par krabek.");
    addPhrase("Lukas Panek, SPOs DK kvůli nemu meni zamek.");
    addPhrase("Lukas Panek, nech si tech poznamek.");
    addPhrase("Lukas Panek, hebky jak beranek.");
    addPhrase("Lukas Panek, cas peprnych radovanek.");
    addPhrase("Lukas Panek, posral clanek.");
    addPhrase("Lukas Panek, naplni ti dzbanek.");
    addPhrase("Lukas Panek, neumi plest copanek.");
    addPhrase("Lukas Panek, na kasel pomaha hermanek.");
    addPhrase("Lukas Panek, svedi me nosanek.");
    addPhrase("Lukas Panek, je mamanek.");
    addPhrase("Lukas Panek, je uplakanek.");
    addPhrase("Lukas Panek, dava góly do branek.");
    addPhrase("Lukas Panek, ceka me cesta na zamek.");
    addPhrase("Lukas Panek, dneska jsem nasranek.");
    addPhrase("Lukas Panek, dela v novinkach neporadek.");
    addPhrase("Lukas Panek, vysral se nam v patek.");
    addPhrase("Lukas Panek, je velkej posranek.");
    addPhrase("Lukas Panek, mam vetsi nosanek.");
    addPhrase("Lukas Panek, ma plnou krabu cigarek.");
    
    Serial.printf("LukasCard: Constructor finished, %d phrases loaded\n", _phrases.size());
    Serial.printf("LukasCard: _label = %p, _label_shadow = %p\n", _label, _label_shadow);
    
    // Test with simple text first
    Serial.println("LukasCard: Testing with simple text first...");
    setText("Test message!");
    
    if (!_phrases.empty()) {
        Serial.printf("LukasCard: Setting initial text: %s\n", _phrases[0].c_str());
        setText(_phrases[0].c_str());
    } else {
        Serial.println("LukasCard: ERROR - No phrases loaded!");
    }
}

/*--------------------------------------------------------------
 *  Label creator – wraps text & adds symmetric padding
 *------------------------------------------------------------*/
lv_obj_t* LukasCard::createStyledLabel(lv_obj_t* parent,
                                          lv_color_t  color,
                                          int16_t     x_offset,
                                          int16_t     y_offset)
{
    /* scrollable container */
    lv_obj_t* cont = lv_obj_create(parent);
    if (x_offset == -1)  _cont        = cont;
    else                 _shadow_cont = cont;

    lv_obj_set_size(cont, lv_pct(95), lv_pct(85));
    lv_obj_set_style_bg_opa   (cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_scroll_dir     (cont, LV_DIR_VER);
    lv_obj_set_scrollbar_mode (cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align              (cont, LV_ALIGN_CENTER, x_offset, y_offset);

    /* symmetric 4 px vertical padding */
    lv_obj_set_style_pad_top   (cont, 4, 0);
    lv_obj_set_style_pad_bottom(cont, 4, 0);

    /* the label itself */
    lv_obj_t* label = lv_label_create(cont);
    if (!label) {
        Serial.printf("LukasCard: ERROR - Failed to create label (offset %d, %d)\n", x_offset, y_offset);
        return nullptr;
    }

    lv_obj_set_style_text_font (label, Style::loudNoisesFont(), 0);
    lv_obj_set_style_text_color(label, color, 0);

    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);   // word‑wrap
    lv_obj_set_width        (label, lv_pct(100));        // keep inside cont

    Serial.printf("LukasCard: Created label %p (offset %d, %d)\n", label, x_offset, y_offset);
    return label;
}



/*--------------------------------------------------------------*/
LukasCard::~LukasCard()
{
    if (isValidObject(_card)) {
        lv_obj_add_flag(_card, LV_OBJ_FLAG_HIDDEN);
        lv_obj_del_async(_card);
    }
}

/*--------------------------------------------------------------*/
lv_obj_t* LukasCard::getCard()            { return _card; }

void LukasCard::addPhrase(const char* p)  { _phrases.push_back(p); }

/*--------------------------------------------------------------*/
void LukasCard::setText(const char* txt)
{
    Serial.printf("LukasCard::setText called with: %s\n", txt);
    if (_label) {        
        lv_label_set_text(_label, txt);
        Serial.println("LukasCard: Main label text set");
    } else {
        Serial.println("LukasCard: ERROR - _label is null!");
    }
    
    if (_label_shadow) {
        lv_label_set_text(_label_shadow, txt);
        Serial.println("LukasCard: Shadow label text set");
    } else {
        Serial.println("LukasCard: ERROR - _label_shadow is null!");
    }
    
    startScrolling();
}

/*--------------------------------------------------------------*/
void LukasCard::cycleNextPhrase()
{
    if (_phrases.empty()) return;

    _current_phrase_index =
        (_current_phrase_index + 1) % _phrases.size();
    setText(_phrases[_current_phrase_index].c_str());

    /* rotate background colour - Czech flag inspired colors plus some extras */
    static const uint32_t colors[] = {
        0x4A4A4A, // Default gray
        0xD52B1E, // Czech red
        0x11457E, // Czech blue  
        0x2C3E50, // Dark blue-gray
        0x27AE60, // Green
        0x8E44AD, // Purple
        0xE67E22, // Orange
        0x2980B9, // Bright blue
        0xC0392B, // Dark red
        0x16A085, // Teal
        0xF39C12  // Yellow/gold
    };
    lv_obj_set_style_bg_color(
        _background,
        lv_color_hex(colors[_current_phrase_index %
                           (sizeof(colors) / sizeof(colors[0]))]),
        0);
}

/*--------------------------------------------------------------*/
bool LukasCard::handleButtonPress(uint8_t button)
{
    if (button == 1) { cycleNextPhrase(); return true; }
    return false;
}

/*--------------------------------------------------------------
 *  Scroll animation – moves the *containers* up & down
 *  at a constant pixel‑per‑second speed
 *------------------------------------------------------------*/
void LukasCard::startScrolling()
{
    if (!_cont || !_label) return;

    /* ensure layout is up‑to‑date */
    lv_obj_update_layout(_cont);

    lv_coord_t label_h = lv_obj_get_height(_label);
    lv_coord_t cont_h  = lv_obj_get_height(_cont);

    /* include vertical padding */
    lv_coord_t pad_top    = lv_obj_get_style_pad_top   (_cont, 0);
    lv_coord_t pad_bottom = lv_obj_get_style_pad_bottom(_cont, 0);

    lv_coord_t distance = label_h + pad_top + pad_bottom - cont_h;
    if (distance <= 0) return;           // no scrolling needed

    /* --- constant speed calculation --------------------------- */
    constexpr uint32_t kPixelsPerSecond = 40;           // adjust to taste
    uint32_t duration_ms = static_cast<uint32_t>(
        (distance * 1000) / kPixelsPerSecond);

    /* reset to top */
    lv_obj_scroll_to_y(_cont, 0, LV_ANIM_OFF);
    if (_shadow_cont)
        lv_obj_scroll_to_y(_shadow_cont, 0, LV_ANIM_OFF);

    /* template animation */
    lv_anim_t base;
    lv_anim_init(&base);
    lv_anim_set_exec_cb   (&base, scroll_y_anim_cb);
    lv_anim_set_values    (&base, 0, distance);
    lv_anim_set_time      (&base, duration_ms);        // down
    lv_anim_set_playback_time(&base, duration_ms);     // up
    lv_anim_set_repeat_count(&base, LV_ANIM_REPEAT_INFINITE);
    /* optional pauses: lv_anim_set_delay(&base, 1000);
                        lv_anim_set_playback_delay(&base, 1000); */

    /* start for main text */
    lv_anim_t a = base;
    lv_anim_set_var(&a, _cont);
    lv_anim_start(&a);

    /* shadow follows */
    if (_shadow_cont) {
        lv_anim_t b = base;
        lv_anim_set_var(&b, _shadow_cont);
        lv_anim_start(&b);
    }
}

/*--------------------------------------------------------------*/
bool LukasCard::isValidObject(lv_obj_t* obj) const
{
    return obj && lv_obj_is_valid(obj);
}