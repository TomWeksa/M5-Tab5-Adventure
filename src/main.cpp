#include <Arduino.h>
#include <M5Unified.h>
#include <stdio.h>
#include <string.h>

namespace {

enum class Screen : uint8_t {
    Field,
    Inventory,
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
    uint16_t color;
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
constexpr uint8_t kMaxButtons = 24;
constexpr int16_t kMaxHealth = 10;
constexpr int16_t kMaxExposure = 12;

Screen currentScreen = Screen::Field;
int16_t health = 9;
int16_t exposure = 0;
int16_t scrap = 8;
uint16_t day = 1;
uint8_t currentSite = 0;
uint8_t actionsTaken = 0;
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
    {"Underpass Clinic", "safe berth", "A medic shack below the ring road. Warm bulbs, bad coffee, guarded doors.", 1, 0},
    {"Neon Spillway", "wet market", "Old ad towers bleed color through toxic rain and illegal stalls.", 3, 0},
    {"Sunken Mall", "retail tomb", "Escalators vanish into black water. Something below keeps the music playing.", 4, 0},
    {"Relay Grave", "antenna field", "Fallen towers tick in the wind. Messages still arrive for the dead.", 5, 0},
    {"Black Reed Verge", "outer exclusion", "A reed sea grown through asphalt. Every shadow looks freshly made.", 6, 0},
};

constexpr uint8_t kItemCount = sizeof(itemCatalog) / sizeof(itemCatalog[0]);
constexpr uint8_t kSiteCount = sizeof(sites) / sizeof(sites[0]);

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
    display.setCursor(width - 330, 16);
    display.printf("Day %u  scrap %d  site risk %u", static_cast<unsigned>(day), scrap, sites[currentSite].risk);
}

void drawStatsPanel(int32_t x, int32_t y, int32_t w, int32_t h) {
    auto& display = M5.Display;
    const uint16_t bg = rgb(8, 12, 17);
    const Stats stats = deriveStats();

    drawPanel(x, y, w, h, rgb(48, 120, 130));
    display.setFont(&fonts::Font4);
    display.setTextColor(TFT_WHITE, bg);
    display.drawString("Runner", x + 14, y + 12);

    drawMeter(x + 16, y + 68, w - 32, 16, health, kMaxHealth, rgb(230, 80, 90), "Body");
    drawMeter(x + 16, y + 122, w - 32, 16, exposure, kMaxExposure, rgb(170, 230, 80), "Exposure");

    const int32_t statY = y + 166;
    display.setFont(&fonts::Font2);
    display.setTextColor(rgb(220, 230, 225), bg);
    display.setCursor(x + 18, statY);
    display.printf("GRIT   %d", stats.grit);
    display.setCursor(x + 18, statY + 26);
    display.printf("TECH   %d", stats.tech);
    display.setCursor(x + 18, statY + 52);
    display.printf("SCAN   %d", stats.scan);
    display.setCursor(x + 18, statY + 78);
    display.printf("GHOST  %d", stats.ghost);
    display.setCursor(x + 18, statY + 104);
    display.printf("FILTER %d", stats.filter);
    display.setCursor(x + 18, statY + 130);
    display.printf("STRAIN %d", stats.strain);

    display.setTextColor(rgb(135, 150, 150), bg);
    display.drawString("No levels. Only kit.", x + 18, y + h - 32);
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

uint8_t randomLootForAction(UiAction action) {
    const uint8_t risk = sites[currentSite].risk;
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

void checkCollapse() {
    if (health > 0 && exposure < kMaxExposure) {
        return;
    }

    currentSite = 0;
    day += 1;
    health = 5;
    exposure = 3;
    scrap = clampInt(scrap - 5, 0, 999);
    setStatus("You wake under clinic lights with your boots still wet. The bill is missing from your pocket.");
}

void resolveFieldAction(UiAction action) {
    const Stats stats = deriveStats();
    const Site& site = sites[currentSite];
    int16_t skill = 0;
    const char* verb = "";

    if (action == UiAction::Scout) {
        skill = stats.scan + stats.tech;
        verb = "scouted";
    } else if (action == UiAction::Scavenge) {
        skill = stats.grit + stats.filter;
        verb = "scavenged";
    } else if (action == UiAction::Sneak) {
        skill = stats.ghost + stats.scan;
        verb = "moved quiet through";
    } else {
        return;
    }

    const int16_t roll = random(1, 7);
    const int16_t target = 5 + site.risk;
    const int16_t total = skill + roll;
    const int16_t ambientDose = clampInt((site.risk + stats.strain - stats.filter) / 3, 0, 4);
    exposure = clampInt(exposure + ambientDose, 0, kMaxExposure);
    ++actionsTaken;

    char message[192];
    if (total >= target) {
        const uint8_t itemId = randomLootForAction(action);
        const bool duplicate = hasCatalogItem(itemId) && itemCatalog[itemId].slot != Slot::Consumable;
        const int16_t gain = site.risk + random(1, 5);
        scrap += gain;

        if (!duplicate && random(0, 100) < 62) {
            char itemMessage[112];
            addItem(itemId, itemMessage, sizeof(itemMessage));
            snprintf(message, sizeof(message), "You %s %s cleanly. +%d scrap. %s", verb, site.name, gain, itemMessage);
        } else {
            scrap += itemCatalog[itemId].value / 3;
            snprintf(message, sizeof(message), "You %s %s cleanly. +%d scrap, plus fenceable salvage.", verb, site.name,
                     gain);
        }
    } else {
        const int16_t wound = action == UiAction::Sneak ? 2 : 1;
        const int16_t dose = 1 + site.risk / 3;
        health = clampInt(health - wound, 0, kMaxHealth);
        exposure = clampInt(exposure + dose, 0, kMaxExposure);
        snprintf(message, sizeof(message), "%s bites back. Check %d vs %d. -%d body, +%d exposure.", site.name, total,
                 target, wound, dose);
    }

    if (actionsTaken >= 4) {
        actionsTaken = 0;
        ++day;
    }

    setStatus(message);
    checkCollapse();
}

void travel() {
    const Stats stats = deriveStats();
    currentSite = static_cast<uint8_t>((currentSite + 1) % kSiteCount);
    if (currentSite == 0) {
        ++day;
    } else {
        exposure = clampInt(exposure + clampInt(sites[currentSite].risk - stats.filter, 0, 2), 0, kMaxExposure);
    }

    char message[160];
    snprintf(message, sizeof(message), "You follow service roads to %s. The city keeps changing behind you.",
             sites[currentSite].name);
    setStatus(message);
    checkCollapse();
}

void restOrRetreat() {
    if (currentSite != 0) {
        currentSite = 0;
        exposure = clampInt(exposure + 1, 0, kMaxExposure);
        setStatus("You cut the run short and limp back to the clinic before the rain gets clever.");
        return;
    }

    const int16_t cost = scrap >= 2 ? 2 : scrap;
    scrap -= cost;
    health = clampInt(health + 4, 0, kMaxHealth);
    exposure = clampInt(exposure - 4, 0, kMaxExposure);
    ++day;
    setStatus("A cot, a drip bag, and static through the wall. You recover what the city has not taken.");
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
        case UiAction::BackToField:
            currentScreen = Screen::Field;
            screenDirty = true;
            break;
        case UiAction::EquipOrUse:
            equipInventorySlot(static_cast<uint8_t>(param));
            break;
        case UiAction::Travel:
            travel();
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
    const int32_t rightW = 268;
    const int32_t centerX = margin + leftW + 14;
    const int32_t centerW = width - centerX - rightW - 2 * margin;
    const uint16_t panelBg = rgb(8, 12, 17);

    drawStatsPanel(margin, top, leftW, contentH);
    drawPackPreview(width - margin - rightW, top, rightW, contentH);

    drawPanel(centerX, top, centerW, contentH, sites[currentSite].color);
    display.setFont(&fonts::Font4);
    display.setTextColor(TFT_WHITE, panelBg);
    display.drawString(sites[currentSite].name, centerX + 18, top + 18);
    display.setFont(&fonts::Font2);
    display.setTextColor(rgb(155, 175, 175), panelBg);
    display.drawString(sites[currentSite].district, centerX + 20, top + 52);
    drawWrappedText(sites[currentSite].description, centerX + 20, top + 88, centerW - 40, 3, rgb(210, 218, 210),
                    panelBg);

    display.drawFastHLine(centerX + 18, top + 176, centerW - 36, rgb(55, 70, 70));
    display.setFont(&fonts::Font4);
    display.setTextColor(rgb(130, 230, 200), panelBg);
    display.drawString("Last Signal", centerX + 18, top + 202);
    drawWrappedText(statusLine, centerX + 20, top + 246, centerW - 40, 5, TFT_WHITE, panelBg);

    const int32_t buttonY = height - bottomH;
    const int32_t gap = 10;
    const int32_t buttonW = (width - gap * 7) / 6;
    addButton("Scout", gap, buttonY + 8, buttonW, 52, UiAction::Scout, 0, rgb(90, 210, 220));
    addButton("Scavenge", gap * 2 + buttonW, buttonY + 8, buttonW, 52, UiAction::Scavenge, 0, rgb(230, 180, 70));
    addButton("Sneak", gap * 3 + buttonW * 2, buttonY + 8, buttonW, 52, UiAction::Sneak, 0, rgb(220, 90, 190));
    addButton("Kit", gap * 4 + buttonW * 3, buttonY + 8, buttonW, 52, UiAction::Inventory, 0, rgb(170, 120, 240));
    addButton("Travel", gap * 5 + buttonW * 4, buttonY + 8, buttonW, 52, UiAction::Travel, 0, rgb(120, 220, 120));
    addButton(currentSite == 0 ? "Rest" : "Retreat", gap * 6 + buttonW * 5, buttonY + 8, buttonW, 52, UiAction::Rest,
              0, rgb(230, 90, 95));

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
    if (currentScreen == Screen::Inventory) {
        drawInventoryScreen();
    } else {
        drawFieldScreen();
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
