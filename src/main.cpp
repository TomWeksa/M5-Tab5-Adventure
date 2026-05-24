#include <Arduino.h>
#include <M5Unified.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "GameContent.h"
#include "GameTypes.h"

namespace {

// Inventory has no intended gameplay limit for now. This large static backing
// store simply avoids heap churn on the embedded target.
constexpr uint8_t kInventoryCapacity = 96;
constexpr uint8_t kEquipSlotCount = 5;
constexpr uint8_t kSiteCapacity = 8;
constexpr uint8_t kMaxButtons = 32;
constexpr int16_t kMaxHealth = 10;
constexpr int16_t kMaxExposure = 12;
constexpr uint8_t kMaxTimeTicks = 8;
constexpr uint8_t kDayStartHour = 7;
constexpr uint8_t kTickHours = 2;
constexpr uint8_t kMaxSiteIntel = 3;
constexpr uint8_t kMaxSiteAttention = 6;
constexpr int16_t kDailyUpkeepValue = 4;
constexpr uint8_t kMaxRewardItems = 12;
constexpr uint8_t kTradeStockCount = 8;
constexpr uint8_t kTradeRowsPerPage = 6;
constexpr uint8_t kBatteryCellItem = 15;
constexpr uint8_t kCleanWaterItem = 16;
constexpr uint8_t kMedPackItem = 17;
constexpr uint8_t kReedskinMantleItem = 18;
constexpr uint8_t kHospitalVinylApronItem = 19;
constexpr uint8_t kStaticBridalVeilItem = 20;
constexpr uint8_t kRoadworkerHuskItem = 21;
constexpr uint8_t kFloodChoirWadersItem = 22;
constexpr uint8_t kAshfallPonchoItem = 23;
constexpr uint8_t kGlasshouseSmockItem = 24;
constexpr uint8_t kDebtCollectorsCoatItem = 25;
constexpr uint8_t kDeadPagerItem = 26;
constexpr uint8_t kMothCompassItem = 27;
constexpr uint8_t kRainCounterItem = 28;
constexpr uint8_t kKindlingScopeItem = 29;
constexpr uint8_t kBoneTuningForkItem = 30;
constexpr uint8_t kCheckoutOracleItem = 31;
constexpr uint8_t kSparrowDroneShellItem = 32;
constexpr uint8_t kGlassLungMeterItem = 33;
constexpr uint8_t kRainKeyItem = 34;
constexpr uint8_t kDrownedRegisterItem = 35;
constexpr uint8_t kServiceWormItem = 36;
constexpr uint8_t kTarTapeRollItem = 37;
constexpr uint8_t kMercyBoltCutterItem = 38;
constexpr uint8_t kPulseStaplerItem = 39;
constexpr uint8_t kSignalChalkItem = 40;
constexpr uint8_t kValveRosaryItem = 41;
constexpr uint8_t kMapScalpelItem = 42;
constexpr uint8_t kFlareHookItem = 43;
constexpr uint8_t kStaticKnifeItem = 44;
constexpr uint8_t kTollHammerItem = 45;
constexpr uint8_t kMercySirenItem = 46;
constexpr uint8_t kRivetSaintItem = 47;
constexpr uint8_t kColdLanternItem = 48;
constexpr uint8_t kCivicBatonItem = 49;
constexpr uint8_t kWhiteReceiptItem = 50;
constexpr uint8_t kSpareHourItem = 51;
constexpr uint8_t kLittleFloodSaintItem = 52;
constexpr uint8_t kApologyEngineItem = 53;
constexpr uint8_t kEmptyNameTagItem = 54;
constexpr uint8_t kBlackReedSeedItem = 55;
constexpr uint8_t kBorrowedShadowItem = 56;
constexpr uint8_t kCopperToothRadioItem = 57;
constexpr uint8_t kMercyBellItem = 58;
constexpr uint8_t kCalendarOfRainsItem = 59;
constexpr uint8_t kRedactedPhotographItem = 60;
constexpr uint8_t kTenthButtonItem = 61;
constexpr uint8_t kGhostTeaAmpouleItem = 62;
constexpr uint8_t kBlackIodineStripItem = 63;
constexpr uint8_t kSleeplessMintItem = 64;
constexpr uint8_t kFenceRunnersSaltItem = 65;
constexpr uint8_t kCheapCourageSyrupItem = 66;
constexpr uint8_t kBlueMilkSachetItem = 67;
constexpr uint8_t kRubberTrenchItem = 1;
constexpr uint8_t kMirrorweaveItem = 2;
constexpr uint8_t kCoilDetectorItem = 3;
constexpr uint8_t kGlassNeedleItem = 4;
constexpr uint8_t kPrybarKitItem = 5;
constexpr uint8_t kSolderRigItem = 6;
constexpr uint8_t kQuietNailgunItem = 7;
constexpr uint8_t kRailPistolItem = 8;
constexpr uint8_t kWarmBatteryItem = 9;
constexpr uint8_t kMourningLensItem = 10;
constexpr uint8_t kNullCharmItem = 11;
constexpr uint8_t kCopperSaintItem = 14;

enum class OutcomeLevel : uint8_t { Failure, Partial, Success, Full };
enum class StoryArc : uint8_t { BatteriesForTheDead, LastSaleAtB2, PersonWhoNeverEntered, Count };

// Runtime state is intentionally plain globals because the Arduino loop is a
// single-scene program and redraws from this state directly.
Screen currentScreen = Screen::Field;
int16_t health = 9;
int16_t exposure = 0;
uint16_t day = 1;
uint8_t currentSite = 0;
uint8_t selectedMapSite = 1;
int16_t selectedInventorySlot = 0;
uint8_t inventoryPage = 0;
uint8_t tradePage = 0;
uint8_t timeTick = 0;
uint8_t siteAttention[kSiteCapacity];
uint8_t siteCache[kSiteCapacity];
uint8_t siteIntel[kSiteCapacity];
LeadKind siteLead[kSiteCapacity];
int16_t inventory[kInventoryCapacity];
int16_t equipped[kEquipSlotCount];
bool tradeOfferSelected[kInventoryCapacity];
bool tradeWantSelected[kTradeStockCount];
uint8_t rewardItems[kMaxRewardItems];
uint8_t rewardCount = 0;
Button buttons[kMaxButtons];
uint8_t buttonCount = 0;
bool screenDirty = true;
bool touchWasPressed = false;
bool warmBatterySpent = false;
bool railPistolSpent = false;
bool quietNailgunSpent = false;
bool mourningLensSpent = false;
bool copperSaintSpent = false;
bool staticVeilSpent = false;
bool debtCoatSpent = false;
bool deadPagerSpent = false;
bool rainKeySpent = false;
bool flareHookSpent = false;
bool mercySirenSpent = false;
bool apologyEngineSpent = false;
bool borrowedShadowSpent = false;
bool mercyBellSpent = false;
bool calendarSpent = false;
uint8_t ghostTeaSite = 255;
bool blackIodineGuard = false;
bool sleeplessMintReady = false;
bool fenceSaltReady = false;
bool cheapCourageReady = false;
bool blueMilkBlurReady = false;
bool nullCharmSpent[kSiteCapacity];
bool ashfallPonchoSpent[kSiteCapacity];
bool tarTapeSpent[kSiteCapacity];
bool signalChalkMark[kSiteCapacity];
uint8_t storyStage[static_cast<uint8_t>(StoryArc::Count)];
uint8_t storyOutcome[static_cast<uint8_t>(StoryArc::Count)];
char statusLine[320] = "The rain tastes metallic. Your kit is the only thing between you and the quiet.";
char rewardTitle[80] = "";
char rewardSummary[320] = "";

const uint8_t tradeStock[kTradeStockCount] = {kBatteryCellItem,        kCleanWaterItem,      kMedPackItem, 12,
                                              13,                      kGhostTeaAmpouleItem, kBlackIodineStripItem,
                                              kBlueMilkSachetItem};

int16_t dailyUpkeepValue();

// Converts 8-bit RGB values to the display's 16-bit color format.
uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return M5.Display.color565(r, g, b);
}

// Clamps values used by meters, stats, and pressure calculations.
int clampInt(int value, int low, int high) {
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

// Returns the player-facing label for an equipment slot.
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

// Maps an item slot to the equipped-array index; consumables cannot equip.
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

// Validates a catalog index before reading item data.
bool validItemId(int16_t itemId) {
    return itemId >= 0 && itemId < static_cast<int16_t>(kItemCount);
}

// Validates an inventory index and confirms it points at a known item.
bool validInventorySlot(int16_t slot) {
    return slot >= 0 && slot < static_cast<int16_t>(kInventoryCapacity) && validItemId(inventory[slot]);
}

// Checks whether an inventory slot is currently equipped in any gear slot.
bool isEquippedInventorySlot(uint8_t invSlot) {
    for (uint8_t i = 0; i < kEquipSlotCount; ++i) {
        if (equipped[i] == invSlot) {
            return true;
        }
    }
    return false;
}

// Checks whether a specific catalog item is currently equipped.
bool equippedCatalogItem(uint8_t itemId) {
    for (uint8_t i = 0; i < kEquipSlotCount; ++i) {
        const int16_t invSlot = equipped[i];
        if (validInventorySlot(invSlot) && inventory[invSlot] == itemId) {
            return true;
        }
    }
    return false;
}

// Several one-shot doses apply to the current site or the next relevant action.
bool ghostTeaApplies(LeadKind lead) {
    return ghostTeaSite == currentSite && (lead == LeadKind::Contact || lead == LeadKind::Trail);
}

uint8_t storyIndex(StoryArc arc) {
    return static_cast<uint8_t>(arc);
}

// Removes an item from the pack and clears any equipment/trade references.
void removeInventorySlot(uint8_t invSlot) {
    if (invSlot >= kInventoryCapacity) {
        return;
    }
    for (uint8_t i = 0; i < kEquipSlotCount; ++i) {
        if (equipped[i] == invSlot) {
            equipped[i] = -1;
        }
    }
    tradeOfferSelected[invSlot] = false;
    inventory[invSlot] = -1;
}

// Totals carried value. This is informational and includes equipped gear.
int16_t carriedTradeValue() {
    int16_t total = 0;
    for (uint8_t i = 0; i < kInventoryCapacity; ++i) {
        if (validInventorySlot(i)) {
            total += itemCatalog[inventory[i]].value;
        }
    }
    return total;
}

// Totals value that can be offered without stripping equipped gear.
int16_t availableTradeValue() {
    int16_t total = 0;
    for (uint8_t i = 0; i < kInventoryCapacity; ++i) {
        if (validInventorySlot(i) && !isEquippedInventorySlot(i)) {
            total += itemCatalog[inventory[i]].value;
        }
    }
    return total;
}

// Rebuilds the active stat profile from equipped items each time it is needed.
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

// Converts the time tick into the in-world hour displayed in the header.
uint8_t currentHour() {
    return static_cast<uint8_t>(kDayStartHour + timeTick * kTickHours);
}

// Some artifacts extend the day rather than changing an action roll.
uint8_t maxTimeTicksForDay() {
    return static_cast<uint8_t>(kMaxTimeTicks + (equippedCatalogItem(kSpareHourItem) ? 1 : 0));
}

// Reports how many action ticks remain before the day ends.
uint8_t timeRemainingTicks() {
    const uint8_t maxTicks = maxTimeTicksForDay();
    return timeTick < maxTicks ? static_cast<uint8_t>(maxTicks - timeTick) : 0;
}

// Combines base site risk, attention, intel, and dusk pressure into live danger.
int16_t effectiveRiskForSite(uint8_t siteIndex) {
    if (siteIndex == 0 || siteIndex >= kSiteCount) {
        return sites[0].risk;
    }

    const int16_t duskPressure = timeTick >= 6 ? 1 : 0;
    return clampInt(sites[siteIndex].risk + siteAttention[siteIndex] / 2 - siteIntel[siteIndex] + duskPressure, 1, 9);
}

// Gives each field action its time cost in ticks.
uint8_t actionTimeCost(UiAction action) {
    switch (action) {
        case UiAction::Observe:
        case UiAction::FollowLead:
        case UiAction::Travel:
        case UiAction::Rest:
            return 1;
        case UiAction::Explore:
            return 2;
        default:
            return 0;
    }
}

// Names a discovered lead for status panels and action buttons.
const char* leadName(LeadKind lead) {
    switch (lead) {
        case LeadKind::Contact:
            return "quiet contact";
        case LeadKind::Cache:
            return "buried cache";
        case LeadKind::Anomaly:
            return "wrong-light anomaly";
        case LeadKind::Door:
            return "sealed service door";
        case LeadKind::Trail:
            return "cleaner footpath";
        case LeadKind::None:
            return "none";
    }
    return "none";
}

// Chooses the verb shown when the player acts on a lead.
const char* leadVerb(LeadKind lead) {
    switch (lead) {
        case LeadKind::Contact:
            return "Approach";
        case LeadKind::Cache:
            return "Recover";
        case LeadKind::Anomaly:
            return "Probe";
        case LeadKind::Door:
            return "Bypass";
        case LeadKind::Trail:
            return "Trace";
        case LeadKind::None:
            return "Follow";
    }
    return "Follow";
}

// Describes which stats are tested when a lead is followed.
const char* leadCheckText(LeadKind lead) {
    switch (lead) {
        case LeadKind::Contact:
            return "GHOST+TECH";
        case LeadKind::Cache:
            return "GRIT+FILTER";
        case LeadKind::Anomaly:
            return "SCAN+FILTER";
        case LeadKind::Door:
            return "TECH+GRIT";
        case LeadKind::Trail:
            return "GHOST+SCAN";
        case LeadKind::None:
            return "--";
    }
    return "--";
}

// Adds atmospheric interpretation to a discovered lead.
const char* leadWhisper(LeadKind lead) {
    switch (lead) {
        case LeadKind::Contact:
            return "Someone is watching the route without wanting to be seen.";
        case LeadKind::Cache:
            return "Fresh scrape marks vanish under dust and runoff.";
        case LeadKind::Anomaly:
            return "The light folds wrong near a place your eyes avoid.";
        case LeadKind::Door:
            return "A sealed access plate still has power behind it.";
        case LeadKind::Trail:
            return "Footprints dodge the hot ground like a practiced prayer.";
        case LeadKind::None:
            return "No lead. Observe, or push in blind.";
    }
    return "No lead.";
}

// Calculates the stat total used to resolve a lead action.
int16_t leadSkill(LeadKind lead, const Stats& stats) {
    switch (lead) {
        case LeadKind::Contact:
            return stats.ghost + stats.tech;
        case LeadKind::Cache:
            return stats.grit + stats.filter;
        case LeadKind::Anomaly:
            return stats.scan + stats.filter;
        case LeadKind::Door:
            return stats.tech + stats.grit;
        case LeadKind::Trail:
            return stats.ghost + stats.scan;
        case LeadKind::None:
            return 0;
    }
    return 0;
}

// Appends a short ability note to a status buffer.
void appendAbilityNote(char* buffer, size_t bufferSize, const char* note) {
    if (note == nullptr || note[0] == '\0') {
        return;
    }
    const size_t used = strlen(buffer);
    if (used + 2 >= bufferSize) {
        return;
    }
    snprintf(buffer + used, bufferSize - used, "%s%s", used == 0 ? "" : " ", note);
}

// Converts the roll margin into a richer result than binary pass/fail.
OutcomeLevel outcomeForRoll(int16_t total, int16_t target) {
    if (total >= target + 3) {
        return OutcomeLevel::Full;
    }
    if (total >= target) {
        return OutcomeLevel::Success;
    }
    if (total >= target - 2) {
        return OutcomeLevel::Partial;
    }
    return OutcomeLevel::Failure;
}

bool outcomeHasReward(OutcomeLevel outcome) {
    return outcome != OutcomeLevel::Failure;
}

const char* outcomeName(OutcomeLevel outcome) {
    switch (outcome) {
        case OutcomeLevel::Full:
            return "clean";
        case OutcomeLevel::Success:
            return "success";
        case OutcomeLevel::Partial:
            return "partial";
        case OutcomeLevel::Failure:
            return "failure";
    }
    return "failure";
}

bool actionUsesTech(UiAction action, LeadKind lead) {
    return action == UiAction::Observe ||
           (action == UiAction::FollowLead && (lead == LeadKind::Contact || lead == LeadKind::Door));
}

// Skill modifiers are item-specific nudges that make equipped gear change verbs.
int16_t abilitySkillDelta(UiAction action, LeadKind lead) {
    int16_t delta = 0;
    if (action == UiAction::Observe) {
        if (equippedCatalogItem(kMourningLensItem)) {
            delta += 1;
        }
        if (equippedCatalogItem(kGlassNeedleItem)) {
            delta += 1;
        }
        if (equippedCatalogItem(kMothCompassItem) || equippedCatalogItem(kRainCounterItem)) {
            delta += 1;
        }
        if ((currentSite == 2 || currentSite == 4) && equippedCatalogItem(kGlasshouseSmockItem)) {
            delta += 2;
        }
        if (currentSite == 1 && equippedCatalogItem(kSparrowDroneShellItem)) {
            delta += 1;
        }
        if (currentSite == 3 && equippedCatalogItem(kCopperToothRadioItem)) {
            delta += 2;
        }
        if (blueMilkBlurReady) {
            delta -= 2;
        }
        if (cheapCourageReady) {
            delta -= 1;
        }
    }
    if (action == UiAction::Explore) {
        if (currentSite == 1 && equippedCatalogItem(kTollHammerItem)) {
            delta += 1;
        }
        if (currentSite == 2 && equippedCatalogItem(kDrownedRegisterItem)) {
            delta += 1;
        }
        if (equippedCatalogItem(kRoadworkerHuskItem) || equippedCatalogItem(kRivetSaintItem)) {
            delta += 1;
        }
        if (cheapCourageReady) {
            delta += 2;
        }
    }
    if (action == UiAction::FollowLead) {
        if ((lead == LeadKind::Contact || lead == LeadKind::Trail) && equippedCatalogItem(kMirrorweaveItem)) {
            delta += 1;
        }
        if (ghostTeaApplies(lead)) {
            delta += 2;
        }
        if (fenceSaltReady && lead == LeadKind::Trail) {
            delta += 2;
        }
        if (lead == LeadKind::Trail && currentSite == 4 && equippedCatalogItem(kReedskinMantleItem)) {
            delta += 2;
        }
        if (lead == LeadKind::Anomaly && equippedCatalogItem(kGlassNeedleItem)) {
            delta += 2;
        }
        if (lead == LeadKind::Anomaly && equippedCatalogItem(kColdLanternItem)) {
            delta += 1;
        }
        if (lead == LeadKind::Door && equippedCatalogItem(kSolderRigItem)) {
            delta += 2;
        }
        if ((lead == LeadKind::Door || lead == LeadKind::Cache) && equippedCatalogItem(kPrybarKitItem)) {
            delta += 1;
        }
        if ((lead == LeadKind::Door || lead == LeadKind::Cache) &&
            (equippedCatalogItem(kMercyBoltCutterItem) || equippedCatalogItem(kValveRosaryItem))) {
            delta += 2;
        }
        if (lead == LeadKind::Door && equippedCatalogItem(kRoadworkerHuskItem)) {
            delta += 1;
        }
        if (lead == LeadKind::Trail && equippedCatalogItem(kCoilDetectorItem)) {
            delta += 1;
        }
        if (lead == LeadKind::Cache && currentSite == 2 && equippedCatalogItem(kCheckoutOracleItem)) {
            delta += 1;
        }
        if ((lead == LeadKind::Contact || lead == LeadKind::Cache) && currentSite == 1 &&
            equippedCatalogItem(kKindlingScopeItem)) {
            delta += 1;
        }
        if ((lead == LeadKind::Contact || lead == LeadKind::Door) && equippedCatalogItem(kStaticKnifeItem)) {
            delta += 1;
        }
        if (lead == LeadKind::Contact && equippedCatalogItem(kRedactedPhotographItem)) {
            delta += 1;
        }
    }
    return delta;
}

// Target modifiers represent gear opening a cleaner or more specialized method.
int16_t abilityTargetDelta(UiAction action, LeadKind lead) {
    int16_t delta = 0;
    if (action == UiAction::FollowLead) {
        if ((lead == LeadKind::Contact || lead == LeadKind::Trail) && equippedCatalogItem(kMirrorweaveItem)) {
            delta -= 1;
        }
        if (lead == LeadKind::Trail && currentSite == 4 && equippedCatalogItem(kReedskinMantleItem)) {
            delta -= 1;
        }
        if (lead == LeadKind::Contact && equippedCatalogItem(kHospitalVinylApronItem)) {
            delta += 1;
        }
        if ((lead == LeadKind::Contact || lead == LeadKind::Door) && equippedCatalogItem(kStaticKnifeItem)) {
            delta -= 1;
        }
        if (lead == LeadKind::Door && equippedCatalogItem(kSolderRigItem)) {
            delta -= 1;
        }
        if (lead == LeadKind::Door && equippedCatalogItem(kRainKeyItem) && !rainKeySpent) {
            delta -= 2;
        }
        if (lead == LeadKind::Door && equippedCatalogItem(kEmptyNameTagItem) && (currentSite == 1 || currentSite == 2)) {
            delta -= 1;
        }
        if ((lead == LeadKind::Door || lead == LeadKind::Cache) && equippedCatalogItem(kPrybarKitItem)) {
            delta -= 1;
        }
        if ((lead == LeadKind::Door || lead == LeadKind::Cache) &&
            (equippedCatalogItem(kMercyBoltCutterItem) || equippedCatalogItem(kValveRosaryItem))) {
            delta -= 1;
        }
        if (lead == LeadKind::Cache && currentSite == 2 &&
            (equippedCatalogItem(kDrownedRegisterItem) || equippedCatalogItem(kFloodChoirWadersItem) ||
             equippedCatalogItem(kCheckoutOracleItem))) {
            delta -= 1;
        }
        if (lead == LeadKind::Anomaly && equippedCatalogItem(kGlassNeedleItem)) {
            delta -= 1;
        }
        if (lead == LeadKind::Anomaly && equippedCatalogItem(kKindlingScopeItem)) {
            delta += 1;
        }
        if (lead == LeadKind::Anomaly && equippedCatalogItem(kFlareHookItem)) {
            delta += 1;
        }
        if ((lead == LeadKind::Door || lead == LeadKind::Cache || lead == LeadKind::Anomaly) &&
            equippedCatalogItem(kGlasshouseSmockItem)) {
            delta += 1;
        }
    }
    return delta;
}

// Dose modifiers make the environment respond to the actual kit, not only stats.
int16_t abilityDoseDelta(UiAction action, LeadKind lead) {
    int16_t delta = 0;
    if (action == UiAction::Explore && currentSite == 2 && equippedCatalogItem(kRubberTrenchItem)) {
        delta -= 1;
    }
    if (action == UiAction::Explore && currentSite == 2 &&
        (equippedCatalogItem(kHospitalVinylApronItem) || equippedCatalogItem(kFloodChoirWadersItem))) {
        delta -= 1;
    }
    if (action == UiAction::Explore && equippedCatalogItem(kAshfallPonchoItem) && !ashfallPonchoSpent[currentSite]) {
        delta -= 1;
    }
    if (action == UiAction::FollowLead && lead == LeadKind::Trail && equippedCatalogItem(kCoilDetectorItem)) {
        delta -= 1;
    }
    if (action == UiAction::FollowLead && lead == LeadKind::Trail &&
        (equippedCatalogItem(kReedskinMantleItem) || equippedCatalogItem(kFlareHookItem))) {
        delta -= 1;
    }
    if (action == UiAction::FollowLead && lead == LeadKind::Cache &&
        (equippedCatalogItem(kHospitalVinylApronItem) || equippedCatalogItem(kTarTapeRollItem))) {
        delta -= 1;
    }
    if (action == UiAction::FollowLead && lead == LeadKind::Anomaly && equippedCatalogItem(kGlassNeedleItem)) {
        delta += 1;
    }
    if (action == UiAction::FollowLead && lead == LeadKind::Anomaly && blackIodineGuard) {
        delta -= 2;
    }
    if (action == UiAction::FollowLead && lead == LeadKind::Anomaly &&
        (equippedCatalogItem(kColdLanternItem) || equippedCatalogItem(kGlassLungMeterItem))) {
        delta -= 1;
    }
    if (fenceSaltReady && (action == UiAction::Travel || action == UiAction::FollowLead) && lead == LeadKind::Trail) {
        delta -= 1;
    }
    if (action == UiAction::Observe && equippedCatalogItem(kMourningLensItem)) {
        delta += 1;
    }
    return delta;
}

// Attention modifiers capture how loudly the current kit solves a situation.
int16_t abilityAttentionDelta(UiAction action, LeadKind lead, bool rewardOutcome) {
    int16_t delta = 0;
    if (rewardOutcome && action == UiAction::FollowLead) {
        if ((lead == LeadKind::Contact || lead == LeadKind::Trail) && equippedCatalogItem(kMirrorweaveItem)) {
            delta -= 1;
        }
        if ((lead == LeadKind::Contact || lead == LeadKind::Trail) && ghostTeaApplies(lead)) {
            delta -= 1;
        }
        if (fenceSaltReady && lead == LeadKind::Trail) {
            delta -= 1;
        }
        if (lead == LeadKind::Door && equippedCatalogItem(kSolderRigItem)) {
            delta -= 1;
        }
        if (lead == LeadKind::Door && equippedCatalogItem(kRainKeyItem) && !rainKeySpent) {
            delta += 2;
        }
        if ((lead == LeadKind::Door || lead == LeadKind::Cache) && equippedCatalogItem(kPrybarKitItem)) {
            delta += 1;
        }
        if ((lead == LeadKind::Door || lead == LeadKind::Cache) &&
            (equippedCatalogItem(kMercyBoltCutterItem) || equippedCatalogItem(kTollHammerItem))) {
            delta += 1;
        }
        if ((lead == LeadKind::Door || lead == LeadKind::Contact) && equippedCatalogItem(kQuietNailgunItem)) {
            delta -= 1;
        }
        if (lead == LeadKind::Contact && equippedCatalogItem(kCivicBatonItem)) {
            delta += 1;
        }
    }
    if (action == UiAction::Observe) {
        if (equippedCatalogItem(kSparrowDroneShellItem) && rewardOutcome) {
            delta -= 1;
        }
        if (fenceSaltReady) {
            delta -= 1;
        }
    }
    if (action == UiAction::Explore) {
        if (equippedCatalogItem(kRailPistolItem) || equippedCatalogItem(kTollHammerItem)) {
            delta += 1;
        }
        if (currentSite == 1 && equippedCatalogItem(kRoadworkerHuskItem)) {
            delta += 1;
        }
    }
    return delta;
}

// Short narrative tags describe why the numbers changed.
void describeActionAbilities(UiAction action, LeadKind lead, char* buffer, size_t bufferSize) {
    buffer[0] = '\0';
    if (action == UiAction::Observe && equippedCatalogItem(kMourningLensItem)) {
        appendAbilityNote(buffer, bufferSize, "The Mourning Lens shows absences as warnings.");
    }
    if (action == UiAction::Observe && equippedCatalogItem(kGlassNeedleItem)) {
        appendAbilityNote(buffer, bufferSize, "The Glass Needle points at the space your eyes keep skipping.");
    }
    if (action == UiAction::Observe && equippedCatalogItem(kDeadPagerItem) && !deadPagerSpent) {
        appendAbilityNote(buffer, bufferSize, "The Dead Pager may turn a bad read into an unwanted appointment.");
    }
    if (action == UiAction::Observe && equippedCatalogItem(kMothCompassItem)) {
        appendAbilityNote(buffer, bufferSize, "The Moth Compass twitches toward profitable rot.");
    }
    if (action == UiAction::Observe && equippedCatalogItem(kSparrowDroneShellItem)) {
        appendAbilityNote(buffer, bufferSize, "The Sparrow Shell marks camera blind spots with one tired eye.");
    }
    if (action == UiAction::Observe && currentSite == 3 && equippedCatalogItem(kCopperToothRadioItem)) {
        appendAbilityNote(buffer, bufferSize, "The Copper Tooth Radio chews two broadcasts down to one usable lead.");
    }
    if (action == UiAction::Explore && equippedCatalogItem(kRoadworkerHuskItem)) {
        appendAbilityNote(buffer, bufferSize, "The Roadworker Husk makes impact feel bureaucratic.");
    }
    if (action == UiAction::Explore && currentSite == 2 && equippedCatalogItem(kDrownedRegisterItem)) {
        appendAbilityNote(buffer, bufferSize, "The Drowned Register insists every Mall find be rung up.");
    }
    if (action == UiAction::FollowLead && lead == LeadKind::Anomaly && equippedCatalogItem(kGlassNeedleItem)) {
        appendAbilityNote(buffer, bufferSize, "The Glass Needle makes the wrong-light profitable.");
    }
    if (action == UiAction::FollowLead && lead == LeadKind::Anomaly && equippedCatalogItem(kColdLanternItem)) {
        appendAbilityNote(buffer, bufferSize, "The Cold Lantern lights the room the world is hiding.");
    }
    if (action == UiAction::FollowLead && lead == LeadKind::Door && equippedCatalogItem(kSolderRigItem)) {
        appendAbilityNote(buffer, bufferSize, "The Solder Rig smells powered locks through the rain.");
    }
    if (action == UiAction::FollowLead && lead == LeadKind::Door && equippedCatalogItem(kRainKeyItem) && !rainKeySpent) {
        appendAbilityNote(buffer, bufferSize, "The Rain Key can cheat one lock, though locks remember.");
    }
    if (action == UiAction::FollowLead && (lead == LeadKind::Door || lead == LeadKind::Cache) &&
        equippedCatalogItem(kMercyBoltCutterItem)) {
        appendAbilityNote(buffer, bufferSize, "The Mercy Bolt Cutter makes late decisions travel through metal.");
    }
    if (action == UiAction::FollowLead && (lead == LeadKind::Door || lead == LeadKind::Cache) &&
        equippedCatalogItem(kPrybarKitItem)) {
        appendAbilityNote(buffer, bufferSize, "The Prybar turns quiet places into negotiations.");
    }
    if (action == UiAction::FollowLead && (lead == LeadKind::Contact || lead == LeadKind::Trail) &&
        equippedCatalogItem(kMirrorweaveItem)) {
        appendAbilityNote(buffer, bufferSize, "Mirrorweave borrows the advert light and gives back a safer silhouette.");
    }
    if (action == UiAction::FollowLead && lead == LeadKind::Trail && currentSite == 4 &&
        equippedCatalogItem(kReedskinMantleItem)) {
        appendAbilityNote(buffer, bufferSize, "The Reedskin Mantle moves in the Verge rhythm.");
    }
    if (action == UiAction::FollowLead && ghostTeaApplies(lead)) {
        appendAbilityNote(buffer, bufferSize, "Ghost Tea makes the site forget where your outline belongs.");
    }
    if (action == UiAction::FollowLead && lead == LeadKind::Trail && equippedCatalogItem(kCoilDetectorItem)) {
        appendAbilityNote(buffer, bufferSize, "The Coil Detector clicks before the road lies.");
    }
    if (action == UiAction::Explore && equippedCatalogItem(kRailPistolItem) && !railPistolSpent) {
        appendAbilityNote(buffer, bufferSize, "The Rail Pistol can still turn a bad push into a loud answer.");
    }
    if (fenceSaltReady && (action == UiAction::Observe || lead == LeadKind::Trail)) {
        appendAbilityNote(buffer, bufferSize, "Fence Runner's Salt keeps heat from finding a grip.");
    }
}

// Picks a site-appropriate lead when observation succeeds.
LeadKind randomLeadForSite(uint8_t siteIndex) {
    if (siteIndex == 1) {
        const LeadKind leads[] = {LeadKind::Contact, LeadKind::Cache, LeadKind::Door, LeadKind::Anomaly};
        return leads[random(0, sizeof(leads) / sizeof(leads[0]))];
    }
    if (siteIndex == 2) {
        const LeadKind leads[] = {LeadKind::Cache, LeadKind::Door, LeadKind::Contact, LeadKind::Anomaly};
        return leads[random(0, sizeof(leads) / sizeof(leads[0]))];
    }
    if (siteIndex == 3) {
        const LeadKind leads[] = {LeadKind::Anomaly, LeadKind::Door, LeadKind::Contact, LeadKind::Trail};
        return leads[random(0, sizeof(leads) / sizeof(leads[0]))];
    }
    if (siteIndex == 4) {
        const LeadKind leads[] = {LeadKind::Anomaly, LeadKind::Trail, LeadKind::Cache, LeadKind::Contact};
        return leads[random(0, sizeof(leads) / sizeof(leads[0]))];
    }

    return LeadKind::None;
}

// Describes the stat check for the field action forecast panel.
const char* actionCheckText(UiAction action) {
    switch (action) {
        case UiAction::Observe:
            return "SCAN+TECH";
        case UiAction::Explore:
            return "GRIT+FILTER";
        case UiAction::FollowLead:
            if ((siteLead[currentSite] == LeadKind::Contact || siteLead[currentSite] == LeadKind::Door) &&
                equippedCatalogItem(kStaticKnifeItem)) {
                return "GHOST+TECH";
            }
            if (siteLead[currentSite] == LeadKind::Door && equippedCatalogItem(kEmptyNameTagItem) &&
                (currentSite == 1 || currentSite == 2)) {
                return "GHOST+TECH";
            }
            if (siteLead[currentSite] == LeadKind::Contact && equippedCatalogItem(kDebtCollectorsCoatItem) &&
                !debtCoatSpent) {
                return "GRIT+GHOST";
            }
            if (siteLead[currentSite] == LeadKind::Door && equippedCatalogItem(kSolderRigItem)) {
                return "TECH+SCAN";
            }
            return leadCheckText(siteLead[currentSite]);
        default:
            return "--";
    }
}

// Returns the compact action label used in the forecast list.
const char* actionLabel(UiAction action) {
    switch (action) {
        case UiAction::Observe:
            return "Observe";
        case UiAction::Explore:
            return "Explore";
        case UiAction::FollowLead:
            return leadVerb(siteLead[currentSite]);
        default:
            return "Move";
    }
}

// Summarizes what a successful action is likely to create.
const char* actionPayoffText(UiAction action) {
    switch (action) {
        case UiAction::Observe:
            return "lead";
        case UiAction::Explore:
            return "encounter";
        case UiAction::FollowLead:
            return leadName(siteLead[currentSite]);
        default:
            return "";
    }
}

// Explains why an unavailable field action is currently blocked.
const char* actionBlockedText(UiAction action) {
    if (currentSite == 0) {
        return "travel";
    }
    if (timeRemainingTicks() < actionTimeCost(action)) {
        return "no light";
    }
    if (action == UiAction::Observe && siteLead[currentSite] != LeadKind::None) {
        return "lead waits";
    }
    if (action == UiAction::Observe && siteIntel[currentSite] >= kMaxSiteIntel) {
        return "mapped";
    }
    if (action == UiAction::FollowLead && siteLead[currentSite] == LeadKind::None) {
        return "no lead";
    }
    if (action == UiAction::Explore && siteCache[currentSite] == 0) {
        return "dry";
    }
    return "";
}

// Selects the stat total used for a field action roll.
int16_t actionSkill(UiAction action, const Stats& stats) {
    const LeadKind lead = siteLead[currentSite];
    int16_t skill = 0;
    switch (action) {
        case UiAction::Observe:
            skill = stats.scan + stats.tech;
            break;
        case UiAction::Explore:
            skill = stats.grit + stats.filter;
            break;
        case UiAction::FollowLead:
            if ((lead == LeadKind::Contact || lead == LeadKind::Door) && equippedCatalogItem(kStaticKnifeItem)) {
                skill = stats.ghost + stats.tech;
            } else if (lead == LeadKind::Door && equippedCatalogItem(kEmptyNameTagItem) &&
                       (currentSite == 1 || currentSite == 2)) {
                skill = stats.ghost + stats.tech;
            } else if (lead == LeadKind::Contact && equippedCatalogItem(kDebtCollectorsCoatItem) && !debtCoatSpent) {
                skill = stats.grit + stats.ghost;
            } else if (lead == LeadKind::Door && equippedCatalogItem(kSolderRigItem)) {
                skill = stats.tech + stats.scan;
            } else {
                skill = leadSkill(lead, stats);
            }
            break;
        default:
            return 0;
    }
    return skill + abilitySkillDelta(action, lead);
}

// Builds the difficulty target from site risk, attention, and action type.
int16_t actionTarget(UiAction action) {
    int16_t target = 4 + effectiveRiskForSite(currentSite);
    if (action == UiAction::Explore) {
        target += 1;
    }
    if (action == UiAction::FollowLead) {
        target += 1;
        if (siteLead[currentSite] == LeadKind::Anomaly || siteLead[currentSite] == LeadKind::Door) {
            target += 1;
        }
        if (siteLead[currentSite] == LeadKind::Trail) {
            target -= 1;
        }
    }
    if (action == UiAction::Explore && siteAttention[currentSite] > 0) {
        target += 1;
    }
    target += abilityTargetDelta(action, siteLead[currentSite]);
    return clampInt(target, 3, 12);
}

// Calculates dose gained by taking a field action.
int16_t actionExposureCost(UiAction action, const Stats& stats) {
    int16_t dose = clampInt((effectiveRiskForSite(currentSite) + stats.strain - stats.filter) / 3, 0, 4);
    if (action == UiAction::Observe) {
        dose = clampInt(dose - 1, 0, 4);
    } else if (action == UiAction::Explore) {
        dose = clampInt(dose + 1, 0, 5);
    } else if (action == UiAction::FollowLead) {
        const LeadKind lead = siteLead[currentSite];
        if (lead == LeadKind::Anomaly) {
            dose = clampInt(dose + 2, 0, 6);
        } else if (lead == LeadKind::Trail || lead == LeadKind::Contact) {
            dose = clampInt(dose - 1, 0, 4);
        }
    }
    return clampInt(dose + abilityDoseDelta(action, siteLead[currentSite]), 0, 6);
}

// Converts a stat-vs-target gap into the percentage shown to the player.
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

// Checks whether a field action is currently meaningful and affordable.
bool fieldActionAvailable(UiAction action) {
    if (currentSite == 0) {
        return false;
    }
    if (timeRemainingTicks() < actionTimeCost(action)) {
        return false;
    }
    if (action == UiAction::Observe) {
        return siteLead[currentSite] == LeadKind::None && siteIntel[currentSite] < kMaxSiteIntel;
    }
    if (action == UiAction::Explore) {
        return siteCache[currentSite] > 0;
    }
    if (action == UiAction::FollowLead) {
        return siteLead[currentSite] != LeadKind::None;
    }
    return true;
}

// Returns whether two sites share a direct authored route on the map.
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

// Converts route availability into travel time.
uint8_t travelTicksToSite(uint8_t targetSite) {
    if (targetSite >= kSiteCount || targetSite == currentSite) {
        return 0;
    }
    return directRoute(currentSite, targetSite) ? 1 : 2;
}

// Estimates route dose based on target danger and the player's current kit.
int16_t routeExposureCost(uint8_t targetSite, const Stats& stats) {
    if (targetSite >= kSiteCount || targetSite == currentSite) {
        return 0;
    }
    if (targetSite == 0) {
        return currentSite == 0 ? 0 : 1;
    }

    const int16_t routeLoad = travelTicksToSite(targetSite) + siteAttention[targetSite] / 2;
    int16_t dose = clampInt((sites[targetSite].risk + routeLoad + stats.strain - stats.filter) / 4, 0, 4);
    if ((targetSite == 4 || currentSite == 4) && equippedCatalogItem(kReedskinMantleItem)) {
        --dose;
    }
    if ((targetSite == 2 || currentSite == 2) &&
        (equippedCatalogItem(kFloodChoirWadersItem) || equippedCatalogItem(kLittleFloodSaintItem))) {
        --dose;
    }
    if (equippedCatalogItem(kRainCounterItem)) {
        --dose;
    }
    if (signalChalkMark[targetSite]) {
        --dose;
    }
    if (fenceSaltReady) {
        --dose;
    }
    return clampInt(dose, 0, 4);
}

// Checks whether the player can reach a destination before the day closes.
bool canTravelToSite(uint8_t targetSite) {
    const uint8_t ticks = travelTicksToSite(targetSite);
    return targetSite < kSiteCount && targetSite != currentSite && ticks > 0 && timeRemainingTicks() >= ticks;
}

// Sets the narrative status line, logs it, and marks the UI for redraw.
void setStatus(const char* text) {
    strncpy(statusLine, text, sizeof(statusLine) - 1);
    statusLine[sizeof(statusLine) - 1] = '\0';
    Serial.println(statusLine);
    screenDirty = true;
}

// Checks for unique gear duplicates before adding rewards.
bool hasCatalogItem(uint8_t itemId) {
    for (uint8_t i = 0; i < kInventoryCapacity; ++i) {
        if (inventory[i] == itemId) {
            return true;
        }
    }
    return false;
}

// Counts valid inventory entries; paging uses this so the pack can grow without
// drawing rows beyond the visible screen.
uint8_t inventoryItemCount() {
    uint8_t count = 0;
    for (uint8_t i = 0; i < kInventoryCapacity; ++i) {
        if (validInventorySlot(i)) {
            ++count;
        }
    }
    return count;
}

// Calculates the last inventory page for the current row count.
uint8_t maxInventoryPage(uint8_t rowsPerPage) {
    const uint8_t rows = rowsPerPage == 0 ? 1 : rowsPerPage;
    const uint8_t count = inventoryItemCount();
    return count == 0 ? 0 : static_cast<uint8_t>((count - 1) / rows);
}

// Keeps the saved inventory page valid after item use or screen-size changes.
void normalizeInventoryPage(uint8_t rowsPerPage) {
    const uint8_t maxPage = maxInventoryPage(rowsPerPage);
    if (inventoryPage > maxPage) {
        inventoryPage = maxPage;
    }
}

// Finds the next unused backing slot in the current pack storage.
int16_t firstEmptyInventorySlot() {
    for (uint8_t i = 0; i < kInventoryCapacity; ++i) {
        if (inventory[i] < 0) {
            return i;
        }
    }
    return -1;
}

// Adds an item to the pack. The design has no intentional pack-size limit; the
// fixed backing store is only there because this target avoids dynamic memory.
bool addItem(uint8_t itemId, char* message, size_t messageSize) {
    const int16_t slot = firstEmptyInventorySlot();
    if (slot < 0) {
        snprintf(message, messageSize, "Found %s, but the pack index is full in this build.", itemCatalog[itemId].name);
        return false;
    }
    inventory[slot] = itemId;
    snprintf(message, messageSize, "Found %s. It is now in your pack.", itemCatalog[itemId].name);
    return true;
}

// Clears the last reward list before resolving a new action or trade.
void clearReward() {
    rewardCount = 0;
    rewardTitle[0] = '\0';
    rewardSummary[0] = '\0';
}

// Records a received item so the reward screen can show it with an icon.
void rememberRewardItem(uint8_t itemId) {
    if (rewardCount < kMaxRewardItems) {
        rewardItems[rewardCount++] = itemId;
    }
}

// Adds an item to the pack and also records it for the reward summary screen.
bool grantRewardItem(uint8_t itemId) {
    char ignored[96];
    const bool added = addItem(itemId, ignored, sizeof(ignored));
    if (added) {
        rememberRewardItem(itemId);
    }
    return added;
}

// Trade rewards are ordinary usable items, weighted by the value being granted.
uint8_t randomTradeGoodForValue(uint8_t targetValue) {
    if (targetValue >= 7 && random(0, 100) < 35) {
        return kMedPackItem;
    }
    if (targetValue >= 6 && random(0, 100) < 30) {
        return random(0, 100) < 50 ? kBlackIodineStripItem : kBlueMilkSachetItem;
    }
    if (targetValue >= 5 && random(0, 100) < 35) {
        return random(0, 100) < 50 ? 12 : kGhostTeaAmpouleItem;
    }
    if (targetValue >= 4 && random(0, 100) < 50) {
        return kCleanWaterItem;
    }
    if (targetValue >= 4 && random(0, 100) < 35) {
        return 13;
    }
    return kBatteryCellItem;
}

// Converts abstract reward value into concrete barter goods in the inventory.
void grantTradeGoods(uint8_t targetValue) {
    uint8_t remaining = targetValue;
    while (remaining > 0 && rewardCount < kMaxRewardItems) {
        const uint8_t itemId = randomTradeGoodForValue(remaining);
        if (!grantRewardItem(itemId)) {
            return;
        }
        const uint8_t value = itemCatalog[itemId].value;
        remaining = value >= remaining ? 0 : static_cast<uint8_t>(remaining - value);
    }
}

// Pays a value demand by consuming unequipped items, preferring small goods.
int16_t payTradeValue(int16_t requiredValue) {
    int16_t paid = 0;
    while (paid < requiredValue) {
        int16_t bestSlot = -1;
        uint8_t bestValue = 255;
        for (uint8_t i = 0; i < kInventoryCapacity; ++i) {
            if (!validInventorySlot(i) || isEquippedInventorySlot(i)) {
                continue;
            }
            const uint8_t value = itemCatalog[inventory[i]].value;
            if (value > 0 && value < bestValue) {
                bestValue = value;
                bestSlot = i;
            }
        }
        if (bestSlot < 0) {
            break;
        }
        paid += itemCatalog[inventory[bestSlot]].value;
        removeInventorySlot(static_cast<uint8_t>(bestSlot));
    }
    return paid;
}

// Clears current trade selections and starts the trade window from page one.
void resetTradeSelections() {
    tradePage = 0;
    for (uint8_t i = 0; i < kInventoryCapacity; ++i) {
        tradeOfferSelected[i] = false;
    }
    for (uint8_t i = 0; i < kTradeStockCount; ++i) {
        tradeWantSelected[i] = false;
    }
}

// Totals the value the player is offering in the current trade.
int16_t tradeOfferValue() {
    int16_t total = 0;
    for (uint8_t i = 0; i < kInventoryCapacity; ++i) {
        if (tradeOfferSelected[i] && validInventorySlot(i)) {
            total += itemCatalog[inventory[i]].value;
        }
    }
    return total;
}

// Totals the value requested from the trader.
int16_t tradeAskValue() {
    int16_t total = 0;
    for (uint8_t i = 0; i < kTradeStockCount; ++i) {
        if (tradeWantSelected[i]) {
            total += itemCatalog[tradeStock[i]].value;
        }
    }
    return total;
}

bool tradeAvailableHere() {
    return currentSite == 0 || currentSite == 1;
}

// Applies an inventory item: consumables resolve immediately, gear equips.
void equipInventorySlot(uint8_t invSlot) {
    if (!validInventorySlot(invSlot)) {
        return;
    }
    const uint8_t itemId = static_cast<uint8_t>(inventory[invSlot]);
    const Item& item = itemCatalog[itemId];
    if (item.use.kind == ItemUseKind::Consume) {
        health = clampInt(health + item.use.healthDelta, 0, kMaxHealth);
        exposure = clampInt(exposure + item.use.exposureDelta, 0, kMaxExposure);
        if (item.use.attentionDelta != 0) {
            siteAttention[currentSite] = clampInt(siteAttention[currentSite] + item.use.attentionDelta, 0, kMaxSiteAttention);
        }
        if (itemId == kGhostTeaAmpouleItem) {
            ghostTeaSite = currentSite;
        } else if (itemId == kBlackIodineStripItem) {
            blackIodineGuard = true;
        } else if (itemId == kSleeplessMintItem) {
            sleeplessMintReady = true;
        } else if (itemId == kFenceRunnersSaltItem) {
            fenceSaltReady = true;
        } else if (itemId == kCheapCourageSyrupItem) {
            cheapCourageReady = true;
        } else if (itemId == kBlueMilkSachetItem) {
            blueMilkBlurReady = true;
        }
        if (item.use.consumedOnUse) {
            removeInventorySlot(invSlot);
        }
        setStatus(item.use.resultText);
        return;
    }

    if (item.use.kind == ItemUseKind::Equip) {
        const int8_t equipSlot = equipIndexForSlot(item.slot);
        if (equipSlot >= 0) {
            equipped[equipSlot] = invSlot;
            setStatus(item.use.resultText);
            return;
        }
    }

    setStatus("You turn the item over in your hands. It has no direct use yet.");
}

// Registers an on-screen button for both drawing and touch handling.
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

// Clears the transient button list before redrawing a screen.
void clearButtons() {
    buttonCount = 0;
}

// Performs a rectangular hit test for a touch point.
bool hitButton(const Button& button, int32_t x, int32_t y) {
    return button.enabled && x >= button.x && x < button.x + button.w && y >= button.y && y < button.y + button.h;
}

// Copies text into a buffer and shortens it until it fits the current font.
void makeTextFit(const char* text, int32_t maxWidth, char* output, size_t outputSize) {
    if (outputSize == 0) {
        return;
    }
    output[0] = '\0';
    if (maxWidth <= 0) {
        return;
    }

    auto& display = M5.Display;
    snprintf(output, outputSize, "%s", text == nullptr ? "" : text);
    if (display.textWidth(output) <= maxWidth) {
        return;
    }

    const char* suffix = "...";
    const int32_t suffixWidth = display.textWidth(suffix);
    if (suffixWidth > maxWidth) {
        output[0] = '\0';
        return;
    }

    size_t len = strlen(output);
    while (len > 0) {
        output[--len] = '\0';
        char candidate[160];
        snprintf(candidate, sizeof(candidate), "%s%s", output, suffix);
        if (display.textWidth(candidate) <= maxWidth) {
            snprintf(output, outputSize, "%s", candidate);
            return;
        }
    }

    snprintf(output, outputSize, "%s", suffix);
}

// Draws a single line that is guaranteed not to overrun its allotted width.
void drawTextFit(const char* text, int32_t x, int32_t y, int32_t maxWidth, uint16_t color, uint16_t background) {
    char fitted[160];
    makeTextFit(text, maxWidth, fitted, sizeof(fitted));
    M5.Display.setTextDatum(textdatum_t::top_left);
    M5.Display.setTextColor(color, background);
    M5.Display.drawString(fitted, x, y);
}

// Formats and draws one fitted line, replacing raw printf calls in tight areas.
void drawFormattedTextFit(int32_t x, int32_t y, int32_t maxWidth, uint16_t color, uint16_t background,
                          const char* format, ...) {
    char buffer[160];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    drawTextFit(buffer, x, y, maxWidth, color, background);
}

// Draws a registered button, including disabled styling.
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
    char fitted[48];
    makeTextFit(button.label, button.w - 16, fitted, sizeof(fitted));
    display.drawString(fitted, button.x + button.w / 2, button.y + button.h / 2);
    display.setTextDatum(textdatum_t::top_left);
}

// Draws short prose blocks without letting long text spill out of panels.
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
            drawTextFit(line, x, y + lineCount * lineHeight, w, color, background);
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
        drawTextFit(line, x, y + lineCount * lineHeight, w, color, background);
    }
}

// Draws a labeled meter for body, exposure, and clock pressure.
void drawMeter(int32_t x, int32_t y, int32_t w, int32_t h, int value, int maxValue, uint16_t color, const char* label) {
    auto& display = M5.Display;
    const uint16_t bg = rgb(6, 10, 14);
    const uint16_t rail = rgb(40, 46, 50);
    const int32_t filled = (w - 4) * clampInt(value, 0, maxValue) / maxValue;

    display.setFont(&fonts::Font2);
    drawTextFit(label, x, y - 22, w, rgb(180, 190, 190), bg);
    display.fillRect(x, y, w, h, bg);
    display.drawRect(x, y, w, h, rail);
    display.fillRect(x + 2, y + 2, filled, h - 4, color);
}

// Draws the shared dark panel style used throughout the interface.
void drawPanel(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t border) {
    auto& display = M5.Display;
    display.fillRoundRect(x, y, w, h, 8, rgb(8, 12, 17));
    display.drawRoundRect(x, y, w, h, 8, border);
}

// Draws the persistent top bar with time, carried value, risk, cache, and attention.
void drawHeader() {
    auto& display = M5.Display;
    const int32_t width = display.width();
    const uint16_t bg = rgb(7, 10, 14);

    display.fillRect(0, 0, width, 64, bg);
    display.drawFastHLine(0, 63, width, rgb(70, 220, 170));
    display.setTextDatum(textdatum_t::top_left);
    display.setFont(&fonts::Font4);
    const int32_t headerInfoX = width > 560 ? width - 430 : 18;
    if (width <= 560) {
        drawTextFit("Neon Exclusion", 18, 8, width - 36, TFT_WHITE, bg);
        display.setFont(&fonts::Font2);
        drawFormattedTextFit(18, 40, width - 36, rgb(180, 190, 190), bg, "D%u %02u:00 value %d risk %d",
                             static_cast<unsigned>(day), static_cast<unsigned>(currentHour()), carriedTradeValue(),
                             effectiveRiskForSite(currentSite));
        return;
    }
    drawTextFit("Neon Exclusion", 18, 12, headerInfoX - 36, TFT_WHITE, bg);

    display.setFont(&fonts::Font2);
    const int32_t headerInfoW = width - headerInfoX - 18;
    drawFormattedTextFit(headerInfoX, 16, headerInfoW, rgb(180, 190, 190), bg,
                         "Day %u  %02u:00  value %d  dusk in %uh", static_cast<unsigned>(day),
                         static_cast<unsigned>(currentHour()), carriedTradeValue(),
                         static_cast<unsigned>(timeRemainingTicks() * kTickHours));
    drawFormattedTextFit(headerInfoX, 38, headerInfoW, rgb(180, 190, 190), bg,
                         "bill %d value  risk %d  cache %u  attention %u", dailyUpkeepValue(),
                         effectiveRiskForSite(currentSite), siteCache[currentSite], siteAttention[currentSite]);
}

// Draws the runner condition meters and contextualized stats.
void drawStatsPanel(int32_t x, int32_t y, int32_t w, int32_t h) {
    auto& display = M5.Display;
    const uint16_t bg = rgb(8, 12, 17);
    const Stats stats = deriveStats();

    drawPanel(x, y, w, h, rgb(48, 120, 130));
    display.setFont(&fonts::Font4);
    drawTextFit("Runner", x + 14, y + 12, w - 28, TFT_WHITE, bg);

    drawMeter(x + 16, y + 56, w - 32, 14, health, kMaxHealth, rgb(230, 80, 90), "Body");
    drawMeter(x + 16, y + 102, w - 32, 14, exposure, kMaxExposure, rgb(170, 230, 80), "Exposure");
    drawMeter(x + 16, y + 148, w - 32, 14, timeTick, maxTimeTicksForDay(), rgb(230, 180, 70), "Clock");

    const int32_t statY = y + 180;
    display.setFont(&fonts::Font2);
    drawFormattedTextFit(x + 18, statY, w - 36, rgb(220, 230, 225), bg, "GRIT breach  %d", stats.grit);
    drawFormattedTextFit(x + 18, statY + 22, w - 36, rgb(220, 230, 225), bg, "TECH bypass  %d", stats.tech);
    drawFormattedTextFit(x + 18, statY + 44, w - 36, rgb(220, 230, 225), bg, "SCAN anomaly %d", stats.scan);
    drawFormattedTextFit(x + 18, statY + 66, w - 36, rgb(220, 230, 225), bg, "GHOST notice %d", stats.ghost);
    drawFormattedTextFit(x + 18, statY + 88, w - 36, rgb(220, 230, 225), bg, "FILTER dose  %d", stats.filter);
    drawFormattedTextFit(x + 18, statY + 110, w - 36, rgb(220, 230, 225), bg, "STRAIN weird %d", stats.strain);
}

// Draws the currently equipped gear by slot.
void drawEquippedList(int32_t x, int32_t y, int32_t w, int32_t h) {
    auto& display = M5.Display;
    const uint16_t bg = rgb(8, 12, 17);
    const char* labels[kEquipSlotCount] = {"Suit", "Detector", "Tool", "Weapon", "Artifact"};

    drawPanel(x, y, w, h, rgb(130, 86, 170));
    display.setFont(&fonts::Font4);
    drawTextFit("Loadout", x + 14, y + 12, w - 28, TFT_WHITE, bg);

    display.setFont(&fonts::Font2);
    for (uint8_t i = 0; i < kEquipSlotCount; ++i) {
        const int32_t rowY = y + 60 + i * 50;
        drawTextFit(labels[i], x + 16, rowY, w - 32, rgb(150, 170, 175), bg);
        const int16_t invSlot = equipped[i];
        const char* name = validInventorySlot(invSlot) ? itemCatalog[inventory[invSlot]].name : "empty";
        drawTextFit(name, x + 16, rowY + 22, w - 32, TFT_WHITE, bg);
    }
}

// Draws a compact pack preview on the field screen.
void drawPackPreview(int32_t x, int32_t y, int32_t w, int32_t h) {
    auto& display = M5.Display;
    const uint16_t bg = rgb(8, 12, 17);

    drawPanel(x, y, w, h, rgb(190, 70, 150));
    display.setFont(&fonts::Font4);
    drawTextFit("Pack", x + 14, y + 12, w - 28, TFT_WHITE, bg);

    display.setFont(&fonts::Font2);
    uint8_t drawn = 0;
    for (uint8_t i = 0; i < kInventoryCapacity && drawn < 7; ++i) {
        if (!validInventorySlot(i)) {
            continue;
        }
        const Item& item = itemCatalog[inventory[i]];
        const int32_t rowY = y + 58 + drawn * 38;
        display.fillRect(x + 14, rowY + 4, 8, 18, item.color);
        drawTextFit(item.name, x + 30, rowY, w - 44, TFT_WHITE, bg);
        drawTextFit(item.tag, x + 30, rowY + 18, w - 44, rgb(135, 150, 150), bg);
        ++drawn;
    }
}

// Draws action outcomes before the player commits time and risk.
void drawActionForecast(int32_t x, int32_t y, int32_t w, int32_t h) {
    auto& display = M5.Display;
    const uint16_t bg = rgb(8, 12, 17);
    const Stats stats = deriveStats();
    const UiAction actions[] = {UiAction::Observe, UiAction::Explore, UiAction::FollowLead};

    drawPanel(x, y, w, h, rgb(190, 70, 150));
    display.setFont(&fonts::Font4);
    drawTextFit("Field Read", x + 14, y + 12, w - 28, TFT_WHITE, bg);

    display.setFont(&fonts::Font2);
    drawFormattedTextFit(x + 16, y + 48, w - 32, rgb(150, 168, 170), bg,
                         "cache %u/%u  intel %u/%u  attention %u/%u", siteCache[currentSite],
                         sites[currentSite].maxCache, siteIntel[currentSite], kMaxSiteIntel,
                         siteAttention[currentSite], kMaxSiteAttention);
    drawFormattedTextFit(x + 16, y + 66, w - 32, rgb(150, 168, 170), bg, "lead: %s", leadName(siteLead[currentSite]));

    for (uint8_t i = 0; i < 3; ++i) {
        const UiAction action = actions[i];
        const int32_t rowY = y + 94 + i * 64;
        const bool enabled = fieldActionAvailable(action);
        const uint16_t rowBg = enabled ? rgb(12, 18, 24) : rgb(16, 16, 18);
        const uint16_t border = enabled ? rgb(90, 210, 220) : rgb(58, 58, 62);
        const int16_t skill = actionSkill(action, stats);
        const int16_t target = actionTarget(action);
        const uint8_t chance = successChance(skill, target);

        display.fillRoundRect(x + 14, rowY, w - 28, 58, 6, rowBg);
        display.drawRoundRect(x + 14, rowY, w - 28, 58, 6, border);
        drawTextFit(actionLabel(action), x + 26, rowY + 8, 84, enabled ? TFT_WHITE : rgb(110, 115, 120), rowBg);
        drawFormattedTextFit(x + 118, rowY + 8, w - 144, enabled ? rgb(180, 210, 205) : rgb(95, 100, 102), rowBg,
                             "%s %d/%d", actionCheckText(action), skill, target);
        drawFormattedTextFit(x + 26, rowY + 30, 132, enabled ? rgb(180, 210, 205) : rgb(95, 100, 102), rowBg,
                             "%u%%  %uh  dose +%d", chance,
                             static_cast<unsigned>(actionTimeCost(action) * kTickHours),
                             actionExposureCost(action, stats));
        drawTextFit(enabled ? actionPayoffText(action) : actionBlockedText(action), x + 168, rowY + 30, w - 196,
                    enabled ? rgb(180, 210, 205) : rgb(95, 100, 102), rowBg);
    }
}

// Chooses loot from action-specific pools with risk-weighted rare finds.
uint8_t randomLootForAction(UiAction action) {
    const uint8_t risk = static_cast<uint8_t>(effectiveRiskForSite(currentSite));
    const bool rare = random(0, 100) < (18 + risk * 5);
    if (rare && risk >= 4) {
        const uint8_t artifacts[] = {9,
                                     10,
                                     11,
                                     14,
                                     kWhiteReceiptItem,
                                     kSpareHourItem,
                                     kLittleFloodSaintItem,
                                     kApologyEngineItem,
                                     kEmptyNameTagItem,
                                     kBlackReedSeedItem,
                                     kBorrowedShadowItem,
                                     kCopperToothRadioItem,
                                     kMercyBellItem,
                                     kCalendarOfRainsItem,
                                     kRedactedPhotographItem,
                                     kTenthButtonItem};
        return artifacts[random(0, sizeof(artifacts) / sizeof(artifacts[0]))];
    }

    if (action == UiAction::Observe) {
        if (currentSite == 3) {
            const uint8_t relayObserve[] = {kDeadPagerItem, kMothCompassItem, kBoneTuningForkItem,
                                            kSparrowDroneShellItem, kCopperToothRadioItem, kBatteryCellItem};
            return relayObserve[random(0, sizeof(relayObserve) / sizeof(relayObserve[0]))];
        }
        if (currentSite == 4) {
            const uint8_t vergeObserve[] = {kReedskinMantleItem, kGlasshouseSmockItem, kBoneTuningForkItem,
                                            kBlackReedSeedItem, kBorrowedShadowItem, kGhostTeaAmpouleItem};
            return vergeObserve[random(0, sizeof(vergeObserve) / sizeof(vergeObserve[0]))];
        }
        const uint8_t observeLoot[] = {4, 10, 11, 12, kRainCounterItem, kKindlingScopeItem, kGlassLungMeterItem,
                                       kSignalChalkItem, kWhiteReceiptItem};
        return observeLoot[random(0, sizeof(observeLoot) / sizeof(observeLoot[0]))];
    }
    if (action == UiAction::FollowLead) {
        const LeadKind lead = siteLead[currentSite];
        if (lead == LeadKind::Contact || lead == LeadKind::Trail) {
            const uint8_t quietLoot[] = {2, 7, 11, 13, kStaticBridalVeilItem, kDebtCollectorsCoatItem,
                                         kDeadPagerItem, kFlareHookItem, kStaticKnifeItem, kMercySirenItem,
                                         kCivicBatonItem, kGhostTeaAmpouleItem, kFenceRunnersSaltItem};
            return quietLoot[random(0, sizeof(quietLoot) / sizeof(quietLoot[0]))];
        }
        if (lead == LeadKind::Anomaly) {
            const uint8_t anomalyLoot[] = {9, 10, 11, 14, kMothCompassItem, kBoneTuningForkItem, kColdLanternItem,
                                           kWhiteReceiptItem, kApologyEngineItem, kBorrowedShadowItem,
                                           kCalendarOfRainsItem, kBlackIodineStripItem, kBlueMilkSachetItem};
            return anomalyLoot[random(0, sizeof(anomalyLoot) / sizeof(anomalyLoot[0]))];
        }
        if (lead == LeadKind::Door || lead == LeadKind::Cache) {
            const uint8_t hardLoot[] = {1, 5, 6, 8, kRoadworkerHuskItem, kFloodChoirWadersItem, kRainKeyItem,
                                        kDrownedRegisterItem, kServiceWormItem, kMercyBoltCutterItem,
                                        kPulseStaplerItem, kValveRosaryItem, kMapScalpelItem, kTenthButtonItem};
            return hardLoot[random(0, sizeof(hardLoot) / sizeof(hardLoot[0]))];
        }
    }

    if (currentSite == 1) {
        const uint8_t spillwayLoot[] = {2, 7, 8, 13, kStaticBridalVeilItem, kDebtCollectorsCoatItem,
                                        kKindlingScopeItem, kTollHammerItem, kMercySirenItem, kCivicBatonItem,
                                        kRedactedPhotographItem, kFenceRunnersSaltItem};
        return spillwayLoot[random(0, sizeof(spillwayLoot) / sizeof(spillwayLoot[0]))];
    }
    if (currentSite == 2) {
        const uint8_t mallLoot[] = {1, 6, 12, kHospitalVinylApronItem, kFloodChoirWadersItem, kCheckoutOracleItem,
                                    kDrownedRegisterItem, kTarTapeRollItem, kValveRosaryItem,
                                    kLittleFloodSaintItem, kTenthButtonItem, kSleeplessMintItem};
        return mallLoot[random(0, sizeof(mallLoot) / sizeof(mallLoot[0]))];
    }
    const uint8_t exploreLoot[] = {1, 6, 8, 9, 12, 13, kAshfallPonchoItem, kRainCounterItem, kRoadworkerHuskItem,
                                   kServiceWormItem, kRivetSaintItem, kCheapCourageSyrupItem};
    return exploreLoot[random(0, sizeof(exploreLoot) / sizeof(exploreLoot[0]))];
}

// Calculates how much attention a field action adds to the current site.
uint8_t actionAttentionGain(UiAction action, bool success) {
    int16_t gain = 0;
    switch (action) {
        case UiAction::Observe:
            gain = success ? 0 : 1;
            break;
        case UiAction::Explore:
            gain = success ? 3 : 4;
            break;
        case UiAction::FollowLead: {
            const LeadKind lead = siteLead[currentSite];
            if (lead == LeadKind::Contact || lead == LeadKind::Trail) {
                gain = success ? 0 : 2;
                break;
            }
            if (lead == LeadKind::Anomaly) {
                gain = success ? 1 : 3;
                break;
            }
            gain = success ? 2 : 3;
            break;
        }
        default:
            return 0;
    }
    return static_cast<uint8_t>(clampInt(gain + abilityAttentionDelta(action, siteLead[currentSite], success), 0,
                                         kMaxSiteAttention));
}

// Lowers attention, fades intel, clears leads, and slowly restocks sites overnight.
void coolSitesForNewDay() {
    for (uint8_t i = 1; i < kSiteCount; ++i) {
        siteAttention[i] = siteAttention[i] > 2 ? static_cast<uint8_t>(siteAttention[i] - 2) : 0;
        siteIntel[i] = siteIntel[i] > 0 ? static_cast<uint8_t>(siteIntel[i] - 1) : 0;
        siteLead[i] = LeadKind::None;
        if (siteCache[i] < sites[i].maxCache) {
            ++siteCache[i];
        }
    }
}

int16_t dailyUpkeepValue() {
    int16_t bill = kDailyUpkeepValue;
    if (equippedCatalogItem(kDebtCollectorsCoatItem)) {
        ++bill;
    }
    if (hasCatalogItem(kLittleFloodSaintItem)) {
        ++bill;
    }
    if (storyOutcome[storyIndex(StoryArc::PersonWhoNeverEntered)] == 2) {
        bill = clampInt(bill - 2, 1, 12);
    }
    return bill;
}

bool otherRunnerPaysDawnBill() {
    const Stats stats = deriveStats();
    const uint8_t arc = storyIndex(StoryArc::PersonWhoNeverEntered);
    if (storyOutcome[arc] == 1 && random(0, 100) < 45) {
        return true;
    }
    if (storyStage[arc] == 0 &&
        (stats.ghost >= 4 || equippedCatalogItem(kNullCharmItem) || equippedCatalogItem(kBorrowedShadowItem) ||
         equippedCatalogItem(kMirrorweaveItem))) {
        storyStage[arc] = 1;
        return true;
    }
    return false;
}

// Advances dawn, applies clinic upkeep in barter goods, and returns to berth.
void startNewDay(const char* lead) {
    ++day;
    timeTick = 0;
    currentSite = 0;
    coolSitesForNewDay();
    warmBatterySpent = false;
    railPistolSpent = false;
    quietNailgunSpent = false;
    mourningLensSpent = false;
    copperSaintSpent = false;
    staticVeilSpent = false;
    debtCoatSpent = false;
    deadPagerSpent = false;
    rainKeySpent = false;
    flareHookSpent = false;
    mercySirenSpent = false;
    apologyEngineSpent = false;
    borrowedShadowSpent = false;
    mercyBellSpent = false;
    calendarSpent = false;
    ghostTeaSite = 255;
    blackIodineGuard = false;
    sleeplessMintReady = false;
    fenceSaltReady = false;
    cheapCourageReady = false;
    blueMilkBlurReady = false;
    for (uint8_t i = 0; i < kSiteCapacity; ++i) {
        nullCharmSpent[i] = false;
        ashfallPonchoSpent[i] = false;
        tarTapeSpent[i] = false;
    }

    char bill[96];
    const int16_t billValue = dailyUpkeepValue();
    const bool otherPaid = otherRunnerPaysDawnBill();
    if (otherPaid) {
        snprintf(bill, sizeof(bill),
                 "Clinic meter already green. The orderly says you paid an hour ago with dry boots and a worse smile.");
    } else {
        const int16_t paid = payTradeValue(billValue);
        if (paid >= billValue) {
            snprintf(bill, sizeof(bill), "Clinic meter paid with goods worth %d.", paid);
        } else {
            const int16_t shortfall = billValue - paid;
            health = clampInt(health - shortfall, 0, kMaxHealth);
            exposure = clampInt(exposure + shortfall, 0, kMaxExposure);
            snprintf(bill, sizeof(bill), "Clinic bill short by %d value. Debt collectors take it out of your body.",
                     shortfall);
        }
    }

    char message[320];
    snprintf(message, sizeof(message), "%s %s", lead, bill);
    if (equippedCatalogItem(kAshfallPonchoItem) && random(0, 100) < 30) {
        for (uint8_t i = 1; i < kSiteCount; ++i) {
            if (siteIntel[i] > 0) {
                --siteIntel[i];
                appendAbilityNote(message, sizeof(message), "Ashfall blurs one map note overnight.");
                break;
            }
        }
    }
    if (equippedCatalogItem(kSpareHourItem) && random(0, 100) < 35) {
        const uint8_t site = random(1, kSiteCount);
        siteLead[site] = randomLeadForSite(site);
        appendAbilityNote(message, sizeof(message), "The Spare Hour mutates a lead while nobody owns the clock.");
    }
    setStatus(message);
}

// Spends action time and resolves forced overnight consequences at curfew.
void spendTime(uint8_t ticks) {
    timeTick = static_cast<uint8_t>(timeTick + ticks);
    if (timeTick < maxTimeTicksForDay()) {
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

// Handles the clinic recovery loop after the runner hits a fail state.
void checkCollapse() {
    if (health > 0 && exposure < kMaxExposure) {
        return;
    }

    if (equippedCatalogItem(kMercyBellItem) && !mercyBellSpent) {
        mercyBellSpent = true;
        health = health <= 0 ? 1 : health;
        exposure = clampInt(exposure, 0, kMaxExposure - 1);
        siteAttention[currentSite] = clampInt(siteAttention[currentSite] + 2, 0, kMaxSiteAttention);
        if (siteCache[currentSite] > 0) {
            --siteCache[currentSite];
        }
        currentSite = 0;
        setStatus("The Mercy Bell rings without a clapper. You collapse at the clinic door and the site loses a cache.");
        return;
    }

    if (equippedCatalogItem(kCopperSaintItem) && !copperSaintSpent) {
        copperSaintSpent = true;
        health = health <= 0 ? 1 : health;
        exposure = clampInt(exposure, 0, kMaxExposure - 1);
        siteAttention[currentSite] = clampInt(siteAttention[currentSite] + 2, 0, kMaxSiteAttention);
        setStatus("The Copper Saint burns hot enough to hurt. You stay standing at 1 body, and the district notices.");
        return;
    }

    currentSite = 0;
    timeTick = 0;
    coolSitesForNewDay();
    day += 1;
    health = 5;
    exposure = 3;
    const int16_t paid = payTradeValue(6);
    char message[180];
    snprintf(message, sizeof(message),
             "You wake under clinic lights with your boots still wet. Collapse care took goods worth %d and a little pride.",
             paid);
    setStatus(message);
}

void advanceBatteriesForDead(UiAction action, OutcomeLevel outcome, LeadKind lead, char* message, size_t messageSize) {
    const uint8_t arc = storyIndex(StoryArc::BatteriesForTheDead);
    if (!outcomeHasReward(outcome)) {
        return;
    }
    if (storyStage[arc] == 1 && currentSite == 1 && action == UiAction::FollowLead && lead == LeadKind::Contact) {
        storyStage[arc] = 2;
        storyOutcome[arc] = 3;
        grantTradeGoods(10);
        siteAttention[3] = clampInt(siteAttention[3] + 2, 0, kMaxSiteAttention);
        appendAbilityNote(message, messageSize,
                          "You sell the frequency. By morning the market rents little radios full of last words.");
        return;
    }
    if (currentSite != 3) {
        return;
    }
    if (storyStage[arc] == 0 &&
        ((action == UiAction::Observe && siteLead[currentSite] == LeadKind::Contact) ||
         (action == UiAction::FollowLead && lead == LeadKind::Contact) || equippedCatalogItem(kDeadPagerItem) ||
         equippedCatalogItem(kCopperToothRadioItem) || equippedCatalogItem(kWarmBatteryItem))) {
        storyStage[arc] = 1;
        appendAbilityNote(message, messageSize,
                          "Batteries for the Dead begins: Sister Clamp asks if you brought power for Station Mercy.");
        return;
    }
    if (storyStage[arc] == 1 && action == UiAction::FollowLead &&
        (lead == LeadKind::Door || lead == LeadKind::Anomaly)) {
        storyStage[arc] = 2;
        if (equippedCatalogItem(kRailPistolItem)) {
            storyOutcome[arc] = 2;
            siteAttention[3] = siteAttention[3] > 2 ? static_cast<uint8_t>(siteAttention[3] - 2) : 0;
            exposure = clampInt(exposure - 1, 0, kMaxExposure);
            appendAbilityNote(message, messageSize,
                              "You silence Station Mercy. Relay Grave is only wind and metal for one honest minute.");
        } else if (equippedCatalogItem(kCopperToothRadioItem) || equippedCatalogItem(kDeadPagerItem)) {
            storyOutcome[arc] = 4;
            if (!hasCatalogItem(kCopperToothRadioItem)) {
                grantRewardItem(kCopperToothRadioItem);
            }
            appendAbilityNote(message, messageSize,
                              "You keep one voice. It moves into your pocket and says it is not lonely too quickly.");
        } else {
            storyOutcome[arc] = 1;
            siteIntel[3] = kMaxSiteIntel;
            appendAbilityNote(message, messageSize,
                              "You preserve the Choir. First Relay observe each day now feels like it knows a path.");
        }
    }
}

void advanceLastSaleAtB2(UiAction action, OutcomeLevel outcome, LeadKind lead, char* message, size_t messageSize) {
    const uint8_t arc = storyIndex(StoryArc::LastSaleAtB2);
    if (currentSite != 2 || !outcomeHasReward(outcome)) {
        return;
    }
    if (storyStage[arc] == 0 && (action == UiAction::Explore || lead == LeadKind::Door)) {
        storyStage[arc] = 1;
        appendAbilityNote(message, messageSize,
                          "The Last Sale at Level B2 begins: the PA reserves something for your account below the water.");
        return;
    }
    if (storyStage[arc] == 1 && action == UiAction::FollowLead &&
        (lead == LeadKind::Door || lead == LeadKind::Cache || lead == LeadKind::Anomaly)) {
        storyStage[arc] = 2;
        if (equippedCatalogItem(kRailPistolItem) || equippedCatalogItem(kSolderRigItem) ||
            equippedCatalogItem(kColdLanternItem)) {
            storyOutcome[arc] = 4;
            siteAttention[2] = siteAttention[2] > 3 ? static_cast<uint8_t>(siteAttention[2] - 3) : 0;
            grantTradeGoods(8);
            appendAbilityNote(message, messageSize,
                              "You shut down the Manager Below. Milo cries because the static is finally honest.");
        } else if (equippedCatalogItem(kDrownedRegisterItem) || equippedCatalogItem(kCheckoutOracleItem) ||
                   equippedCatalogItem(kEmptyNameTagItem)) {
            storyOutcome[arc] = 3;
            siteCache[2] = clampInt(siteCache[2] + 2, 0, sites[2].maxCache);
            siteAttention[2] = clampInt(siteAttention[2] + 2, 0, kMaxSiteAttention);
            appendAbilityNote(message, messageSize,
                              "You feed the Manager Below. Mall cache improves, and hunger gets a loyalty card.");
        } else if (equippedCatalogItem(kWhiteReceiptItem) || equippedCatalogItem(kTenthButtonItem)) {
            storyOutcome[arc] = 2;
            if (!hasCatalogItem(kTenthButtonItem)) {
                grantRewardItem(kTenthButtonItem);
            }
            exposure = clampInt(exposure + 1, 0, kMaxExposure);
            appendAbilityNote(message, messageSize,
                              "You pay with a memory. The speakers thank you in a voice that used to be warm.");
        } else {
            storyOutcome[arc] = 1;
            siteIntel[2] = kMaxSiteIntel;
            if (!hasCatalogItem(kMedPackItem)) {
                grantRewardItem(kMedPackItem);
            }
            appendAbilityNote(message, messageSize,
                              "You save Hessa's family cleanly. The Mall announces a closing time and means mercy.");
        }
    }
}

void advancePersonWhoNeverEntered(UiAction action, OutcomeLevel outcome, LeadKind lead, char* message,
                                  size_t messageSize) {
    const uint8_t arc = storyIndex(StoryArc::PersonWhoNeverEntered);
    if (!outcomeHasReward(outcome)) {
        return;
    }
    if (storyStage[arc] == 1 && currentSite == 1 &&
        (action == UiAction::Observe || lead == LeadKind::Contact || lead == LeadKind::Door)) {
        storyStage[arc] = 2;
        appendAbilityNote(message, messageSize,
                          "The Person Who Never Entered continues: Mink says your face has traffic.");
        return;
    }
    if (storyStage[arc] == 2 && currentSite == 4 &&
        (lead == LeadKind::Trail || lead == LeadKind::Anomaly || action == UiAction::Explore)) {
        storyStage[arc] = 3;
        if (equippedCatalogItem(kRailPistolItem)) {
            storyOutcome[arc] = 0;
            siteAttention[1] = siteAttention[1] > 2 ? static_cast<uint8_t>(siteAttention[1] - 2) : 0;
            if (!hasCatalogItem(kBorrowedShadowItem)) {
                grantRewardItem(kBorrowedShadowItem);
            }
            appendAbilityNote(message, messageSize,
                              "You erase the Other Runner. By dawn, the cameras remember less.");
        } else if (equippedCatalogItem(kNullCharmItem) || equippedCatalogItem(kBorrowedShadowItem)) {
            storyOutcome[arc] = 1;
            appendAbilityNote(message, messageSize,
                              "You bargain with the Other Runner. Your shadow learns the clinic route.");
        } else if (equippedCatalogItem(kEmptyNameTagItem) || equippedCatalogItem(kDebtCollectorsCoatItem)) {
            storyOutcome[arc] = 2;
            siteIntel[1] = kMaxSiteIntel;
            appendAbilityNote(message, messageSize,
                              "You trade identities. Nobody can prove you are you, which is almost freedom.");
        } else {
            storyOutcome[arc] = 3;
            grantTradeGoods(12);
            siteAttention[1] = clampInt(siteAttention[1] + 3, 0, kMaxSiteAttention);
            appendAbilityNote(message, messageSize,
                              "You sell the duplicate. By morning, three people have bought your face.");
        }
    }
}

void advanceStoryArcs(UiAction action, OutcomeLevel outcome, LeadKind lead, char* message, size_t messageSize) {
    advanceBatteriesForDead(action, outcome, lead, message, messageSize);
    advanceLastSaleAtB2(action, outcome, lead, message, messageSize);
    advancePersonWhoNeverEntered(action, outcome, lead, message, messageSize);
}

// Resolves observe, explore, and lead actions through one risk/reward pipeline.
void resolveFieldAction(UiAction action) {
    if (!fieldActionAvailable(action)) {
        setStatus("That move is not open right now. Either the site is dry, the light is wrong, or you need a lead.");
        return;
    }

    clearReward();
    const Stats stats = deriveStats();
    const Site& site = sites[currentSite];
    const LeadKind lead = siteLead[currentSite];
    char actionTitle[24];
    snprintf(actionTitle, sizeof(actionTitle), "%s", actionLabel(action));
    const int16_t skill = actionSkill(action, stats);
    const int16_t target = actionTarget(action);
    const int16_t total = skill + random(1, 7);
    int16_t ambientDose = actionExposureCost(action, stats);
    OutcomeLevel outcome = outcomeForRoll(total, target);
    char abilityNote[220];
    describeActionAbilities(action, lead, abilityNote, sizeof(abilityNote));

    bool railForced = false;
    bool quietSaved = false;
    bool lensGhost = false;
    bool rainKeyOpened = false;
    bool deadPagerCall = false;
    bool apologySaved = false;
    bool sirenSaved = false;
    if (outcome == OutcomeLevel::Failure && action == UiAction::Explore && equippedCatalogItem(kRailPistolItem) &&
        !railPistolSpent) {
        outcome = OutcomeLevel::Success;
        railPistolSpent = true;
        railForced = true;
        appendAbilityNote(abilityNote, sizeof(abilityNote),
                          "The Rail Pistol gives a bright answer; the district writes down the question.");
    }
    if (action == UiAction::FollowLead && lead == LeadKind::Door && equippedCatalogItem(kRainKeyItem) &&
        !rainKeySpent && outcome != OutcomeLevel::Full) {
        outcome = OutcomeLevel::Success;
        rainKeySpent = true;
        rainKeyOpened = true;
        appendAbilityNote(abilityNote, sizeof(abilityNote), "The Rain Key opens what has not been built yet.");
    }
    if (outcome == OutcomeLevel::Failure && action == UiAction::FollowLead &&
        (lead == LeadKind::Door || lead == LeadKind::Contact) && equippedCatalogItem(kQuietNailgunItem) &&
        !quietNailgunSpent) {
        outcome = OutcomeLevel::Partial;
        quietNailgunSpent = true;
        quietSaved = true;
        appendAbilityNote(abilityNote, sizeof(abilityNote),
                          "The Quiet Nailgun turns failure into a close, ugly partial.");
    }
    if (outcome == OutcomeLevel::Failure && action == UiAction::FollowLead && lead == LeadKind::Contact &&
        equippedCatalogItem(kApologyEngineItem) && !apologyEngineSpent) {
        outcome = OutcomeLevel::Partial;
        apologyEngineSpent = true;
        apologySaved = true;
        appendAbilityNote(abilityNote, sizeof(abilityNote), "The Apology Engine spends your leverage before you can.");
    }
    if (outcome == OutcomeLevel::Failure && action == UiAction::FollowLead && lead == LeadKind::Contact &&
        equippedCatalogItem(kMercySirenItem) && !mercySirenSpent) {
        outcome = OutcomeLevel::Partial;
        mercySirenSpent = true;
        sirenSaved = true;
        appendAbilityNote(abilityNote, sizeof(abilityNote), "The Mercy Siren reminds everyone to step away.");
    }
    if (outcome == OutcomeLevel::Failure && action == UiAction::Observe && equippedCatalogItem(kMourningLensItem) &&
        !mourningLensSpent) {
        outcome = OutcomeLevel::Partial;
        mourningLensSpent = true;
        lensGhost = true;
        appendAbilityNote(abilityNote, sizeof(abilityNote),
                          "The Mourning Lens keeps the failed observation as a ghost lead.");
    }
    if (outcome == OutcomeLevel::Failure && action == UiAction::Observe && equippedCatalogItem(kDeadPagerItem) &&
        !deadPagerSpent) {
        outcome = OutcomeLevel::Partial;
        deadPagerSpent = true;
        deadPagerCall = true;
        appendAbilityNote(abilityNote, sizeof(abilityNote), "The Dead Pager answers failure with a contact number.");
    }

    const bool rewardOutcome = outcomeHasReward(outcome);
    uint8_t attentionGain = actionAttentionGain(action, rewardOutcome);
    if (outcome == OutcomeLevel::Partial) {
        ambientDose = clampInt(ambientDose + 1, 0, 6);
        attentionGain = clampInt(attentionGain + 1, 0, kMaxSiteAttention);
    }
    if (railForced) {
        ambientDose = clampInt(ambientDose + 1, 0, 6);
        attentionGain = clampInt(attentionGain + 4, 0, kMaxSiteAttention);
    }
    if (quietSaved) {
        attentionGain = clampInt(attentionGain + 1, 0, kMaxSiteAttention);
    }
    if (rainKeyOpened) {
        attentionGain = clampInt(attentionGain + 2, 0, kMaxSiteAttention);
    }
    if (apologySaved) {
        attentionGain = clampInt(attentionGain - 1, 0, kMaxSiteAttention);
    }
    if (sirenSaved) {
        attentionGain = clampInt(attentionGain + 2, 0, kMaxSiteAttention);
    }
    if (lensGhost) {
        ambientDose = clampInt(ambientDose + 1, 0, 6);
    }
    if (action == UiAction::Observe && outcome == OutcomeLevel::Failure && equippedCatalogItem(kStaticBridalVeilItem) &&
        !staticVeilSpent) {
        staticVeilSpent = true;
        attentionGain = 0;
        ambientDose = clampInt(ambientDose + 1, 0, 6);
        appendAbilityNote(abilityNote, sizeof(abilityNote), "The Static Bridal Veil takes the failed camera heat as strain.");
    }
    if (attentionGain > 0 && !borrowedShadowSpent && equippedCatalogItem(kBorrowedShadowItem) &&
        (action == UiAction::Observe || (action == UiAction::FollowLead && lead == LeadKind::Contact))) {
        borrowedShadowSpent = true;
        attentionGain = 0;
        appendAbilityNote(abilityNote, sizeof(abilityNote), "The Borrowed Shadow stands where you should have been seen.");
    }
    if (attentionGain > 0 && equippedCatalogItem(kNullCharmItem) && !nullCharmSpent[currentSite]) {
        nullCharmSpent[currentSite] = true;
        attentionGain = attentionGain > 1 ? static_cast<uint8_t>(attentionGain - 1) : 0;
        appendAbilityNote(abilityNote, sizeof(abilityNote),
                          "The Null Charm eats the first clean witness, and a little certainty with it.");
    }

    exposure = clampInt(exposure + ambientDose, 0, kMaxExposure);
    siteAttention[currentSite] = clampInt(siteAttention[currentSite] + attentionGain, 0, kMaxSiteAttention);

    char message[320];
    bool keepLeadAfterFollow = false;
    if (action == UiAction::Observe) {
        if (rewardOutcome) {
            LeadKind foundLead = deadPagerCall ? LeadKind::Contact : randomLeadForSite(currentSite);
            if (currentSite == 3 && equippedCatalogItem(kCopperToothRadioItem) && outcome == OutcomeLevel::Full &&
                random(0, 100) < 60) {
                foundLead = random(0, 100) < 50 ? LeadKind::Trail : LeadKind::Anomaly;
            }
            if (currentSite == 4 && equippedCatalogItem(kBoneTuningForkItem) && random(0, 100) < 45) {
                foundLead = LeadKind::Trail;
            }
            if (equippedCatalogItem(kBlackReedSeedItem) && random(0, 100) < 30) {
                foundLead = LeadKind::Trail;
            }
            const uint8_t gainValue = outcome == OutcomeLevel::Partial ? 0 : random(0, 2);
            siteLead[currentSite] = foundLead;
            siteIntel[currentSite] =
                clampInt(siteIntel[currentSite] + (outcome == OutcomeLevel::Partial ? 0 : 1), 0, kMaxSiteIntel);
            if (equippedCatalogItem(kSignalChalkItem) && outcome != OutcomeLevel::Partial) {
                signalChalkMark[currentSite] = true;
            }
            if (gainValue > 0) {
                grantTradeGoods(static_cast<uint8_t>(gainValue + 2));
            }
            snprintf(message, sizeof(message),
                     "%s %s %d/%d %s. Lead: %s. %s",
                     site.observeText, actionCheckText(action), total, target, outcomeName(outcome), leadName(foundLead),
                     leadWhisper(foundLead));
        } else {
            exposure = clampInt(exposure + 1, 0, kMaxExposure);
            snprintf(message, sizeof(message),
                     "You wait too long in bad cover. %s %d/%d. Attention +%u, exposure creeps up.",
                     actionCheckText(action), total, target, attentionGain);
        }
    } else if (action == UiAction::Explore) {
        if (rewardOutcome) {
            if (siteCache[currentSite] > 0) {
                --siteCache[currentSite];
            }

            uint8_t gain = static_cast<uint8_t>(effectiveRiskForSite(currentSite) + random(1, 5));
            if (outcome == OutcomeLevel::Partial) {
                gain = static_cast<uint8_t>(clampInt(gain / 2, 1, 99));
            }
            if (outcome == OutcomeLevel::Full) {
                gain = static_cast<uint8_t>(gain + 2);
            }
            if (currentSite == 1 && equippedCatalogItem(kTollHammerItem)) {
                gain = static_cast<uint8_t>(gain + 2);
            }
            if (currentSite == 2 &&
                (equippedCatalogItem(kDrownedRegisterItem) || equippedCatalogItem(kCheckoutOracleItem))) {
                gain = static_cast<uint8_t>(gain + 1);
            }
            grantTradeGoods(gain);
            if (siteLead[currentSite] == LeadKind::None && random(0, 100) < 55) {
                const LeadKind foundLead = randomLeadForSite(currentSite);
                siteLead[currentSite] = foundLead;
                snprintf(message, sizeof(message),
                         "You actively explore %s. %s %d/%d %s. Goods worth about %d, plus a %s.",
                         site.name, actionCheckText(action), total, target, outcomeName(outcome), gain,
                         leadName(foundLead));
            } else {
                const uint8_t itemId = randomLootForAction(action);
                const bool duplicate = hasCatalogItem(itemId) && itemCatalog[itemId].use.kind != ItemUseKind::Consume;
                uint8_t itemChance = 58;
                if (currentSite == 2 &&
                    (equippedCatalogItem(kDrownedRegisterItem) || equippedCatalogItem(kCheckoutOracleItem))) {
                    itemChance = 76;
                }
                if (!duplicate && random(0, 100) < itemChance) {
                    grantRewardItem(itemId);
                    snprintf(message, sizeof(message),
                             "You push through %s. %s %d/%d %s. Goods worth about %d and %s.",
                             site.name, actionCheckText(action), total, target, outcomeName(outcome), gain,
                             itemCatalog[itemId].name);
                } else {
                    grantTradeGoods(static_cast<uint8_t>(itemCatalog[itemId].value / 2));
                    snprintf(message, sizeof(message),
                             "You push through %s. %s %d/%d %s. Goods and salvage worth about %d. Cache %u.",
                             site.name, actionCheckText(action), total, target, outcomeName(outcome), gain,
                             siteCache[currentSite]);
                }
            }
        } else {
            int16_t wound = 1 + effectiveRiskForSite(currentSite) / 5;
            int16_t dose = 1 + effectiveRiskForSite(currentSite) / 3;
            if (equippedCatalogItem(kRoadworkerHuskItem) || equippedCatalogItem(kRivetSaintItem)) {
                wound = clampInt(wound - 1, 0, 4);
                appendAbilityNote(abilityNote, sizeof(abilityNote), "The heavy kit turns a bad hit into bruising paperwork.");
            }
            if (equippedCatalogItem(kTarTapeRollItem) && !tarTapeSpent[currentSite]) {
                tarTapeSpent[currentSite] = true;
                dose = clampInt(dose - 1, 0, 6);
                appendAbilityNote(abilityNote, sizeof(abilityNote), "Tar Tape seals the leak long enough to blame you later.");
            }
            if (equippedCatalogItem(kPulseStaplerItem) && availableTradeValue() >= 2 && wound > 0) {
                payTradeValue(2);
                --wound;
                exposure = clampInt(exposure + 1, 0, kMaxExposure);
                appendAbilityNote(abilityNote, sizeof(abilityNote),
                                  "The Pulse Stapler spends goods to close the wound, then hums under skin.");
            }
            if (cheapCourageReady && wound > 0) {
                --wound;
                cheapCourageReady = false;
                appendAbilityNote(abilityNote, sizeof(abilityNote),
                                  "Cheap Courage ignores the first hurt and leaves the bill for later.");
            }
            health = clampInt(health - wound, 0, kMaxHealth);
            exposure = clampInt(exposure + dose, 0, kMaxExposure);
            snprintf(message, sizeof(message),
                     "%s punishes blind movement. %s %d/%d. Attention +%u, -%d body, +%d exposure.",
                     site.name, actionCheckText(action), total, target, attentionGain, wound, dose);
        }
    } else if (action == UiAction::FollowLead) {
        if (rewardOutcome) {
            const int16_t risk = effectiveRiskForSite(currentSite);
            int16_t gain = risk + random(2, 6);
            if (outcome == OutcomeLevel::Partial) {
                gain = clampInt(gain / 2, 1, 99);
            }
            if (outcome == OutcomeLevel::Full) {
                gain += 2;
            }

            if (lead == LeadKind::Contact) {
                if (equippedCatalogItem(kCivicBatonItem)) {
                    gain += 2;
                }
                if (equippedCatalogItem(kRedactedPhotographItem) && outcome != OutcomeLevel::Partial) {
                    siteIntel[currentSite] = clampInt(siteIntel[currentSite] + 1, 0, kMaxSiteIntel);
                }
                if (apologySaved) {
                    gain = clampInt(gain - 2, 1, 99);
                }
                siteIntel[currentSite] =
                    clampInt(siteIntel[currentSite] + (outcome == OutcomeLevel::Partial ? 0 : 1), 0, kMaxSiteIntel);
                grantTradeGoods(static_cast<uint8_t>(gain));
                siteAttention[currentSite] =
                    siteAttention[currentSite] > 0 ? static_cast<uint8_t>(siteAttention[currentSite] - 1) : 0;
                snprintf(message, sizeof(message),
                         "You approach the quiet contact correctly. %s %d/%d %s. Goods worth about %d, attention softens.",
                         actionCheckText(action), total, target, outcomeName(outcome), gain);
            } else if (lead == LeadKind::Trail) {
                siteIntel[currentSite] =
                    clampInt(siteIntel[currentSite] + (outcome == OutcomeLevel::Partial ? 0 : 1), 0, kMaxSiteIntel);
                siteAttention[currentSite] =
                    siteAttention[currentSite] > 2 ? static_cast<uint8_t>(siteAttention[currentSite] - 3) : 0;
                exposure = clampInt(exposure - 1, 0, kMaxExposure);
                if (equippedCatalogItem(kMapScalpelItem) && siteIntel[currentSite] > 0 && outcome == OutcomeLevel::Full) {
                    siteIntel[currentSite] = static_cast<uint8_t>(siteIntel[currentSite] - 1);
                    signalChalkMark[currentSite] = true;
                    appendAbilityNote(abilityNote, sizeof(abilityNote),
                                      "The Map Scalpel cuts one intel into a temporary shortcut.");
                }
                snprintf(message, sizeof(message),
                         "You trace the cleaner footpath. %s %d/%d %s. Attention drops, exposure drops.",
                         actionCheckText(action), total, target, outcomeName(outcome));
            } else {
                if (siteCache[currentSite] > 0) {
                    --siteCache[currentSite];
                }
                const uint8_t itemId = randomLootForAction(action);
                const bool duplicate = hasCatalogItem(itemId) && itemCatalog[itemId].use.kind != ItemUseKind::Consume;
                if (lead == LeadKind::Anomaly) {
                    exposure = clampInt(exposure + 1, 0, kMaxExposure);
                    gain += 2;
                    if (blackIodineGuard) {
                        exposure = clampInt(exposure - 1, 0, kMaxExposure);
                        blackIodineGuard = false;
                        appendAbilityNote(abilityNote, sizeof(abilityNote),
                                          "Black Iodine catches the bonus exposure before it reaches your bones.");
                    }
                } else if (lead == LeadKind::Door) {
                    gain += 3;
                    if (equippedCatalogItem(kServiceWormItem) && outcome == OutcomeLevel::Full) {
                        siteLead[currentSite] = LeadKind::Cache;
                        keepLeadAfterFollow = true;
                        appendAbilityNote(abilityNote, sizeof(abilityNote),
                                          "The Service Worm finds a cache behind the bypass.");
                    }
                    if (currentSite == 2 && equippedCatalogItem(kTenthButtonItem) && outcome != OutcomeLevel::Partial &&
                        random(0, 100) < 45) {
                        grantRewardItem(random(0, 100) < 50 ? kWhiteReceiptItem : kDrownedRegisterItem);
                        appendAbilityNote(abilityNote, sizeof(abilityNote),
                                          "The Tenth Button opens a lower-floor cache that the map denies.");
                    }
                } else if (lead == LeadKind::Cache && currentSite == 2 &&
                           (equippedCatalogItem(kFloodChoirWadersItem) || equippedCatalogItem(kDrownedRegisterItem))) {
                    gain += 2;
                }
                if (!duplicate || itemCatalog[itemId].use.kind == ItemUseKind::Consume) {
                    grantTradeGoods(static_cast<uint8_t>(gain));
                    grantRewardItem(itemId);
                    snprintf(message, sizeof(message), "You %s the %s. %s %d/%d %s. Goods worth about %d and %s.",
                             leadVerb(lead), leadName(lead), actionCheckText(action), total, target,
                             outcomeName(outcome), gain, itemCatalog[itemId].name);
                } else {
                    grantTradeGoods(static_cast<uint8_t>(gain + itemCatalog[itemId].value / 2));
                    snprintf(message, sizeof(message),
                             "You %s the %s. %s %d/%d %s. Goods and valuable fragments worth about %d.",
                             leadVerb(lead), leadName(lead), actionCheckText(action), total, target,
                             outcomeName(outcome), gain);
                }
            }
            if (!keepLeadAfterFollow) {
                siteLead[currentSite] = LeadKind::None;
            }
        } else {
            int16_t wound = lead == LeadKind::Contact || lead == LeadKind::Trail ? 1 : 2;
            int16_t dose = lead == LeadKind::Anomaly ? 3 : 1 + effectiveRiskForSite(currentSite) / 4;
            if (lead == LeadKind::Trail && equippedCatalogItem(kFlareHookItem) && !flareHookSpent) {
                flareHookSpent = true;
                wound = 0;
                currentSite = 0;
                appendAbilityNote(abilityNote, sizeof(abilityNote),
                                  "The Flare Hook turns the bad trail into a forced retreat instead of a wound.");
            }
            if ((lead == LeadKind::Cache || lead == LeadKind::Door) && equippedCatalogItem(kTarTapeRollItem) &&
                !tarTapeSpent[currentSite]) {
                tarTapeSpent[currentSite] = true;
                dose = clampInt(dose - 1, 0, 6);
                appendAbilityNote(abilityNote, sizeof(abilityNote), "Tar Tape seals one dirty consequence.");
            }
            if ((lead == LeadKind::Contact || lead == LeadKind::Door) && equippedCatalogItem(kStaticKnifeItem)) {
                wound = clampInt(wound - 1, 0, 4);
                exposure = clampInt(exposure + 1, 0, kMaxExposure);
                appendAbilityNote(abilityNote, sizeof(abilityNote),
                                  "The Static Knife takes the mistake as buzzing strain instead of blood.");
            }
            if (lead == LeadKind::Anomaly && equippedCatalogItem(kColdLanternItem)) {
                dose = clampInt(dose - 1, 0, 6);
                appendAbilityNote(abilityNote, sizeof(abilityNote),
                                  "The Cold Lantern shows which part of the wrong-light not to breathe.");
            }
            if (lead == LeadKind::Contact && equippedCatalogItem(kRedactedPhotographItem)) {
                siteLead[currentSite] = LeadKind::Anomaly;
                appendAbilityNote(abilityNote, sizeof(abilityNote),
                                  "The Redacted Photograph turns the failed contact into something stranger.");
            } else {
                siteLead[currentSite] = LeadKind::None;
            }
            health = clampInt(health - wound, 0, kMaxHealth);
            exposure = clampInt(exposure + dose, 0, kMaxExposure);
            snprintf(message, sizeof(message),
                     "The %s turns bad. %s %d/%d. Lead lost, attention +%u, -%d body, +%d exposure.",
                     leadName(lead), actionCheckText(action), total, target, attentionGain, wound, dose);
        }
    } else {
        return;
    }

    uint8_t spentTicks = actionTimeCost(action);
    if (action == UiAction::Observe && sleeplessMintReady && spentTicks > 0) {
        --spentTicks;
        sleeplessMintReady = false;
        appendAbilityNote(abilityNote, sizeof(abilityNote), "Sleepless Mint steals the hour before it lands.");
    }
    if (action == UiAction::Explore && currentSite == 2 && equippedCatalogItem(kDrownedRegisterItem)) {
        ++spentTicks;
        appendAbilityNote(abilityNote, sizeof(abilityNote), "The Drowned Register adds a slow, wet checkout.");
    }
    if (action == UiAction::Explore && equippedCatalogItem(kAshfallPonchoItem) && !ashfallPonchoSpent[currentSite]) {
        ashfallPonchoSpent[currentSite] = true;
        appendAbilityNote(abilityNote, sizeof(abilityNote), "The Ashfall Poncho spends its one clean shrug for this site.");
    }
    if (rewardOutcome && actionUsesTech(action, lead) && equippedCatalogItem(kWarmBatteryItem) && !warmBatterySpent &&
        spentTicks > 0) {
        --spentTicks;
        warmBatterySpent = true;
        appendAbilityNote(abilityNote, sizeof(abilityNote),
                          "The Warm Battery refunds an hour that should have been gone.");
    }
    if (action == UiAction::Explore && cheapCourageReady) {
        cheapCourageReady = false;
    }
    if (action == UiAction::FollowLead && lead == LeadKind::Trail && fenceSaltReady) {
        fenceSaltReady = false;
    }
    if (action == UiAction::FollowLead && blackIodineGuard && lead == LeadKind::Anomaly) {
        blackIodineGuard = false;
    }
    if (action == UiAction::FollowLead && ghostTeaApplies(lead)) {
        ghostTeaSite = 255;
    }
    if (blueMilkBlurReady) {
        blueMilkBlurReady = false;
    }
    advanceStoryArcs(action, outcome, lead, message, sizeof(message));
    appendAbilityNote(message, sizeof(message), abilityNote);
    snprintf(rewardTitle, sizeof(rewardTitle), "%s Complete", actionTitle);
    snprintf(rewardSummary, sizeof(rewardSummary), "%s", message);
    setStatus(message);
    spendTime(spentTicks);
    checkCollapse();
    currentScreen = Screen::Reward;
    screenDirty = true;
}

// Moves the runner between sites, spending daylight and applying route dose.
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

    int16_t routeDose = routeExposureCost(targetSite, stats);
    char routeNote[140] = "";
    if (equippedCatalogItem(kCalendarOfRainsItem) && !calendarSpent && routeDose > 0) {
        calendarSpent = true;
        const int16_t reroll = clampInt(routeDose + random(-1, 3), 0, 4);
        routeDose = reroll;
        appendAbilityNote(routeNote, sizeof(routeNote), "Calendar of Rains makes the forecast binding.");
    }
    if (signalChalkMark[targetSite]) {
        signalChalkMark[targetSite] = false;
        appendAbilityNote(routeNote, sizeof(routeNote), "Signal Chalk mark washes into the tires.");
    }
    if (fenceSaltReady) {
        fenceSaltReady = false;
        appendAbilityNote(routeNote, sizeof(routeNote), "Fence Runner's Salt keeps the route heat slippery.");
    }
    currentSite = targetSite;
    selectedMapSite = targetSite;
    if (currentSite != 0) {
        exposure = clampInt(exposure + routeDose, 0, kMaxExposure);
    } else {
        exposure = clampInt(exposure + routeDose, 0, kMaxExposure);
    }

    char message[160];
    if (equippedCatalogItem(kRoadworkerHuskItem) && (targetSite == 1 || targetSite == 3)) {
        siteAttention[targetSite] = clampInt(siteAttention[targetSite] + 1, 0, kMaxSiteAttention);
    }

    snprintf(message, sizeof(message), "You trust the signal map and ride %uh to %s. Route dose +%d. Risk now %d. %s",
             static_cast<unsigned>(travelTicks * kTickHours), sites[currentSite].name, routeDose,
             effectiveRiskForSite(currentSite), routeNote);
    setStatus(message);
    currentScreen = Screen::Field;
    spendTime(travelTicks);
    checkCollapse();
}

// Resting at the clinic heals; resting elsewhere retreats to the safe berth.
void restOrRetreat() {
    if (currentSite != 0) {
        currentSite = 0;
        exposure = clampInt(exposure + 1, 0, kMaxExposure);
        setStatus("You cut the run short and spend time limping back to the clinic before the rain gets clever.");
        spendTime(actionTimeCost(UiAction::Rest));
        checkCollapse();
        return;
    }

    const int16_t paid = payTradeValue(2);
    health = clampInt(health + 4, 0, kMaxHealth);
    exposure = clampInt(exposure - 4, 0, kMaxExposure);
    char message[160];
    snprintf(message, sizeof(message),
             "A cot, a drip bag, and static through the wall. You pay goods worth %d and recover what the city has not taken.",
             paid);
    startNewDay(message);
    checkCollapse();
}

uint8_t maxTradePage() {
    const uint8_t count = inventoryItemCount();
    return count == 0 ? 0 : static_cast<uint8_t>((count - 1) / kTradeRowsPerPage);
}

void normalizeTradePage() {
    const uint8_t maxPage = maxTradePage();
    if (tradePage > maxPage) {
        tradePage = maxPage;
    }
}

bool tradeCanComplete() {
    const int16_t ask = tradeAskValue();
    return ask > 0 && tradeOfferValue() >= ask;
}

void completeTrade() {
    const int16_t offer = tradeOfferValue();
    const int16_t ask = tradeAskValue();
    if (ask <= 0) {
        setStatus("Choose what you want from the trader first.");
        return;
    }
    if (offer < ask) {
        setStatus("The trader shakes their head. Your side of the blanket is still light.");
        return;
    }

    clearReward();
    for (uint8_t i = 0; i < kInventoryCapacity; ++i) {
        if (tradeOfferSelected[i] && validInventorySlot(i)) {
            removeInventorySlot(i);
        }
    }
    for (uint8_t i = 0; i < kTradeStockCount; ++i) {
        if (tradeWantSelected[i]) {
            grantRewardItem(tradeStock[i]);
        }
    }

    snprintf(rewardTitle, sizeof(rewardTitle), "Trade Complete");
    snprintf(rewardSummary, sizeof(rewardSummary), "You traded goods worth %d for goods worth %d. No one gives change.",
             offer, ask);
    setStatus(rewardSummary);
    resetTradeSelections();
    currentScreen = Screen::Reward;
    screenDirty = true;
}

// Dispatches button presses into game actions and screen changes.
void handleAction(UiAction action, int16_t param) {
    switch (action) {
        case UiAction::Observe:
        case UiAction::Explore:
        case UiAction::FollowLead:
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
        case UiAction::InspectItem:
            if (validInventorySlot(param)) {
                selectedInventorySlot = param;
                currentScreen = Screen::ItemDetail;
                screenDirty = true;
            }
            break;
        case UiAction::EquipOrUse:
            equipInventorySlot(static_cast<uint8_t>(param));
            if (!validInventorySlot(param)) {
                currentScreen = Screen::Inventory;
            }
            break;
        case UiAction::InventoryPrev:
            if (inventoryPage > 0) {
                --inventoryPage;
                screenDirty = true;
            }
            break;
        case UiAction::InventoryNext:
            ++inventoryPage;
            screenDirty = true;
            break;
        case UiAction::OpenTrade:
            if (tradeAvailableHere()) {
                resetTradeSelections();
                currentScreen = Screen::Trade;
                screenDirty = true;
            } else {
                setStatus("No one here is willing to spread a trade blanket.");
            }
            break;
        case UiAction::ToggleTradeOffer:
            if (param >= 0 && param < static_cast<int16_t>(kInventoryCapacity) && validInventorySlot(param) &&
                !isEquippedInventorySlot(static_cast<uint8_t>(param))) {
                tradeOfferSelected[param] = !tradeOfferSelected[param];
                screenDirty = true;
            }
            break;
        case UiAction::ToggleTradeWant:
            if (param >= 0 && param < static_cast<int16_t>(kTradeStockCount)) {
                tradeWantSelected[param] = !tradeWantSelected[param];
                screenDirty = true;
            }
            break;
        case UiAction::TradePrev:
            if (tradePage > 0) {
                --tradePage;
                screenDirty = true;
            }
            break;
        case UiAction::TradeNext:
            ++tradePage;
            normalizeTradePage();
            screenDirty = true;
            break;
        case UiAction::AcceptTrade:
            completeTrade();
            break;
        case UiAction::ClearTrade:
            resetTradeSelections();
            screenDirty = true;
            break;
        case UiAction::ContinueReward:
            currentScreen = Screen::Field;
            screenDirty = true;
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

// Draws the main play screen: place text, stats, forecast, and actions.
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
    drawTextFit(sites[currentSite].name, centerX + 18, top + 18, centerW - 36, TFT_WHITE, panelBg);
    display.setFont(&fonts::Font2);
    drawTextFit(sites[currentSite].district, centerX + 20, top + 52, centerW - 40, rgb(155, 175, 175), panelBg);
    drawWrappedText(sites[currentSite].description, centerX + 20, top + 88, centerW - 40, 3, rgb(210, 218, 210),
                    panelBg);
    display.setFont(&fonts::Font2);
    drawFormattedTextFit(centerX + 20, top + 156, centerW - 40, rgb(180, 210, 205), panelBg,
                         "risk %d  cache %u  intel %u  attention %u", effectiveRiskForSite(currentSite),
                         siteCache[currentSite], siteIntel[currentSite], siteAttention[currentSite]);
    drawFormattedTextFit(centerX + 20, top + 176, centerW - 40, rgb(180, 210, 205), panelBg, "lead: %s",
                         leadName(siteLead[currentSite]));

    display.drawFastHLine(centerX + 18, top + 198, centerW - 36, rgb(55, 70, 70));
    display.setFont(&fonts::Font4);
    drawTextFit("Last Signal", centerX + 18, top + 222, centerW - 36, rgb(130, 230, 200), panelBg);
    drawWrappedText(statusLine, centerX + 20, top + 266, centerW - 40, 4, TFT_WHITE, panelBg);

    const int32_t buttonY = height - bottomH;
    const int32_t gap = 10;
    const int32_t buttonW = (width - gap * 7) / 6;
    if (currentSite == 0) {
        addButton("Trade", gap, buttonY + 8, buttonW, 52, UiAction::OpenTrade, 0, rgb(90, 210, 220),
                  tradeAvailableHere());
    } else {
        addButton("Observe 2h", gap, buttonY + 8, buttonW, 52, UiAction::Observe, 0, rgb(90, 210, 220),
                  fieldActionAvailable(UiAction::Observe));
    }
    addButton("Explore 4h", gap * 2 + buttonW, buttonY + 8, buttonW, 52, UiAction::Explore, 0, rgb(230, 180, 70),
              fieldActionAvailable(UiAction::Explore));
    char leadButton[24];
    snprintf(leadButton, sizeof(leadButton), "%s 2h", leadVerb(siteLead[currentSite]));
    addButton(leadButton, gap * 3 + buttonW * 2, buttonY + 8, buttonW, 52, UiAction::FollowLead, 0,
              rgb(220, 90, 190), fieldActionAvailable(UiAction::FollowLead));
    addButton("Kit", gap * 4 + buttonW * 3, buttonY + 8, buttonW, 52, UiAction::Inventory, 0, rgb(170, 120, 240));
    addButton("Map", gap * 5 + buttonW * 4, buttonY + 8, buttonW, 52, UiAction::Map, 0, rgb(120, 220, 120));
    addButton(currentSite == 0 ? "Rest" : "Retreat", gap * 6 + buttonW * 5, buttonY + 8, buttonW, 52, UiAction::Rest,
              0, rgb(230, 90, 95));

    for (uint8_t i = 0; i < buttonCount; ++i) {
        drawButton(buttons[i]);
    }
}

// Converts a map pin's horizontal permille coordinate into pixels.
int32_t mapPinX(const MapPin& pin, int32_t x, int32_t w) {
    return x + 26 + static_cast<int32_t>(pin.xPermille) * (w - 52) / 1000;
}

// Converts a map pin's vertical permille coordinate into pixels.
int32_t mapPinY(const MapPin& pin, int32_t y, int32_t h) {
    return y + 26 + static_cast<int32_t>(pin.yPermille) * (h - 52) / 1000;
}

// Deterministic pseudo-noise used for satellite terrain texture.
uint32_t mapNoise(uint32_t seed) {
    seed ^= seed >> 16;
    seed *= 2246822519UL;
    seed ^= seed >> 13;
    seed *= 3266489917UL;
    seed ^= seed >> 16;
    return seed;
}

// Chooses the terrain stain color around each site on the satellite map.
uint16_t satelliteSiteStain(uint8_t siteIndex) {
    switch (siteIndex) {
        case 1:
            return rgb(72, 32, 62);
        case 2:
            return rgb(36, 52, 78);
        case 3:
            return rgb(74, 62, 32);
        case 4:
            return rgb(42, 76, 38);
        default:
            return rgb(36, 60, 58);
    }
}

// Draws a simple thick line by offsetting the same segment in four directions.
void drawThickLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint16_t color, uint8_t radius) {
    auto& display = M5.Display;
    display.drawLine(x0, y0, x1, y1, color);
    for (uint8_t i = 1; i <= radius; ++i) {
        display.drawLine(x0 + i, y0, x1 + i, y1, color);
        display.drawLine(x0 - i, y0, x1 - i, y1, color);
        display.drawLine(x0, y0 + i, x1, y1 + i, color);
        display.drawLine(x0, y0 - i, x1, y1 - i, color);
    }
}

// Draws a quadratic curve used for natural rivers, roads, and walked paths.
void drawCurvedTerrainStroke(int32_t ax, int32_t ay, int32_t bx, int32_t by, int32_t bendX, int32_t bendY,
                             uint16_t color, uint8_t radius, uint8_t steps) {
    const int32_t cx = (ax + bx) / 2 + bendX;
    const int32_t cy = (ay + by) / 2 + bendY;
    int32_t prevX = ax;
    int32_t prevY = ay;

    for (uint8_t i = 1; i <= steps; ++i) {
        const int32_t t = i;
        const int32_t inv = steps - i;
        const int32_t denom = static_cast<int32_t>(steps) * static_cast<int32_t>(steps);
        const int32_t px = (inv * inv * ax + 2 * inv * t * cx + t * t * bx) / denom;
        const int32_t py = (inv * inv * ay + 2 * inv * t * cy + t * t * by) / denom;
        drawThickLine(prevX, prevY, px, py, color, radius);
        prevX = px;
        prevY = py;
    }
}

// Draws route edges as uneven foot-made paths instead of straight UI lines.
void drawWalkedRoute(uint8_t edgeIndex, int32_t ax, int32_t ay, int32_t bx, int32_t by, uint16_t signalColor,
                     bool highlighted) {
    const int32_t dx = bx - ax;
    const int32_t dy = by - ay;
    const int32_t direction = (edgeIndex % 2) == 0 ? 1 : -1;
    const int32_t bend = 22 + static_cast<int32_t>((edgeIndex * 19U) % 31U);
    const int32_t bendX = direction * (-dy) * bend / 170 + static_cast<int32_t>((edgeIndex * 17U) % 23U) - 11;
    const int32_t bendY = direction * dx * bend / 170 + static_cast<int32_t>((edgeIndex * 29U) % 27U) - 13;

    drawCurvedTerrainStroke(ax, ay, bx, by, bendX, bendY, rgb(20, 25, 22), 4, 14);
    drawCurvedTerrainStroke(ax, ay, bx, by, bendX, bendY, rgb(74, 63, 42), highlighted ? 3 : 2, 14);

    if (highlighted) {
        drawCurvedTerrainStroke(ax, ay, bx, by, bendX, bendY, signalColor, 1, 14);
    } else {
        drawCurvedTerrainStroke(ax, ay, bx, by, bendX, bendY, rgb(58, 72, 64), 0, 14);
    }

    for (uint8_t i = 2; i < 14; i += 3) {
        const int32_t t = i;
        const int32_t inv = 14 - i;
        const int32_t cx = (ax + bx) / 2 + bendX;
        const int32_t cy = (ay + by) / 2 + bendY;
        const int32_t px = (inv * inv * ax + 2 * inv * t * cx + t * t * bx) / (14 * 14);
        const int32_t py = (inv * inv * ay + 2 * inv * t * cy + t * t * by) / (14 * 14);
        M5.Display.fillCircle(px + static_cast<int32_t>((edgeIndex + i) % 5) - 2, py, highlighted ? 3 : 2,
                              highlighted ? signalColor : rgb(86, 75, 50));
    }
}

// Paints the satellite-style terrain layer with noise, stains, roads, and water.
void drawMapBackground(int32_t x, int32_t y, int32_t w, int32_t h) {
    auto& display = M5.Display;
    const uint16_t bg = rgb(6, 9, 8);

    drawPanel(x, y, w, h, rgb(46, 120, 110));
    display.setClipRect(x + 2, y + 2, w - 4, h - 4);
    display.fillRoundRect(x + 2, y + 2, w - 4, h - 4, 8, bg);

    for (uint8_t i = 0; i < 105; ++i) {
        const uint32_t seed = mapNoise(static_cast<uint32_t>(i) * 4099UL + static_cast<uint32_t>(day) * 193UL);
        const int32_t px = x + 12 + static_cast<int32_t>(seed % static_cast<uint32_t>(w - 24));
        const int32_t py = y + 12 + static_cast<int32_t>((seed >> 9) % static_cast<uint32_t>(h - 24));
        const int32_t radius = 8 + static_cast<int32_t>((seed >> 18) % 28U);
        uint16_t color = rgb(18, 28, 24);
        switch ((seed >> 27) & 0x03U) {
            case 0:
                color = rgb(22, 34, 26);
                break;
            case 1:
                color = rgb(33, 31, 24);
                break;
            case 2:
                color = rgb(16, 31, 36);
                break;
            default:
                color = rgb(37, 25, 31);
                break;
        }
        display.fillCircle(px, py, radius, color);
    }

    drawCurvedTerrainStroke(x + 34, y + h - 68, x + w - 38, y + 62, -34, 42, rgb(12, 35, 42), 10, 18);
    drawCurvedTerrainStroke(x + 34, y + h - 68, x + w - 38, y + 62, -34, 42, rgb(18, 58, 62), 6, 18);
    drawCurvedTerrainStroke(x + 58, y + 76, x + w - 78, y + h - 86, 46, -28, rgb(44, 43, 36), 4, 16);
    drawCurvedTerrainStroke(x + 86, y + h - 44, x + w - 120, y + 108, -58, -16, rgb(31, 34, 31), 3, 15);

    for (uint8_t i = 0; i < kMapPinCount; ++i) {
        const int32_t px = mapPinX(mapPins[i], x, w);
        const int32_t py = mapPinY(mapPins[i], y, h);
        const uint16_t stain = satelliteSiteStain(mapPins[i].site);
        display.fillCircle(px, py, 38 + siteAttention[mapPins[i].site] * 5, stain);
        display.fillCircle(px + 19, py - 12, 22, stain);
        display.fillCircle(px - 18, py + 15, 18, stain);
    }

    for (uint8_t i = 0; i < 32; ++i) {
        const uint32_t seed = mapNoise(0x5A7E1111UL + static_cast<uint32_t>(i) * 991UL);
        const int32_t px = x + 24 + static_cast<int32_t>(seed % static_cast<uint32_t>(w - 48));
        const int32_t py = y + 24 + static_cast<int32_t>((seed >> 10) % static_cast<uint32_t>(h - 48));
        const int32_t rw = 7 + static_cast<int32_t>((seed >> 20) % 18U);
        const int32_t rh = 3 + static_cast<int32_t>((seed >> 25) % 9U);
        const uint16_t color = ((seed >> 4) & 1U) != 0 ? rgb(44, 45, 40) : rgb(27, 31, 30);
        display.fillRect(px, py, rw, rh, color);
        if ((seed & 0x03U) == 0) {
            display.drawFastHLine(px - 2, py + rh + 2, rw + 4, rgb(18, 22, 20));
        }
    }

    display.setFont(&fonts::Font2);
    drawTextFit("ORBITAL PASS DEGRADED / GROUND TRUTH PATCHED BY WALKED ROUTES", x + 24, y + h - 30, w - 48,
                rgb(82, 112, 96), bg);
    drawTextFit("mud, roof, poison water, attention bloom", x + 24, y + 16, w - 48, rgb(82, 112, 96), bg);
    display.clearClipRect();
}

// Draws every authored route and highlights paths connected to current focus.
void drawMapRoutes(int32_t x, int32_t y, int32_t w, int32_t h) {
    for (uint8_t i = 0; i < kRouteEdgeCount; ++i) {
        const MapPin& a = mapPins[routeEdges[i].from];
        const MapPin& b = mapPins[routeEdges[i].to];
        const int32_t ax = mapPinX(a, x, w);
        const int32_t ay = mapPinY(a, y, h);
        const int32_t bx = mapPinX(b, x, w);
        const int32_t by = mapPinY(b, y, h);
        const bool active = routeEdges[i].from == currentSite || routeEdges[i].to == currentSite;
        const bool selected = routeEdges[i].from == selectedMapSite || routeEdges[i].to == selectedMapSite;
        const bool highlighted = active || selected;
        const uint16_t color = active ? rgb(110, 230, 180) : (selected ? rgb(180, 120, 230) : rgb(72, 92, 70));
        drawWalkedRoute(i, ax, ay, bx, by, color, highlighted);
    }
}

// Draws the shared marker base under each destination icon.
void drawIconBase(int32_t x, int32_t y, uint16_t accent, bool selected) {
    auto& display = M5.Display;
    display.fillCircle(x + 3, y + 5, 23, rgb(3, 5, 5));
    display.fillCircle(x, y, 22, rgb(18, 23, 22));
    display.fillCircle(x - 4, y - 5, 13, rgb(30, 36, 34));
    display.drawCircle(x, y, 23, selected ? TFT_WHITE : rgb(86, 100, 94));
    display.drawCircle(x, y, 19, accent);
}

// Draws the Underpass Clinic marker.
void drawClinicIcon(int32_t x, int32_t y) {
    auto& display = M5.Display;
    display.fillRoundRect(x - 14, y - 9, 28, 20, 3, rgb(92, 96, 90));
    display.fillRect(x - 10, y - 13, 20, 6, rgb(125, 130, 120));
    display.drawRect(x - 14, y - 9, 28, 20, rgb(185, 195, 180));
    display.fillRect(x - 3, y - 7, 6, 16, rgb(150, 30, 38));
    display.fillRect(x - 9, y - 1, 18, 6, rgb(150, 30, 38));
    display.drawFastHLine(x - 12, y + 13, 24, rgb(45, 52, 48));
    display.fillCircle(x + 13, y - 9, 3, rgb(170, 220, 210));
}

// Draws the Neon Spillway marker.
void drawSpillwayIcon(int32_t x, int32_t y) {
    auto& display = M5.Display;
    display.fillRoundRect(x - 17, y - 13, 34, 13, 3, rgb(82, 76, 70));
    display.drawRect(x - 16, y - 12, 32, 11, rgb(142, 135, 122));
    display.fillCircle(x - 9, y - 5, 5, rgb(20, 38, 44));
    display.fillCircle(x + 9, y - 5, 5, rgb(20, 38, 44));
    drawCurvedTerrainStroke(x - 12, y - 1, x - 3, y + 15, -8, 3, rgb(20, 105, 118), 3, 6);
    drawCurvedTerrainStroke(x + 8, y - 1, x + 15, y + 16, 7, 2, rgb(32, 130, 140), 3, 6);
    display.drawFastHLine(x - 18, y - 16, 36, rgb(200, 62, 165));
    display.drawFastHLine(x - 16, y - 18, 24, rgb(92, 30, 80));
}

// Draws the Sunken Mall marker.
void drawMallIcon(int32_t x, int32_t y) {
    auto& display = M5.Display;
    display.fillRoundRect(x - 17, y - 12, 34, 25, 2, rgb(82, 78, 68));
    display.drawRect(x - 16, y - 11, 32, 23, rgb(150, 142, 116));
    display.fillRect(x - 10, y - 7, 20, 6, rgb(38, 74, 82));
    display.fillRect(x - 12, y + 3, 8, 7, rgb(24, 45, 52));
    display.fillRect(x + 4, y + 3, 8, 7, rgb(24, 45, 52));
    display.drawFastHLine(x - 20, y + 17, 40, rgb(26, 52, 62));
    display.drawFastHLine(x - 18, y + 20, 33, rgb(22, 42, 52));
    display.drawLine(x - 16, y - 16, x + 16, y - 16, rgb(120, 116, 96));
    display.drawLine(x - 13, y - 16, x - 18, y - 12, rgb(72, 70, 62));
}

// Draws the Relay Grave marker.
void drawRelayIcon(int32_t x, int32_t y) {
    auto& display = M5.Display;
    display.drawLine(x, y - 19, x - 12, y + 15, rgb(156, 148, 118));
    display.drawLine(x, y - 19, x + 12, y + 15, rgb(156, 148, 118));
    display.drawLine(x - 12, y + 15, x + 12, y + 15, rgb(110, 100, 82));
    display.drawLine(x - 8, y + 5, x + 8, y + 5, rgb(116, 108, 90));
    display.drawLine(x - 5, y - 5, x + 5, y - 5, rgb(116, 108, 90));
    display.drawLine(x - 18, y + 14, x + 17, y - 12, rgb(42, 44, 38));
    display.drawCircle(x, y - 20, 5, rgb(230, 180, 75));
    display.drawCircle(x, y - 20, 10, rgb(118, 88, 44));
    display.drawFastHLine(x + 10, y + 20, 18, rgb(74, 70, 56));
}

// Draws the Black Reed Verge marker.
void drawVergeIcon(int32_t x, int32_t y) {
    auto& display = M5.Display;
    display.fillCircle(x, y + 3, 19, rgb(20, 42, 23));
    for (int8_t i = -4; i <= 4; ++i) {
        const int32_t baseX = x + i * 4;
        const int32_t topX = baseX + (i % 2 == 0 ? -5 : 5);
        const int32_t topY = y - 19 + (i % 3) * 3;
        display.drawLine(baseX, y + 17, topX, topY, rgb(82, 120, 54));
        display.drawLine(baseX + 1, y + 17, topX + 1, topY, rgb(28, 48, 24));
    }
    display.drawLine(x - 18, y + 10, x + 18, y - 10, rgb(68, 65, 52));
    display.drawLine(x - 12, y + 17, x + 21, y + 2, rgb(45, 43, 35));
    display.fillCircle(x + 7, y - 4, 5, rgb(12, 18, 14));
}

// Selects and draws the correct site-specific destination icon.
void drawDestinationIcon(uint8_t siteIndex, int32_t x, int32_t y, bool selected) {
    drawIconBase(x, y, sites[siteIndex].color, selected);
    switch (siteIndex) {
        case 0:
            drawClinicIcon(x, y);
            break;
        case 1:
            drawSpillwayIcon(x, y);
            break;
        case 2:
            drawMallIcon(x, y);
            break;
        case 3:
            drawRelayIcon(x, y);
            break;
        case 4:
            drawVergeIcon(x, y);
            break;
    }
}

// Draws all map pins, overlays attention/intel rings, and registers hit areas.
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

        if (siteAttention[pin.site] > 0) {
            display.drawCircle(px, py, 29 + siteAttention[pin.site] * 2, rgb(120, 45, 58));
        }
        if (siteIntel[pin.site] > 0) {
            display.drawCircle(px, py, 28, rgb(80, 180, 170));
        }

        drawDestinationIcon(pin.site, px, py, selected);
        if (here) {
            display.drawCircle(px, py, 33, rgb(110, 230, 180));
            display.drawCircle(px, py, 38, rgb(45, 95, 88));
        }

        display.setFont(&fonts::Font2);
        int32_t labelX = px + 29;
        int32_t labelW = x + w - labelX - 8;
        if (labelW < 54) {
            labelX = px - 86;
            labelW = px - labelX - 28;
        }
        drawTextFit(pin.callSign, labelX, py - 10, labelW, selected ? TFT_WHITE : rgb(180, 198, 190), bg);
        addButton("", px - 50, py - 42, 132, 84, UiAction::SelectSite, pin.site, site.color, true, false);
    }
}

// Draws the selected destination's narrative and travel information.
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
    drawTextFit(site.name, x + 16, y + 16, w - 32, TFT_WHITE, bg);
    display.setFont(&fonts::Font2);
    drawTextFit(site.district, x + 18, y + 52, w - 36, rgb(160, 180, 178), bg);

    drawFormattedTextFit(x + 18, y + 86, w - 36, rgb(125, 230, 205), bg,
                         "risk %d  cache %u/%u  intel %u  attention %u", effectiveRiskForSite(siteIndex),
                         siteCache[siteIndex], site.maxCache, siteIntel[siteIndex], siteAttention[siteIndex]);

    drawWrappedText(pin.whisper, x + 18, y + 122, w - 36, 3, rgb(215, 222, 212), bg);
    drawWrappedText(site.description, x + 18, y + 198, w - 36, 4, rgb(165, 180, 176), bg);

    display.drawFastHLine(x + 16, y + h - 120, w - 32, rgb(55, 70, 70));
    if (here) {
        drawTextFit("You are already inside this signal.", x + 18, y + h - 96, w - 36, rgb(200, 210, 205), bg);
    } else {
        drawFormattedTextFit(x + 18, y + h - 96, w - 36, rgb(200, 210, 205), bg, "route %s  %uh  dose +%d",
                             directRoute(currentSite, siteIndex) ? "direct" : "cross-town",
                             static_cast<unsigned>(ticks * kTickHours), routeDose);
    }
    drawTextFit(travelReady ? "The route is open." : (here ? "Current location." : "Curfew blocks this route."),
                x + 18, y + h - 70, w - 36, rgb(200, 210, 205), bg);
}

// Draws the map screen as a degraded satellite pass plus in-world annotations.
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

// Draws an item's stat deltas in a compact six-stat row.
void drawStatDelta(const Item& item, int32_t x, int32_t y, int32_t maxWidth, uint16_t bg) {
    M5.Display.setFont(&fonts::Font2);
    drawFormattedTextFit(x, y, maxWidth, rgb(160, 180, 180), bg, "G%+d T%+d S%+d H%+d F%+d X%+d", item.grit,
                         item.tech, item.scan, item.ghost, item.filter, item.strain);
}

// Draws the common field-photo frame behind every item illustration.
void drawItemImageFrame(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t accent) {
    auto& display = M5.Display;
    display.fillRoundRect(x, y, w, h, 8, rgb(4, 7, 8));
    display.drawRoundRect(x, y, w, h, 8, accent);
    display.fillRoundRect(x + 12, y + 12, w - 24, h - 24, 6, rgb(14, 17, 16));
    display.drawFastHLine(x + 24, y + h - 36, w - 48, rgb(45, 48, 42));
    for (uint8_t i = 0; i < 18; ++i) {
        const uint32_t seed = mapNoise(static_cast<uint32_t>(i) * 733UL + static_cast<uint32_t>(accent) * 17UL);
        const int32_t px = x + 18 + static_cast<int32_t>(seed % static_cast<uint32_t>(w - 36));
        const int32_t py = y + 18 + static_cast<int32_t>((seed >> 11) % static_cast<uint32_t>(h - 36));
        display.drawPixel(px, py, rgb(38, 44, 40));
    }
}

// Draws item-specific generated art using the item's image kind.
void drawItemImage(const Item& item, int32_t x, int32_t y, int32_t w, int32_t h) {
    auto& display = M5.Display;
    const int32_t cx = x + w / 2;
    const int32_t cy = y + h / 2 + 6;

    drawItemImageFrame(x, y, w, h, item.color);

    switch (item.image) {
        case ItemImageKind::PatchMask:
            display.fillRoundRect(cx - 38, cy - 48, 76, 88, 26, rgb(54, 62, 58));
            display.fillRoundRect(cx - 25, cy - 20, 50, 42, 12, rgb(78, 86, 80));
            display.fillCircle(cx - 15, cy - 35, 11, rgb(24, 44, 48));
            display.fillCircle(cx + 15, cy - 35, 11, rgb(24, 44, 48));
            display.drawCircle(cx - 15, cy - 35, 12, rgb(150, 160, 150));
            display.drawCircle(cx + 15, cy - 35, 12, rgb(150, 160, 150));
            display.fillRect(cx - 20, cy + 5, 40, 17, rgb(36, 42, 40));
            display.drawLine(cx - 45, cy - 20, cx - 68, cy + 10, rgb(82, 92, 84));
            display.drawLine(cx + 45, cy - 20, cx + 68, cy + 10, rgb(82, 92, 84));
            break;
        case ItemImageKind::RubberTrench:
            display.fillTriangle(cx, cy - 70, cx - 55, cy + 58, cx + 55, cy + 58, rgb(36, 58, 46));
            display.fillTriangle(cx, cy - 52, cx - 22, cy + 50, cx + 22, cy + 50, rgb(48, 82, 60));
            display.drawLine(cx, cy - 62, cx, cy + 56, rgb(110, 130, 110));
            display.drawLine(cx - 22, cy - 12, cx - 58, cy + 30, rgb(26, 40, 34));
            display.drawLine(cx + 22, cy - 12, cx + 58, cy + 30, rgb(26, 40, 34));
            display.fillRect(cx - 18, cy - 50, 36, 10, rgb(74, 92, 74));
            break;
        case ItemImageKind::MirrorweaveCoat:
            display.fillTriangle(cx, cy - 70, cx - 58, cy + 58, cx + 58, cy + 58, rgb(70, 42, 86));
            display.fillTriangle(cx, cy - 52, cx - 24, cy + 50, cx + 24, cy + 50, rgb(110, 70, 128));
            for (int8_t i = -4; i <= 4; ++i) {
                display.drawLine(cx - 46, cy - 48 + i * 18, cx + 42, cy - 62 + i * 18, rgb(185, 100, 210));
                display.drawLine(cx - 38, cy - 58 + i * 18, cx + 48, cy - 42 + i * 18, rgb(72, 170, 190));
            }
            break;
        case ItemImageKind::CoilDetector:
            display.drawCircle(cx - 28, cy - 18, 38, rgb(120, 92, 55));
            display.drawCircle(cx - 28, cy - 18, 28, rgb(180, 140, 78));
            display.drawCircle(cx - 28, cy - 18, 18, rgb(60, 48, 36));
            drawThickLine(cx + 4, cy + 6, cx + 62, cy + 58, rgb(76, 82, 76), 3);
            display.fillRoundRect(cx + 48, cy + 44, 34, 18, 5, rgb(38, 45, 42));
            display.drawLine(cx - 52, cy - 46, cx + 8, cy + 12, rgb(210, 180, 95));
            break;
        case ItemImageKind::GlassNeedle:
            display.fillTriangle(cx - 10, cy - 70, cx + 18, cy + 42, cx - 38, cy + 42, rgb(44, 92, 96));
            display.drawTriangle(cx - 10, cy - 70, cx + 18, cy + 42, cx - 38, cy + 42, rgb(165, 245, 220));
            display.fillRect(cx - 16, cy + 38, 15, 38, rgb(55, 58, 54));
            display.drawLine(cx + 4, cy - 40, cx - 28, cy + 36, rgb(220, 255, 240));
            display.drawCircle(cx + 30, cy + 16, 12, rgb(80, 230, 190));
            break;
        case ItemImageKind::PrybarKit:
            drawThickLine(cx - 54, cy + 42, cx + 48, cy - 45, rgb(92, 86, 74), 5);
            drawThickLine(cx - 54, cy + 42, cx + 48, cy - 45, rgb(150, 142, 120), 2);
            display.drawCircle(cx + 52, cy - 48, 15, rgb(160, 150, 128));
            display.fillRoundRect(cx - 62, cy - 44, 54, 28, 5, rgb(76, 54, 38));
            display.drawLine(cx - 54, cy - 28, cx - 14, cy - 28, rgb(170, 130, 85));
            break;
        case ItemImageKind::SolderRig:
            display.fillRoundRect(cx - 60, cy + 4, 58, 44, 5, rgb(42, 48, 50));
            display.fillRect(cx - 52, cy + 14, 38, 8, rgb(230, 110, 70));
            drawThickLine(cx - 4, cy + 20, cx + 58, cy - 40, rgb(78, 70, 64), 4);
            drawThickLine(cx + 48, cy - 30, cx + 72, cy - 58, rgb(195, 180, 145), 2);
            display.drawLine(cx - 2, cy + 8, cx + 30, cy + 48, rgb(28, 28, 26));
            display.drawLine(cx + 30, cy + 48, cx + 60, cy + 28, rgb(28, 28, 26));
            break;
        case ItemImageKind::QuietNailgun:
            display.fillRoundRect(cx - 60, cy - 30, 95, 34, 6, rgb(66, 66, 62));
            display.fillRect(cx + 28, cy - 22, 42, 10, rgb(38, 42, 40));
            display.fillTriangle(cx - 20, cy + 0, cx + 22, cy + 0, cx + 0, cy + 55, rgb(48, 50, 48));
            display.fillRect(cx - 48, cy + 4, 22, 42, rgb(54, 44, 38));
            display.drawFastHLine(cx - 52, cy - 18, 78, rgb(128, 132, 120));
            display.fillCircle(cx + 48, cy - 8, 4, rgb(210, 80, 160));
            break;
        case ItemImageKind::RailPistol:
            display.fillRoundRect(cx - 65, cy - 32, 108, 32, 5, rgb(78, 72, 70));
            display.fillRect(cx + 35, cy - 26, 42, 8, rgb(160, 150, 128));
            display.fillRect(cx - 8, cy - 42, 50, 9, rgb(52, 56, 58));
            display.fillTriangle(cx - 24, cy - 2, cx + 20, cy - 2, cx - 4, cy + 56, rgb(56, 46, 44));
            display.drawFastHLine(cx - 56, cy - 16, 84, rgb(230, 75, 92));
            display.drawLine(cx + 42, cy - 12, cx + 78, cy - 12, rgb(230, 210, 160));
            break;
        case ItemImageKind::WarmBattery:
            display.fillRoundRect(cx - 42, cy - 58, 84, 112, 8, rgb(72, 58, 38));
            display.fillRect(cx - 20, cy - 70, 40, 14, rgb(110, 92, 56));
            display.drawRoundRect(cx - 42, cy - 58, 84, 112, 8, rgb(238, 176, 72));
            for (uint8_t i = 0; i < 4; ++i) {
                display.drawFastHLine(cx - 26, cy - 28 + i * 24, 52, rgb(230, 185, 82));
            }
            display.fillCircle(cx, cy + 5, 18, rgb(240, 160, 72));
            break;
        case ItemImageKind::MourningLens:
            display.fillCircle(cx, cy - 6, 58, rgb(18, 24, 34));
            display.fillCircle(cx, cy - 6, 44, rgb(38, 54, 82));
            display.fillCircle(cx - 12, cy - 20, 18, rgb(88, 118, 170));
            display.drawCircle(cx, cy - 6, 60, rgb(132, 150, 190));
            display.drawLine(cx - 36, cy - 44, cx + 42, cy + 34, rgb(190, 205, 220));
            display.drawLine(cx + 8, cy - 56, cx - 22, cy + 42, rgb(90, 110, 150));
            display.fillRoundRect(cx - 24, cy + 52, 48, 16, 5, rgb(50, 45, 42));
            break;
        case ItemImageKind::NullCharm:
            display.fillTriangle(cx, cy - 62, cx - 48, cy + 22, cx + 48, cy + 22, rgb(34, 88, 70));
            display.fillTriangle(cx, cy - 42, cx - 28, cy + 12, cx + 28, cy + 12, rgb(12, 20, 18));
            display.drawTriangle(cx, cy - 62, cx - 48, cy + 22, cx + 48, cy + 22, rgb(120, 230, 170));
            display.drawLine(cx, cy + 22, cx, cy + 68, rgb(94, 120, 110));
            display.fillCircle(cx, cy - 6, 7, rgb(120, 230, 170));
            display.drawCircle(cx, cy - 6, 23, rgb(44, 110, 88));
            break;
        case ItemImageKind::IodineAmpoule:
            display.fillRoundRect(cx - 18, cy - 64, 36, 120, 14, rgb(72, 78, 68));
            display.fillRoundRect(cx - 13, cy - 30, 26, 76, 10, rgb(150, 96, 38));
            display.drawRoundRect(cx - 18, cy - 64, 36, 120, 14, rgb(210, 225, 190));
            display.drawFastHLine(cx - 15, cy - 38, 30, rgb(230, 235, 200));
            display.fillTriangle(cx - 10, cy - 66, cx + 10, cy - 66, cx, cy - 84, rgb(160, 170, 150));
            display.drawLine(cx + 9, cy - 52, cx - 8, cy + 42, rgb(230, 190, 125));
            break;
        case ItemImageKind::CannedCoffee:
            display.fillRoundRect(cx - 34, cy - 58, 68, 116, 10, rgb(105, 66, 42));
            display.fillRect(cx - 31, cy - 34, 62, 56, rgb(160, 100, 58));
            display.drawRoundRect(cx - 34, cy - 58, 68, 116, 10, rgb(190, 150, 105));
            display.drawFastHLine(cx - 28, cy - 46, 56, rgb(210, 180, 130));
            display.drawFastHLine(cx - 28, cy + 48, 56, rgb(65, 48, 38));
            display.fillCircle(cx, cy - 6, 15, rgb(56, 36, 28));
            display.drawCircle(cx, cy - 6, 23, rgb(215, 160, 85));
            break;
        case ItemImageKind::CopperSaint:
            display.fillCircle(cx, cy - 42, 22, rgb(150, 82, 42));
            display.fillRoundRect(cx - 25, cy - 20, 50, 86, 8, rgb(176, 92, 48));
            display.fillTriangle(cx - 25, cy + 12, cx - 66, cy + 56, cx - 20, cy + 48, rgb(120, 64, 38));
            display.fillTriangle(cx + 25, cy + 12, cx + 66, cy + 56, cx + 20, cy + 48, rgb(120, 64, 38));
            display.drawCircle(cx, cy - 42, 28, rgb(230, 150, 70));
            display.drawLine(cx, cy - 10, cx, cy + 52, rgb(80, 45, 32));
            display.fillCircle(cx - 8, cy - 45, 3, rgb(36, 24, 18));
            display.fillCircle(cx + 8, cy - 45, 3, rgb(36, 24, 18));
            break;
    }
}

// Draws a compact item icon for reward and barter rows.
void drawMiniItemIcon(const Item& item, int32_t x, int32_t y, int32_t size, bool selected) {
    auto& display = M5.Display;
    const uint16_t bg = selected ? rgb(24, 34, 35) : rgb(10, 14, 16);
    display.fillRoundRect(x, y, size, size, 6, bg);
    display.drawRoundRect(x, y, size, size, 6, selected ? rgb(120, 240, 190) : item.color);
    display.fillCircle(x + size / 2, y + size / 2, size / 4, item.color);
    switch (item.slot) {
        case Slot::Suit:
            display.drawLine(x + size / 2, y + 8, x + 8, y + size - 8, rgb(180, 190, 180));
            display.drawLine(x + size / 2, y + 8, x + size - 8, y + size - 8, rgb(180, 190, 180));
            break;
        case Slot::Detector:
            display.drawCircle(x + size / 2, y + size / 2, size / 3, rgb(170, 220, 220));
            break;
        case Slot::Tool:
            display.drawLine(x + 10, y + size - 10, x + size - 10, y + 10, rgb(220, 190, 110));
            break;
        case Slot::Weapon:
            display.drawFastHLine(x + 9, y + size / 2, size - 18, rgb(230, 90, 120));
            break;
        case Slot::Artifact:
            display.drawCircle(x + size / 2, y + size / 2, size / 3, rgb(130, 240, 180));
            display.drawCircle(x + size / 2, y + size / 2, size / 5, rgb(40, 80, 60));
            break;
        case Slot::Consumable:
            display.fillRoundRect(x + size / 2 - 6, y + 8, 12, size - 16, 4, rgb(220, 230, 210));
            display.fillRect(x + size / 2 - 4, y + size / 2, 8, size / 3, item.color);
            break;
    }
}

// Forward declaration lets item detail return to the inventory on bad state.
void drawInventoryScreen();

// Converts item use kind into compact detail-panel text.
const char* itemUseKindText(ItemUseKind kind) {
    switch (kind) {
        case ItemUseKind::Equip:
            return "equip";
        case ItemUseKind::Consume:
            return "consume";
        case ItemUseKind::None:
            return "passive";
    }
    return "passive";
}

// Chooses the primary action button label for an item detail card.
const char* itemActionLabel(const Item& item, bool equippedNow) {
    if (equippedNow) {
        return "Equipped";
    }
    if (item.use.label != nullptr && item.use.label[0] != '\0') {
        return item.use.label;
    }
    if (item.use.kind == ItemUseKind::Consume) {
        return "Use";
    }
    if (item.use.kind == ItemUseKind::Equip) {
        return "Equip";
    }
    return "No Use";
}

// Draws the inspectable item card with art, flavor text, stats, and action.
void drawItemDetailScreen() {
    auto& display = M5.Display;
    clearButtons();
    display.fillScreen(rgb(3, 6, 9));
    drawHeader();

    if (!validInventorySlot(selectedInventorySlot)) {
        currentScreen = Screen::Inventory;
        drawInventoryScreen();
        return;
    }

    const int32_t width = display.width();
    const int32_t height = display.height();
    const int32_t margin = 18;
    const int32_t top = 82;
    const int32_t bottomH = 72;
    const int32_t imageW = 340;
    const int32_t imageH = height - top - bottomH - margin;
    const int32_t detailX = margin + imageW + 16;
    const int32_t detailW = width - detailX - margin;
    const uint8_t itemId = static_cast<uint8_t>(inventory[selectedInventorySlot]);
    const Item& item = itemCatalog[itemId];
    const bool equippedNow = isEquippedInventorySlot(static_cast<uint8_t>(selectedInventorySlot));
    const uint16_t bg = rgb(8, 12, 17);

    drawItemImage(item, margin, top, imageW, imageH);

    drawPanel(detailX, top, detailW, imageH, item.color);
    display.setFont(&fonts::Font4);
    drawTextFit(item.name, detailX + 18, top + 16, detailW - 36, TFT_WHITE, bg);

    display.setFont(&fonts::Font2);
    const int32_t metaW = equippedNow ? detailW - 160 : detailW - 40;
    drawFormattedTextFit(detailX + 20, top + 54, metaW, rgb(160, 180, 178), bg, "%s / %s  value %u",
                         slotName(item.slot), item.tag, static_cast<unsigned>(item.value));
    if (equippedNow) {
        drawTextFit("EQUIPPED", detailX + detailW - 116, top + 54, 98, rgb(120, 240, 190), bg);
    }

    display.drawFastHLine(detailX + 18, top + 88, detailW - 36, rgb(55, 70, 70));
    display.setTextColor(rgb(215, 222, 212), bg);
    drawWrappedText(item.description, detailX + 20, top + 112, detailW - 40, 4, rgb(215, 222, 212), bg);

    display.setTextColor(rgb(125, 230, 205), bg);
    drawTextFit("Field Read", detailX + 20, top + 222, detailW - 40, rgb(125, 230, 205), bg);
    drawWrappedText(item.fieldRead, detailX + 20, top + 252, detailW - 40, 3, rgb(170, 188, 184), bg);

    display.drawFastHLine(detailX + 18, top + imageH - 84, detailW - 36, rgb(55, 70, 70));
    drawTextFit("Effects", detailX + 20, top + imageH - 62, 70, rgb(210, 220, 215), bg);
    drawStatDelta(item, detailX + 96, top + imageH - 62, detailW - 132, bg);
    drawFormattedTextFit(detailX + 20, top + imageH - 36, detailW - 40, rgb(145, 160, 158), bg,
                         "use %s  body %+d  dose %+d  attention %+d", itemUseKindText(item.use.kind),
                         item.use.healthDelta, item.use.exposureDelta, item.use.attentionDelta);

    const int32_t buttonY = height - bottomH;
    addButton("Pack", margin, buttonY + 8, 160, 52, UiAction::Inventory, 0, rgb(90, 210, 220));
    addButton("Field", margin + 174, buttonY + 8, 150, 52, UiAction::BackToField, 0, rgb(120, 220, 120));
    const bool actionEnabled = item.use.kind == ItemUseKind::Consume || (item.use.kind == ItemUseKind::Equip && !equippedNow);
    addButton(itemActionLabel(item, equippedNow), width - margin - 230, buttonY + 8, 230, 52, UiAction::EquipOrUse,
              selectedInventorySlot, item.color, actionEnabled);

    for (uint8_t i = 0; i < buttonCount; ++i) {
        drawButton(buttons[i]);
    }
}

// Draws the pack and equipped list, paging through carried items as needed.
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
    drawTextFit("Inventory", listX + 18, top + 16, listW - 36, TFT_WHITE, bg);

    display.setFont(&fonts::Font2);
    display.setTextColor(rgb(150, 168, 170), bg);
    const int32_t listInnerH = height - top - bottomH - margin - 92;
    uint8_t rowsPerPage = static_cast<uint8_t>(listInnerH > rowH ? listInnerH / rowH : 1);
    if (rowsPerPage > kInventoryCapacity) {
        rowsPerPage = kInventoryCapacity;
    }
    normalizeInventoryPage(rowsPerPage);
    const uint8_t inventoryCount = inventoryItemCount();
    const uint8_t maxPage = maxInventoryPage(rowsPerPage);
    drawFormattedTextFit(listX + 18, top + 52, listW - 36, rgb(150, 168, 170), bg,
                         "Tap an item to inspect it.  %u carried  page %u/%u",
                         static_cast<unsigned>(inventoryCount), static_cast<unsigned>(inventoryPage + 1),
                         static_cast<unsigned>(maxPage + 1));

    const uint8_t startRow = static_cast<uint8_t>(inventoryPage * rowsPerPage);
    uint8_t seen = 0;
    uint8_t drawn = 0;
    for (uint8_t i = 0; i < kInventoryCapacity; ++i) {
        if (!validInventorySlot(i)) {
            continue;
        }
        if (seen++ < startRow) {
            continue;
        }
        if (drawn >= rowsPerPage) {
            break;
        }
        const Item& item = itemCatalog[inventory[i]];
        const int32_t rowY = top + 88 + drawn * rowH;
        const bool equippedNow = isEquippedInventorySlot(i);
        const uint16_t rowBg = equippedNow ? rgb(18, 28, 35) : bg;

        display.fillRoundRect(listX + 16, rowY, listW - 32, rowH - 8, 6, rowBg);
        display.drawRoundRect(listX + 16, rowY, listW - 32, rowH - 8, 6, equippedNow ? rgb(100, 230, 180) : item.color);
        display.fillRect(listX + 28, rowY + 13, 10, 24, item.color);
        drawTextFit(item.name, listX + 50, rowY + 8, 198, TFT_WHITE, rowBg);
        drawTextFit(slotName(item.slot), listX + 260, rowY + 8, listW - 408, rgb(150, 168, 170), rowBg);
        drawStatDelta(item, listX + 50, rowY + 30, equippedNow ? listW - 220 : listW - 82, rowBg);
        if (equippedNow) {
            drawTextFit("EQUIPPED", listX + listW - 126, rowY + 18, 100, rgb(120, 240, 190), rowBg);
        }
        addButton("", listX + 16, rowY, listW - 32, rowH - 8, UiAction::InspectItem, i, item.color, true, false);
        ++drawn;
    }

    const int32_t buttonY = height - bottomH;
    addButton("Field", margin, buttonY + 8, 190, 52, UiAction::BackToField, 0, rgb(90, 210, 220));
    addButton("Prev", margin + 204, buttonY + 8, 140, 52, UiAction::InventoryPrev, 0, rgb(170, 120, 240),
              inventoryPage > 0);
    addButton("Next", margin + 358, buttonY + 8, 140, 52, UiAction::InventoryNext, 0, rgb(170, 120, 240),
              inventoryPage < maxPage);
    addButton("Trade", width - margin - 180, buttonY + 8, 180, 52, UiAction::OpenTrade, 0, rgb(230, 180, 70),
              tradeAvailableHere());
    for (uint8_t i = 0; i < buttonCount; ++i) {
        drawButton(buttons[i]);
    }
}

// Draws the post-action receipt so rewards are visible as actual items.
void drawRewardScreen() {
    auto& display = M5.Display;
    clearButtons();
    display.fillScreen(rgb(3, 6, 9));
    drawHeader();

    const int32_t width = display.width();
    const int32_t height = display.height();
    const int32_t margin = 18;
    const int32_t top = 82;
    const int32_t bottomH = 72;
    const int32_t panelH = height - top - bottomH - margin;
    const uint16_t bg = rgb(8, 12, 17);

    drawPanel(margin, top, width - margin * 2, panelH, rgb(120, 210, 180));
    display.setFont(&fonts::Font4);
    drawTextFit(rewardTitle[0] == '\0' ? "Action Complete" : rewardTitle, margin + 18, top + 16, width - 72,
                TFT_WHITE, bg);
    drawWrappedText(rewardSummary[0] == '\0' ? statusLine : rewardSummary, margin + 20, top + 58, width - 76, 4,
                    rgb(210, 222, 214), bg);

    display.setFont(&fonts::Font4);
    drawTextFit("Received", margin + 20, top + 158, width - 76, rgb(130, 230, 200), bg);
    if (rewardCount == 0) {
        display.setFont(&fonts::Font2);
        drawTextFit("No goods recovered. Sometimes the Zone only pays in information.", margin + 22, top + 204,
                    width - 80, rgb(160, 178, 174), bg);
    }

    const int32_t cardW = (width - margin * 2 - 60) / 4;
    for (uint8_t i = 0; i < rewardCount; ++i) {
        const Item& item = itemCatalog[rewardItems[i]];
        const int32_t col = i % 4;
        const int32_t row = i / 4;
        const int32_t cardX = margin + 20 + col * (cardW + 12);
        const int32_t cardY = top + 202 + row * 112;
        display.fillRoundRect(cardX, cardY, cardW, 96, 6, rgb(12, 18, 22));
        display.drawRoundRect(cardX, cardY, cardW, 96, 6, item.color);
        drawMiniItemIcon(item, cardX + 12, cardY + 16, 56, false);
        display.setFont(&fonts::Font2);
        drawTextFit(item.name, cardX + 80, cardY + 18, cardW - 92, TFT_WHITE, rgb(12, 18, 22));
        drawFormattedTextFit(cardX + 80, cardY + 44, cardW - 92, rgb(150, 168, 170), rgb(12, 18, 22),
                             "%s  value %u", item.tag, static_cast<unsigned>(item.value));
    }

    const int32_t buttonY = height - bottomH;
    addButton("Continue", width - margin - 210, buttonY + 8, 210, 52, UiAction::ContinueReward, 0,
              rgb(120, 220, 160));
    addButton("Kit", margin, buttonY + 8, 150, 52, UiAction::Inventory, 0, rgb(170, 120, 240));
    for (uint8_t i = 0; i < buttonCount; ++i) {
        drawButton(buttons[i]);
    }
}

// Draws a two-sided barter window where value must balance before acceptance.
void drawTradeScreen() {
    auto& display = M5.Display;
    clearButtons();
    normalizeTradePage();
    display.fillScreen(rgb(3, 6, 9));
    drawHeader();

    const int32_t width = display.width();
    const int32_t height = display.height();
    const int32_t margin = 18;
    const int32_t top = 82;
    const int32_t bottomH = 72;
    const int32_t panelH = height - top - bottomH - margin;
    const int32_t gap = 16;
    const int32_t leftW = (width - margin * 2 - gap) / 2;
    const int32_t rightX = margin + leftW + gap;
    const int32_t rightW = width - rightX - margin;
    const int32_t rowH = 64;
    const uint16_t bg = rgb(8, 12, 17);

    drawPanel(margin, top, leftW, panelH, rgb(90, 210, 220));
    drawPanel(rightX, top, rightW, panelH, rgb(230, 180, 70));

    display.setFont(&fonts::Font4);
    drawTextFit("Your Offer", margin + 16, top + 16, leftW - 32, TFT_WHITE, bg);
    drawTextFit("Trader Blanket", rightX + 16, top + 16, rightW - 32, TFT_WHITE, bg);
    display.setFont(&fonts::Font2);
    drawFormattedTextFit(margin + 18, top + 52, leftW - 36, rgb(150, 168, 170), bg,
                         "available %d  offered %d  page %u/%u", availableTradeValue(), tradeOfferValue(),
                         static_cast<unsigned>(tradePage + 1), static_cast<unsigned>(maxTradePage() + 1));
    drawFormattedTextFit(rightX + 18, top + 52, rightW - 36, rgb(150, 168, 170), bg,
                         "wanted %d  balance %+d", tradeAskValue(), tradeOfferValue() - tradeAskValue());

    const uint8_t startRow = static_cast<uint8_t>(tradePage * kTradeRowsPerPage);
    uint8_t seen = 0;
    uint8_t drawn = 0;
    for (uint8_t i = 0; i < kInventoryCapacity && drawn < kTradeRowsPerPage; ++i) {
        if (!validInventorySlot(i)) {
            continue;
        }
        if (seen++ < startRow) {
            continue;
        }
        const Item& item = itemCatalog[inventory[i]];
        const bool equipped = isEquippedInventorySlot(i);
        const bool selected = tradeOfferSelected[i];
        const int32_t rowY = top + 88 + drawn * rowH;
        const uint16_t rowBg = selected ? rgb(22, 34, 35) : rgb(12, 18, 22);
        display.fillRoundRect(margin + 14, rowY, leftW - 28, rowH - 8, 6, rowBg);
        display.drawRoundRect(margin + 14, rowY, leftW - 28, rowH - 8, 6,
                              equipped ? rgb(70, 70, 74) : (selected ? rgb(120, 240, 190) : item.color));
        drawMiniItemIcon(item, margin + 24, rowY + 8, 42, selected);
        drawTextFit(item.name, margin + 76, rowY + 8, leftW - 182, equipped ? rgb(120, 126, 128) : TFT_WHITE, rowBg);
        drawFormattedTextFit(margin + 76, rowY + 32, leftW - 182, rgb(150, 168, 170), rowBg, "%s value %u",
                             item.tag, static_cast<unsigned>(item.value));
        drawTextFit(equipped ? "equipped" : (selected ? "offer" : "tap"), margin + leftW - 92, rowY + 20, 62,
                    equipped ? rgb(120, 126, 128) : rgb(120, 240, 190), rowBg);
        addButton("", margin + 14, rowY, leftW - 28, rowH - 8, UiAction::ToggleTradeOffer, i, item.color, !equipped,
                  false);
        ++drawn;
    }

    for (uint8_t i = 0; i < kTradeStockCount; ++i) {
        const Item& item = itemCatalog[tradeStock[i]];
        const bool selected = tradeWantSelected[i];
        const int32_t rowY = top + 88 + i * rowH;
        const uint16_t rowBg = selected ? rgb(34, 28, 20) : rgb(12, 18, 22);
        display.fillRoundRect(rightX + 14, rowY, rightW - 28, rowH - 8, 6, rowBg);
        display.drawRoundRect(rightX + 14, rowY, rightW - 28, rowH - 8, 6,
                              selected ? rgb(230, 190, 90) : item.color);
        drawMiniItemIcon(item, rightX + 24, rowY + 8, 42, selected);
        drawTextFit(item.name, rightX + 76, rowY + 8, rightW - 176, TFT_WHITE, rowBg);
        drawFormattedTextFit(rightX + 76, rowY + 32, rightW - 176, rgb(150, 168, 170), rowBg, "%s value %u",
                             item.tag, static_cast<unsigned>(item.value));
        drawTextFit(selected ? "want" : "tap", rightX + rightW - 88, rowY + 20, 58, rgb(230, 190, 90), rowBg);
        addButton("", rightX + 14, rowY, rightW - 28, rowH - 8, UiAction::ToggleTradeWant, i, item.color, true, false);
    }

    const int32_t buttonY = height - bottomH;
    addButton("Field", margin, buttonY + 8, 140, 52, UiAction::BackToField, 0, rgb(90, 210, 220));
    addButton("Clear", margin + 154, buttonY + 8, 130, 52, UiAction::ClearTrade, 0, rgb(160, 160, 170));
    addButton("Prev", margin + 298, buttonY + 8, 120, 52, UiAction::TradePrev, 0, rgb(170, 120, 240), tradePage > 0);
    addButton("Next", margin + 432, buttonY + 8, 120, 52, UiAction::TradeNext, 0, rgb(170, 120, 240),
              tradePage < maxTradePage());
    addButton("Trade", width - margin - 210, buttonY + 8, 210, 52, UiAction::AcceptTrade, 0, rgb(120, 220, 160),
              tradeCanComplete());

    for (uint8_t i = 0; i < buttonCount; ++i) {
        drawButton(buttons[i]);
    }
}

// Routes the dirty-screen redraw to the active screen renderer.
void drawCurrentScreen() {
    switch (currentScreen) {
        case Screen::Inventory:
            drawInventoryScreen();
            break;
        case Screen::ItemDetail:
            drawItemDetailScreen();
            break;
        case Screen::Map:
            drawMapScreen();
            break;
        case Screen::Reward:
            drawRewardScreen();
            break;
        case Screen::Trade:
            drawTradeScreen();
            break;
        case Screen::Field:
            drawFieldScreen();
            break;
    }
    screenDirty = false;
}

// Resolves item and site palette values after the display is initialized.
void initializeColors() {
    for (uint8_t i = 0; i < kItemCount; ++i) {
        itemCatalog[i].color = rgb(itemCatalog[i].tintR, itemCatalog[i].tintG, itemCatalog[i].tintB);
    }

    sites[0].color = rgb(80, 180, 160);
    sites[1].color = rgb(220, 70, 180);
    sites[2].color = rgb(80, 130, 220);
    sites[3].color = rgb(230, 180, 70);
    sites[4].color = rgb(120, 220, 90);
}

// Seeds the starting game state, inventory, equipment, and site caches.
void initializeGame() {
    warmBatterySpent = false;
    railPistolSpent = false;
    quietNailgunSpent = false;
    mourningLensSpent = false;
    copperSaintSpent = false;
    staticVeilSpent = false;
    debtCoatSpent = false;
    deadPagerSpent = false;
    rainKeySpent = false;
    flareHookSpent = false;
    mercySirenSpent = false;
    apologyEngineSpent = false;
    borrowedShadowSpent = false;
    mercyBellSpent = false;
    calendarSpent = false;
    ghostTeaSite = 255;
    blackIodineGuard = false;
    sleeplessMintReady = false;
    fenceSaltReady = false;
    cheapCourageReady = false;
    blueMilkBlurReady = false;
    for (uint8_t i = 0; i < kInventoryCapacity; ++i) {
        inventory[i] = -1;
        tradeOfferSelected[i] = false;
    }
    for (uint8_t i = 0; i < kEquipSlotCount; ++i) {
        equipped[i] = -1;
    }
    for (uint8_t i = 0; i < kTradeStockCount; ++i) {
        tradeWantSelected[i] = false;
    }
    for (uint8_t i = 0; i < kSiteCount; ++i) {
        siteAttention[i] = 0;
        siteIntel[i] = 0;
        siteCache[i] = sites[i].maxCache;
        siteLead[i] = LeadKind::None;
    }
    for (uint8_t i = 0; i < kSiteCapacity; ++i) {
        nullCharmSpent[i] = false;
        ashfallPonchoSpent[i] = false;
        tarTapeSpent[i] = false;
        signalChalkMark[i] = false;
    }
    for (uint8_t i = 0; i < static_cast<uint8_t>(StoryArc::Count); ++i) {
        storyStage[i] = 0;
        storyOutcome[i] = 0;
    }

    inventory[0] = 0;
    inventory[1] = 3;
    inventory[2] = 5;
    inventory[3] = 7;
    inventory[4] = 12;
    inventory[5] = 13;
    inventory[6] = kBatteryCellItem;
    inventory[7] = kCleanWaterItem;
    inventory[8] = kMedPackItem;
    equipped[0] = 0;
    equipped[1] = 1;
    equipped[2] = 2;
    equipped[3] = 3;
}

// TouchState records the current press and edge-triggered first tap.
struct TouchState {
    bool pressed = false;
    bool justPressed = false;
    int32_t x = 0;
    int32_t y = 0;
};

// Reads and converts raw M5 touch data into screen coordinates.
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

// Sends the first touch on a button to the action dispatcher.
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

// Arduino setup initializes hardware, authored colors, game state, and first draw.
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

// Arduino loop updates touch input and redraws only when game state changes.
void loop() {
    M5.update();
    const TouchState touch = readTouch();
    handleTouch(touch);

    if (screenDirty) {
        drawCurrentScreen();
    }

    delay(12);
}
