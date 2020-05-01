// Copyright © Mattias Larsson Sköld 2020

#pragma once

#ifdef DEBUG
#define debugln(x) Serial.println(x)
#define debug(x) Serial.print(x)

#else

#define debugln(x)
#define debug(x)

#endif
