#include <Arduino.h>
#include <M5Unified.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "GameContent.h"
#include "GameTypes.h"

namespace {

// The carried pack has a gameplay limit, while the static backing store stays
// larger so the embedded target avoids heap churn and expensive compaction.
constexpr uint8_t kInventoryCapacity = 96;
constexpr uint8_t kPackItemLimit = 20;
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
constexpr uint8_t kMaxEncounterChoices = 4;
constexpr uint8_t kNoItemRequirement = 255;
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
constexpr uint8_t kIodineAmpouleItem = 12;
constexpr uint8_t kCannedCoffeeItem = 13;
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
enum class DawnOfferKind : uint8_t { Work, Rumour, Greed };

struct DawnOffer {
    DawnOfferKind kind = DawnOfferKind::Work;
    uint8_t site = 0;
    LeadKind lead = LeadKind::None;
    uint8_t rewardBiasItem = 0;
    int8_t riskDelta = 0;
    bool active = false;
    const char* title = "";
    char hook[132] = "";
};

struct RuntimeEncounterChoice {
    char label[34] = "";
    char text[190] = "";
    char impact[160] = "";
    uint8_t requiredItem = kNoItemRequirement;
    int8_t skillDelta = 0;
    int8_t targetDelta = 0;
    int8_t doseDelta = 0;
    int8_t attentionDelta = 0;
    int8_t timeDelta = 0;
    bool enabled = true;
    bool consumesItem = false;
};

// Runtime state is intentionally plain globals because the Arduino loop is a
// single-scene program and redraws from this state directly.
Screen currentScreen = Screen::Field;
int16_t health = 9;
int16_t exposure = 0;
uint16_t day = 1;
uint8_t currentSite = 0;
uint8_t selectedMapSite = 1;
int16_t selectedInventorySlot = 0;
UiAction selectedActionDetail = UiAction::Observe;
UiAction pendingEncounterAction = UiAction::Explore;
int8_t activeEncounterChoice = -1;
int8_t selectedDawnOffer = -1;
uint8_t pinnedLeadSite = 255;
LeadKind pinnedLead = LeadKind::None;
uint8_t inventoryPage = 0;
uint8_t itemTextPage = 0;
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
DawnOffer dawnOffers[3];
RuntimeEncounterChoice encounterChoices[kMaxEncounterChoices];
uint8_t encounterChoiceCount = 0;
char encounterTitle[90] = "";
char encounterBody[420] = "";
bool packOverflowed = false;
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
bool iodineShieldReady = false;
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
StoryArc pendingStoryArc = StoryArc::Count;
char statusLine[320] = "The rain tastes metallic. Your kit is the only thing between you and the quiet.";
char rewardTitle[80] = "";
char rewardSummary[320] = "";
char rewardChanged[320] = "";
char rewardNext[220] = "";
char dialogueTitle[80] = "";
char dialogueBody[360] = "";
bool dialogueQueued = false;
Screen dialogueNextScreen = Screen::Field;
uint16_t dialogueAccent = 0;

const uint8_t tradeStock[kTradeStockCount] = {kBatteryCellItem,        kCleanWaterItem,      kMedPackItem,
                                              kIodineAmpouleItem,      kCannedCoffeeItem,    kGhostTeaAmpouleItem,
                                              kBlackIodineStripItem,
                                              kBlueMilkSachetItem};

int16_t dailyUpkeepValue();
void setStatus(const char* text);
bool packHasRoom();
void resolveFieldAction(UiAction action);

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

bool exposureProtectionActive() {
    return iodineShieldReady || blackIodineGuard || blueMilkBlurReady;
}

uint8_t storyIndex(StoryArc arc) {
    return static_cast<uint8_t>(arc);
}

bool hasPendingStoryDecision() {
    return pendingStoryArc != StoryArc::Count;
}

// Tracker labels frame story arcs as in-world rumours instead of menu quests.
const char* storyTitle(StoryArc arc) {
    switch (arc) {
        case StoryArc::BatteriesForTheDead:
            return "Batteries for the Dead";
        case StoryArc::LastSaleAtB2:
            return "The Last Sale at Level B2";
        case StoryArc::PersonWhoNeverEntered:
            return "The Person Who Never Entered";
        case StoryArc::Count:
            return "";
    }
    return "";
}

const char* storyDecisionPrompt(StoryArc arc) {
    switch (arc) {
        case StoryArc::BatteriesForTheDead:
            return "Station Mercy is awake enough to be dangerous. Sister Clamp, Dr. Vell, the market, and the voice in your kit all want different futures for the signal.";
        case StoryArc::LastSaleAtB2:
            return "The Manager Below unlocks Level B2 and demands a customer decision. Hessa needs medicine, Milo needs truth, and the Mall wants to keep selling hunger back to itself.";
        case StoryArc::PersonWhoNeverEntered:
            return "The Other Runner waits in the reeds wearing your outline. The cameras, the clinic, and the market all need one version of you to become useful.";
        case StoryArc::Count:
            return "";
    }
    return "";
}

const char* storyChoiceLabel(StoryArc arc, uint8_t choice) {
    switch (arc) {
        case StoryArc::BatteriesForTheDead: {
            const char* labels[] = {"Preserve", "Silence", "Sell", "Keep Voice"};
            return choice < 4 ? labels[choice] : "";
        }
        case StoryArc::LastSaleAtB2: {
            const char* labels[] = {"Save Family", "Pay Memory", "Feed PA", "Shut PA"};
            return choice < 4 ? labels[choice] : "";
        }
        case StoryArc::PersonWhoNeverEntered: {
            const char* labels[] = {"Erase", "Bargain", "Trade ID", "Sell Face"};
            return choice < 4 ? labels[choice] : "";
        }
        case StoryArc::Count:
            return "";
    }
    return "";
}

const char* storyChoiceText(StoryArc arc, uint8_t choice) {
    switch (arc) {
        case StoryArc::BatteriesForTheDead:
            if (choice == 0) {
                return "Keep Station Mercy powered. Relay observations improve, but the Choir keeps touching sleep.";
            }
            if (choice == 1) {
                return "Burn out the cabinet. Relay heat and exposure pressure fall, but Sister Clamp loses the Choir.";
            }
            if (choice == 2) {
                return "Sell the tuned frequency. Take goods now; Relay Grave gets hotter as smugglers harvest last words.";
            }
            return "Isolate one voice for your kit. Gain a radio voice that helps and haunts in equal measure.";
        case StoryArc::LastSaleAtB2:
            if (choice == 0) {
                return "Recover medicine without feeding the PA. Hessa becomes a Mall contact and the family route stabilizes.";
            }
            if (choice == 1) {
                return "Pay with a memory. Gain a deep Mall artifact; the speakers keep a warm voice.";
            }
            if (choice == 2) {
                return "Help the Manager classify scavengers. Caches improve, but the Mall gets crueler.";
            }
            return "Sever the Manager Below. Heat drops and electronics pay out, but PA rewards vanish.";
        case StoryArc::PersonWhoNeverEntered:
            if (choice == 0) {
                return "Erase the duplicate loop. Spillway heat falls; some people forget useful meetings.";
            }
            if (choice == 1) {
                return "Let the Other Runner help. It can pay bills, but may act like a debt collector with your shadow.";
            }
            if (choice == 2) {
                return "Trade identities. Debt pressure drops, but friendly contacts may know the wrong you.";
            }
            return "Sell the duplicate footage. Take a large payout while your face becomes market property.";
        case StoryArc::Count:
            return "";
    }
    return "";
}

bool storyResolved(StoryArc arc) {
    const uint8_t stage = storyStage[storyIndex(arc)];
    switch (arc) {
        case StoryArc::BatteriesForTheDead:
        case StoryArc::LastSaleAtB2:
            return stage >= 2;
        case StoryArc::PersonWhoNeverEntered:
            return stage >= 3;
        case StoryArc::Count:
            return false;
    }
    return false;
}

const char* storyStatusLabel(StoryArc arc) {
    const uint8_t stage = storyStage[storyIndex(arc)];
    if (storyResolved(arc)) {
        return "resolved";
    }
    if (stage > 0) {
        return "active";
    }
    return "quiet";
}

const char* storyLeadText(StoryArc arc) {
    const uint8_t stage = storyStage[storyIndex(arc)];
    const uint8_t outcome = storyOutcome[storyIndex(arc)];
    switch (arc) {
        case StoryArc::BatteriesForTheDead:
            if (stage == 0) {
                return "Relay Grave has cabinets that answer when the power is fresh.";
            }
            if (stage == 1) {
                return "Sister Clamp wants power for Station Mercy. Follow a Relay Door or Anomaly, or sell the tuned frequency through a Spillway Contact.";
            }
            if (outcome == 2) {
                return "Station Mercy was silenced. Relay Grave is quieter, and Sister Clamp will remember that kind of mercy.";
            }
            if (outcome == 3) {
                return "The frequency was sold. Spillway traders profit from last words while Relay heat climbs.";
            }
            if (outcome == 4) {
                return "One voice lives in the kit now. It says it is not lonely too quickly.";
            }
            return "The Choir was preserved. Relay observations sometimes feel like directions from the dead.";
        case StoryArc::LastSaleAtB2:
            if (stage == 0) {
                return "The Sunken Mall PA still knows customer accounts below the waterline.";
            }
            if (stage == 1) {
                return "The PA has reserved something on Level B2. Follow a Mall Door, Cache, or Anomaly lead.";
            }
            if (outcome == 4) {
                return "The Manager Below was shut down. The Mall is quieter, freer, and less predictable.";
            }
            if (outcome == 3) {
                return "The Manager Below was fed. Mall caches improve, but hunger now has rules.";
            }
            if (outcome == 2) {
                return "A memory paid the bill. The speakers own a warm voice now.";
            }
            return "Hessa's family got medicine without feeding the PA. The Mall remembers a clean sale.";
        case StoryArc::PersonWhoNeverEntered:
            if (stage == 0) {
                return "High-Ghost kit may leave a clinic receipt with the wrong boots under it.";
            }
            if (stage == 1) {
                return "Someone with your face paid the clinic. Ask around the Neon Spillway camera market.";
            }
            if (stage == 2) {
                return "Mink says your face has traffic. Follow the impossible trail into Black Reed Verge.";
            }
            if (outcome == 1) {
                return "The Other Runner bargains from the margins. Sometimes your shadow pays the clinic.";
            }
            if (outcome == 2) {
                return "Identities were traded. The debt lightens, but some contacts know the wrong you.";
            }
            if (outcome == 3) {
                return "The duplicate was sold. Spillway heat will grow wherever your face circulates.";
            }
            return "The Other Runner was erased. Cameras remember less, and so do a few useful people.";
        case StoryArc::Count:
            return "";
    }
    return "";
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

// Adds one item's passive stat profile to a running stat total.
void addItemStats(Stats& stats, const Item& item) {
    stats.grit += item.grit;
    stats.tech += item.tech;
    stats.scan += item.scan;
    stats.ghost += item.ghost;
    stats.filter += item.filter;
    stats.strain += item.strain;
}

// Keeps derived runner stats inside the range the UI and checks expect.
Stats clampStats(Stats stats) {
    stats.grit = clampInt(stats.grit, 0, 9);
    stats.tech = clampInt(stats.tech, 0, 9);
    stats.scan = clampInt(stats.scan, 0, 9);
    stats.ghost = clampInt(stats.ghost, 0, 9);
    stats.filter = clampInt(stats.filter, 0, 9);
    stats.strain = clampInt(stats.strain, 0, 9);
    return stats;
}

// Rebuilds stats with an optional inventory-slot replacement for previewing equipment.
Stats deriveStatsWithReplacement(int16_t replacementInvSlot) {
    Stats stats;
    int8_t replacementEquipSlot = -1;
    if (validInventorySlot(replacementInvSlot)) {
        const Item& replacement = itemCatalog[inventory[replacementInvSlot]];
        if (replacement.use.kind == ItemUseKind::Equip) {
            replacementEquipSlot = equipIndexForSlot(replacement.slot);
        }
    }

    for (uint8_t i = 0; i < kEquipSlotCount; ++i) {
        int16_t invSlot = equipped[i];
        if (replacementEquipSlot == static_cast<int8_t>(i)) {
            invSlot = replacementInvSlot;
        }
        if (validInventorySlot(invSlot)) {
            addItemStats(stats, itemCatalog[inventory[invSlot]]);
        }
    }
    if (exposure >= 4 && !exposureProtectionActive()) {
        --stats.scan;
    }
    if (exposure >= 7 && !exposureProtectionActive()) {
        --stats.grit;
    }
    return clampStats(stats);
}

// Rebuilds the active stat profile from equipped items each time it is needed.
Stats deriveStats() {
    return deriveStatsWithReplacement(-1);
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
            return "Ghost + Tech";
        case LeadKind::Cache:
            return "Grit + Filter";
        case LeadKind::Anomaly:
            return "Scan + Filter";
        case LeadKind::Door:
            return "Tech + Grit";
        case LeadKind::Trail:
            return "Ghost + Scan";
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

uint8_t chanceForThreshold(int16_t skill, int16_t target) {
    const int16_t needed = target - skill;
    if (needed <= 1) {
        return 100;
    }
    if (needed > 6) {
        return 0;
    }
    return static_cast<uint8_t>((7 - needed) * 100 / 6);
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

const char* dawnOfferKindName(DawnOfferKind kind) {
    switch (kind) {
        case DawnOfferKind::Work:
            return "Work";
        case DawnOfferKind::Rumour:
            return "Rumour";
        case DawnOfferKind::Greed:
            return "Greed";
    }
    return "Offer";
}

uint16_t dawnOfferColor(DawnOfferKind kind) {
    switch (kind) {
        case DawnOfferKind::Work:
            return rgb(120, 220, 160);
        case DawnOfferKind::Rumour:
            return rgb(130, 230, 220);
        case DawnOfferKind::Greed:
            return rgb(230, 180, 70);
    }
    return rgb(130, 230, 200);
}

uint8_t nextRumourSite() {
    if (storyStage[storyIndex(StoryArc::BatteriesForTheDead)] == 0) {
        return 3;
    }
    if (storyStage[storyIndex(StoryArc::LastSaleAtB2)] == 0) {
        return 2;
    }
    if (storyStage[storyIndex(StoryArc::PersonWhoNeverEntered)] <= 1) {
        return 1;
    }
    return static_cast<uint8_t>(1 + (day % (kSiteCount - 1)));
}

LeadKind rumourLeadForSite(uint8_t site) {
    if (site == 3) {
        return LeadKind::Contact;
    }
    if (site == 2) {
        return LeadKind::Door;
    }
    if (site == 1) {
        return LeadKind::Contact;
    }
    return LeadKind::Trail;
}

void setDawnOffer(uint8_t index, DawnOfferKind kind, uint8_t site, LeadKind lead, uint8_t rewardBiasItem,
                  int8_t riskDelta, const char* title, const char* hook) {
    if (index >= 3) {
        return;
    }
    DawnOffer& offer = dawnOffers[index];
    offer.kind = kind;
    offer.site = site;
    offer.lead = lead;
    offer.rewardBiasItem = rewardBiasItem;
    offer.riskDelta = riskDelta;
    offer.active = true;
    offer.title = title;
    snprintf(offer.hook, sizeof(offer.hook), "%s", hook);
}

void generateDawnOffers() {
    selectedDawnOffer = -1;
    const uint8_t workSite = static_cast<uint8_t>(1 + ((day + 1) % (kSiteCount - 1)));
    char workHook[132];
    snprintf(workHook, sizeof(workHook), "%s needs a quiet supply run. The clinic tray will be kinder.",
             sites[workSite].name);
    setDawnOffer(0, DawnOfferKind::Work, workSite, LeadKind::Cache, kCleanWaterItem, 0, "Supply Run", workHook);

    const uint8_t rumourSite = nextRumourSite();
    const LeadKind rumourLead = rumourLeadForSite(rumourSite);
    char rumourHook[132];
    snprintf(rumourHook, sizeof(rumourHook), "%s is repeating a story around a %s. Observe or follow it.",
             sites[rumourSite].name, leadName(rumourLead));
    setDawnOffer(1, DawnOfferKind::Rumour, rumourSite, rumourLead, kDeadPagerItem, 0, "Unsettled Rumour", rumourHook);

    const uint8_t greedSite = day % 2 == 0 ? 4 : 3;
    const LeadKind greedLead = greedSite == 4 ? LeadKind::Trail : LeadKind::Anomaly;
    const uint8_t greedItem = greedSite == 4 ? kBlackReedSeedItem : kWhiteReceiptItem;
    char greedHook[132];
    snprintf(greedHook, sizeof(greedHook), "%s has a costly %s with artifact weather around it.",
             sites[greedSite].name, leadName(greedLead));
    setDawnOffer(2, DawnOfferKind::Greed, greedSite, greedLead, greedItem, 1, "Bad Treasure", greedHook);
}

bool hasPinnedLead() {
    return pinnedLeadSite > 0 && pinnedLeadSite < kSiteCount && pinnedLead != LeadKind::None;
}

void clearPinnedLead() {
    pinnedLeadSite = 255;
    pinnedLead = LeadKind::None;
}

bool selectDawnOffer(int16_t index) {
    if (index < 0 || index >= 3 || !dawnOffers[index].active) {
        return false;
    }
    if (selectedDawnOffer == index) {
        return false;
    }

    selectedDawnOffer = static_cast<int8_t>(index);
    const DawnOffer& offer = dawnOffers[index];
    char message[180];
    snprintf(message, sizeof(message), "You mark %s at %s as today's focus.",
             dawnOfferKindName(offer.kind), sites[offer.site].name);
    setStatus(message);
    return true;
}

void pinCurrentLead() {
    if (currentSite == 0 || siteLead[currentSite] == LeadKind::None) {
        setStatus("There is no live lead here to pin.");
        return;
    }

    pinnedLeadSite = currentSite;
    pinnedLead = siteLead[currentSite];
    siteAttention[currentSite] = clampInt(siteAttention[currentSite] + 1, 0, kMaxSiteAttention);
    char message[180];
    snprintf(message, sizeof(message), "You pin the %s at %s. It will survive one dawn, but the site notices.",
             leadName(pinnedLead), sites[pinnedLeadSite].name);
    setStatus(message);
}

void clearPinnedLeadWithStatus() {
    if (!hasPinnedLead()) {
        setStatus("No lead is pinned right now.");
        return;
    }

    char message[160];
    snprintf(message, sizeof(message), "You stop guarding the pinned %s at %s.",
             leadName(pinnedLead), sites[pinnedLeadSite].name);
    clearPinnedLead();
    setStatus(message);
}

// Describes the stat check for the field action forecast panel.
const char* actionCheckText(UiAction action) {
    switch (action) {
        case UiAction::Observe:
            return "Scan + Tech";
        case UiAction::Explore:
            return "Grit + Filter";
        case UiAction::FollowLead:
            if ((siteLead[currentSite] == LeadKind::Contact || siteLead[currentSite] == LeadKind::Door) &&
                equippedCatalogItem(kStaticKnifeItem)) {
                return "Ghost + Tech";
            }
            if (siteLead[currentSite] == LeadKind::Door && equippedCatalogItem(kEmptyNameTagItem) &&
                (currentSite == 1 || currentSite == 2)) {
                return "Ghost + Tech";
            }
            if (siteLead[currentSite] == LeadKind::Contact && equippedCatalogItem(kDebtCollectorsCoatItem) &&
                !debtCoatSpent) {
                return "Grit + Ghost";
            }
            if (siteLead[currentSite] == LeadKind::Door && equippedCatalogItem(kSolderRigItem)) {
                return "Tech + Scan";
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
    if (action == UiAction::Explore && !packHasRoom()) {
        return "pack full";
    }
    if (action == UiAction::Explore && siteCache[currentSite] == 0) {
        return "dry";
    }
    return "";
}

void buildAreaPressureSynopsis(char* buffer, size_t bufferSize) {
    buffer[0] = '\0';
    if (currentSite == 0) {
        snprintf(buffer, bufferSize, "Clinic light is stable. Risk comes from bills, wounds, and what you carry back.");
        return;
    }

    const Site& site = sites[currentSite];
    if (site.risk <= 3) {
        appendAbilityNote(buffer, bufferSize, "The ground is ugly but readable.");
    } else if (site.risk <= 5) {
        appendAbilityNote(buffer, bufferSize, "The place is hostile even before it notices you.");
    } else {
        appendAbilityNote(buffer, bufferSize, "The outer rules are thin here; every action has teeth.");
    }

    if (siteAttention[currentSite] >= 4) {
        appendAbilityNote(buffer, bufferSize, "People and systems are actively watching for your shape.");
    } else if (siteAttention[currentSite] > 0) {
        appendAbilityNote(buffer, bufferSize, "Recent noise has made the site alert.");
    } else {
        appendAbilityNote(buffer, bufferSize, "Nothing has focused on you yet.");
    }

    if (siteIntel[currentSite] >= 2) {
        appendAbilityNote(buffer, bufferSize, "Your notes cut through the worst guesswork.");
    } else if (siteIntel[currentSite] == 1) {
        appendAbilityNote(buffer, bufferSize, "You have one useful read on the place.");
    } else {
        appendAbilityNote(buffer, bufferSize, "You are mostly reading rain and instinct.");
    }

    if (timeTick >= 6) {
        appendAbilityNote(buffer, bufferSize, "Dusk pressure is making every route meaner.");
    }

    if (siteLead[currentSite] != LeadKind::None) {
        appendAbilityNote(buffer, bufferSize, leadWhisper(siteLead[currentSite]));
    }

    if (exposure >= 10 && !exposureProtectionActive()) {
        appendAbilityNote(buffer, bufferSize,
                          "Your hands are shaking; clean outcomes need iodine or blue milk protection.");
    } else if (exposure >= 7 && !exposureProtectionActive()) {
        appendAbilityNote(buffer, bufferSize, "Sick light under the skin makes blind exploration cost more.");
    } else if (exposure >= 4 && !exposureProtectionActive()) {
        appendAbilityNote(buffer, bufferSize, "Buzzing exposure makes anomaly work less trustworthy.");
    }
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
        if (siteLead[currentSite] == LeadKind::Anomaly) {
            target += 1;
        } else if (siteLead[currentSite] == LeadKind::Trail) {
            target -= 1;
        }
    }
    if (exposure >= 4 && action == UiAction::FollowLead && siteLead[currentSite] == LeadKind::Anomaly &&
        !exposureProtectionActive()) {
        target += 1;
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
        if (exposure >= 7 && !exposureProtectionActive()) {
            dose = clampInt(dose + 1, 0, 6);
        }
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
    return chanceForThreshold(skill, target);
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
        return siteCache[currentSite] > 0 && packHasRoom();
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

void queueDialogue(const char* title, const char* body, uint16_t accent) {
    if (!dialogueQueued) {
        snprintf(dialogueTitle, sizeof(dialogueTitle), "%s", title == nullptr ? "Signal" : title);
        snprintf(dialogueBody, sizeof(dialogueBody), "%s", body == nullptr ? "" : body);
        dialogueAccent = accent;
        dialogueQueued = true;
        return;
    }

    appendAbilityNote(dialogueBody, sizeof(dialogueBody), body);
    if (title != nullptr && title[0] != '\0') {
        snprintf(dialogueTitle, sizeof(dialogueTitle), "%s", title);
    }
    dialogueAccent = accent;
}

void openQueuedDialogue(Screen nextScreen) {
    if (dialogueQueued) {
        dialogueNextScreen = nextScreen;
        currentScreen = Screen::Dialogue;
    } else {
        currentScreen = nextScreen;
    }
    screenDirty = true;
}

void continueDialogue() {
    dialogueQueued = false;
    dialogueTitle[0] = '\0';
    dialogueBody[0] = '\0';
    const Screen next = dialogueNextScreen;
    dialogueNextScreen = Screen::Field;
    currentScreen = next;
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

int16_t firstInventorySlotForCatalogItem(uint8_t itemId) {
    for (uint8_t i = 0; i < kInventoryCapacity; ++i) {
        if (inventory[i] == itemId) {
            return i;
        }
    }
    return -1;
}

bool consumeCatalogItem(uint8_t itemId) {
    const int16_t slot = firstInventorySlotForCatalogItem(itemId);
    if (slot < 0 || isEquippedInventorySlot(static_cast<uint8_t>(slot))) {
        return false;
    }
    removeInventorySlot(static_cast<uint8_t>(slot));
    return true;
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

bool packHasRoom() {
    return inventoryItemCount() < kPackItemLimit;
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

// Adds an item to the pack while enforcing the player-facing item-count limit.
// The larger backing store remains an implementation detail for cheap storage.
bool addItem(uint8_t itemId, char* message, size_t messageSize) {
    if (!packHasRoom()) {
        snprintf(message, messageSize, "Found %s, but the pack is full. Something useful stays behind.",
                 itemCatalog[itemId].name);
        return false;
    }
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
    packOverflowed = false;
    rewardTitle[0] = '\0';
    rewardSummary[0] = '\0';
    rewardChanged[0] = '\0';
    rewardNext[0] = '\0';
}

bool activeEncounterChoiceValid() {
    return activeEncounterChoice >= 0 && activeEncounterChoice < static_cast<int8_t>(encounterChoiceCount);
}

RuntimeEncounterChoice* selectedEncounterChoice() {
    return activeEncounterChoiceValid() ? &encounterChoices[activeEncounterChoice] : nullptr;
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
    } else {
        packOverflowed = true;
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

bool itemProtectedFromAutoPay(uint8_t itemId) {
    return itemId == kIodineAmpouleItem || itemId == kMedPackItem || itemId == kGhostTeaAmpouleItem ||
           itemId == kBlackIodineStripItem || itemId == kBlueMilkSachetItem;
}

// Pays a value demand by consuming unequipped items, preferring small goods.
int16_t payTradeValue(int16_t requiredValue) {
    int16_t paid = 0;
    while (paid < requiredValue) {
        int16_t bestSlot = -1;
        uint8_t bestValue = 255;
        for (uint8_t i = 0; i < kInventoryCapacity; ++i) {
            if (!validInventorySlot(i) || isEquippedInventorySlot(i) ||
                itemProtectedFromAutoPay(static_cast<uint8_t>(inventory[i]))) {
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

bool itemCanPayForClinicTreatment(uint8_t itemId) {
    return itemId == kIodineAmpouleItem || itemId == kMedPackItem || itemId == kBlackIodineStripItem ||
           itemId == kBlueMilkSachetItem;
}

int16_t firstClinicTreatmentSlot() {
    for (uint8_t i = 0; i < kInventoryCapacity; ++i) {
        if (validInventorySlot(i) && itemCanPayForClinicTreatment(static_cast<uint8_t>(inventory[i]))) {
            return i;
        }
    }
    return -1;
}

bool clinicTreatmentAvailable() {
    return firstClinicTreatmentSlot() >= 0;
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
        if (itemId == kIodineAmpouleItem) {
            iodineShieldReady = true;
        } else if (itemId == kGhostTeaAmpouleItem) {
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

// Renders or measures a wrapped prose block, skipping whole lines for paged text windows.
uint16_t renderPagedWrappedText(const char* text, int32_t x, int32_t y, int32_t w, uint8_t maxLines,
                                uint16_t firstLine, uint16_t color, uint16_t background, bool render) {
    auto& display = M5.Display;
    display.setFont(&fonts::Font2);
    display.setTextColor(color, background);

    char line[160] = "";
    char word[48] = "";
    uint8_t wordLen = 0;
    uint16_t lineIndex = 0;
    uint8_t drawn = 0;
    const int32_t lineHeight = 22;

    const auto flushLine = [&]() {
        if (line[0] == '\0') {
            return;
        }
        if (render && lineIndex >= firstLine && drawn < maxLines) {
            drawTextFit(line, x, y + drawn * lineHeight, w, color, background);
            ++drawn;
        }
        ++lineIndex;
        line[0] = '\0';
    };

    const auto flushWord = [&]() {
        if (wordLen == 0) {
            return;
        }

        char candidate[208];
        if (line[0] == '\0') {
            snprintf(candidate, sizeof(candidate), "%s", word);
        } else {
            snprintf(candidate, sizeof(candidate), "%s %s", line, word);
        }

        if (display.textWidth(candidate) > w && line[0] != '\0') {
            flushLine();
            snprintf(line, sizeof(line), "%s", word);
        } else {
            snprintf(line, sizeof(line), "%s", candidate);
        }

        wordLen = 0;
        word[0] = '\0';
    };

    for (const char* cursor = text; *cursor != '\0'; ++cursor) {
        if (*cursor == ' ' || *cursor == '\t') {
            flushWord();
        } else if (*cursor == '\n') {
            flushWord();
            flushLine();
        } else if (wordLen < sizeof(word) - 1) {
            word[wordLen++] = *cursor;
            word[wordLen] = '\0';
        }
    }
    flushWord();
    flushLine();

    return lineIndex;
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

void drawDawnOfferCards(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t panelBg) {
    auto& display = M5.Display;
    display.setFont(&fonts::Font4);
    drawTextFit("Morning Board", x, y, w, rgb(130, 230, 200), panelBg);

    const int32_t cardTop = y + 48;
    const int32_t gap = 8;
    const int32_t cardH = (h - 48 - gap * 2) / 3;
    display.setFont(&fonts::Font2);
    for (uint8_t i = 0; i < 3; ++i) {
        const DawnOffer& offer = dawnOffers[i];
        const int32_t cardY = cardTop + i * (cardH + gap);
        const bool selected = selectedDawnOffer == static_cast<int8_t>(i) && offer.active;
        const uint16_t accent = selected ? TFT_WHITE : (offer.active ? dawnOfferColor(offer.kind) : rgb(72, 82, 84));
        const uint16_t stripe = offer.active ? dawnOfferColor(offer.kind) : rgb(72, 82, 84);
        const uint16_t cardBg = selected ? rgb(18, 30, 32) : (offer.active ? rgb(12, 18, 24) : rgb(12, 14, 16));
        display.fillRoundRect(x, cardY, w, cardH, 6, cardBg);
        display.drawRoundRect(x, cardY, w, cardH, 6, accent);
        display.fillRect(x + 10, cardY + 10, 7, cardH - 20, stripe);
        drawFormattedTextFit(x + 26, cardY + 10, w - (selected ? 126 : 52),
                             offer.active ? TFT_WHITE : rgb(120, 126, 128), cardBg,
                             "%s: %s", dawnOfferKindName(offer.kind), offer.title);
        if (selected) {
            drawTextFit("FOCUS", x + w - 72, cardY + 10, 52, rgb(130, 230, 200), cardBg);
        }
        drawFormattedTextFit(x + 26, cardY + 32, w - 52, rgb(150, 168, 170), cardBg, "%s / %s",
                             sites[offer.site].name, leadName(offer.lead));
        drawWrappedText(offer.active ? offer.hook : "The chalk has gone cold.", x + 26, cardY + 54, w - 52,
                        2, offer.active ? rgb(190, 208, 204) : rgb(110, 116, 118), cardBg);
        addButton("", x, cardY, w, cardH, UiAction::SelectDawnOffer, i, stripe, offer.active, false);
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
    if (currentSite == 0) {
        drawDawnOfferCards(x + 16, y + 56, w - 32, h - 76, bg);
        return;
    }

    display.setFont(&fonts::Font2);
    drawFormattedTextFit(x + 16, y + 48, w - 32, rgb(150, 168, 170), bg,
                         "cache %u/%u  intel %u/%u  attention %u/%u", siteCache[currentSite],
                         sites[currentSite].maxCache, siteIntel[currentSite], kMaxSiteIntel,
                         siteAttention[currentSite], kMaxSiteAttention);
    drawFormattedTextFit(x + 16, y + 66, w - 32, rgb(150, 168, 170), bg, "lead: %s", leadName(siteLead[currentSite]));
    char synopsis[260];
    buildAreaPressureSynopsis(synopsis, sizeof(synopsis));
    drawWrappedText(synopsis, x + 16, y + 84, w - 32, 2, rgb(178, 198, 194), bg);

    for (uint8_t i = 0; i < 3; ++i) {
        const UiAction action = actions[i];
        const int32_t rowY = y + 126 + i * 96;
        const bool enabled = fieldActionAvailable(action);
        const uint16_t rowBg = enabled ? rgb(12, 18, 24) : rgb(16, 16, 18);
        const uint16_t border = enabled ? rgb(90, 210, 220) : rgb(58, 58, 62);
        const int16_t skill = actionSkill(action, stats);
        const int16_t target = actionTarget(action);
        uint8_t cleanChance = chanceForThreshold(skill, target + 3);
        if (exposure >= 10 && !exposureProtectionActive()) {
            cleanChance = 0;
        }
        const uint8_t safeChance = chanceForThreshold(skill, target);
        const uint8_t partialChance = chanceForThreshold(skill, target - 2);
        char abilityPreview[220];
        describeActionAbilities(action, siteLead[currentSite], abilityPreview, sizeof(abilityPreview));

        display.fillRoundRect(x + 14, rowY, w - 28, 86, 6, rowBg);
        display.drawRoundRect(x + 14, rowY, w - 28, 86, 6, border);
        drawTextFit(actionLabel(action), x + 26, rowY + 8, 84, enabled ? TFT_WHITE : rgb(110, 115, 120), rowBg);
        drawFormattedTextFit(x + 116, rowY + 8, w - 142, enabled ? rgb(180, 210, 205) : rgb(95, 100, 102), rowBg,
                             "%s %d/%d", actionCheckText(action), skill, target);
        drawFormattedTextFit(x + 26, rowY + 30, w - 52, enabled ? rgb(180, 210, 205) : rgb(95, 100, 102), rowBg,
                             "clean %u%%  safe %u%%  partial+ %u%%", cleanChance, safeChance, partialChance);
        drawFormattedTextFit(x + 26, rowY + 50, 132, enabled ? rgb(180, 210, 205) : rgb(95, 100, 102), rowBg,
                             "%uh  dose +%d",
                             static_cast<unsigned>(actionTimeCost(action) * kTickHours),
                             actionExposureCost(action, stats));
        drawTextFit(enabled ? actionPayoffText(action) : actionBlockedText(action), x + 168, rowY + 50, w - 196,
                    enabled ? rgb(180, 210, 205) : rgb(95, 100, 102), rowBg);
        drawTextFit(abilityPreview[0] != '\0' ? abilityPreview : "The kit stays quiet.", x + 26,
                    rowY + 68, w - 52, enabled ? rgb(150, 188, 184) : rgb(95, 100, 102), rowBg);
        addButton("", x + 14, rowY, w - 28, 86, UiAction::OpenActionDetail, static_cast<int16_t>(action), border,
                  true, false);
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

bool dawnOfferMatchesAction(const DawnOffer& offer, UiAction action, LeadKind lead) {
    if (!offer.active || offer.site != currentSite) {
        return false;
    }
    if (action == UiAction::Observe) {
        return offer.kind == DawnOfferKind::Rumour;
    }
    if (action == UiAction::Explore) {
        return offer.kind != DawnOfferKind::Rumour;
    }
    if (action == UiAction::FollowLead) {
        return offer.lead == lead || offer.lead == LeadKind::None;
    }
    return false;
}

int8_t matchingDawnOffer(UiAction action, LeadKind lead) {
    if (selectedDawnOffer < 0 || selectedDawnOffer >= 3) {
        return -1;
    }

    const DawnOffer& offer = dawnOffers[selectedDawnOffer];
    return dawnOfferMatchesAction(offer, action, lead) ? selectedDawnOffer : -1;
}

void applyDawnOfferReward(UiAction action, OutcomeLevel outcome, LeadKind lead, char* message, size_t messageSize) {
    if (!outcomeHasReward(outcome)) {
        return;
    }
    const int8_t match = matchingDawnOffer(action, lead);
    if (match < 0) {
        return;
    }

    DawnOffer& offer = dawnOffers[match];
    offer.active = false;
    selectedDawnOffer = -1;
    if (offer.kind == DawnOfferKind::Work) {
        grantTradeGoods(3);
        appendAbilityNote(message, messageSize, "The clinic board job pays in dry goods and fewer questions.");
        appendAbilityNote(rewardChanged, sizeof(rewardChanged), "The morning board pays in trade goods.");
    } else if (offer.kind == DawnOfferKind::Rumour) {
        siteIntel[currentSite] = clampInt(siteIntel[currentSite] + 1, 0, kMaxSiteIntel);
        appendAbilityNote(message, messageSize, "The rumour sharpens into a useful mark on the map.");
        appendAbilityNote(rewardChanged, sizeof(rewardChanged), "The board leaves cleaner intel behind.");
    } else {
        if (outcome == OutcomeLevel::Full || outcome == OutcomeLevel::Success) {
            grantRewardItem(offer.rewardBiasItem);
            appendAbilityNote(message, messageSize, "The hinted oddity is real enough to weigh down your pack.");
            appendAbilityNote(rewardChanged, sizeof(rewardChanged), "The dangerous mark pays with an oddity.");
        } else {
            grantTradeGoods(2);
            appendAbilityNote(message, messageSize, "The miracle slips away, but useful fragments remain.");
            appendAbilityNote(rewardChanged, sizeof(rewardChanged), "The dangerous mark pays in fragments.");
        }
        siteAttention[currentSite] = clampInt(siteAttention[currentSite] + 1 + offer.riskDelta, 0, kMaxSiteAttention);
    }
}

void setRewardNextMoveForAction(UiAction action, OutcomeLevel outcome, LeadKind lead) {
    if (rewardCount > 0) {
        const Item& item = itemCatalog[rewardItems[rewardCount - 1]];
        if (item.slot == Slot::Detector) {
            snprintf(rewardNext, sizeof(rewardNext), "%s wants a quiet site and a patient read.", item.name);
            return;
        }
        if (item.slot == Slot::Tool) {
            snprintf(rewardNext, sizeof(rewardNext), "%s belongs near a stubborn door or a nervous cache.", item.name);
            return;
        }
        if (item.slot == Slot::Suit) {
            snprintf(rewardNext, sizeof(rewardNext), "%s will change which routes feel survivable.", item.name);
            return;
        }
        if (item.slot == Slot::Artifact) {
            snprintf(rewardNext, sizeof(rewardNext), "%s is not finished with tomorrow.", item.name);
            return;
        }
        if (item.slot == Slot::Consumable) {
            snprintf(rewardNext, sizeof(rewardNext), "%s is best spent where the city bites hardest.", item.name);
            return;
        }
        snprintf(rewardNext, sizeof(rewardNext), "%s will keep its value when the clinic tray comes out.", item.name);
        return;
    }

    if (action == UiAction::Observe && siteLead[currentSite] != LeadKind::None) {
        snprintf(rewardNext, sizeof(rewardNext), "The %s will not wait politely for dawn.", leadName(siteLead[currentSite]));
    } else if (outcome == OutcomeLevel::Failure) {
        snprintf(rewardNext, sizeof(rewardNext), "The body asks for a cheaper plan before the city does.");
    } else if (action == UiAction::Explore && siteCache[currentSite] == 0) {
        snprintf(rewardNext, sizeof(rewardNext), "This cache is dry until the city forgets who emptied it.");
    } else if (lead != LeadKind::None) {
        snprintf(rewardNext, sizeof(rewardNext), "The %s wants a kit built for that kind of trouble.", leadName(lead));
    } else {
        snprintf(rewardNext, sizeof(rewardNext), "The clinic board will have new handwriting by morning.");
    }
}

// Lowers attention, fades intel, clears most leads, and slowly restocks sites overnight.
bool coolSitesForNewDay() {
    bool preservedPinnedLead = false;
    for (uint8_t i = 1; i < kSiteCount; ++i) {
        siteAttention[i] = siteAttention[i] > 2 ? static_cast<uint8_t>(siteAttention[i] - 2) : 0;
        siteIntel[i] = siteIntel[i] > 0 ? static_cast<uint8_t>(siteIntel[i] - 1) : 0;
        if (hasPinnedLead() && i == pinnedLeadSite) {
            siteLead[i] = pinnedLead;
            preservedPinnedLead = true;
        } else {
            siteLead[i] = LeadKind::None;
        }
        if (siteCache[i] < sites[i].maxCache) {
            ++siteCache[i];
        }
    }
    return preservedPinnedLead;
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
    const uint8_t dawnPinnedSite = pinnedLeadSite;
    const LeadKind dawnPinnedLead = pinnedLead;
    const bool keptPinnedLead = coolSitesForNewDay();
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
    iodineShieldReady = false;
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
        queueDialogue("Out of the Ordinary", bill, rgb(220, 90, 190));
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
    if (keptPinnedLead) {
        char pinNote[140];
        snprintf(pinNote, sizeof(pinNote), "Pinned lead survives: %s at %s.",
                 leadName(dawnPinnedLead), sites[dawnPinnedSite].name);
        appendAbilityNote(message, sizeof(message), pinNote);
        clearPinnedLead();
    }
    if (equippedCatalogItem(kAshfallPonchoItem) && random(0, 100) < 30) {
        for (uint8_t i = 1; i < kSiteCount; ++i) {
            if (siteIntel[i] > 0) {
                --siteIntel[i];
                const char* note = "Ashfall blurs one map note overnight.";
                appendAbilityNote(message, sizeof(message), note);
                queueDialogue("Map Note Lost", note, rgb(150, 160, 165));
                break;
            }
        }
    }
    if (equippedCatalogItem(kSpareHourItem) && random(0, 100) < 35) {
        const uint8_t site = random(1, kSiteCount);
        siteLead[site] = randomLeadForSite(site);
        const char* note = "The Spare Hour mutates a lead while nobody owns the clock.";
        appendAbilityNote(message, sizeof(message), note);
        queueDialogue("Clock Slip", note, rgb(230, 180, 70));
    }
    generateDawnOffers();
    appendAbilityNote(message, sizeof(message), "The clinic board posts work, rumour, and greed for today.");
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
void applyCollapseScar(uint8_t collapseSite, char* message, size_t messageSize) {
    if (collapseSite == 0 || collapseSite >= kSiteCount) {
        appendAbilityNote(message, messageSize, "The clinic saves the body, but the glow stays expensive.");
        return;
    }

    const uint8_t scar = random(0, 4);
    char note[150];
    if (scar == 0 && siteLead[collapseSite] != LeadKind::None) {
        siteLead[collapseSite] = LeadKind::None;
        snprintf(note, sizeof(note), "The clinic saves the body. The lead at %s does not wait.",
                 sites[collapseSite].name);
    } else if (scar <= 1) {
        siteAttention[collapseSite] = clampInt(siteAttention[collapseSite] + 2, 0, kMaxSiteAttention);
        snprintf(note, sizeof(note), "%s learns the shape of your collapse. Attention rises.",
                 sites[collapseSite].name);
    } else if (scar == 2 && siteCache[collapseSite] > 0) {
        --siteCache[collapseSite];
        snprintf(note, sizeof(note), "Someone reaches %s before you wake. One cache is gone.",
                 sites[collapseSite].name);
    } else {
        siteIntel[collapseSite] = siteIntel[collapseSite] > 0 ? static_cast<uint8_t>(siteIntel[collapseSite] - 1) : 0;
        snprintf(note, sizeof(note), "Your notes from %s come back damp, partial, and hard to trust.",
                 sites[collapseSite].name);
    }
    appendAbilityNote(message, messageSize, note);
}

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
        const char* message =
            "The Mercy Bell rings without a clapper. You collapse at the clinic door and the site loses a cache.";
        setStatus(message);
        queueDialogue("Artifact Intervention", message, rgb(230, 180, 70));
        return;
    }

    if (equippedCatalogItem(kCopperSaintItem) && !copperSaintSpent) {
        copperSaintSpent = true;
        health = health <= 0 ? 1 : health;
        exposure = clampInt(exposure, 0, kMaxExposure - 1);
        siteAttention[currentSite] = clampInt(siteAttention[currentSite] + 2, 0, kMaxSiteAttention);
        const char* message =
            "The Copper Saint burns hot enough to hurt. You stay standing at 1 body, and the district notices.";
        setStatus(message);
        queueDialogue("Artifact Intervention", message, rgb(210, 130, 60));
        return;
    }

    const uint8_t collapseSite = currentSite;
    currentSite = 0;
    timeTick = 0;
    if (coolSitesForNewDay()) {
        clearPinnedLead();
    }
    day += 1;
    health = 4;
    exposure = 5;
    const int16_t paid = payTradeValue(8);
    char message[320];
    snprintf(message, sizeof(message),
             "You wake under clinic lights with your boots still wet. Collapse care took goods worth %d and left the glow in your teeth.",
             paid);
    applyCollapseScar(collapseSite, message, sizeof(message));
    setStatus(message);
}

void requestStoryDecision(StoryArc arc, char* message, size_t messageSize) {
    if (storyResolved(arc)) {
        return;
    }
    pendingStoryArc = arc;
    const char* note = "The rumour stops moving and waits for your hands.";
    appendAbilityNote(message, messageSize, note);
    queueDialogue("Decision Point", storyDecisionPrompt(arc), rgb(230, 180, 70));
}

void completeStoryDecision(uint8_t choice) {
    if (!hasPendingStoryDecision() || choice >= 4) {
        currentScreen = Screen::Field;
        screenDirty = true;
        return;
    }

    const StoryArc arc = pendingStoryArc;
    const uint8_t index = storyIndex(arc);
    pendingStoryArc = StoryArc::Count;

    char message[320];
    if (arc == StoryArc::BatteriesForTheDead) {
        storyStage[index] = 2;
        if (choice == 0) {
            storyOutcome[index] = 1;
            siteIntel[3] = kMaxSiteIntel;
            snprintf(message, sizeof(message),
                     "You preserve the Choir. Relay Grave hums like a hospital ward full of names; some are directions, some are bait.");
        } else if (choice == 1) {
            storyOutcome[index] = 2;
            siteAttention[3] = siteAttention[3] > 2 ? static_cast<uint8_t>(siteAttention[3] - 2) : 0;
            exposure = clampInt(exposure - 1, 0, kMaxExposure);
            snprintf(message, sizeof(message),
                     "You silence Station Mercy. The cabinet dies with hot dust; for one honest minute, Relay Grave is wind and metal.");
        } else if (choice == 2) {
            storyOutcome[index] = 3;
            grantTradeGoods(10);
            siteAttention[3] = clampInt(siteAttention[3] + 2, 0, kMaxSiteAttention);
            snprintf(message, sizeof(message),
                     "You sell the frequency. By morning the market rents little radios full of mothers, murderers, and weather.");
        } else {
            storyOutcome[index] = 4;
            if (!hasCatalogItem(kCopperToothRadioItem)) {
                grantRewardItem(kCopperToothRadioItem);
            }
            snprintf(message, sizeof(message),
                     "You keep one voice. It moves into your pocket, says it is not lonely, and says this too quickly.");
        }
    } else if (arc == StoryArc::LastSaleAtB2) {
        storyStage[index] = 2;
        if (choice == 0) {
            storyOutcome[index] = 1;
            siteIntel[2] = kMaxSiteIntel;
            if (!hasCatalogItem(kMedPackItem)) {
                grantRewardItem(kMedPackItem);
            }
            snprintf(message, sizeof(message),
                     "You save Hessa's family cleanly. For the first time in years, the Mall announces a closing time and means mercy.");
        } else if (choice == 1) {
            storyOutcome[index] = 2;
            if (!hasCatalogItem(kTenthButtonItem)) {
                grantRewardItem(kTenthButtonItem);
            }
            exposure = clampInt(exposure + 1, 0, kMaxExposure);
            snprintf(message, sizeof(message),
                     "You pay with a memory. The speakers thank you in a voice that used to belong to somebody warm.");
        } else if (choice == 2) {
            storyOutcome[index] = 3;
            siteCache[2] = clampInt(siteCache[2] + 2, 0, sites[2].maxCache);
            siteAttention[2] = clampInt(siteAttention[2] + 2, 0, kMaxSiteAttention);
            snprintf(message, sizeof(message),
                     "You feed the Manager Below. By dawn, hunger has loyalty cards, armed greeters, and rules.");
        } else {
            storyOutcome[index] = 4;
            siteAttention[2] = siteAttention[2] > 3 ? static_cast<uint8_t>(siteAttention[2] - 3) : 0;
            grantTradeGoods(8);
            snprintf(message, sizeof(message),
                     "You shut down the PA. The final announcement is static; Milo cries because it is finally honest.");
        }
    } else if (arc == StoryArc::PersonWhoNeverEntered) {
        storyStage[index] = 3;
        if (choice == 0) {
            storyOutcome[index] = 0;
            siteAttention[1] = siteAttention[1] > 2 ? static_cast<uint8_t>(siteAttention[1] - 2) : 0;
            if (!hasCatalogItem(kBorrowedShadowItem)) {
                grantRewardItem(kBorrowedShadowItem);
            }
            snprintf(message, sizeof(message),
                     "You erase the Other Runner. By dawn, the cameras remember less, and so do two people who owed favours.");
        } else if (choice == 1) {
            storyOutcome[index] = 1;
            snprintf(message, sizeof(message),
                     "You bargain with the Other Runner. Your shadow learns the clinic route; its courtesy is the problem.");
        } else if (choice == 2) {
            storyOutcome[index] = 2;
            siteIntel[1] = kMaxSiteIntel;
            snprintf(message, sizeof(message),
                     "You trade identities. Nobody can prove you are you; it feels like freedom until the clinic asks who to save.");
        } else {
            storyOutcome[index] = 3;
            grantTradeGoods(12);
            siteAttention[1] = clampInt(siteAttention[1] + 3, 0, kMaxSiteAttention);
            snprintf(message, sizeof(message),
                     "You sell the duplicate. By morning, three people have bought your face, and two wear it better.");
        }
    } else {
        snprintf(message, sizeof(message), "The rumour folds back into the rain.");
    }

    snprintf(rewardTitle, sizeof(rewardTitle), "%s", storyTitle(arc));
    snprintf(rewardSummary, sizeof(rewardSummary), "%s", message);
    snprintf(rewardChanged, sizeof(rewardChanged), "The rumour has teeth now; routes and pressure shift around it.");
    snprintf(rewardNext, sizeof(rewardNext), "The board will not read the same in the morning.");
    setStatus(message);
    currentScreen = Screen::Reward;
    screenDirty = true;
}

void advanceBatteriesForDead(UiAction action, OutcomeLevel outcome, LeadKind lead, char* message, size_t messageSize) {
    const uint8_t arc = storyIndex(StoryArc::BatteriesForTheDead);
    if (!outcomeHasReward(outcome)) {
        return;
    }
    if (storyStage[arc] == 1 && currentSite == 1 && action == UiAction::FollowLead && lead == LeadKind::Contact) {
        requestStoryDecision(StoryArc::BatteriesForTheDead, message, messageSize);
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
        const char* note = "Sister Clamp asks if you brought power for Station Mercy.";
        appendAbilityNote(message, messageSize, note);
        queueDialogue("Rumour Updated", note, rgb(230, 180, 70));
        return;
    }
    if (storyStage[arc] == 1 && action == UiAction::FollowLead &&
        (lead == LeadKind::Door || lead == LeadKind::Anomaly)) {
        requestStoryDecision(StoryArc::BatteriesForTheDead, message, messageSize);
    }
}

void advanceLastSaleAtB2(UiAction action, OutcomeLevel outcome, LeadKind lead, char* message, size_t messageSize) {
    const uint8_t arc = storyIndex(StoryArc::LastSaleAtB2);
    if (currentSite != 2 || !outcomeHasReward(outcome)) {
        return;
    }
    if (storyStage[arc] == 0 && (action == UiAction::Explore || lead == LeadKind::Door)) {
        storyStage[arc] = 1;
        const char* note = "The PA reserves something for your account below the water.";
        appendAbilityNote(message, messageSize, note);
        queueDialogue("Rumour Updated", note, rgb(90, 210, 220));
        return;
    }
    if (storyStage[arc] == 1 && action == UiAction::FollowLead &&
        (lead == LeadKind::Door || lead == LeadKind::Cache || lead == LeadKind::Anomaly)) {
        requestStoryDecision(StoryArc::LastSaleAtB2, message, messageSize);
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
        const char* note = "Mink says your face has traffic.";
        appendAbilityNote(message, messageSize, note);
        queueDialogue("Rumour Updated", note, rgb(220, 90, 190));
        return;
    }
    if (storyStage[arc] == 2 && currentSite == 4 &&
        (lead == LeadKind::Trail || lead == LeadKind::Anomaly || action == UiAction::Explore)) {
        requestStoryDecision(StoryArc::PersonWhoNeverEntered, message, messageSize);
    }
}

void advanceStoryArcs(UiAction action, OutcomeLevel outcome, LeadKind lead, char* message, size_t messageSize) {
    advanceBatteriesForDead(action, outcome, lead, message, messageSize);
    advanceLastSaleAtB2(action, outcome, lead, message, messageSize);
    advancePersonWhoNeverEntered(action, outcome, lead, message, messageSize);
}

void clearEncounterChoices() {
    encounterChoiceCount = 0;
    activeEncounterChoice = -1;
    encounterTitle[0] = '\0';
    encounterBody[0] = '\0';
}

void addEncounterChoice(const char* label, const char* text, const char* impact, int8_t skillDelta,
                        int8_t targetDelta, int8_t doseDelta, int8_t attentionDelta, int8_t timeDelta,
                        bool enabled = true, uint8_t requiredItem = kNoItemRequirement, bool consumesItem = false) {
    if (encounterChoiceCount >= kMaxEncounterChoices) {
        return;
    }

    RuntimeEncounterChoice& choice = encounterChoices[encounterChoiceCount++];
    snprintf(choice.label, sizeof(choice.label), "%s", label);
    snprintf(choice.text, sizeof(choice.text), "%s", text);
    snprintf(choice.impact, sizeof(choice.impact), "%s", impact);
    choice.requiredItem = requiredItem;
    choice.skillDelta = skillDelta;
    choice.targetDelta = targetDelta;
    choice.doseDelta = doseDelta;
    choice.attentionDelta = attentionDelta;
    choice.timeDelta = timeDelta;
    choice.enabled = enabled;
    choice.consumesItem = consumesItem;
}

const char* siteScenePrompt(uint8_t siteIndex, UiAction action, LeadKind lead) {
    if (action == UiAction::Observe) {
        if (siteIndex == 1) {
            return "You settle under a broken advert board. Cameras pan over tarp roofs while vendors pretend not to count your breaths.";
        }
        if (siteIndex == 2) {
            return "You crouch above black water. Somewhere below, the Mall PA tests a dead speaker with your pulse.";
        }
        if (siteIndex == 3) {
            return "You wait among antenna shadows. The cabinets tick like teeth cooling after a meal.";
        }
        if (siteIndex == 4) {
            return "The reeds move late, after the wind is gone. A road sign points toward a road that has not admitted existing.";
        }
        return "You take a long read of the place and let the easy lies pass first.";
    }
    if (action == UiAction::Explore) {
        if (siteIndex == 1) {
            return "A half-shuttered stall leaks blue light and battery stink. Two routes lead in: public and loud, or wet and close to the cameras.";
        }
        if (siteIndex == 2) {
            return "A pharmacy shutter hangs open over waist-high water. Labels drift inside like tiny white fish.";
        }
        if (siteIndex == 3) {
            return "A relay cabinet has opened itself. The floor around it is dry, which is the first bad sign.";
        }
        if (siteIndex == 4) {
            return "The reeds part around a strip of old road. Something has left a clean box of salvage in the centre, as if setting a table.";
        }
        return "The site offers a way in, and does not bother pretending it is safe.";
    }

    if (lead == LeadKind::Contact) {
        return "The contact waits where sightlines cross badly. They know a route, or a price, or a lie with practical value.";
    }
    if (lead == LeadKind::Cache) {
        return "The cache sits behind scrape marks and fresh runoff. Someone hid it in a hurry and left the place listening.";
    }
    if (lead == LeadKind::Anomaly) {
        return "The wrong-light gathers in a corner your eyes keep skipping. Every tool in your kit feels slightly heavier.";
    }
    if (lead == LeadKind::Door) {
        return "The service door still has power behind it. The lock face is wet, warm, and very sure of itself.";
    }
    if (lead == LeadKind::Trail) {
        return "A cleaner footpath bends through the worst ground. It looks useful enough to be bait.";
    }
    return "The lead is thin, but not gone. The choice is how much of yourself to put behind it.";
}

void addItemAwareEncounterChoices(UiAction action, LeadKind lead) {
    if (action == UiAction::Observe) {
        addEncounterChoice("Wait It Out", "Let the scene repeat until the false rhythms expose themselves.",
                           "The city breathes past you instead of through you.", 1, 0, -1, 0, 0);
        addEncounterChoice("Push The Read", "Move closer before the pattern settles. The reward may be clearer, if you are.",
                           "The pattern sharpens because you stand where it can smell you.", 0, -1, 1, 1, 0);
        const bool hasDetector = equippedCatalogItem(kCoilDetectorItem) || equippedCatalogItem(kGlassNeedleItem) ||
                                 equippedCatalogItem(kMothCompassItem) || equippedCatalogItem(kRainCounterItem) ||
                                 equippedCatalogItem(kDeadPagerItem);
        addEncounterChoice("Trust Detector", "Let the kit decide which detail is lying first.",
                           hasDetector ? "The detector chooses which lie deserves your attention."
                                       : "No equipped detector answers the scene.",
                           2, -1, 0, 0, 0, hasDetector);
        addEncounterChoice("Stay Invisible", "Read from bad cover and leave no obvious shape behind.",
                           "Bad cover keeps your outline thin and your certainty thinner.", -1, 1, 0, -1, 0);
        return;
    }

    if (action == UiAction::Explore) {
        addEncounterChoice("Sweep Edges", "Work the perimeter and leave the obvious prize alone until last.",
                           "The edges take longer, but they bite softer.", 0, 1, -1, -1, 1);
        addEncounterChoice("Cut Straight In", "Cross the open ground before fear catches up.",
                           "Speed buys reach and sells your silhouette.", 1, 0, 1, 2, 0);
        const bool hasTool = equippedCatalogItem(kPrybarKitItem) || equippedCatalogItem(kSolderRigItem) ||
                             equippedCatalogItem(kMercyBoltCutterItem) || equippedCatalogItem(kServiceWormItem);
        addEncounterChoice("Use The Tool", "Let your kit turn the obstacle into a method.",
                           hasTool ? "Metal, firmware, and stubborn hands make the obstacle legible."
                                   : "No suitable tool is ready in your hands.",
                           2, -1, 0,
                           hasTool && equippedCatalogItem(kSolderRigItem) ? -1 : 1, 0, hasTool);
        const bool hasBattery = hasCatalogItem(kBatteryCellItem);
        addEncounterChoice("Spend Battery", "Burn a cell to wake a dead panel or blind a cheap alarm.",
                           hasBattery ? "The cell dies hot, and a cheap alarm forgets your face."
                                      : "No Battery Cell waits in the pack.",
                           1,
                           -1, 0, -2, 0, hasBattery, kBatteryCellItem, true);
        return;
    }

    addEncounterChoice("Take It Slow", "Follow the lead on its own terms and accept the time it demands.",
                       "The lead settles when you stop trying to own it.", 1, 0, 0, 0, 0);
    addEncounterChoice("Force It", "Solve the problem before it finishes becoming one.",
                       "The problem opens because you make enough noise to be answered.", 2, 0, 1, 2, 0);

    if (lead == LeadKind::Anomaly) {
        const bool hasIodine = hasCatalogItem(kIodineAmpouleItem) || hasCatalogItem(kBlackIodineStripItem);
        const uint8_t item = hasCatalogItem(kBlackIodineStripItem) ? kBlackIodineStripItem : kIodineAmpouleItem;
        addEncounterChoice("Dose First", "Stain the mouth before touching the impossible edge.",
                           hasIodine ? "The dose takes the first bite so your hands can stay useful."
                                     : "No Iodine or Black Iodine waits in the pack.",
                           2, -2, -2, 0, 0, hasIodine, item, true);
    } else if (lead == LeadKind::Contact) {
        const bool hasTea = hasCatalogItem(kGhostTeaAmpouleItem);
        addEncounterChoice("Drink Ghost Tea", "Arrive as someone easier to forget.",
                           hasTea ? "The tea makes your outline sound like somebody else's weather."
                                  : "No Ghost Tea waits in the pack.",
                           2, -1, 0,
                           -2, 0, hasTea, kGhostTeaAmpouleItem, true);
    } else if (lead == LeadKind::Trail) {
        const bool hasSalt = hasCatalogItem(kFenceRunnersSaltItem);
        addEncounterChoice("Salt The Gloves", "Make your hands remember fences the path has not shown you yet.",
                           hasSalt ? "The salt gives your hands a route your eyes have not earned."
                                   : "No Fence Runner's Salt waits in the pack.",
                           2, -1, -1, -1, 0, hasSalt, kFenceRunnersSaltItem, true);
    } else if (lead == LeadKind::Door) {
        const bool hasDoorTool = equippedCatalogItem(kSolderRigItem) || equippedCatalogItem(kRainKeyItem) ||
                                 equippedCatalogItem(kPrybarKitItem) || equippedCatalogItem(kMercyBoltCutterItem);
        addEncounterChoice("Work The Lock", "Use the kit's favourite kind of violence.",
                           hasDoorTool ? "The lock stops being a wall and becomes a conversation."
                                       : "No door tool is ready in your hands.",
                           2, -1, 0, equippedCatalogItem(kPrybarKitItem) ? 1 : -1, 0, hasDoorTool);
    } else {
        const bool hasWater = hasCatalogItem(kCleanWaterItem);
        addEncounterChoice("Spend Water", "Wash grit from the find before it gets into your hands and lungs.",
                           hasWater ? "Clean water carries the grit away before your lungs learn it."
                                    : "No Clean Water waits in the pack.",
                           1, -1,
                           -2, 0, 0, hasWater, kCleanWaterItem, true);
    }

    addEncounterChoice("Mark And Leave", "Take the smallest useful truth and do not force the rest.",
                       "The mark keeps the thread alive without teaching the site your name.", -1, 1, -1, -2, 0);
}

void buildEncounterForAction(UiAction action) {
    clearEncounterChoices();
    pendingEncounterAction = action;
    const LeadKind lead = siteLead[currentSite];
    snprintf(encounterTitle, sizeof(encounterTitle), "%s: %s", actionLabel(action), sites[currentSite].name);
    snprintf(encounterBody, sizeof(encounterBody), "%s", siteScenePrompt(currentSite, action, lead));
    addItemAwareEncounterChoices(action, lead);
}

void formatEncounterDrift(const RuntimeEncounterChoice& choice, char* out, size_t outSize) {
    out[0] = '\0';
    if (choice.skillDelta > 0) {
        appendAbilityNote(out, outSize, "Odds up.");
    } else if (choice.skillDelta < 0) {
        appendAbilityNote(out, outSize, "Odds down.");
    }
    if (choice.targetDelta < 0) {
        appendAbilityNote(out, outSize, "Wall softens.");
    } else if (choice.targetDelta > 0) {
        appendAbilityNote(out, outSize, "Wall bites.");
    }
    if (choice.doseDelta > 0) {
        appendAbilityNote(out, outSize, "Glow up.");
    } else if (choice.doseDelta < 0) {
        appendAbilityNote(out, outSize, "Glow low.");
    }
    if (choice.attentionDelta > 0) {
        appendAbilityNote(out, outSize, "Eyes gather.");
    } else if (choice.attentionDelta < 0) {
        appendAbilityNote(out, outSize, "Eyes slide.");
    }
    if (choice.timeDelta > 0) {
        appendAbilityNote(out, outSize, "Daylight spent.");
    } else if (choice.timeDelta < 0) {
        appendAbilityNote(out, outSize, "Daylight back.");
    }
    if (out[0] == '\0') {
        snprintf(out, outSize, "Pressure steady.");
    }
}

void applyEncounterItemUse(const RuntimeEncounterChoice& choice) {
    if (!choice.consumesItem || choice.requiredItem == kNoItemRequirement) {
        return;
    }
    if (!consumeCatalogItem(choice.requiredItem)) {
        return;
    }

    if (choice.requiredItem == kIodineAmpouleItem) {
        exposure = clampInt(exposure - 3, 0, kMaxExposure);
        iodineShieldReady = true;
    } else if (choice.requiredItem == kBlackIodineStripItem) {
        exposure = clampInt(exposure - 2, 0, kMaxExposure);
        blackIodineGuard = true;
    } else if (choice.requiredItem == kGhostTeaAmpouleItem) {
        ghostTeaSite = currentSite;
    } else if (choice.requiredItem == kFenceRunnersSaltItem) {
        fenceSaltReady = true;
    } else if (choice.requiredItem == kCleanWaterItem) {
        exposure = clampInt(exposure - 1, 0, kMaxExposure);
    }
}

void openEncounterForAction(UiAction action) {
    if (!fieldActionAvailable(action)) {
        setStatus("That move is not open right now. Either the site is dry, the light is wrong, or you need a lead.");
        return;
    }
    buildEncounterForAction(action);
    currentScreen = Screen::Encounter;
    screenDirty = true;
}

void chooseEncounter(uint8_t index) {
    if (index >= encounterChoiceCount || !encounterChoices[index].enabled) {
        return;
    }
    activeEncounterChoice = static_cast<int8_t>(index);
    applyEncounterItemUse(encounterChoices[index]);
    resolveFieldAction(pendingEncounterAction);
    activeEncounterChoice = -1;
}

const char* siteExploreEncounterText(uint8_t siteIndex, OutcomeLevel outcome) {
    if (siteIndex == 1) {
        return outcome == OutcomeLevel::Failure
                   ? "A checkpoint lens catches your outline through hanging cables."
                   : "A stall runner opens a shutter just wide enough for your hands and not your name.";
    }
    if (siteIndex == 2) {
        return outcome == OutcomeLevel::Failure
                   ? "Mall water lifts a slick of pharmacy labels and old blood around your boots."
                   : "A drowned service counter coughs up stock that was never scanned out.";
    }
    if (siteIndex == 3) {
        return outcome == OutcomeLevel::Failure
                   ? "A dead cabinet begins broadcasting your breathing with a one-second delay."
                   : "The antenna field clicks into a rhythm you can loot between heartbeats.";
    }
    if (siteIndex == 4) {
        return outcome == OutcomeLevel::Failure
                   ? "The reeds close behind you and leave your route looking unused."
                   : "A strip of road appears under the reeds, dry as a held breath.";
    }
    return outcome == OutcomeLevel::Failure ? "The site refuses the shape of your plan."
                                            : "The place yields something because you caught it looking away.";
}

const char* leadEncounterText(LeadKind lead, OutcomeLevel outcome) {
    if (lead == LeadKind::Contact) {
        return outcome == OutcomeLevel::Failure
                   ? "The contact smiles at the wrong person behind you and the meeting curdles."
                   : "The contact speaks in prices, routes, and omissions.";
    }
    if (lead == LeadKind::Cache) {
        return outcome == OutcomeLevel::Failure
                   ? "The cache is real, but the approach is watched and badly timed."
                   : "The cache opens with the wet sigh of a thing tired of being hidden.";
    }
    if (lead == LeadKind::Anomaly) {
        return outcome == OutcomeLevel::Failure
                   ? "The wrong-light notices the probe and folds toward your teeth."
                   : "The anomaly leaves a clean edge where the world forgot to be dangerous.";
    }
    if (lead == LeadKind::Door) {
        return outcome == OutcomeLevel::Failure
                   ? "The service door refuses quietly, then reports the attempt loudly."
                   : "The service door gives up a corridor that smells of hot dust and old authority.";
    }
    if (lead == LeadKind::Trail) {
        return outcome == OutcomeLevel::Failure
                   ? "The trail doubles back through a place your map insists is not there."
                   : "The footpath keeps to the cold ground and lets you borrow its luck.";
    }
    return "The lead has already gone thin in the rain.";
}

void applyFieldEncounterAndRepercussion(UiAction action, OutcomeLevel outcome, LeadKind lead, uint8_t actionSite,
                                        char* message, size_t messageSize) {
    if (actionSite == 0 || actionSite >= kSiteCount) {
        return;
    }

    char encounter[190];
    if (action == UiAction::Explore) {
        snprintf(encounter, sizeof(encounter), "%s", siteExploreEncounterText(actionSite, outcome));
    } else if (action == UiAction::FollowLead) {
        snprintf(encounter, sizeof(encounter), "%s", leadEncounterText(lead, outcome));
    } else {
        snprintf(encounter, sizeof(encounter), "You hold still until %s shows its working parts.",
                 sites[actionSite].name);
    }
    appendAbilityNote(message, messageSize, encounter);

    char impact[180];
    snprintf(impact, sizeof(impact), "%s settles at attention %u/%u; cache %u/%u remains.",
             sites[actionSite].name, siteAttention[actionSite], kMaxSiteAttention, siteCache[actionSite],
             sites[actionSite].maxCache);
    appendAbilityNote(rewardChanged, sizeof(rewardChanged), impact);

    if (action == UiAction::Explore && siteAttention[actionSite] >= 5) {
        if (siteCache[actionSite] > 0) {
            --siteCache[actionSite];
            snprintf(impact, sizeof(impact), "Your noise brings scavengers in behind you; one cache disappears.");
        } else {
            siteIntel[actionSite] =
                siteIntel[actionSite] > 0 ? static_cast<uint8_t>(siteIntel[actionSite] - 1) : 0;
            snprintf(impact, sizeof(impact), "Heat curdles your notes; one useful read goes stale.");
        }
        appendAbilityNote(message, messageSize, impact);
        appendAbilityNote(rewardChanged, sizeof(rewardChanged), impact);
    }

    if (packOverflowed) {
        const char* overflow = "The pack is full; at least one find stays in the rain.";
        appendAbilityNote(message, messageSize, overflow);
        appendAbilityNote(rewardChanged, sizeof(rewardChanged), overflow);
    }
}

// Resolves observe, explore, and lead actions through one risk/reward pipeline.
void resolveFieldAction(UiAction action) {
    if (!fieldActionAvailable(action)) {
        setStatus("That move is not open right now. Either the site is dry, the light is wrong, or you need a lead.");
        return;
    }

    clearReward();
    const Stats stats = deriveStats();
    const uint8_t actionSite = currentSite;
    const Site& site = sites[currentSite];
    const LeadKind lead = siteLead[currentSite];
    char actionTitle[24];
    snprintf(actionTitle, sizeof(actionTitle), "%s", actionLabel(action));
    RuntimeEncounterChoice* encounterChoice = selectedEncounterChoice();
    int16_t skill = actionSkill(action, stats);
    int16_t target = actionTarget(action);
    if (encounterChoice != nullptr) {
        skill += encounterChoice->skillDelta;
        target += encounterChoice->targetDelta;
    }
    target = clampInt(target, 3, 12);
    const int16_t total = skill + random(1, 7);
    int16_t ambientDose = actionExposureCost(action, stats);
    if (encounterChoice != nullptr) {
        ambientDose = clampInt(ambientDose + encounterChoice->doseDelta, 0, 6);
    }
    OutcomeLevel outcome = outcomeForRoll(total, target);
    char abilityNote[220];
    describeActionAbilities(action, lead, abilityNote, sizeof(abilityNote));
    if (encounterChoice != nullptr) {
        char choiceNote[190];
        snprintf(choiceNote, sizeof(choiceNote), "%s", encounterChoice->impact);
        appendAbilityNote(abilityNote, sizeof(abilityNote), choiceNote);
    }
    if (outcome == OutcomeLevel::Full && exposure >= 10 && !exposureProtectionActive()) {
        outcome = OutcomeLevel::Success;
        appendAbilityNote(abilityNote, sizeof(abilityNote),
                          "Sick light keeps this from becoming clean; iodine or blue milk would steady the edge.");
    }

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
    if (encounterChoice != nullptr) {
        attentionGain = clampInt(attentionGain + encounterChoice->attentionDelta, 0, kMaxSiteAttention);
    }
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
                     "%s The pattern gives up a %s. %s",
                     site.observeText, leadName(foundLead), leadWhisper(foundLead));
        } else {
            exposure = clampInt(exposure + 1, 0, kMaxExposure);
            snprintf(message, sizeof(message),
                     "You wait too long in bad cover. Eyes find the shape of you, and the glow creeps under your collar.");
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
                         "You actively explore %s. The place pays in goods worth about %d, then points toward a %s.",
                         site.name, gain, leadName(foundLead));
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
                             "You push through %s. The haul is goods worth about %d and %s.",
                             site.name, gain, itemCatalog[itemId].name);
                } else {
                    grantTradeGoods(static_cast<uint8_t>(itemCatalog[itemId].value / 2));
                    snprintf(message, sizeof(message),
                             "You push through %s. Goods and salvage worth about %d come loose. Cache %u.",
                             site.name, gain, siteCache[currentSite]);
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
                     "%s punishes blind movement. The hit costs %d body and leaves %d exposure behind.",
                     site.name, wound, dose);
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
                         "You approach the quiet contact correctly. Goods worth about %d change hands, and attention softens.",
                         gain);
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
                         "You trace the cleaner footpath. Attention drops, exposure drops, and the ground feels less hungry.");
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
                    snprintf(message, sizeof(message), "You %s the %s. It leaves goods worth about %d and %s.",
                             leadVerb(lead), leadName(lead), gain, itemCatalog[itemId].name);
                } else {
                    grantTradeGoods(static_cast<uint8_t>(gain + itemCatalog[itemId].value / 2));
                    snprintf(message, sizeof(message),
                             "You %s the %s. Goods and valuable fragments worth about %d come free.",
                             leadVerb(lead), leadName(lead), gain);
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
                     "The %s turns bad. The thread snaps; it costs %d body and leaves %d exposure behind.",
                     leadName(lead), wound, dose);
        }
    } else {
        return;
    }

    uint8_t spentTicks = actionTimeCost(action);
    if (encounterChoice != nullptr) {
        spentTicks = static_cast<uint8_t>(clampInt(spentTicks + encounterChoice->timeDelta, 0, maxTimeTicksForDay()));
    }
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
    if (iodineShieldReady) {
        iodineShieldReady = false;
    }
    if (hasPinnedLead() && pinnedLeadSite == actionSite && siteLead[actionSite] != pinnedLead) {
        clearPinnedLead();
    }
    snprintf(rewardChanged, sizeof(rewardChanged),
             "%s at %s; daylight -%uh, exposure +%d, attention +%u.",
             outcomeName(outcome), site.name, static_cast<unsigned>(spentTicks * kTickHours), ambientDose,
             static_cast<unsigned>(attentionGain));
    if (encounterChoice != nullptr) {
        appendAbilityNote(rewardChanged, sizeof(rewardChanged), encounterChoice->impact);
    }
    applyFieldEncounterAndRepercussion(action, outcome, lead, actionSite, message, sizeof(message));
    pendingStoryArc = StoryArc::Count;
    advanceStoryArcs(action, outcome, lead, message, sizeof(message));
    applyDawnOfferReward(action, outcome, lead, message, sizeof(message));
    appendAbilityNote(message, sizeof(message), abilityNote);
    setRewardNextMoveForAction(action, outcome, lead);
    snprintf(rewardTitle, sizeof(rewardTitle), "%s Complete", actionTitle);
    snprintf(rewardSummary, sizeof(rewardSummary), "%s", message);
    setStatus(message);
    spendTime(spentTicks);
    checkCollapse();
    openQueuedDialogue(hasPendingStoryDecision() ? Screen::StoryDecision : Screen::Reward);
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
    spendTime(travelTicks);
    checkCollapse();
    openQueuedDialogue(Screen::Field);
}

// Resting at the clinic heals; resting elsewhere retreats to the safe berth.
void properClinicTreatment() {
    if (currentSite != 0) {
        setStatus("Proper treatment only happens under clinic lights, with a tray and a locked cabinet.");
        return;
    }

    const int16_t supplySlot = firstClinicTreatmentSlot();
    if (supplySlot < 0) {
        setStatus("Proper treatment needs Iodine, Black Iodine, Blue Milk, or a Med Pack. Barter alone stays in the cheap ward.");
        return;
    }

    const uint8_t itemId = static_cast<uint8_t>(inventory[supplySlot]);
    const char* supplyName = itemCatalog[itemId].name;
    removeInventorySlot(static_cast<uint8_t>(supplySlot));
    health = clampInt(health + 5, 0, kMaxHealth);
    exposure = clampInt(exposure - 5, 0, kMaxExposure);
    iodineShieldReady = false;
    blackIodineGuard = false;
    blueMilkBlurReady = false;

    char message[220];
    snprintf(message, sizeof(message),
             "The orderly opens the good cabinet and spends %s. Proper treatment restores body and cuts the glow down.",
             supplyName);
    startNewDay(message);
    checkCollapse();
    openQueuedDialogue(Screen::Field);
}

void restOrRetreat() {
    if (currentSite != 0) {
        currentSite = 0;
        exposure = clampInt(exposure + 1, 0, kMaxExposure);
        setStatus("You cut the run short and spend time limping back to the clinic before the rain gets clever.");
        spendTime(actionTimeCost(UiAction::Rest));
        checkCollapse();
        openQueuedDialogue(Screen::Field);
        return;
    }

    const int16_t paid = payTradeValue(2);
    health = clampInt(health + 3, 0, kMaxHealth);
    exposure = clampInt(exposure - 1, 0, kMaxExposure);
    char message[160];
    snprintf(message, sizeof(message),
             "Cheap rest buys a cot and a dirty drip. Goods worth %d fix what is bleeding; the glow mostly stays.",
             paid);
    startNewDay(message);
    checkCollapse();
    openQueuedDialogue(Screen::Field);
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
    snprintf(rewardChanged, sizeof(rewardChanged), "The blanket comes back lighter in one place and heavier in another.");
    if (packOverflowed) {
        const char* overflow = "The pack will not close; some trade goods stay on the blanket.";
        appendAbilityNote(rewardSummary, sizeof(rewardSummary), overflow);
        appendAbilityNote(rewardChanged, sizeof(rewardChanged), overflow);
    }
    snprintf(rewardNext, sizeof(rewardNext), "New weight changes what the next route can ask of you.");
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
            openEncounterForAction(action);
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
        case UiAction::Tracker:
            currentScreen = Screen::Tracker;
            screenDirty = true;
            break;
        case UiAction::ChooseStory:
            completeStoryDecision(static_cast<uint8_t>(param));
            break;
        case UiAction::ChooseEncounter:
            chooseEncounter(static_cast<uint8_t>(param));
            break;
        case UiAction::OpenActionDetail:
            selectedActionDetail = static_cast<UiAction>(param);
            currentScreen = Screen::ActionDetail;
            screenDirty = true;
            break;
        case UiAction::SelectDawnOffer:
            if (selectDawnOffer(param)) {
                screenDirty = true;
            }
            break;
        case UiAction::PinLead:
            pinCurrentLead();
            screenDirty = true;
            break;
        case UiAction::ClearPinnedLead:
            clearPinnedLeadWithStatus();
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
                itemTextPage = 0;
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
        case UiAction::ItemTextPrev:
            if (itemTextPage > 0) {
                --itemTextPage;
                screenDirty = true;
            }
            break;
        case UiAction::ItemTextNext:
            ++itemTextPage;
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
        case UiAction::ContinueDialogue:
            continueDialogue();
            break;
        case UiAction::Travel:
            travelToSite(static_cast<uint8_t>(param));
            break;
        case UiAction::ProperTreatment:
            properClinicTreatment();
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
    const int32_t buttonW = (width - gap * 8) / 7;
    if (currentSite == 0) {
        addButton("Trade", gap, buttonY + 8, buttonW, 52, UiAction::OpenTrade, 0, rgb(90, 210, 220),
                  tradeAvailableHere());
        addButton("Treat", gap * 2 + buttonW, buttonY + 8, buttonW, 52, UiAction::ProperTreatment, 0,
                  rgb(120, 220, 160), clinicTreatmentAvailable());
        addButton("Board", gap * 3 + buttonW * 2, buttonY + 8, buttonW, 52, UiAction::None, 0,
                  rgb(160, 160, 170), false);
    } else {
        addButton("Observe 2h", gap, buttonY + 8, buttonW, 52, UiAction::Observe, 0, rgb(90, 210, 220),
                  fieldActionAvailable(UiAction::Observe));
        addButton("Explore 4h", gap * 2 + buttonW, buttonY + 8, buttonW, 52, UiAction::Explore, 0,
                  rgb(230, 180, 70), fieldActionAvailable(UiAction::Explore));
        char leadButton[24];
        snprintf(leadButton, sizeof(leadButton), "%s 2h", leadVerb(siteLead[currentSite]));
        addButton(leadButton, gap * 3 + buttonW * 2, buttonY + 8, buttonW, 52, UiAction::FollowLead, 0,
                  rgb(220, 90, 190), fieldActionAvailable(UiAction::FollowLead));
    }
    addButton("Kit", gap * 4 + buttonW * 3, buttonY + 8, buttonW, 52, UiAction::Inventory, 0, rgb(170, 120, 240));
    addButton("Map", gap * 5 + buttonW * 4, buttonY + 8, buttonW, 52, UiAction::Map, 0, rgb(120, 220, 120));
    addButton("Leads", gap * 6 + buttonW * 5, buttonY + 8, buttonW, 52, UiAction::Tracker, 0, rgb(130, 230, 200));
    addButton(currentSite == 0 ? "Rest 2v" : "Retreat", gap * 7 + buttonW * 6, buttonY + 8, buttonW, 52,
              UiAction::Rest, 0, rgb(230, 90, 95));

    for (uint8_t i = 0; i < buttonCount; ++i) {
        drawButton(buttons[i]);
    }
}

void drawChanceBar(int32_t x, int32_t y, int32_t w, const char* label, uint8_t value, uint16_t color,
                   uint16_t bg) {
    auto& display = M5.Display;
    const uint16_t rail = rgb(38, 46, 50);
    const int32_t fillW = (w - 4) * clampInt(value, 0, 100) / 100;

    display.setFont(&fonts::Font2);
    drawTextFit(label, x, y, w - 54, rgb(180, 198, 194), bg);
    drawFormattedTextFit(x + w - 50, y, 48, rgb(180, 198, 194), bg, "%u%%", static_cast<unsigned>(value));
    display.fillRect(x, y + 22, w, 14, bg);
    display.drawRect(x, y + 22, w, 14, rail);
    display.fillRect(x + 2, y + 24, fillW, 10, color);
}

// Draws the expanded action read opened from the compact field forecast rows.
void drawActionDetailScreen() {
    if (currentSite == 0) {
        currentScreen = Screen::Field;
        drawFieldScreen();
        return;
    }

    auto& display = M5.Display;
    clearButtons();
    display.fillScreen(rgb(3, 6, 9));
    drawHeader();

    UiAction action = selectedActionDetail;
    if (action != UiAction::Observe && action != UiAction::Explore && action != UiAction::FollowLead) {
        action = UiAction::Observe;
    }

    const Stats stats = deriveStats();
    const LeadKind lead = siteLead[currentSite];
    const int16_t skill = actionSkill(action, stats);
    const int16_t target = actionTarget(action);
    uint8_t cleanChance = chanceForThreshold(skill, target + 3);
    if (exposure >= 10 && !exposureProtectionActive()) {
        cleanChance = 0;
    }
    const uint8_t safeChance = chanceForThreshold(skill, target);
    const uint8_t partialChance = chanceForThreshold(skill, target - 2);
    const uint8_t failChance = static_cast<uint8_t>(100 - partialChance);
    const int16_t dose = actionExposureCost(action, stats);
    const bool enabled = fieldActionAvailable(action);
    const uint16_t bg = rgb(8, 12, 17);
    const uint16_t accent = enabled ? sites[currentSite].color : rgb(90, 90, 98);

    const int32_t width = display.width();
    const int32_t height = display.height();
    const int32_t margin = 18;
    const int32_t top = 82;
    const int32_t bottomH = 72;
    const int32_t panelH = height - top - bottomH - margin;
    const int32_t innerX = margin + 20;
    const int32_t innerW = width - margin * 2 - 40;
    const int32_t detailTop = top + 100;
    const int32_t gap = 20;
    const int32_t leftW = 370;
    const int32_t rightX = innerX + leftW + gap;
    const int32_t rightW = innerW - leftW - gap;

    drawPanel(margin, top, width - margin * 2, panelH, accent);
    display.setFont(&fonts::Font4);
    drawFormattedTextFit(innerX, top + 16, innerW, TFT_WHITE, bg, "%s at %s", actionLabel(action),
                         sites[currentSite].name);
    display.setFont(&fonts::Font2);
    char detailSubtitle[120];
    if (enabled) {
        snprintf(detailSubtitle, sizeof(detailSubtitle), "%s / threshold %d", actionCheckText(action), target);
    } else {
        snprintf(detailSubtitle, sizeof(detailSubtitle), "%s", actionBlockedText(action));
    }
    drawTextFit(detailSubtitle, innerX, top + 52, innerW, enabled ? rgb(178, 198, 194) : rgb(210, 130, 130), bg);
    display.drawFastHLine(innerX, top + 82, innerW, rgb(55, 70, 70));

    display.fillRoundRect(innerX, detailTop, leftW, 346, 6, rgb(12, 18, 24));
    display.drawRoundRect(innerX, detailTop, leftW, 346, 6, accent);
    display.setFont(&fonts::Font4);
    drawTextFit("Readout", innerX + 16, detailTop + 14, leftW - 32, rgb(130, 230, 200), rgb(12, 18, 24));
    display.setFont(&fonts::Font2);
    drawFormattedTextFit(innerX + 18, detailTop + 54, leftW - 36, rgb(190, 208, 204), rgb(12, 18, 24),
                         "Method: %s", actionCheckText(action));
    drawFormattedTextFit(innerX + 18, detailTop + 78, leftW - 36, rgb(190, 208, 204), rgb(12, 18, 24),
                         "Edge %d / threshold %d", skill, target);
    drawFormattedTextFit(innerX + 18, detailTop + 102, leftW - 36, rgb(150, 168, 170), rgb(12, 18, 24),
                         "Cost %uh   exposure +%d", static_cast<unsigned>(actionTimeCost(action) * kTickHours),
                         dose);
    drawFormattedTextFit(innerX + 18, detailTop + 126, leftW - 36, rgb(150, 168, 170), rgb(12, 18, 24),
                         "Prize: %s", enabled ? actionPayoffText(action) : actionBlockedText(action));

    drawChanceBar(innerX + 18, detailTop + 164, leftW - 36, "Clean", cleanChance, rgb(120, 220, 160),
                  rgb(12, 18, 24));
    drawChanceBar(innerX + 18, detailTop + 210, leftW - 36, "Safe", safeChance, rgb(90, 210, 220),
                  rgb(12, 18, 24));
    drawChanceBar(innerX + 18, detailTop + 256, leftW - 36, "Partial+", partialChance, rgb(230, 180, 70),
                  rgb(12, 18, 24));
    drawChanceBar(innerX + 18, detailTop + 302, leftW - 36, "Failure", failChance, rgb(230, 90, 95),
                  rgb(12, 18, 24));

    char synopsis[300];
    buildAreaPressureSynopsis(synopsis, sizeof(synopsis));
    char abilityPreview[260];
    describeActionAbilities(action, lead, abilityPreview, sizeof(abilityPreview));
    char focusRead[240];
    if (selectedDawnOffer >= 0 && selectedDawnOffer < 3 && dawnOffers[selectedDawnOffer].active) {
        const DawnOffer& offer = dawnOffers[selectedDawnOffer];
        snprintf(focusRead, sizeof(focusRead), "%s at %s. %s",
                 offer.title, sites[offer.site].name,
                 dawnOfferMatchesAction(offer, action, lead) ? "The board leans toward this."
                                                             : "The board is pulling elsewhere.");
    } else {
        focusRead[0] = '\0';
    }

    display.fillRoundRect(rightX, detailTop, rightW, 346, 6, rgb(12, 18, 24));
    display.drawRoundRect(rightX, detailTop, rightW, 346, 6, rgb(90, 210, 220));
    display.setFont(&fonts::Font4);
    drawTextFit("Pressure", rightX + 16, detailTop + 14, rightW - 32, rgb(130, 230, 200), rgb(12, 18, 24));
    display.setFont(&fonts::Font2);
    drawWrappedText(synopsis, rightX + 18, detailTop + 54, rightW - 36, 4, rgb(190, 208, 204), rgb(12, 18, 24));
    drawWrappedText(abilityPreview[0] != '\0' ? abilityPreview : "The kit gives no warning.",
                    rightX + 18, detailTop + 160, rightW - 36, 4, rgb(190, 208, 204), rgb(12, 18, 24));
    if (focusRead[0] != '\0') {
        drawWrappedText(focusRead, rightX + 18, detailTop + 276, rightW - 36, 2, rgb(230, 180, 70),
                        rgb(12, 18, 24));
    }

    const int32_t leadY = detailTop + 350;
    display.fillRoundRect(innerX, leadY, innerW, 72, 6, rgb(12, 18, 24));
    display.drawRoundRect(innerX, leadY, innerW, 72, 6, rgb(220, 90, 190));
    drawFormattedTextFit(innerX + 18, leadY + 14, innerW - 36, rgb(190, 208, 204), rgb(12, 18, 24),
                         "Lead: %s", leadName(lead));
    drawWrappedText(leadWhisper(lead), innerX + 18, leadY + 38, innerW - 36, 1, rgb(150, 168, 170),
                    rgb(12, 18, 24));

    const int32_t buttonY = height - bottomH;
    addButton("Field", margin, buttonY + 8, 150, 52, UiAction::BackToField, 0, rgb(90, 210, 220));
    addButton("Kit", margin + 164, buttonY + 8, 140, 52, UiAction::Inventory, 0, rgb(170, 120, 240));
    const bool canPin = currentSite != 0 && lead != LeadKind::None;
    const bool currentPin = hasPinnedLead() && pinnedLeadSite == currentSite && pinnedLead == lead;
    addButton(currentPin ? "Clear Pin" : "Pin Lead", margin + 318, buttonY + 8, 160, 52,
              currentPin ? UiAction::ClearPinnedLead : UiAction::PinLead, 0, rgb(220, 90, 190), canPin || currentPin);
    addButton("Choose Approach", width - margin - 230, buttonY + 8, 230, 52, action, 0, accent, enabled);

    for (uint8_t i = 0; i < buttonCount; ++i) {
        drawButton(buttons[i]);
    }
}

// Draws the choose-your-own-adventure scene that sits between intent and roll.
void drawEncounterScreen() {
    if (currentSite == 0 || encounterChoiceCount == 0) {
        currentScreen = Screen::Field;
        drawFieldScreen();
        return;
    }

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
    const int32_t innerX = margin + 20;
    const int32_t innerW = width - margin * 2 - 40;
    const uint16_t bg = rgb(8, 12, 17);
    const uint16_t accent = sites[currentSite].color;

    drawPanel(margin, top, width - margin * 2, panelH, accent);
    display.setFont(&fonts::Font4);
    drawTextFit(encounterTitle[0] == '\0' ? "Encounter" : encounterTitle, innerX, top + 16, innerW, TFT_WHITE, bg);
    display.setFont(&fonts::Font2);
    drawWrappedText(encounterBody[0] == '\0' ? sites[currentSite].description : encounterBody, innerX, top + 56,
                    innerW, 3, rgb(210, 222, 214), bg);

    char pressure[220];
    buildAreaPressureSynopsis(pressure, sizeof(pressure));
    drawWrappedText(pressure, innerX, top + 126, innerW, 2, rgb(150, 168, 170), bg);
    display.drawFastHLine(innerX, top + 178, innerW, rgb(55, 70, 70));

    const int32_t cardTop = top + 198;
    const int32_t cardGap = 10;
    const int32_t cardW = (innerW - cardGap) / 2;
    const int32_t cardH = (panelH - 228 - cardGap) / 2;
    for (uint8_t i = 0; i < encounterChoiceCount; ++i) {
        const RuntimeEncounterChoice& choice = encounterChoices[i];
        const int32_t col = i % 2;
        const int32_t row = i / 2;
        const int32_t x = innerX + col * (cardW + cardGap);
        const int32_t y = cardTop + row * (cardH + cardGap);
        const uint16_t choiceAccent = choice.enabled ? (i == 0 ? rgb(90, 210, 220)
                                                     : i == 1 ? rgb(230, 180, 70)
                                                     : i == 2 ? rgb(170, 120, 240)
                                                              : rgb(130, 230, 200))
                                             : rgb(72, 76, 80);
        const uint16_t cardBg = choice.enabled ? rgb(12, 18, 24) : rgb(14, 14, 16);

        display.fillRoundRect(x, y, cardW, cardH, 6, cardBg);
        display.drawRoundRect(x, y, cardW, cardH, 6, choiceAccent);
        display.fillRect(x + 12, y + 12, 8, cardH - 24, choiceAccent);
        display.setFont(&fonts::Font4);
        drawTextFit(choice.label, x + 34, y + 10, cardW - 202, choice.enabled ? TFT_WHITE : rgb(110, 115, 120),
                    cardBg);
        display.setFont(&fonts::Font2);
        drawWrappedText(choice.text, x + 34, y + 42, cardW - 68, 2,
                        choice.enabled ? rgb(190, 208, 204) : rgb(110, 115, 120), cardBg);

        char rules[180];
        if (!choice.enabled && choice.requiredItem != kNoItemRequirement) {
            snprintf(rules, sizeof(rules), "Missing: %s.", itemCatalog[choice.requiredItem].name);
        } else {
            formatEncounterDrift(choice, rules, sizeof(rules));
        }
        drawTextFit(rules, x + 34, y + cardH - 28, cardW - 202,
                    choice.enabled ? rgb(150, 168, 170) : rgb(110, 115, 120), cardBg);
        addButton("Choose", x + cardW - 152, y + cardH - 52, 124, 38, UiAction::ChooseEncounter, i, choiceAccent,
                  choice.enabled);
    }

    const int32_t buttonY = height - bottomH;
    addButton("Field", margin, buttonY + 8, 150, 52, UiAction::BackToField, 0, rgb(90, 210, 220));
    addButton("Kit", margin + 164, buttonY + 8, 140, 52, UiAction::Inventory, 0, rgb(170, 120, 240));
    addButton("Map", margin + 318, buttonY + 8, 140, 52, UiAction::Map, 0, rgb(120, 220, 120));

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

// Appends one named stat delta to a compact item stat line.
void appendStatDeltaText(char* buffer, size_t bufferSize, const char* name, int8_t value, bool includeZeroes) {
    if (!includeZeroes && value == 0) {
        return;
    }
    const size_t used = strlen(buffer);
    if (used >= bufferSize - 1) {
        return;
    }
    snprintf(buffer + used, bufferSize - used, "%s%s %+d", used == 0 ? "" : "  ", name, value);
}

// Draws an item's stat deltas with full names, keeping inventory rows readable.
void drawStatDelta(const Item& item, int32_t x, int32_t y, int32_t maxWidth, uint16_t bg) {
    M5.Display.setFont(&fonts::Font2);
    char stats[160] = "";
    appendStatDeltaText(stats, sizeof(stats), "Grit", item.grit, false);
    appendStatDeltaText(stats, sizeof(stats), "Tech", item.tech, false);
    appendStatDeltaText(stats, sizeof(stats), "Scan", item.scan, false);
    appendStatDeltaText(stats, sizeof(stats), "Ghost", item.ghost, false);
    appendStatDeltaText(stats, sizeof(stats), "Filter", item.filter, false);
    appendStatDeltaText(stats, sizeof(stats), "Strain", item.strain, false);
    if (stats[0] == '\0') {
        snprintf(stats, sizeof(stats), "No passive stat changes");
    }
    drawTextFit(stats, x, y, maxWidth, rgb(160, 180, 180), bg);
}

// Draws the full six-stat spread on item detail cards.
void drawFullItemStats(const Item& item, int32_t x, int32_t y, int32_t maxWidth, uint16_t bg) {
    M5.Display.setFont(&fonts::Font2);
    drawFormattedTextFit(x, y, maxWidth, rgb(185, 205, 200), bg, "Grit %+d  Tech %+d  Scan %+d", item.grit,
                         item.tech, item.scan);
    drawFormattedTextFit(x, y + 22, maxWidth, rgb(185, 205, 200), bg, "Ghost %+d  Filter %+d  Strain %+d",
                         item.ghost, item.filter, item.strain);
}

// Draws the current runner profile in two compact lines with full stat names.
void drawRunnerStatsRows(const Stats& stats, int32_t x, int32_t y, int32_t maxWidth, uint16_t color, uint16_t bg) {
    M5.Display.setFont(&fonts::Font2);
    drawFormattedTextFit(x, y, maxWidth, color, bg, "Grit %d  Tech %d  Scan %d", stats.grit, stats.tech, stats.scan);
    drawFormattedTextFit(x, y + 22, maxWidth, color, bg, "Ghost %d  Filter %d  Strain %d", stats.ghost,
                         stats.filter, stats.strain);
}

// Draws the net change from replacing the currently equipped item in the same slot.
void drawRunnerNetRows(const Stats& current, const Stats& preview, int32_t x, int32_t y, int32_t maxWidth,
                       uint16_t bg) {
    M5.Display.setFont(&fonts::Font2);
    const uint16_t color = rgb(190, 220, 210);
    drawFormattedTextFit(x, y, maxWidth, color, bg, "Grit %+d  Tech %+d  Scan %+d", preview.grit - current.grit,
                         preview.tech - current.tech, preview.scan - current.scan);
    drawFormattedTextFit(x, y + 22, maxWidth, color, bg, "Ghost %+d  Filter %+d  Strain %+d",
                         preview.ghost - current.ghost, preview.filter - current.filter,
                         preview.strain - current.strain);
}

// Draws all runner stats in one fixed-height line for compact preview tables.
void drawRunnerStatsLine(const char* label, const Stats& stats, int32_t x, int32_t y, int32_t maxWidth,
                         uint16_t color, uint16_t bg) {
    M5.Display.setFont(&fonts::Font2);
    drawFormattedTextFit(x, y, maxWidth, color, bg, "%s: Grit %d  Tech %d  Scan %d  Ghost %d  Filter %d  Strain %d",
                         label, stats.grit, stats.tech, stats.scan, stats.ghost, stats.filter, stats.strain);
}

// Draws the replacement delta in one fixed-height line so it cannot collide with effects.
void drawRunnerNetLine(const char* label, const Stats& current, const Stats& preview, int32_t x, int32_t y,
                       int32_t maxWidth, uint16_t bg) {
    M5.Display.setFont(&fonts::Font2);
    drawFormattedTextFit(x, y, maxWidth, rgb(190, 220, 210), bg,
                         "%s: Grit %+d  Tech %+d  Scan %+d  Ghost %+d  Filter %+d  Strain %+d", label,
                         preview.grit - current.grit, preview.tech - current.tech, preview.scan - current.scan,
                         preview.ghost - current.ghost, preview.filter - current.filter,
                         preview.strain - current.strain);
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
    const bool canPreviewEquip = item.use.kind == ItemUseKind::Equip && equipIndexForSlot(item.slot) >= 0;
    const Stats currentStats = deriveStats();
    const Stats previewStats = canPreviewEquip ? deriveStatsWithReplacement(selectedInventorySlot) : currentStats;
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

    const int32_t effectsH = 156;
    const int32_t previewH = 100;
    const int32_t effectsTop = top + imageH - effectsH;
    const int32_t previewTop = effectsTop - previewH - 10;
    const int32_t readTitleY = top + 104;
    const int32_t readBoxY = top + 134;
    const int32_t readBoxH = previewTop - readBoxY - 14;
    const int32_t readX = detailX + 20;
    const int32_t readW = detailW - 40;
    const uint16_t readBg = rgb(10, 16, 20);
    const int32_t readableH = readBoxH > 18 ? readBoxH - 18 : 22;
    uint8_t readLines = static_cast<uint8_t>(readableH / 22);
    if (readLines < 1) {
        readLines = 1;
    }
    char readText[640];
    snprintf(readText, sizeof(readText), "%s\n\n%s", item.description, item.fieldRead);
    const uint16_t totalReadLines =
        renderPagedWrappedText(readText, readX + 12, readBoxY + 10, readW - 24, readLines, 0,
                               rgb(205, 218, 212), readBg, false);
    const uint8_t maxTextPage = totalReadLines <= readLines ? 0 : static_cast<uint8_t>((totalReadLines - 1) / readLines);
    if (itemTextPage > maxTextPage) {
        itemTextPage = maxTextPage;
    }
    const bool textNeedsPaging = maxTextPage > 0;

    display.drawFastHLine(detailX + 18, top + 88, detailW - 36, rgb(55, 70, 70));
    display.setFont(&fonts::Font2);
    drawTextFit("Object Read", readX, readTitleY, textNeedsPaging ? readW - 250 : readW, rgb(125, 230, 205), bg);
    if (textNeedsPaging) {
        drawFormattedTextFit(detailX + detailW - 260, readTitleY + 4, 72, rgb(150, 168, 170), bg, "%u/%u",
                             static_cast<unsigned>(itemTextPage + 1), static_cast<unsigned>(maxTextPage + 1));
        addButton("Prev", detailX + detailW - 176, readTitleY - 6, 76, 36, UiAction::ItemTextPrev, 0, item.color,
                  itemTextPage > 0);
        addButton("Next", detailX + detailW - 92, readTitleY - 6, 72, 36, UiAction::ItemTextNext, 0, item.color,
                  itemTextPage < maxTextPage);
    }
    display.fillRoundRect(readX, readBoxY, readW, readBoxH, 6, readBg);
    display.drawRoundRect(readX, readBoxY, readW, readBoxH, 6, rgb(40, 58, 60));
    renderPagedWrappedText(readText, readX + 12, readBoxY + 10, readW - 24, readLines,
                           static_cast<uint16_t>(itemTextPage) * readLines, rgb(205, 218, 212), readBg, true);

    display.drawFastHLine(detailX + 18, previewTop, detailW - 36, rgb(55, 70, 70));
    drawTextFit("Runner", detailX + 20, previewTop + 16, detailW - 40, rgb(125, 230, 205), bg);
    drawRunnerStatsLine("Current", currentStats, detailX + 20, previewTop + 46, detailW - 40, rgb(220, 230, 225), bg);
    if (canPreviewEquip) {
        drawRunnerNetLine(equippedNow ? "Net" : "Net if equipped", currentStats, previewStats, detailX + 20,
                          previewTop + 70, detailW - 40, bg);
    } else {
        drawTextFit("No slot changes. The body keeps the receipt.", detailX + 20, previewTop + 70, detailW - 40,
                    rgb(190, 220, 210), bg);
    }

    display.drawFastHLine(detailX + 18, effectsTop, detailW - 36, rgb(55, 70, 70));
    drawTextFit("Pull", detailX + 20, effectsTop + 14, detailW - 40, rgb(210, 220, 215), bg);
    drawTextFit("Carried", detailX + 20, effectsTop + 38, detailW - 40, rgb(150, 168, 170), bg);
    drawFullItemStats(item, detailX + 20, effectsTop + 60, detailW - 40, bg);
    drawTextFit("Spent", detailX + 20, effectsTop + 108, detailW - 40, rgb(150, 168, 170), bg);
    drawFormattedTextFit(detailX + 20, effectsTop + 130, detailW - 40, rgb(185, 205, 200), bg,
                         "%s  Body %+d  Dose %+d  Attention %+d", itemUseKindText(item.use.kind),
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
                         "Pack %u/%u  page %u/%u",
                         static_cast<unsigned>(inventoryCount), static_cast<unsigned>(kPackItemLimit),
                         static_cast<unsigned>(inventoryPage + 1), static_cast<unsigned>(maxPage + 1));

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
    drawTextFit(rewardTitle[0] == '\0' ? "Field Note" : rewardTitle, margin + 18, top + 16, width - 72,
                TFT_WHITE, bg);
    drawWrappedText(rewardSummary[0] == '\0' ? statusLine : rewardSummary, margin + 20, top + 58, width - 76, 3,
                    rgb(210, 222, 214), bg);
    if (rewardChanged[0] != '\0') {
        drawWrappedText(rewardChanged, margin + 20, top + 122, width - 76, 2, rgb(230, 180, 70), bg);
    }
    if (rewardNext[0] != '\0') {
        drawWrappedText(rewardNext, margin + 20, top + 174, width - 76, 2, rgb(130, 230, 200), bg);
    }

    display.setFont(&fonts::Font4);
    drawTextFit("Recovered", margin + 20, top + 218, width - 76, rgb(130, 230, 200), bg);
    if (rewardCount == 0) {
        display.setFont(&fonts::Font2);
        drawTextFit("Nothing fits your hands. Sometimes the Zone pays in information.", margin + 22, top + 260,
                    width - 80, rgb(160, 178, 174), bg);
    }

    const int32_t cardW = (width - margin * 2 - 60) / 4;
    for (uint8_t i = 0; i < rewardCount; ++i) {
        const Item& item = itemCatalog[rewardItems[i]];
        const int32_t col = i % 4;
        const int32_t row = i / 4;
        const int32_t cardX = margin + 20 + col * (cardW + 12);
        const int32_t cardY = top + 258 + row * 112;
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

// Draws the in-world rumour board that tracks side-story progress.
void drawTrackerScreen() {
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
    const uint16_t bg = rgb(8, 12, 17);

    uint8_t activeCount = 0;
    uint8_t resolvedCount = 0;
    for (uint8_t i = 0; i < static_cast<uint8_t>(StoryArc::Count); ++i) {
        const StoryArc arc = static_cast<StoryArc>(i);
        if (storyResolved(arc)) {
            ++resolvedCount;
        } else if (storyStage[i] > 0) {
            ++activeCount;
        }
    }

    drawPanel(margin, top, width - margin * 2, contentH, rgb(130, 230, 200));
    display.setFont(&fonts::Font4);
    drawTextFit("Rumour Board", margin + 18, top + 16, width - margin * 2 - 36, TFT_WHITE, bg);
    display.setFont(&fonts::Font2);
    drawFormattedTextFit(margin + 20, top + 54, width - margin * 2 - 40, rgb(160, 180, 178), bg,
                         "active %u  resolved %u  quiet %u", activeCount, resolvedCount,
                         static_cast<unsigned>(static_cast<uint8_t>(StoryArc::Count) - activeCount - resolvedCount));

    const int32_t cardGap = 14;
    const int32_t cardH = (contentH - 98 - cardGap * 2) / 3;
    const int32_t cardX = margin + 16;
    const int32_t cardW = width - margin * 2 - 32;
    const int32_t cardTop = top + 86;
    for (uint8_t i = 0; i < static_cast<uint8_t>(StoryArc::Count); ++i) {
        const StoryArc arc = static_cast<StoryArc>(i);
        const int32_t y = cardTop + i * (cardH + cardGap);
        const bool resolved = storyResolved(arc);
        const bool active = storyStage[i] > 0 && !resolved;
        const uint16_t accent = resolved ? rgb(120, 220, 160) : active ? rgb(230, 180, 70) : rgb(72, 92, 104);
        const uint16_t cardBg = active ? rgb(14, 20, 24) : bg;

        display.fillRoundRect(cardX, y, cardW, cardH, 6, cardBg);
        display.drawRoundRect(cardX, y, cardW, cardH, 6, accent);
        display.fillRect(cardX + 12, y + 14, 8, cardH - 28, accent);

        display.setFont(&fonts::Font4);
        drawTextFit(storyTitle(arc), cardX + 32, y + 12, cardW - 180, TFT_WHITE, cardBg);
        display.setFont(&fonts::Font2);
        drawTextFit(storyStatusLabel(arc), cardX + cardW - 126, y + 18, 96, accent, cardBg);
        drawWrappedText(storyLeadText(arc), cardX + 32, y + 50, cardW - 64, 3, rgb(178, 198, 194), cardBg);
    }

    const int32_t buttonY = height - bottomH;
    addButton("Field", margin, buttonY + 8, 160, 52, UiAction::BackToField, 0, rgb(90, 210, 220));
    addButton("Map", margin + 174, buttonY + 8, 150, 52, UiAction::Map, 0, rgb(120, 220, 120));
    addButton("Kit", margin + 338, buttonY + 8, 150, 52, UiAction::Inventory, 0, rgb(170, 120, 240));

    for (uint8_t i = 0; i < buttonCount; ++i) {
        drawButton(buttons[i]);
    }
}

// Draws a forced story branch once an arc reaches its decision point.
void drawStoryDecisionScreen() {
    if (!hasPendingStoryDecision()) {
        currentScreen = Screen::Tracker;
        drawTrackerScreen();
        return;
    }

    auto& display = M5.Display;
    clearButtons();
    display.fillScreen(rgb(3, 6, 9));
    drawHeader();

    const StoryArc arc = pendingStoryArc;
    const int32_t width = display.width();
    const int32_t height = display.height();
    const int32_t margin = 18;
    const int32_t top = 82;
    const int32_t bottomH = 72;
    const int32_t contentH = height - top - bottomH - margin;
    const uint16_t bg = rgb(8, 12, 17);

    drawPanel(margin, top, width - margin * 2, contentH, rgb(230, 180, 70));
    display.setFont(&fonts::Font4);
    drawTextFit(storyTitle(arc), margin + 18, top + 16, width - margin * 2 - 36, TFT_WHITE, bg);
    display.setFont(&fonts::Font2);
    drawWrappedText(storyDecisionPrompt(arc), margin + 20, top + 58, width - margin * 2 - 40, 3,
                    rgb(190, 208, 204), bg);

    display.drawFastHLine(margin + 18, top + 132, width - margin * 2 - 36, rgb(70, 86, 82));
    drawTextFit("The rumour wants a shape", margin + 20, top + 148, width - margin * 2 - 40,
                rgb(130, 230, 200), bg);

    const int32_t choiceGap = 10;
    const int32_t choiceTop = top + 180;
    const int32_t choiceH = (contentH - 112 - choiceGap * 3) / 4;
    const int32_t choiceX = margin + 16;
    const int32_t choiceW = width - margin * 2 - 32;
    for (uint8_t i = 0; i < 4; ++i) {
        const int32_t y = choiceTop + i * (choiceH + choiceGap);
        const uint16_t accent = i == 0 ? rgb(120, 220, 160)
                                : i == 1 ? rgb(120, 180, 235)
                                : i == 2 ? rgb(230, 180, 70)
                                         : rgb(220, 90, 190);
        const uint16_t cardBg = rgb(12, 18, 24);
        display.fillRoundRect(choiceX, y, choiceW, choiceH, 6, cardBg);
        display.drawRoundRect(choiceX, y, choiceW, choiceH, 6, accent);
        display.fillRect(choiceX + 12, y + 12, 8, choiceH - 24, accent);
        display.setFont(&fonts::Font2);
        drawWrappedText(storyChoiceText(arc, i), choiceX + 34, y + 14, choiceW - 210, 2, rgb(190, 208, 204), cardBg);
        addButton(storyChoiceLabel(arc, i), choiceX + choiceW - 158, y + 16, 134, choiceH - 32, UiAction::ChooseStory,
                  i, accent);
    }

    const int32_t buttonY = height - bottomH;
    drawTextFit("The city keeps the receipt.", margin + 18, buttonY + 24,
                width - margin * 2 - 36, rgb(150, 168, 170), rgb(3, 6, 9));

    for (uint8_t i = 0; i < buttonCount; ++i) {
        drawButton(buttons[i]);
    }
}

// Draws a modal event notice for quest updates and strange interruptions.
void drawDialogueScreen() {
    auto& display = M5.Display;
    clearButtons();
    display.fillScreen(rgb(2, 5, 8));
    drawHeader();

    const int32_t width = display.width();
    const int32_t height = display.height();
    const int32_t modalW = width - 120;
    const int32_t modalH = height - 190;
    const int32_t x = (width - modalW) / 2;
    const int32_t y = 104;
    const uint16_t bg = rgb(8, 12, 17);
    const uint16_t accent = dialogueAccent == 0 ? rgb(130, 230, 200) : dialogueAccent;

    display.fillRoundRect(x + 8, y + 10, modalW, modalH, 8, rgb(1, 3, 5));
    drawPanel(x, y, modalW, modalH, accent);
    display.fillRect(x + 18, y + 18, 8, modalH - 36, accent);

    display.setFont(&fonts::Font4);
    drawTextFit(dialogueTitle[0] == '\0' ? "Signal" : dialogueTitle, x + 42, y + 22, modalW - 84, TFT_WHITE, bg);
    display.drawFastHLine(x + 42, y + 68, modalW - 84, rgb(55, 70, 70));

    display.setFont(&fonts::Font2);
    drawWrappedText(dialogueBody[0] == '\0' ? statusLine : dialogueBody, x + 42, y + 92, modalW - 84, 7,
                    rgb(210, 222, 214), bg);

    const int32_t buttonY = y + modalH - 72;
    const char* label = dialogueNextScreen == Screen::StoryDecision ? "Choose" : "Continue";
    addButton(label, x + modalW - 220, buttonY, 180, 52, UiAction::ContinueDialogue, 0, accent);

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
        case Screen::Tracker:
            drawTrackerScreen();
            break;
        case Screen::StoryDecision:
            drawStoryDecisionScreen();
            break;
        case Screen::Dialogue:
            drawDialogueScreen();
            break;
        case Screen::ActionDetail:
            drawActionDetailScreen();
            break;
        case Screen::Encounter:
            drawEncounterScreen();
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
    itemTextPage = 0;
    selectedActionDetail = UiAction::Observe;
    pendingEncounterAction = UiAction::Explore;
    activeEncounterChoice = -1;
    clearEncounterChoices();
    selectedDawnOffer = -1;
    clearPinnedLead();
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
    iodineShieldReady = false;
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
    pendingStoryArc = StoryArc::Count;
    dialogueQueued = false;
    dialogueNextScreen = Screen::Field;
    dialogueTitle[0] = '\0';
    dialogueBody[0] = '\0';
    dialogueAccent = 0;

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
    generateDawnOffers();
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
