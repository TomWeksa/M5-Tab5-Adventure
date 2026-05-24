#pragma once

#include "GameTypes.h"

// Content tables live outside the runtime loop so destinations and items can
// grow into authored data instead of hard-coded control flow.
extern Item itemCatalog[];
extern Site sites[];
extern MapPin mapPins[];
extern RouteEdge routeEdges[];

extern const uint8_t kItemCount;
extern const uint8_t kSiteCount;
extern const uint8_t kMapPinCount;
extern const uint8_t kRouteEdgeCount;
