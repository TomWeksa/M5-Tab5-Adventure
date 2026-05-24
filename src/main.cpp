#include <Arduino.h>
#include <M5Unified.h>
#include <stdio.h>
#include <string.h>

namespace {

enum class Screen : uint8_t {
    Field,
    Inventory,
    Map,
};

enum class Slot : uint8_t {
    Suit,
    Detector,
    Tool,
    Weapon,
    Artifact,
    Consumable,
};

enum class UiAction : uint8_t {
    None,
    Scout,
    Scavenge,
    Sneak,
    Inventory,
    Map,
    SelectSite,
    BackToField,
    EquipOrUse,
    Travel,
    Rest,
};

struct Item {
    const char* name;
    const char* tag;
    Slot slot;
    int8_t grit;
    int8_t tech;
    int8_t scan;
    int8_t ghost;
    int8_t filter;
    int8_t strain;
    uint8_t value;
    uint16_t color;
    const char* description;
};

struct Site {
    const char* name;
    const char* district;
    const char* description;
    uint8_t risk;
    uint8_t maxCache;
    uint16_t color;
};

struct MapPin {
    uint8_t site;
    uint16_t xPermille;
    uint16_t yPermille;
    const char* callSign;
    const char* whisper;
};

struct RouteEdge {
    uint8_t from;
    uint8_t to;
    const char* name;
};

struct Stats {
    int16_t grit = 1;
    int16_t tech = 1;
    int16_t scan = 0;
    int16_t ghost = 0;
    int16_t filter = 0;
    int16_t strain = 0;
};

struct Button {
    int32_t x = 0;
    int32_t y = 0;
    int32_t w = 0;
    int32_t h = 0;
    UiAction action = UiAction::None;
    int16_t param = 0;
    bool enabled = true;
    bool visible = true;
    uint16_t accent = TFT_CYAN;
    char label[28] = "";
};

constexpr uint8_t kInventoryCapacity = 12;
constexpr uint8_t kEquipSlotCount = 5;
constexpr uint8_t kSiteCapacity = 5;
constexpr uint8_t kMaxButtons = 24;
constexpr int16_t kMaxHealth = 10;
constexpr int16_t kMaxExposure = 12;
constexpr uint8_t kMaxTimeTicks = 8;
constexpr uint8_t kDayStartHour = 7;
constexpr uint8_t kTickHours = 2;
constexpr uint8_t kMaxSiteIntel = 3;
constexpr uint8_t kMaxSiteHeat = 6;
constexpr int16_t kDailyUpkeep = 4;

Screen currentScreen = Screen::Field;
int16_t health = 9;
int16_t exposure = 0;
int16_t scrap = 8;
uint16_t day = 1;
uint8_t currentSite = 0;
uint8_t selectedMapSite = 1;
uint8_t timeTick = 0;
uint8_t siteHeat[kSiteCapacity];
uint8_t siteCache[kSiteCapacity];
uint8_t siteIntel[kSiteCapacity];
int16_t inventory[kInventoryCapacity];
int16_t equipped[kEquipSlotCount];
Button buttons[kMaxButtons];
uint8_t buttonCount = 0;
bool screenDirty = true;
bool touchWasPressed = false;
char statusLine[192] = "The rain tastes metallic. Your kit is the only thing between you and the quiet.";

uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return M5.Display.color565(r, g, b);
}

Item itemCatalog[] = {
    {"Patch Mask", "suit", Slot::Suit, 0, 0, 0, 0, 1, 0, 6, 0, "A patched respirator and tarred hood. Bad fashion, good filters."},
    {"Rubber Trench", "suit", Slot::Suit, 1, 0, 0, -1, 2, 0, 14, 0, "Heavy sealed coat from a cleanup crew that never came back."},
    {"Mirrorweave Coat", "suit", Slot::Suit, 0, 1, 0, 2, 1, 0, 18, 0, "Chameleon threads shimmer under old advert light."},
    {"Coil Detector", "detector", Slot::Detector, 0, 0, 1, 0, 0, 0, 8, 0, "Cheap copper loop detector. It screams before the world bends."},
    {"Glass Needle", "detector", Slot::Detector, 0, 1, 3, 0, 0, 1, 20, 0, "A precise anomaly pick. Every reading leaves a headache."},
    {"Prybar Kit", "tool", Slot::Tool, 1, 0, 0, 0, 0, 0, 7, 0, "A prybar, pliers, tape and stubbornness in a cracked roll."},
    {"Solder Rig", "tool", Slot::Tool, 0, 3, 0, 0, 0, 0, 19, 0, "Battery iron, fiber patcher and black market firmware clips."},
    {"Quiet Nailgun", "weapon", Slot::Weapon, 1, 0, 0, 2, 0, 0, 12, 0, "Industrial nailer tuned for close work and low attention."},
    {"Rail Pistol", "weapon", Slot::Weapon, 3, 0, 0, -1, 0, 1, 24, 0, "Hard recoil, bright flash, very persuasive."},
    {"Warm Battery", "artifact", Slot::Artifact, 0, 2, 0, 0, 0, 2, 30, 0, "A dry cell that charges itself and warms when lied to."},
    {"Mourning Lens", "artifact", Slot::Artifact, 0, 0, 3, 0, 0, 2, 34, 0, "Smoke-dark glass that shows paths people failed to take."},
    {"Null Charm", "artifact", Slot::Artifact, 0, 0, 0, 2, 1, 1, 28, 0, "A cold trinket that makes cameras lose interest."},
    {"Iodine Ampoule", "dose", Slot::Consumable, 0, 0, 0, 0, 0, 0, 5, 0, "Bitter anti-rad dose. You only taste pennies for an hour."},
    {"Canned Coffee", "dose", Slot::Consumable, 0, 0, 0, 0, 0, 0, 4, 0, "Pre-collapse stimulant syrup. Technically food."},
    {"Copper Saint", "artifact", Slot::Artifact, 2, 0, 0, 0, 0, 2, 38, 0, "A little idol that hums in broken service tunnels."},
};

Site sites[] = {
    {"Underpass Clinic", "safe berth", "A medic shack below the ring road. Warm bulbs, bad coffee, guarded doors.", 1, 0, 0},
    {"Neon Spillway", "wet market", "Old ad towers bleed color through toxic rain and illegal stalls.", 3, 4, 0},
    {"Sunken Mall", "retail tomb", "Escalators vanish into black water. Something below keeps the music playing.", 4, 5, 0},
    {"Relay Grave", "antenna field", "Fallen towers tick in the wind. Messages still arrive for the dead.", 5, 4, 0},
    {"Black Reed Verge", "outer exclusion", "A reed sea grown through asphalt. Every shadow looks freshly made.", 6, 3, 0},
};

MapPin mapPins[] = {
    {0, 150, 700, "CLINIC", "Blue ward light. The map stabilizes here and nowhere else."},
    {1, 320, 510, "SPILL", "Ad ghosts stutter: BUY CLEAN WATER / BUY CLEAN SKIN."},
    {2, 510, 670, "MALL", "Retail hymns leak from drowned escalators after dusk."},
    {3, 660, 360, "RELAY", "Antenna shadows point against the wind."},
    {4, 850, 190, "VERGE", "The reedline redraws itself between blinks."},
};

RouteEdge routeEdges[] = {
    {0, 1, "drain road"},
    {0, 2, "service tunnel"},
    {1, 2, "flood arcade"},
    {1, 3, "signage spine"},
    {2, 3, "maintenance tram"},
    {2, 4, "reed culvert"},
    {3, 4, "antenna ridge"},
};

constexpr uint8_t kItemCount = sizeof(itemCatalog) / sizeof(itemCatalog[0]);
constexpr uint8_t kSiteCount = sizeof(sites) / sizeof(sites[0]);
constexpr uint8_t kMapPinCount = sizeof(mapPins) / sizeof(mapPins[0]);
constexpr uint8_t kRouteEdgeCount = sizeof(routeEdges) / sizeof(routeEdges[0]);
static_assert(kSiteCount <= kSiteCapacity, "site state arrays need more capacity");
static_assert(kMapPinCount == kSiteCount, "map pins must match sites");

int clampInt(int value, int low, int high) {
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

const char* slotName(Slot slot) {
    switch (slot) {
        case Slot::Suit:
            return "Suit";
        case Slot::Detector:
            return "Detector";
        case Slot::Tool:
            return "Tool";
        case Slot::Weapon:
            return "Weapon";
        case Slot::Artifact:
            return "Artifact";
        case Slot::Consumable:
            return "Dose";
    }
    return "Item";
}

int8_t equipIndexForSlot(Slot slot) {
    switch (slot) {
        case Slot::Suit:
            return 0;
        case Slot::Detector:
            return 1;
        case Slot::Tool:
            return 2;
        case Slot::Weapon:
            return 3;
        case Slot::Artifact:
            return 4;
        case Slot::Consumable:
            return -1;
    }
    return -1;
}

bool validItemId(int16_t itemId) {
    return itemId >= 0 && itemId < static_cast<int16_t>(kItemCount);
}

bool validInventorySlot(int16_t slot) {
    return slot >= 0 && slot < static_cast<int16_t>(kInventoryCapacity) && validItemId(inventory[slot]);
}

Stats deriveStats() {
    Stats stats;
    for (uint8_t i = 0; i < kEquipSlotCount; ++i) {
        const int16_t invSlot = equipped[i];
        if (!validInventorySlot(invSlot)) {
            continue;
        }
        const Item& item = itemCatalog[inventory[invSlot]];
        stats.grit += item.grit;
        stats.tech += item.tech;
        stats.scan += item.scan;
        stats.ghost += item.ghost;
        stats.filter += item.filter;
        stats.strain += item.strain;
    }
    stats.grit = clampInt(stats.grit, 0, 9);
    stats.tech = clampInt(stats.tech, 0, 9);
    stats.scan = clampInt(stats.scan, 0, 9);
    stats.ghost = clampInt(stats.ghost, 0, 9);
    stats.filter = clampInt(stats.filter, 0, 9);
    stats.strain = clampInt(stats.strain, 0, 9);
    return stats;
}

uint8_t currentHour() {
    return static_cast<uint8_t>(kDayStartHour + timeTick * kTickHours);
}

uint8_t timeRemainingTicks() {
    return timeTick < kMaxTimeTicks ? static_cast<uint8_t>(kMaxTimeTicks - timeTick) : 0;
}

int16_t effectiveRiskForSite(uint8_t siteIndex) {
    if (siteIndex == 0 || siteIndex >= kSiteCount) {
        return sites[0].risk;
    }

    const int16_t duskPressure = timeTick >= 6 ? 1 : 0;
    return clampInt(sites[siteIndex].risk + siteHeat[siteIndex] / 2 - siteIntel[siteIndex] + duskPressure, 1, 9);
}

uint8_t actionTimeCost(UiAction action) {
    switch (action) {
        case UiAction::Scout:
        case UiAction::Sneak:
        case UiAction::Travel:
        case UiAction::Rest:
            return 1;
        case UiAction::Scavenge:
            return 2;
        default:
            return 0;
    }
}

const char* actionCheckText(UiAction action) {
    switch (action) {
        case UiAction::Scout:
            return "SCAN+TECH";
        case UiAction::Scavenge:
            return "GRIT+FILTER";
        case UiAction::Sneak:
            return "GHOST+SCAN";
        default:
            return "--";
    }
}

const char* actionLabel(UiAction action) {
    switch (action) {
        case UiAction::Scout:
            return "Scout";
        case UiAction::Scavenge:
            return "Scavenge";
        case UiAction::Sneak:
            return "Sneak";
        default:
            return "Move";
    }
}

const char* actionPayoffText(UiAction action) {
    switch (action) {
        case UiAction::Scout:
            return "intel";
        case UiAction::Scavenge:
            return "salvage";
        case UiAction::Sneak:
            return "low heat";
        default:
            return "";
    }
}

const char* actionBlockedText(UiAction action) {
    if (currentSite == 0) {
        return "travel";
    }
    if (timeRemainingTicks() < actionTimeCost(action)) {
        return "no light";
    }
    if (action == UiAction::Scout && siteIntel[currentSite] >= kMaxSiteIntel) {
        return "mapped";
    }
    if ((action == UiAction::Scavenge || action == UiAction::Sneak) && siteCache[currentSite] == 0) {
        return "dry";
    }
    return "";
}

int16_t actionSkill(UiAction action, const Stats& stats) {
    switch (action) {
        case UiAction::Scout:
            return stats.scan + stats.tech;
        case UiAction::Scavenge:
            return stats.grit + stats.filter;
        case UiAction::Sneak:
            return stats.ghost + stats.scan;
        default:
            return 0;
    }
}

int16_t actionTarget(UiAction action) {
    int16_t target = 5 + effectiveRiskForSite(currentSite);
    if (action == UiAction::Scavenge) {
        target += 1;
    }
    if (action == UiAction::Sneak && siteHeat[currentSite] > 0) {
        target += 1;
    }
    return target;
}

int16_t actionExposureCost(UiAction action, const Stats& stats) {
    int16_t dose = clampInt((effectiveRiskForSite(currentSite) + stats.strain - stats.filter) / 3, 0, 4);
    if (action == UiAction::Scout) {
        dose = clampInt(dose - 1, 0, 4);
    } else if (action == UiAction::Scavenge) {
        dose = clampInt(dose + 1, 0, 5);
    }
    return dose;
}

uint8_t successChance(int16_t skill, int16_t target) {
    const int16_t needed = target - skill;
    if (needed <= 1) {
        return 100;
    }
    if (needed > 6) {
        return 0;
    }
    return static_cast<uint8_t>((7 - needed) * 100 / 6);
}

bool fieldActionAvailable(UiAction action) {
    if (currentSite == 0) {
        return false;
    }
    if (timeRemainingTicks() < actionTimeCost(action)) {
        return false;
    }
    if (action == UiAction::Scout) {
        return siteIntel[currentSite] < kMaxSiteIntel;
    }
    if (action == UiAction::Scavenge || action == UiAction::Sneak) {
        return siteCache[currentSite] > 0;
    }
    return true;
}

bool directRoute(uint8_t a, uint8_t b) {
    if (a == b) {
        return true;
    }

    for (uint8_t i = 0; i < kRouteEdgeCount; ++i) {
        if ((routeEdges[i].from == a && routeEdges[i].to == b) || (routeEdges[i].from == b && routeEdges[i].to == a)) {
            return true;
        }
    }
    return false;
}

uint8_t travelTicksToSite(uint8_t targetSite) {
    if (targetSite >= kSiteCount || targetSite == currentSite) {
        return 0;
    }
    return directRoute(currentSite, targetSite) ? 1 : 2;
}

int16_t routeExposureCost(uint8_t targetSite, const Stats& stats) {
    if (targetSite >= kSiteCount || targetSite == currentSite) {
        return 0;
    }
    if (targetSite == 0) {
        return currentSite == 0 ? 0 : 1;
    }

    const int16_t routeLoad = travelTicksToSite(targetSite) + siteHeat[targetSite] / 2;
    return clampInt((sites[targetSite].risk + routeLoad + stats.strain - stats.filter) / 4, 0, 4);
}

bool canTravelToSite(uint8_t targetSite) {
    const uint8_t ticks = travelTicksToSite(targetSite);
    return targetSite < kSiteCount && targetSite != currentSite && ticks > 0 && timeRemainingTicks() >= ticks;
}

void setStatus(const char* text) {
    strncpy(statusLine, text, sizeof(statusLine) - 1);
    statusLine[sizeof(statusLine) - 1] = '\0';
    Serial.println(statusLine);
    screenDirty = true;
}

bool hasCatalogItem(uint8_t itemId) {
    for (uint8_t i = 0; i < kInventoryCapacity; ++i) {
        if (inventory[i] == itemId) {
            return true;
        }
    }
    return false;
}

int16_t firstEmptyInventorySlot() {
    for (uint8_t i = 0; i < kInventoryCapacity; ++i) {
        if (inventory[i] < 0) {
            return i;
        }
    }
    return -1;
}

bool addItem(uint8_t itemId, char* message, size_t messageSize) {
    const int16_t slot = firstEmptyInventorySlot();
    if (slot < 0) {
        scrap += itemCatalog[itemId].value / 2;
        snprintf(message, messageSize, "Pack full. You fenced %s on the spot for %u scrap.", itemCatalog[itemId].name,
                 static_cast<unsigned>(itemCatalog[itemId].value / 2));
        return false;
    }
    inventory[slot] = itemId;
    snprintf(message, messageSize, "Found %s. It is now in your pack.", itemCatalog[itemId].name);
    return true;
}

void equipInventorySlot(uint8_t invSlot) {
    if (!validInventorySlot(invSlot)) {
        return;
    }
    const Item& item = itemCatalog[inventory[invSlot]];
    const int8_t equipSlot = equipIndexForSlot(item.slot);
    if (equipSlot < 0) {
        if (inventory[invSlot] == 12) {
            exposure = clampInt(exposure - 5, 0, kMaxExposure);
            inventory[invSlot] = -1;
            setStatus("The iodine burns all the way down. Exposure drops, hands stop shaking.");
        } else if (inventory[invSlot] == 13) {
            health = clampInt(health + 2, 0, kMaxHealth);
            inventory[invSlot] = -1;
            setStatus("Coffee syrup, cold and foul. You feel almost alive.");
        }
        return;
    }

    equipped[equipSlot] = invSlot;
    char message[128];
    snprintf(message, sizeof(message), "Equipped %s. In this place, gear is growth.", item.name);
    setStatus(message);
}

void addButton(const char* label, int32_t x, int32_t y, int32_t w, int32_t h, UiAction action, int16_t param,
               uint16_t accent, bool enabled = true, bool visible = true) {
    if (buttonCount >= kMaxButtons) {
        return;
    }
    Button& button = buttons[buttonCount++];
    button.x = x;
    button.y = y;
    button.w = w;
    button.h = h;
    button.action = action;
    button.param = param;
    button.enabled = enabled;
    button.visible = visible;
    button.accent = accent;
    strncpy(button.label, label, sizeof(button.label) - 1);
    button.label[sizeof(button.label) - 1] = '\0';
}

void clearButtons() {
    buttonCount = 0;
}

bool hitButton(const Button& button, int32_t x, int32_t y) {
    return button.enabled && x >= button.x && x < button.x + button.w && y >= button.y && y < button.y + button.h;
}

void drawButton(const Button& button) {
    if (!button.visible) {
        return;
    }
    auto& display = M5.Display;
    const uint16_t fill = button.enabled ? rgb(14, 21, 28) : rgb(20, 20, 22);
    const uint16_t text = button.enabled ? TFT_WHITE : rgb(110, 115, 120);
    const uint16_t border = button.enabled ? button.accent : rgb(60, 60, 64);

    display.fillRoundRect(button.x, button.y, button.w, button.h, 6, fill);
    display.drawRoundRect(button.x, button.y, button.w, button.h, 6, border);
    display.setTextDatum(textdatum_t::middle_center);
    display.setFont(&fonts::Font2);
    display.setTextColor(text, fill);
    display.drawString(button.label, button.x + button.w / 2, button.y + button.h / 2);
    display.setTextDatum(textdatum_t::top_left);
}

void drawWrappedText(const char* text, int32_t x, int32_t y, int32_t w, uint8_t maxLines, uint16_t color,
                     uint16_t background) {
    auto& display = M5.Display;
    display.setFont(&fonts::Font2);
    display.setTextColor(color, background);

    char line[96] = "";
    char word[32] = "";
    uint8_t wordLen = 0;
    uint8_t lineCount = 0;
    const int32_t lineHeight = 22;

    const auto flushWord = [&]() {
        if (wordLen == 0 || lineCount >= maxLines) {
            wordLen = 0;
            word[0] = '\0';
            return;
        }

        char candidate[128];
        if (line[0] == '\0') {
            snprintf(candidate, sizeof(candidate), "%s", word);
        } else {
            snprintf(candidate, sizeof(candidate), "%s %s", line, word);
        }

        if (display.textWidth(candidate) > w && line[0] != '\0') {
            display.drawString(line, x, y + lineCount * lineHeight);
            ++lineCount;
            snprintf(line, sizeof(line), "%s", word);
        } else {
            snprintf(line, sizeof(line), "%s", candidate);
        }

        wordLen = 0;
        word[0] = '\0';
    };

    for (const char* cursor = text; *cursor != '\0' && lineCount < maxLines; ++cursor) {
        if (*cursor == ' ') {
            flushWord();
        } else if (wordLen < sizeof(word) - 1) {
            word[wordLen++] = *cursor;
            word[wordLen] = '\0';
        }
    }
    flushWord();

    if (line[0] != '\0' && lineCount < maxLines) {
        display.drawString(line, x, y + lineCount * lineHeight);
    }
}

void drawMeter(int32_t x, int32_t y, int32_t w, int32_t h, int value, int maxValue, uint16_t color, const char* label) {
    auto& display = M5.Display;
    const uint16_t bg = rgb(6, 10, 14);
    const uint16_t rail = rgb(40, 46, 50);
    const int32_t filled = (w - 4) * clampInt(value, 0, maxValue) / maxValue;

    display.setFont(&fonts::Font2);
    display.setTextColor(rgb(180, 190, 190), bg);
    display.drawString(label, x, y - 22);
    display.fillRect(x, y, w, h, bg);
    display.drawRect(x, y, w, h, rail);
    display.fillRect(x + 2, y + 2, filled, h - 4, color);
}

void drawPanel(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t border) {
    auto& display = M5.Display;
    display.fillRoundRect(x, y, w, h, 8, rgb(8, 12, 17));
    display.drawRoundRect(x, y, w, h, 8, border);
}

void drawHeader() {
    auto& display = M5.Display;
    const int32_t width = display.width();
    const uint16_t bg = rgb(7, 10, 14);

    display.fillRect(0, 0, width, 64, bg);
    display.drawFastHLine(0, 63, width, rgb(70, 220, 170));
    display.setTextDatum(textdatum_t::top_left);
    display.setFont(&fonts::Font4);
    display.setTextColor(TFT_WHITE, bg);
    display.drawString("Neon Exclusion", 18, 12);

    display.setFont(&fonts::Font2);
    display.setTextColor(rgb(180, 190, 190), bg);
    display.setCursor(width - 430, 16);
    display.printf("Day %u  %02u:00  scrap %d  dusk in %uh", static_cast<unsigned>(day),
                   static_cast<unsigned>(currentHour()), scrap,
                   static_cast<unsigned>(timeRemainingTicks() * kTickHours));
    display.setCursor(width - 430, 38);
    display.printf("bill %d at dawn  risk %d  cache %u  heat %u", kDailyUpkeep, effectiveRiskForSite(currentSite),
                   siteCache[currentSite], siteHeat[currentSite]);
}

void drawStatsPanel(int32_t x, int32_t y, int32_t w, int32_t h) {
    auto& display = M5.Display;
    const uint16_t bg = rgb(8, 12, 17);
    const Stats stats = deriveStats();

    drawPanel(x, y, w, h, rgb(48, 120, 130));
    display.setFont(&fonts::Font4);
    display.setTextColor(TFT_WHITE, bg);
    display.drawString("Runner", x + 14, y + 12);

    drawMeter(x + 16, y + 56, w - 32, 14, health, kMaxHealth, rgb(230, 80, 90), "Body");
    drawMeter(x + 16, y + 102, w - 32, 14, exposure, kMaxExposure, rgb(170, 230, 80), "Exposure");
    drawMeter(x + 16, y + 148, w - 32, 14, timeTick, kMaxTimeTicks, rgb(230, 180, 70), "Clock");

    const int32_t statY = y + 180;
    display.setFont(&fonts::Font2);
    display.setTextColor(rgb(220, 230, 225), bg);
    display.setCursor(x + 18, statY);
    display.printf("GRIT breach  %d", stats.grit);
    display.setCursor(x + 18, statY + 22);
    display.printf("TECH bypass  %d", stats.tech);
    display.setCursor(x + 18, statY + 44);
    display.printf("SCAN anomaly %d", stats.scan);
    display.setCursor(x + 18, statY + 66);
    display.printf("GHOST heat   %d", stats.ghost);
    display.setCursor(x + 18, statY + 88);
    display.printf("FILTER dose  %d", stats.filter);
    display.setCursor(x + 18, statY + 110);
    display.printf("STRAIN weird %d", stats.strain);
}

void drawEquippedList(int32_t x, int32_t y, int32_t w, int32_t h) {
    auto& display = M5.Display;
    const uint16_t bg = rgb(8, 12, 17);
    const char* labels[kEquipSlotCount] = {"Suit", "Detector", "Tool", "Weapon", "Artifact"};

    drawPanel(x, y, w, h, rgb(130, 86, 170));
    display.setFont(&fonts::Font4);
    display.setTextColor(TFT_WHITE, bg);
    display.drawString("Loadout", x + 14, y + 12);

    display.setFont(&fonts::Font2);
    for (uint8_t i = 0; i < kEquipSlotCount; ++i) {
        const int32_t rowY = y + 60 + i * 50;
        display.setTextColor(rgb(150, 170, 175), bg);
        display.drawString(labels[i], x + 16, rowY);
        display.setTextColor(TFT_WHITE, bg);
        const int16_t invSlot = equipped[i];
        const char* name = validInventorySlot(invSlot) ? itemCatalog[inventory[invSlot]].name : "empty";
        display.drawString(name, x + 16, rowY + 22);
    }
}

void drawPackPreview(int32_t x, int32_t y, int32_t w, int32_t h) {
    auto& display = M5.Display;
    const uint16_t bg = rgb(8, 12, 17);

    drawPanel(x, y, w, h, rgb(190, 70, 150));
    display.setFont(&fonts::Font4);
    display.setTextColor(TFT_WHITE, bg);
    display.drawString("Pack", x + 14, y + 12);

    display.setFont(&fonts::Font2);
    uint8_t drawn = 0;
    for (uint8_t i = 0; i < kInventoryCapacity && drawn < 7; ++i) {
        if (!validInventorySlot(i)) {
            continue;
        }
        const Item& item = itemCatalog[inventory[i]];
        const int32_t rowY = y + 58 + drawn * 38;
        display.fillRect(x + 14, rowY + 4, 8, 18, item.color);
        display.setTextColor(TFT_WHITE, bg);
        display.drawString(item.name, x + 30, rowY);
        display.setTextColor(rgb(135, 150, 150), bg);
        display.drawString(item.tag, x + 30, rowY + 18);
        ++drawn;
    }
}

void drawActionForecast(int32_t x, int32_t y, int32_t w, int32_t h) {
    auto& display = M5.Display;
    const uint16_t bg = rgb(8, 12, 17);
    const Stats stats = deriveStats();
    const UiAction actions[] = {UiAction::Scout, UiAction::Scavenge, UiAction::Sneak};

    drawPanel(x, y, w, h, rgb(190, 70, 150));
    display.setFont(&fonts::Font4);
    display.setTextColor(TFT_WHITE, bg);
    display.drawString("Move Forecast", x + 14, y + 12);

    display.setFont(&fonts::Font2);
    display.setTextColor(rgb(150, 168, 170), bg);
    display.setCursor(x + 16, y + 48);
    display.printf("cache %u/%u  intel %u/%u  heat %u/%u", siteCache[currentSite], sites[currentSite].maxCache,
                   siteIntel[currentSite], kMaxSiteIntel, siteHeat[currentSite], kMaxSiteHeat);

    for (uint8_t i = 0; i < 3; ++i) {
        const UiAction action = actions[i];
        const int32_t rowY = y + 82 + i * 70;
        const bool enabled = fieldActionAvailable(action);
        const uint16_t rowBg = enabled ? rgb(12, 18, 24) : rgb(16, 16, 18);
        const uint16_t border = enabled ? rgb(90, 210, 220) : rgb(58, 58, 62);
        const int16_t skill = actionSkill(action, stats);
        const int16_t target = actionTarget(action);
        const uint8_t chance = successChance(skill, target);

        display.fillRoundRect(x + 14, rowY, w - 28, 58, 6, rowBg);
        display.drawRoundRect(x + 14, rowY, w - 28, 58, 6, border);
        display.setTextColor(enabled ? TFT_WHITE : rgb(110, 115, 120), rowBg);
        display.drawString(actionLabel(action), x + 26, rowY + 8);
        display.setTextColor(enabled ? rgb(180, 210, 205) : rgb(95, 100, 102), rowBg);
        display.setCursor(x + 126, rowY + 8);
        display.printf("%s %d/%d", actionCheckText(action), skill, target);
        display.setCursor(x + 26, rowY + 30);
        display.printf("%u%%  %uh  dose +%d", chance, static_cast<unsigned>(actionTimeCost(action) * kTickHours),
                       actionExposureCost(action, stats));
        display.setCursor(x + 166, rowY + 30);
        display.print(enabled ? actionPayoffText(action) : actionBlockedText(action));
    }
}

uint8_t randomLootForAction(UiAction action) {
    const uint8_t risk = static_cast<uint8_t>(effectiveRiskForSite(currentSite));
    const bool rare = random(0, 100) < (18 + risk * 5);
    if (rare && risk >= 4) {
        const uint8_t artifacts[] = {9, 10, 11, 14};
        return artifacts[random(0, sizeof(artifacts) / sizeof(artifacts[0]))];
    }

    if (action == UiAction::Scout) {
        const uint8_t scoutLoot[] = {4, 10, 11, 12};
        return scoutLoot[random(0, sizeof(scoutLoot) / sizeof(scoutLoot[0]))];
    }
    if (action == UiAction::Sneak) {
        const uint8_t sneakLoot[] = {2, 7, 11, 13};
        return sneakLoot[random(0, sizeof(sneakLoot) / sizeof(sneakLoot[0]))];
    }

    const uint8_t scavengeLoot[] = {1, 6, 8, 9, 12, 13};
    return scavengeLoot[random(0, sizeof(scavengeLoot) / sizeof(scavengeLoot[0]))];
}

uint8_t actionHeatGain(UiAction action, bool success) {
    switch (action) {
        case UiAction::Scout:
            return success ? 1 : 2;
        case UiAction::Scavenge:
            return success ? 3 : 4;
        case UiAction::Sneak:
            return success ? 0 : 2;
        default:
            return 0;
    }
}

void coolSitesForNewDay() {
    for (uint8_t i = 1; i < kSiteCount; ++i) {
        siteHeat[i] = siteHeat[i] > 2 ? static_cast<uint8_t>(siteHeat[i] - 2) : 0;
        siteIntel[i] = siteIntel[i] > 0 ? static_cast<uint8_t>(siteIntel[i] - 1) : 0;
        if (siteCache[i] < sites[i].maxCache) {
            ++siteCache[i];
        }
    }
}

void startNewDay(const char* lead) {
    ++day;
    timeTick = 0;
    currentSite = 0;
    coolSitesForNewDay();

    char bill[80];
    if (scrap >= kDailyUpkeep) {
        scrap -= kDailyUpkeep;
        snprintf(bill, sizeof(bill), "Clinic meter paid: -%d scrap.", kDailyUpkeep);
    } else {
        const int16_t shortfall = kDailyUpkeep - scrap;
        scrap = 0;
        health = clampInt(health - shortfall, 0, kMaxHealth);
        exposure = clampInt(exposure + shortfall, 0, kMaxExposure);
        snprintf(bill, sizeof(bill), "Short on rent. Debt collectors take it out of your body.");
    }

    char message[192];
    snprintf(message, sizeof(message), "%s %s", lead, bill);
    setStatus(message);
}

void spendTime(uint8_t ticks) {
    timeTick = static_cast<uint8_t>(timeTick + ticks);
    if (timeTick < kMaxTimeTicks) {
        screenDirty = true;
        return;
    }

    char lead[128];
    if (currentSite != 0) {
        const Stats stats = deriveStats();
        const int16_t risk = effectiveRiskForSite(currentSite);
        const int16_t nightDose = clampInt((risk + stats.strain - stats.filter) / 2, 1, 5);
        const int16_t wound = clampInt(risk / 3, 1, 3);
        exposure = clampInt(exposure + nightDose, 0, kMaxExposure);
        health = clampInt(health - wound, 0, kMaxHealth);
        snprintf(lead, sizeof(lead), "Curfew sirens lock the district. You flee through black rain.");
    } else {
        snprintf(lead, sizeof(lead), "Dawn leaks through the clinic shutters.");
    }
    startNewDay(lead);
}

void checkCollapse() {
    if (health > 0 && exposure < kMaxExposure) {
        return;
    }

    currentSite = 0;
    timeTick = 0;
    coolSitesForNewDay();
    day += 1;
    health = 5;
    exposure = 3;
    scrap = clampInt(scrap - 6, 0, 999);
    setStatus("You wake under clinic lights with your boots still wet. Collapse care cost 6 scrap and a little pride.");
}

void resolveFieldAction(UiAction action) {
    if (!fieldActionAvailable(action)) {
        setStatus("That move is no longer sensible here. The clock, the cache, or the route says no.");
        return;
    }

    const Stats stats = deriveStats();
    const Site& site = sites[currentSite];
    const int16_t skill = actionSkill(action, stats);
    const int16_t target = actionTarget(action);
    const int16_t total = skill + random(1, 7);
    const int16_t ambientDose = actionExposureCost(action, stats);
    const bool success = total >= target;
    const char* verb = "";

    if (action == UiAction::Scout) {
        verb = "scouted";
    } else if (action == UiAction::Scavenge) {
        verb = "scavenged";
    } else if (action == UiAction::Sneak) {
        verb = "moved quiet through";
    } else {
        return;
    }

    exposure = clampInt(exposure + ambientDose, 0, kMaxExposure);
    siteHeat[currentSite] = clampInt(siteHeat[currentSite] + actionHeatGain(action, success), 0, kMaxSiteHeat);

    char message[192];
    if (success && action == UiAction::Scout) {
        siteIntel[currentSite] = clampInt(siteIntel[currentSite] + 1, 0, kMaxSiteIntel);
        const int16_t gain = random(0, 3);
        scrap += gain;
        snprintf(message, sizeof(message),
                 "You %s %s. %s %d vs %d. Intel +1, heat +%u, +%d scrap. Cache reads %u.",
                 verb, site.name, actionCheckText(action), total, target, actionHeatGain(action, success), gain,
                 siteCache[currentSite]);
    } else if (success) {
        if (siteCache[currentSite] > 0) {
            --siteCache[currentSite];
        }
        const uint8_t itemId = randomLootForAction(action);
        const bool duplicate = hasCatalogItem(itemId) && itemCatalog[itemId].slot != Slot::Consumable;
        const int16_t gain = effectiveRiskForSite(currentSite) + random(1, 5) + (action == UiAction::Scavenge ? 2 : 0);
        scrap += gain;

        if (!duplicate && random(0, 100) < 62) {
            char itemMessage[112];
            addItem(itemId, itemMessage, sizeof(itemMessage));
            snprintf(message, sizeof(message), "You %s %s. %s %d vs %d. +%d scrap. %s Cache %u.",
                     verb, site.name, actionCheckText(action), total, target, gain, itemMessage, siteCache[currentSite]);
        } else {
            scrap += itemCatalog[itemId].value / 3;
            snprintf(message, sizeof(message),
                     "You %s %s. %s %d vs %d. +%d scrap, plus fenceable salvage. Cache %u.",
                     verb, site.name, actionCheckText(action), total, target, gain, siteCache[currentSite]);
        }
    } else {
        const int16_t wound = action == UiAction::Sneak ? 2 : 1 + effectiveRiskForSite(currentSite) / 5;
        const int16_t dose = 1 + effectiveRiskForSite(currentSite) / 3;
        health = clampInt(health - wound, 0, kMaxHealth);
        exposure = clampInt(exposure + dose, 0, kMaxExposure);
        snprintf(message, sizeof(message),
                 "%s bites back. %s %d vs %d. Heat +%u, -%d body, +%d exposure.",
                 site.name, actionCheckText(action), total, target, actionHeatGain(action, success), wound, dose);
    }

    setStatus(message);
    spendTime(actionTimeCost(action));
    checkCollapse();
}

void travelToSite(uint8_t targetSite) {
    const Stats stats = deriveStats();
    if (targetSite >= kSiteCount) {
        return;
    }
    if (targetSite == currentSite) {
        currentScreen = Screen::Field;
        screenDirty = true;
        return;
    }

    const uint8_t travelTicks = travelTicksToSite(targetSite);
    if (timeRemainingTicks() < travelTicks) {
        setStatus("The map refuses the route. Not enough daylight remains before curfew teeth close.");
        return;
    }

    const int16_t routeDose = routeExposureCost(targetSite, stats);
    currentSite = targetSite;
    selectedMapSite = targetSite;
    if (currentSite != 0) {
        exposure = clampInt(exposure + routeDose, 0, kMaxExposure);
    } else {
        exposure = clampInt(exposure + routeDose, 0, kMaxExposure);
    }

    char message[160];
    snprintf(message, sizeof(message), "You trust the signal map and ride %uh to %s. Route dose +%d. Risk now %d.",
             static_cast<unsigned>(travelTicks * kTickHours), sites[currentSite].name, routeDose,
             effectiveRiskForSite(currentSite));
    setStatus(message);
    currentScreen = Screen::Field;
    spendTime(travelTicks);
    checkCollapse();
}

void restOrRetreat() {
    if (currentSite != 0) {
        currentSite = 0;
        exposure = clampInt(exposure + 1, 0, kMaxExposure);
        setStatus("You cut the run short and spend time limping back to the clinic before the rain gets clever.");
        spendTime(actionTimeCost(UiAction::Rest));
        checkCollapse();
        return;
    }

    const int16_t cost = scrap >= 2 ? 2 : scrap;
    scrap -= cost;
    health = clampInt(health + 4, 0, kMaxHealth);
    exposure = clampInt(exposure - 4, 0, kMaxExposure);
    startNewDay("A cot, a drip bag, and static through the wall. You recover what the city has not taken.");
    checkCollapse();
}

void handleAction(UiAction action, int16_t param) {
    switch (action) {
        case UiAction::Scout:
        case UiAction::Scavenge:
        case UiAction::Sneak:
            resolveFieldAction(action);
            break;
        case UiAction::Inventory:
            currentScreen = Screen::Inventory;
            screenDirty = true;
            break;
        case UiAction::Map:
            selectedMapSite = currentSite == 0 ? 1 : currentSite;
            currentScreen = Screen::Map;
            screenDirty = true;
            break;
        case UiAction::SelectSite:
            if (param >= 0 && param < static_cast<int16_t>(kSiteCount)) {
                selectedMapSite = static_cast<uint8_t>(param);
                screenDirty = true;
            }
            break;
        case UiAction::BackToField:
            currentScreen = Screen::Field;
            screenDirty = true;
            break;
        case UiAction::EquipOrUse:
            equipInventorySlot(static_cast<uint8_t>(param));
            break;
        case UiAction::Travel:
            travelToSite(static_cast<uint8_t>(param));
            break;
        case UiAction::Rest:
            restOrRetreat();
            break;
        case UiAction::None:
            break;
    }
}

void drawFieldScreen() {
    auto& display = M5.Display;
    clearButtons();
    display.fillScreen(rgb(3, 6, 9));
    drawHeader();

    const int32_t width = display.width();
    const int32_t height = display.height();
    const int32_t margin = 18;
    const int32_t top = 82;
    const int32_t bottomH = 72;
    const int32_t contentH = height - top - bottomH - margin;
    const int32_t leftW = 238;
    const int32_t rightW = 286;
    const int32_t centerX = margin + leftW + 14;
    const int32_t centerW = width - centerX - rightW - 2 * margin;
    const uint16_t panelBg = rgb(8, 12, 17);

    drawStatsPanel(margin, top, leftW, contentH);
    drawActionForecast(width - margin - rightW, top, rightW, contentH);

    drawPanel(centerX, top, centerW, contentH, sites[currentSite].color);
    display.setFont(&fonts::Font4);
    display.setTextColor(TFT_WHITE, panelBg);
    display.drawString(sites[currentSite].name, centerX + 18, top + 18);
    display.setFont(&fonts::Font2);
    display.setTextColor(rgb(155, 175, 175), panelBg);
    display.drawString(sites[currentSite].district, centerX + 20, top + 52);
    drawWrappedText(sites[currentSite].description, centerX + 20, top + 88, centerW - 40, 3, rgb(210, 218, 210),
                    panelBg);
    display.setFont(&fonts::Font2);
    display.setTextColor(rgb(180, 210, 205), panelBg);
    display.setCursor(centerX + 20, top + 156);
    display.printf("risk %d  cache %u  intel %u  heat %u", effectiveRiskForSite(currentSite), siteCache[currentSite],
                   siteIntel[currentSite], siteHeat[currentSite]);

    display.drawFastHLine(centerX + 18, top + 176, centerW - 36, rgb(55, 70, 70));
    display.setFont(&fonts::Font4);
    display.setTextColor(rgb(130, 230, 200), panelBg);
    display.drawString("Last Signal", centerX + 18, top + 202);
    drawWrappedText(statusLine, centerX + 20, top + 246, centerW - 40, 5, TFT_WHITE, panelBg);

    const int32_t buttonY = height - bottomH;
    const int32_t gap = 10;
    const int32_t buttonW = (width - gap * 7) / 6;
    addButton("Scout 2h", gap, buttonY + 8, buttonW, 52, UiAction::Scout, 0, rgb(90, 210, 220),
              fieldActionAvailable(UiAction::Scout));
    addButton("Scav 4h", gap * 2 + buttonW, buttonY + 8, buttonW, 52, UiAction::Scavenge, 0, rgb(230, 180, 70),
              fieldActionAvailable(UiAction::Scavenge));
    addButton("Sneak 2h", gap * 3 + buttonW * 2, buttonY + 8, buttonW, 52, UiAction::Sneak, 0, rgb(220, 90, 190),
              fieldActionAvailable(UiAction::Sneak));
    addButton("Kit", gap * 4 + buttonW * 3, buttonY + 8, buttonW, 52, UiAction::Inventory, 0, rgb(170, 120, 240));
    addButton("Map", gap * 5 + buttonW * 4, buttonY + 8, buttonW, 52, UiAction::Map, 0, rgb(120, 220, 120));
    addButton(currentSite == 0 ? "Rest" : "Retreat", gap * 6 + buttonW * 5, buttonY + 8, buttonW, 52, UiAction::Rest,
              0, rgb(230, 90, 95));

    for (uint8_t i = 0; i < buttonCount; ++i) {
        drawButton(buttons[i]);
    }
}

int32_t mapPinX(const MapPin& pin, int32_t x, int32_t w) {
    return x + 26 + static_cast<int32_t>(pin.xPermille) * (w - 52) / 1000;
}

int32_t mapPinY(const MapPin& pin, int32_t y, int32_t h) {
    return y + 26 + static_cast<int32_t>(pin.yPermille) * (h - 52) / 1000;
}

void drawMapBackground(int32_t x, int32_t y, int32_t w, int32_t h) {
    auto& display = M5.Display;
    const uint16_t bg = rgb(4, 8, 11);

    drawPanel(x, y, w, h, rgb(46, 120, 110));
    for (int32_t gx = x + 32; gx < x + w - 24; gx += 58) {
        display.drawFastVLine(gx, y + 18, h - 36, rgb(13, 28, 32));
    }
    for (int32_t gy = y + 30; gy < y + h - 20; gy += 46) {
        display.drawFastHLine(x + 18, gy, w - 36, rgb(12, 25, 30));
    }

    for (uint8_t i = 0; i < 42; ++i) {
        const uint32_t seed = static_cast<uint32_t>(i) * 1103515245UL + static_cast<uint32_t>(day) * 977UL;
        const int32_t px = x + 18 + static_cast<int32_t>(seed % static_cast<uint32_t>(w - 36));
        const int32_t py = y + 18 + static_cast<int32_t>((seed / 97U) % static_cast<uint32_t>(h - 36));
        display.drawPixel(px, py, rgb(30, 70, 72));
        if ((i % 9) == 0) {
            display.drawFastHLine(px - 5, py, 11, rgb(40, 90, 82));
        }
    }

    display.setFont(&fonts::Font2);
    display.setTextColor(rgb(45, 90, 88), bg);
    display.drawString("CITY SURVEY REFUSED / SIGNAL MAP RECONSTRUCTED FROM BAD DATA", x + 24, y + h - 30);
    display.drawString("red routes mean attention, green routes mean lies", x + 24, y + 16);
}

void drawMapRoutes(int32_t x, int32_t y, int32_t w, int32_t h) {
    auto& display = M5.Display;
    for (uint8_t i = 0; i < kRouteEdgeCount; ++i) {
        const MapPin& a = mapPins[routeEdges[i].from];
        const MapPin& b = mapPins[routeEdges[i].to];
        const int32_t ax = mapPinX(a, x, w);
        const int32_t ay = mapPinY(a, y, h);
        const int32_t bx = mapPinX(b, x, w);
        const int32_t by = mapPinY(b, y, h);
        const bool active = routeEdges[i].from == currentSite || routeEdges[i].to == currentSite;
        const bool selected = routeEdges[i].from == selectedMapSite || routeEdges[i].to == selectedMapSite;
        const uint16_t color = active ? rgb(110, 230, 180) : (selected ? rgb(180, 120, 230) : rgb(45, 72, 78));

        display.drawLine(ax, ay, bx, by, color);
        if (active || selected) {
            display.drawLine(ax + 1, ay, bx + 1, by, color);
        }
    }
}

void drawMapPins(int32_t x, int32_t y, int32_t w, int32_t h) {
    auto& display = M5.Display;
    const uint16_t bg = rgb(4, 8, 11);

    for (uint8_t i = 0; i < kMapPinCount; ++i) {
        const MapPin& pin = mapPins[i];
        const Site& site = sites[pin.site];
        const int32_t px = mapPinX(pin, x, w);
        const int32_t py = mapPinY(pin, y, h);
        const bool here = pin.site == currentSite;
        const bool selected = pin.site == selectedMapSite;
        const int32_t r = here ? 18 : 14;

        if (siteHeat[pin.site] > 0) {
            display.drawCircle(px, py, r + 8 + siteHeat[pin.site] * 2, rgb(120, 45, 58));
        }
        if (siteIntel[pin.site] > 0) {
            display.drawCircle(px, py, r + 6, rgb(80, 180, 170));
        }

        display.fillCircle(px, py, r, site.color);
        display.drawCircle(px, py, r + 3, selected ? TFT_WHITE : rgb(130, 150, 145));
        if (here) {
            display.drawCircle(px, py, r + 10, rgb(110, 230, 180));
            display.drawCircle(px, py, r + 15, rgb(45, 95, 88));
        }

        display.setFont(&fonts::Font2);
        display.setTextColor(selected ? TFT_WHITE : rgb(180, 198, 190), bg);
        display.drawString(pin.callSign, px + 20, py - 9);
        addButton("", px - 46, py - 40, 120, 78, UiAction::SelectSite, pin.site, site.color, true, false);
    }
}

void drawMapDetailPanel(int32_t x, int32_t y, int32_t w, int32_t h) {
    auto& display = M5.Display;
    const uint16_t bg = rgb(8, 12, 17);
    const uint8_t siteIndex = selectedMapSite < kSiteCount ? selectedMapSite : currentSite;
    const Site& site = sites[siteIndex];
    const MapPin& pin = mapPins[siteIndex];
    const Stats stats = deriveStats();
    const uint8_t ticks = travelTicksToSite(siteIndex);
    const int16_t routeDose = routeExposureCost(siteIndex, stats);
    const bool here = siteIndex == currentSite;
    const bool travelReady = canTravelToSite(siteIndex);

    drawPanel(x, y, w, h, site.color);
    display.setFont(&fonts::Font4);
    display.setTextColor(TFT_WHITE, bg);
    display.drawString(site.name, x + 16, y + 16);
    display.setFont(&fonts::Font2);
    display.setTextColor(rgb(160, 180, 178), bg);
    display.drawString(site.district, x + 18, y + 52);

    display.setTextColor(rgb(125, 230, 205), bg);
    display.setCursor(x + 18, y + 86);
    display.printf("risk %d  cache %u/%u  intel %u  heat %u", effectiveRiskForSite(siteIndex), siteCache[siteIndex],
                   site.maxCache, siteIntel[siteIndex], siteHeat[siteIndex]);

    display.setTextColor(rgb(215, 222, 212), bg);
    drawWrappedText(pin.whisper, x + 18, y + 122, w - 36, 3, rgb(215, 222, 212), bg);
    drawWrappedText(site.description, x + 18, y + 198, w - 36, 4, rgb(165, 180, 176), bg);

    display.drawFastHLine(x + 16, y + h - 120, w - 32, rgb(55, 70, 70));
    display.setTextColor(rgb(200, 210, 205), bg);
    display.setCursor(x + 18, y + h - 96);
    if (here) {
        display.print("You are already inside this signal.");
    } else {
        display.printf("route %s  %uh  dose +%d", directRoute(currentSite, siteIndex) ? "direct" : "cross-town",
                       static_cast<unsigned>(ticks * kTickHours), routeDose);
    }
    display.setCursor(x + 18, y + h - 70);
    display.print(travelReady ? "The route is open." : (here ? "Current location." : "Curfew blocks this route."));
}

void drawMapScreen() {
    auto& display = M5.Display;
    clearButtons();
    display.fillScreen(rgb(2, 5, 8));
    drawHeader();

    const int32_t width = display.width();
    const int32_t height = display.height();
    const int32_t margin = 18;
    const int32_t top = 82;
    const int32_t bottomH = 72;
    const int32_t detailW = 360;
    const int32_t mapX0 = margin;
    const int32_t mapY0 = top;
    const int32_t mapW = width - detailW - margin * 3;
    const int32_t mapH = height - top - bottomH - margin;
    const int32_t detailX = mapX0 + mapW + margin;

    drawMapBackground(mapX0, mapY0, mapW, mapH);
    drawMapRoutes(mapX0, mapY0, mapW, mapH);
    drawMapPins(mapX0, mapY0, mapW, mapH);
    drawMapDetailPanel(detailX, top, detailW, mapH);

    const int32_t buttonY = height - bottomH;
    char travelLabel[24];
    const uint8_t ticks = travelTicksToSite(selectedMapSite);
    snprintf(travelLabel, sizeof(travelLabel), ticks > 0 ? "Travel %uh" : "Travel", static_cast<unsigned>(ticks * kTickHours));
    addButton("Field", margin, buttonY + 8, 160, 52, UiAction::BackToField, 0, rgb(90, 210, 220));
    addButton("Kit", margin + 174, buttonY + 8, 150, 52, UiAction::Inventory, 0, rgb(170, 120, 240));
    addButton(travelLabel, width - margin - 230, buttonY + 8, 230, 52, UiAction::Travel, selectedMapSite,
              sites[selectedMapSite].color, canTravelToSite(selectedMapSite));

    for (uint8_t i = 0; i < buttonCount; ++i) {
        drawButton(buttons[i]);
    }
}

void drawStatDelta(const Item& item, int32_t x, int32_t y, uint16_t bg) {
    auto& display = M5.Display;
    display.setFont(&fonts::Font2);
    display.setTextColor(rgb(160, 180, 180), bg);
    display.setCursor(x, y);
    display.printf("G%+d T%+d S%+d H%+d F%+d X%+d", item.grit, item.tech, item.scan, item.ghost, item.filter,
                   item.strain);
}

bool isEquippedInventorySlot(uint8_t invSlot) {
    for (uint8_t i = 0; i < kEquipSlotCount; ++i) {
        if (equipped[i] == invSlot) {
            return true;
        }
    }
    return false;
}

void drawInventoryScreen() {
    auto& display = M5.Display;
    clearButtons();
    display.fillScreen(rgb(3, 6, 9));
    drawHeader();

    const int32_t width = display.width();
    const int32_t height = display.height();
    const int32_t margin = 18;
    const int32_t top = 82;
    const int32_t bottomH = 72;
    const int32_t leftW = 330;
    const int32_t listX = margin + leftW + 16;
    const int32_t listW = width - listX - margin;
    const int32_t rowH = 58;
    const uint16_t bg = rgb(8, 12, 17);

    drawEquippedList(margin, top, leftW, height - top - bottomH - margin);
    drawPanel(listX, top, listW, height - top - bottomH - margin, rgb(170, 120, 240));

    display.setFont(&fonts::Font4);
    display.setTextColor(TFT_WHITE, bg);
    display.drawString("Inventory", listX + 18, top + 16);

    display.setFont(&fonts::Font2);
    display.setTextColor(rgb(150, 168, 170), bg);
    display.drawString("Tap gear to equip. Tap doses to use.", listX + 18, top + 52);

    uint8_t drawn = 0;
    for (uint8_t i = 0; i < kInventoryCapacity; ++i) {
        if (!validInventorySlot(i)) {
            continue;
        }
        const Item& item = itemCatalog[inventory[i]];
        const int32_t rowY = top + 88 + drawn * rowH;
        const bool equippedNow = isEquippedInventorySlot(i);
        const uint16_t rowBg = equippedNow ? rgb(18, 28, 35) : bg;

        display.fillRoundRect(listX + 16, rowY, listW - 32, rowH - 8, 6, rowBg);
        display.drawRoundRect(listX + 16, rowY, listW - 32, rowH - 8, 6, equippedNow ? rgb(100, 230, 180) : item.color);
        display.fillRect(listX + 28, rowY + 13, 10, 24, item.color);
        display.setTextColor(TFT_WHITE, rowBg);
        display.drawString(item.name, listX + 50, rowY + 8);
        display.setTextColor(rgb(150, 168, 170), rowBg);
        display.drawString(slotName(item.slot), listX + 260, rowY + 8);
        drawStatDelta(item, listX + 50, rowY + 30, rowBg);
        if (equippedNow) {
            display.setTextColor(rgb(120, 240, 190), rowBg);
            display.drawString("EQUIPPED", listX + listW - 126, rowY + 18);
        }
        addButton("", listX + 16, rowY, listW - 32, rowH - 8, UiAction::EquipOrUse, i, item.color, true, false);
        ++drawn;
    }

    const int32_t buttonY = height - bottomH;
    addButton("Field", margin, buttonY + 8, 190, 52, UiAction::BackToField, 0, rgb(90, 210, 220));
    for (uint8_t i = 0; i < buttonCount; ++i) {
        drawButton(buttons[i]);
    }
}

void drawCurrentScreen() {
    switch (currentScreen) {
        case Screen::Inventory:
            drawInventoryScreen();
            break;
        case Screen::Map:
            drawMapScreen();
            break;
        case Screen::Field:
            drawFieldScreen();
            break;
    }
    screenDirty = false;
}

void initializeColors() {
    itemCatalog[0].color = rgb(110, 150, 150);
    itemCatalog[1].color = rgb(90, 180, 120);
    itemCatalog[2].color = rgb(190, 90, 210);
    itemCatalog[3].color = rgb(80, 200, 220);
    itemCatalog[4].color = rgb(90, 230, 190);
    itemCatalog[5].color = rgb(220, 180, 70);
    itemCatalog[6].color = rgb(230, 120, 70);
    itemCatalog[7].color = rgb(210, 80, 160);
    itemCatalog[8].color = rgb(230, 70, 90);
    itemCatalog[9].color = rgb(240, 190, 80);
    itemCatalog[10].color = rgb(120, 150, 250);
    itemCatalog[11].color = rgb(120, 230, 170);
    itemCatalog[12].color = rgb(170, 230, 80);
    itemCatalog[13].color = rgb(180, 120, 80);
    itemCatalog[14].color = rgb(210, 130, 60);

    sites[0].color = rgb(80, 180, 160);
    sites[1].color = rgb(220, 70, 180);
    sites[2].color = rgb(80, 130, 220);
    sites[3].color = rgb(230, 180, 70);
    sites[4].color = rgb(120, 220, 90);
}

void initializeGame() {
    for (uint8_t i = 0; i < kInventoryCapacity; ++i) {
        inventory[i] = -1;
    }
    for (uint8_t i = 0; i < kEquipSlotCount; ++i) {
        equipped[i] = -1;
    }
    for (uint8_t i = 0; i < kSiteCount; ++i) {
        siteHeat[i] = 0;
        siteIntel[i] = 0;
        siteCache[i] = sites[i].maxCache;
    }

    inventory[0] = 0;
    inventory[1] = 3;
    inventory[2] = 5;
    inventory[3] = 7;
    inventory[4] = 12;
    inventory[5] = 13;
    equipped[0] = 0;
    equipped[1] = 1;
    equipped[2] = 2;
    equipped[3] = 3;
}

struct TouchState {
    bool pressed = false;
    bool justPressed = false;
    int32_t x = 0;
    int32_t y = 0;
};

TouchState readTouch() {
    TouchState touch;
    lgfx::touch_point_t point;
    const uint8_t touchCount = M5.Display.getTouchRaw(&point, 1);
    if (touchCount > 0) {
        M5.Display.convertRawXY(&point, 1);
        touch.pressed = true;
        touch.justPressed = !touchWasPressed;
        touch.x = point.x;
        touch.y = point.y;
    }
    touchWasPressed = touch.pressed;
    return touch;
}

void handleTouch(const TouchState& touch) {
    if (!touch.justPressed) {
        return;
    }

    for (int i = buttonCount - 1; i >= 0; --i) {
        if (hitButton(buttons[i], touch.x, touch.y)) {
            handleAction(buttons[i].action, buttons[i].param);
            return;
        }
    }
}

}  // namespace

void setup() {
    Serial.begin(115200);
    delay(50);
    Serial.println();
    Serial.println("[neon exclusion] boot");

    auto cfg = M5.config();
    M5.begin(cfg);

    M5.Display.setRotation(1);
    M5.Display.setBrightness(180);
    M5.Display.setTextWrap(false);
    randomSeed(static_cast<uint32_t>(micros()));

    initializeColors();
    initializeGame();
    drawCurrentScreen();
}

void loop() {
    M5.update();
    const TouchState touch = readTouch();
    handleTouch(touch);

    if (screenDirty) {
        drawCurrentScreen();
    }

    delay(12);
}
