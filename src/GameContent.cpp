#include "GameContent.h"

// Item definitions are authored data. Runtime code equips or consumes them by
// reading ItemUseProfile, which keeps future item behavior extensible.
Item itemCatalog[] = {
    {"Patch Mask", "suit", Slot::Suit, ItemImageKind::PatchMask, 0, 0, 0, 0, 1, 0, 6, 110, 150, 150, 0,
     {ItemUseKind::Equip, 0, 0, 0, false, "Equip", "You settle the mask straps until each breath sounds borrowed."},
     "A patched respirator and tarred hood. Bad fashion, good filters.",
     "Worn against weather, bad air, panic sweat, and whatever the rain is learning to become."},
    {"Rubber Trench", "suit", Slot::Suit, ItemImageKind::RubberTrench, 1, 0, 0, -1, 2, 0, 14, 90, 180, 120, 0,
     {ItemUseKind::Equip, 0, 0, 0, false, "Equip", "The trench coat seals with a rubber sigh and a smell of old cleanup crews."},
     "Heavy sealed coat from a cleanup crew that never came back.",
     "Heavy protection for hot rain and sharp debris. It keeps you alive, but not graceful."},
    {"Mirrorweave Coat", "suit", Slot::Suit, ItemImageKind::MirrorweaveCoat, 0, 1, 0, 2, 1, 0, 18, 190, 90, 210, 0,
     {ItemUseKind::Equip, 0, 0, 0, false, "Equip", "Mirrorweave threads take the light and give back someone less obvious."},
     "Chameleon threads shimmer under old advert light.",
     "A stealth coat with firmware in the fabric. Cameras dislike it. So do honest mirrors."},
    {"Coil Detector", "detector", Slot::Detector, ItemImageKind::CoilDetector, 0, 0, 1, 0, 0, 0, 8, 80, 200, 220, 0,
     {ItemUseKind::Equip, 0, 0, 0, false, "Equip", "The coil wakes with a dry click and begins listening under the pavement."},
     "Cheap copper loop detector. It screams before the world bends.",
     "Useful when the world lies visually. Crude, sturdy, and loud enough to keep you cautious."},
    {"Glass Needle", "detector", Slot::Detector, ItemImageKind::GlassNeedle, 0, 1, 3, 0, 0, 1, 20, 90, 230, 190, 0,
     {ItemUseKind::Equip, 0, 0, 0, false, "Equip", "The glass needle points somewhere your hand does not want to go."},
     "A precise anomaly pick. Every reading leaves a headache.",
     "Fine anomaly work at a cost. It finds the quiet impossibilities and leaves a little static behind your eyes."},
    {"Prybar Kit", "tool", Slot::Tool, ItemImageKind::PrybarKit, 1, 0, 0, 0, 0, 0, 7, 220, 180, 70, 0,
     {ItemUseKind::Equip, 0, 0, 0, false, "Equip", "You roll the kit tight. The prybar knocks once against your boot."},
     "A prybar, pliers, tape and stubbornness in a cracked roll.",
     "Turns locked places into negotiations. Also turns quiet places into loud ones."},
    {"Solder Rig", "tool", Slot::Tool, ItemImageKind::SolderRig, 0, 3, 0, 0, 0, 0, 19, 230, 120, 70, 0,
     {ItemUseKind::Equip, 0, 0, 0, false, "Equip", "The solder rig warms against your palm and starts looking for ports."},
     "Battery iron, fiber patcher and black market firmware clips.",
     "For powered doors, dead drones, wet panels, and anything pretending to be offline."},
    {"Quiet Nailgun", "weapon", Slot::Weapon, ItemImageKind::QuietNailgun, 1, 0, 0, 2, 0, 0, 12, 210, 80, 160, 0,
     {ItemUseKind::Equip, 0, 0, 0, false, "Equip", "The nailgun settles under your coat, blunt and professionally quiet."},
     "Industrial nailer tuned for close work and low attention.",
     "A problem solver with a small sound. Good for threats, locks, and bad ideas."},
    {"Rail Pistol", "weapon", Slot::Weapon, ItemImageKind::RailPistol, 3, 0, 0, -1, 0, 1, 24, 230, 70, 90, 0,
     {ItemUseKind::Equip, 0, 0, 0, false, "Equip", "The rail pistol arms bright enough to make nearby shadows take a step back."},
     "Hard recoil, bright flash, very persuasive.",
     "A loud answer. It wins arguments and starts investigations."},
    {"Warm Battery", "artifact", Slot::Artifact, ItemImageKind::WarmBattery, 0, 2, 0, 0, 0, 2, 30, 240, 190, 80, 0,
     {ItemUseKind::Equip, 0, 0, 0, false, "Equip", "The battery warms when you lie to yourself about why you kept it."},
     "A dry cell that charges itself and warms when lied to.",
     "Not equipment exactly. More like a small rule from somewhere else, pretending to be power storage."},
    {"Mourning Lens", "artifact", Slot::Artifact, ItemImageKind::MourningLens, 0, 0, 3, 0, 0, 2, 34, 120, 150, 250, 0,
     {ItemUseKind::Equip, 0, 0, 0, false, "Equip", "The lens darkens the world and reveals the edges of old mistakes."},
     "Smoke-dark glass that shows paths people failed to take.",
     "It sharpens observation by showing absence. The strain is remembering what was never yours."},
    {"Null Charm", "artifact", Slot::Artifact, ItemImageKind::NullCharm, 0, 0, 0, 2, 1, 1, 28, 120, 230, 170, 0,
     {ItemUseKind::Equip, 0, 0, 0, false, "Equip", "The charm goes cold. For a moment, even your reflection loses interest."},
     "A cold trinket that makes cameras lose interest.",
     "Carried invisibility with a price tag written in missing minutes."},
    {"Iodine Ampoule", "dose", Slot::Consumable, ItemImageKind::IodineAmpoule, 0, 0, 0, 0, 0, 0, 5, 170, 230, 80, 0,
     {ItemUseKind::Consume, 0, -5, 0, true, "Use Dose", "The iodine burns all the way down. Exposure drops, hands stop shaking."},
     "Bitter anti-rad dose. You only taste pennies for an hour.",
     "Temporary mercy against accumulated dose. Useful before the shaking makes decisions for you."},
    {"Canned Coffee", "dose", Slot::Consumable, ItemImageKind::CannedCoffee, 0, 0, 0, 0, 0, 0, 4, 180, 120, 80, 0,
     {ItemUseKind::Consume, 2, 0, 0, true, "Drink", "Coffee syrup, cold and foul. You feel almost alive."},
     "Pre-collapse stimulant syrup. Technically food.",
     "A small bodily loan from a company that is probably still billing somebody."},
    {"Copper Saint", "artifact", Slot::Artifact, ItemImageKind::CopperSaint, 2, 0, 0, 0, 0, 2, 38, 210, 130, 60, 0,
     {ItemUseKind::Equip, 0, 0, 0, false, "Equip", "The little copper figure hums like a service tunnel full of prayers."},
     "A little idol that hums in broken service tunnels.",
     "A heavy charm of courage or foolishness. In the exclusion, those are often the same material."},
};

const uint8_t kItemCount = sizeof(itemCatalog) / sizeof(itemCatalog[0]);

// Site text gives the world sensory anchors: ground, weather, smell, sound, and
// the people who survive by reading small changes in the Zone.
Site sites[] = {
    {"Underpass Clinic", "safe berth",
     "A medic shack below the ring road, where runoff clicks through drain grates and warm bulbs turn the damp concrete amber. It smells of bad coffee, antiseptic, diesel coats, and fear kept politely quiet; orderlies, fence-runners, and debt collectors all lower their voices near the triage curtain.",
     "From the clinic steps you watch the waiting line shift whenever the road lights flicker; people hide tremors in their coat pockets, and the orderlies read every cough like weather.",
     1, 0, 0},
    {"Neon Spillway", "wet market",
     "Old ad towers bleed color through toxic rain and illegal stalls crowd the flood channel. Fry oil, wet plastic, ozone, and incense hang under the tarps while smugglers haggle with scavengers, courier kids, and checkpoint guards pretending not to shop.",
     "You count patrol reflections in the spillway water, note which vendors stop talking first, and spot a courier marking safe stalls with chalk dust under the table lips.",
     3, 4, 0},
    {"Sunken Mall", "retail tomb",
     "Escalators vanish into black water and the public address system still whispers discounts to empty atriums. The air tastes of algae, copper wiring, perfume samples, and drowned plaster; families in patched waders sift storefronts while something below keeps the music playing.",
     "You hold still above the atrium and listen: water slaps tile, a child laughs once behind a shutter, and the old shop speakers skip whenever something moves below.",
     4, 5, 0},
    {"Relay Grave", "antenna field",
     "Fallen towers tick in the wind across a hill of cable bones and shattered insulators. Messages still arrive for the dead, buzzing from rusted cabinets that smell of hot dust and mouse nests, and the antenna pilgrims trade batteries for frequencies they swear contain names.",
     "You watch the tower shadows crawl the wrong direction and mark which cabinets click in answer; an antenna pilgrim leaves offerings where the static briefly sounds human.",
     5, 4, 0},
    {"Black Reed Verge", "outer exclusion",
     "A reed sea has grown through the asphalt until the road looks woven from knives. Sweet chemical rot rises from the ditches, insects rasp inside the fog, and quiet homesteads on scaffold legs watch the stalks for the moment every shadow leans the wrong way.",
     "You crouch in the ditch grass and let the reedline teach you its rhythm; hidden homes go dark in sequence, then one patch of stalks keeps moving after the wind stops.",
     6, 3, 0},
};

const uint8_t kSiteCount = sizeof(sites) / sizeof(sites[0]);

// Map pins read like field annotations rather than neutral labels.
MapPin mapPins[] = {
    {0, 150, 700, "CLINIC", "Blue ward light, wet concrete, and a queue of people counting pills in their palms."},
    {1, 320, 510, "SPILL", "Ad ghosts stutter: BUY CLEAN WATER / BUY CLEAN SKIN, while vendors laugh under leaking tarps."},
    {2, 510, 670, "MALL", "Retail hymns leak from drowned escalators after dusk. Boots splash where music should end."},
    {3, 660, 360, "RELAY", "Antenna shadows point against the wind. The cabinets answer voices nobody transmitted."},
    {4, 850, 190, "VERGE", "The reedline redraws itself between blinks. Smoke from hidden homes smells sweet and wrong."},
};

const uint8_t kMapPinCount = sizeof(mapPins) / sizeof(mapPins[0]);

// Route edges are named so travel text can refer to lived-in paths.
RouteEdge routeEdges[] = {
    {0, 1, "drain road"},
    {0, 2, "service tunnel"},
    {1, 2, "flood arcade"},
    {1, 3, "signage spine"},
    {2, 3, "maintenance tram"},
    {2, 4, "reed culvert"},
    {3, 4, "antenna ridge"},
};

const uint8_t kRouteEdgeCount = sizeof(routeEdges) / sizeof(routeEdges[0]);
