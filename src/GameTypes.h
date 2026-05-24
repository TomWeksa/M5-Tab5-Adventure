#pragma once

#include <Arduino.h>

// Screens are the primary modes the touch UI can render.
enum class Screen : uint8_t {
    Field,
    Inventory,
    ItemDetail,
    Map,
    Reward,
    Trade,
    Tracker,
    StoryDecision,
};

// Slots describe broad item categories and also drive equipment placement.
enum class Slot : uint8_t {
    Suit,
    Detector,
    Tool,
    Weapon,
    Artifact,
    Consumable,
};

// Item image kinds select the generated illustration used on the item detail card.
enum class ItemImageKind : uint8_t {
    PatchMask,
    RubberTrench,
    MirrorweaveCoat,
    CoilDetector,
    GlassNeedle,
    PrybarKit,
    SolderRig,
    QuietNailgun,
    RailPistol,
    WarmBattery,
    MourningLens,
    NullCharm,
    IodineAmpoule,
    CannedCoffee,
    CopperSaint,
};

// Item use kinds keep equipment and consumables in the same item definition.
enum class ItemUseKind : uint8_t {
    None,
    Equip,
    Consume,
};

// Leads are actionable observations that can become a distinct player choice.
enum class LeadKind : uint8_t {
    None,
    Contact,
    Cache,
    Anomaly,
    Door,
    Trail,
};

// UI actions are stored on buttons and interpreted by the input dispatcher.
enum class UiAction : uint8_t {
    None,
    Observe,
    Explore,
    FollowLead,
    Inventory,
    Map,
    Tracker,
    ChooseStory,
    SelectSite,
    BackToField,
    InspectItem,
    EquipOrUse,
    InventoryPrev,
    InventoryNext,
    OpenTrade,
    ToggleTradeOffer,
    ToggleTradeWant,
    TradePrev,
    TradeNext,
    AcceptTrade,
    ClearTrade,
    ContinueReward,
    Travel,
    Rest,
};

// ItemUseProfile defines the data-driven result of activating an item.
struct ItemUseProfile {
    ItemUseKind kind;
    int8_t healthDelta;
    int8_t exposureDelta;
    int8_t attentionDelta;
    bool consumedOnUse;
    const char* label;
    const char* resultText;
};

// Item is the single shared shape for gear, artifacts, weapons, and doses.
struct Item {
    const char* name;
    const char* tag;
    Slot slot;
    ItemImageKind image;
    int8_t grit;
    int8_t tech;
    int8_t scan;
    int8_t ghost;
    int8_t filter;
    int8_t strain;
    uint8_t value;
    uint8_t tintR;
    uint8_t tintG;
    uint8_t tintB;
    uint16_t color;
    ItemUseProfile use;
    const char* description;
    const char* fieldRead;
};

// Site holds the authored description and mechanical pressure for a destination.
struct Site {
    const char* name;
    const char* district;
    const char* description;
    // Specific passage shown when the player takes time to observe the site.
    const char* observeText;
    uint8_t risk;
    uint8_t maxCache;
    uint16_t color;
};

// MapPin stores a destination's satellite position and its in-world map note.
struct MapPin {
    uint8_t site;
    uint16_t xPermille;
    uint16_t yPermille;
    const char* callSign;
    const char* whisper;
};

// RouteEdge connects two pins with a named path drawn as a walked route.
struct RouteEdge {
    uint8_t from;
    uint8_t to;
    const char* name;
};

// Stats are recalculated from equipped items, then used by risks and actions.
struct Stats {
    int16_t grit = 1;
    int16_t tech = 1;
    int16_t scan = 0;
    int16_t ghost = 0;
    int16_t filter = 0;
    int16_t strain = 0;
};

// Button keeps rendering geometry and the action payload together for touch hit tests.
struct Button {
    int32_t x = 0;
    int32_t y = 0;
    int32_t w = 0;
    int32_t h = 0;
    UiAction action = UiAction::None;
    int16_t param = 0;
    bool enabled = true;
    bool visible = true;
    uint16_t accent = 0;
    char label[28] = "";
};
