/*
 * Copyright 2022 WolkAbout Technology s.r.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef WOLK_TYPES_H
#define WOLK_TYPES_H

#include "size_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char UNIT_NUMERIC[ITEM_UNIT_SIZE];
extern const char UNIT_BOOLEAN[ITEM_UNIT_SIZE];
extern const char UNIT_PERCENT[ITEM_UNIT_SIZE];
extern const char UNIT_DECIBEL[ITEM_UNIT_SIZE];
extern const char UNIT_LOCATION[ITEM_UNIT_SIZE];
extern const char UNIT_TEXT[ITEM_UNIT_SIZE];
extern const char UNIT_METRES_PER_SQUARE_SECOND[ITEM_UNIT_SIZE];
extern const char UNIT_G[ITEM_UNIT_SIZE];
extern const char UNIT_MOLE[ITEM_UNIT_SIZE];
extern const char UNIT_ATOM[ITEM_UNIT_SIZE];
extern const char UNIT_RADIAN[ITEM_UNIT_SIZE];
extern const char UNIT_REVOLUTION[ITEM_UNIT_SIZE];
extern const char UNIT_DEGREE_ANGLE[ITEM_UNIT_SIZE];
extern const char UNIT_MINUTE_ANGLE[ITEM_UNIT_SIZE];
extern const char UNIT_SECOND_ANGLE[ITEM_UNIT_SIZE];
extern const char UNIT_CENTIRADIAN[ITEM_UNIT_SIZE];
extern const char UNIT_GRADE[ITEM_UNIT_SIZE];
extern const char UNIT_SQUARE_METRE[ITEM_UNIT_SIZE];
extern const char UNIT_ARE[ITEM_UNIT_SIZE];
extern const char UNIT_HECTARE[ITEM_UNIT_SIZE];
extern const char UNIT_KATAL[ITEM_UNIT_SIZE];
extern const char UNIT_BIT[ITEM_UNIT_SIZE];
extern const char UNIT_BYTE[ITEM_UNIT_SIZE];
extern const char UNIT_SECOND[ITEM_UNIT_SIZE];
extern const char UNIT_MINUTE[ITEM_UNIT_SIZE];
extern const char UNIT_HOUR[ITEM_UNIT_SIZE];
extern const char UNIT_DAY[ITEM_UNIT_SIZE];
extern const char UNIT_WEEK[ITEM_UNIT_SIZE];
extern const char UNIT_YEAR[ITEM_UNIT_SIZE];
extern const char UNIT_MONTH[ITEM_UNIT_SIZE];
extern const char UNIT_DAY_SIDEREAL[ITEM_UNIT_SIZE];
extern const char UNIT_YEAR_SIDEREAL[ITEM_UNIT_SIZE];
extern const char UNIT_YEAR_CALENDAR[ITEM_UNIT_SIZE];
extern const char UNIT_POISE[ITEM_UNIT_SIZE];
extern const char UNIT_FARAD[ITEM_UNIT_SIZE];
extern const char UNIT_COULOMB[ITEM_UNIT_SIZE];
extern const char UNIT_E[ITEM_UNIT_SIZE];
extern const char UNIT_FARADAY[ITEM_UNIT_SIZE];
extern const char UNIT_FRANKLIN[ITEM_UNIT_SIZE];
extern const char UNIT_SIEMENS[ITEM_UNIT_SIZE];
extern const char UNIT_AMPERE[ITEM_UNIT_SIZE];
extern const char UNIT_GILBERT[ITEM_UNIT_SIZE];
extern const char UNIT_HENRY[ITEM_UNIT_SIZE];
extern const char UNIT_VOLT[ITEM_UNIT_SIZE];
extern const char UNIT_CENTIVOLT[ITEM_UNIT_SIZE];
extern const char UNIT_MILLIVOLT[ITEM_UNIT_SIZE];
extern const char UNIT_OHM[ITEM_UNIT_SIZE];
extern const char UNIT_JOULE[ITEM_UNIT_SIZE];
extern const char UNIT_ERG[ITEM_UNIT_SIZE];
extern const char UNIT_ELECTRON_VOLT[ITEM_UNIT_SIZE];
extern const char UNIT_NEWTON[ITEM_UNIT_SIZE];
extern const char UNIT_DYNE[ITEM_UNIT_SIZE];
extern const char UNIT_KILOGRAM_FORCE[ITEM_UNIT_SIZE];
extern const char UNIT_POUND_FORCE[ITEM_UNIT_SIZE];
extern const char UNIT_HERTZ[ITEM_UNIT_SIZE];
extern const char UNIT_MEGAHERTZ[ITEM_UNIT_SIZE];
extern const char UNIT_GIGAHERTZ[ITEM_UNIT_SIZE];
extern const char UNIT_LUX[ITEM_UNIT_SIZE];
extern const char UNIT_LAMBERT[ITEM_UNIT_SIZE];
extern const char UNIT_STOKE[ITEM_UNIT_SIZE];
extern const char UNIT_METRE[ITEM_UNIT_SIZE];
extern const char UNIT_KILOMETRE[ITEM_UNIT_SIZE];
extern const char UNIT_CENTIMETRE[ITEM_UNIT_SIZE];
extern const char UNIT_MILLIMETRE[ITEM_UNIT_SIZE];
extern const char UNIT_FOOT[ITEM_UNIT_SIZE];
extern const char UNIT_FOOT_SURVEY_US[ITEM_UNIT_SIZE];
extern const char UNIT_YARD[ITEM_UNIT_SIZE];
extern const char UNIT_INCH[ITEM_UNIT_SIZE];
extern const char UNIT_MILE[ITEM_UNIT_SIZE];
extern const char UNIT_NAUTICAL_MILE[ITEM_UNIT_SIZE];
extern const char UNIT_ANGSTROM[ITEM_UNIT_SIZE];
extern const char UNIT_ASTRONOMICAL_UNIT[ITEM_UNIT_SIZE];
extern const char UNIT_LIGHT_YEAR[ITEM_UNIT_SIZE];
extern const char UNIT_PARSEC[ITEM_UNIT_SIZE];
extern const char UNIT_POINT[ITEM_UNIT_SIZE];
extern const char UNIT_PIXEL[ITEM_UNIT_SIZE];
extern const char UNIT_LUMEN[ITEM_UNIT_SIZE];
extern const char UNIT_CANDELA[ITEM_UNIT_SIZE];
extern const char UNIT_WEBER[ITEM_UNIT_SIZE];
extern const char UNIT_MAXWELL[ITEM_UNIT_SIZE];
extern const char UNIT_TESLA[ITEM_UNIT_SIZE];
extern const char UNIT_GAUSS[ITEM_UNIT_SIZE];
extern const char UNIT_KILOGRAM[ITEM_UNIT_SIZE];
extern const char UNIT_GRAM[ITEM_UNIT_SIZE];
extern const char UNIT_ATOMIC_MASS[ITEM_UNIT_SIZE];
extern const char UNIT_ELECTRON_MASS[ITEM_UNIT_SIZE];
extern const char UNIT_POUND[ITEM_UNIT_SIZE];
extern const char UNIT_OUNCE[ITEM_UNIT_SIZE];
extern const char UNIT_TON_US[ITEM_UNIT_SIZE];
extern const char UNIT_TON_UK[ITEM_UNIT_SIZE];
extern const char UNIT_METRIC_TON[ITEM_UNIT_SIZE];
extern const char UNIT_WATT[ITEM_UNIT_SIZE];
extern const char UNIT_HORSEPOWER[ITEM_UNIT_SIZE];
extern const char UNIT_PASCAL[ITEM_UNIT_SIZE];
extern const char UNIT_HECTOPASCAL[ITEM_UNIT_SIZE];
extern const char UNIT_ATMOSPHERE[ITEM_UNIT_SIZE];
extern const char UNIT_BAR[ITEM_UNIT_SIZE];
extern const char UNIT_MILLIBAR[ITEM_UNIT_SIZE];
extern const char UNIT_MILLIMETER_OF_MERCURY[ITEM_UNIT_SIZE];
extern const char UNIT_INCH_OF_MERCURY[ITEM_UNIT_SIZE];
extern const char UNIT_GRAY[ITEM_UNIT_SIZE];
extern const char UNIT_RAD[ITEM_UNIT_SIZE];
extern const char UNIT_SIEVERT[ITEM_UNIT_SIZE];
extern const char UNIT_REM[ITEM_UNIT_SIZE];
extern const char UNIT_BECQUEREL[ITEM_UNIT_SIZE];
extern const char UNIT_CURIE[ITEM_UNIT_SIZE];
extern const char UNIT_RUTHERFORD[ITEM_UNIT_SIZE];
extern const char UNIT_ROENTGEN[ITEM_UNIT_SIZE];
extern const char UNIT_STERADIAN[ITEM_UNIT_SIZE];
extern const char UNIT_SPHERE[ITEM_UNIT_SIZE];
extern const char UNIT_KELVIN[ITEM_UNIT_SIZE];
extern const char UNIT_CELSIUS[ITEM_UNIT_SIZE];
extern const char UNIT_RANKINE[ITEM_UNIT_SIZE];
extern const char UNIT_FAHRENHEIT[ITEM_UNIT_SIZE];
extern const char UNIT_METRES_PER_SECOND[ITEM_UNIT_SIZE];
extern const char UNIT_MILES_PER_HOUR[ITEM_UNIT_SIZE];
extern const char UNIT_KILOMETRES_PER_HOUR[ITEM_UNIT_SIZE];
extern const char UNIT_KNOT[ITEM_UNIT_SIZE];
extern const char UNIT_MACH[ITEM_UNIT_SIZE];
extern const char UNIT_C[ITEM_UNIT_SIZE];
extern const char UNIT_CUBIC_METRE[ITEM_UNIT_SIZE];
extern const char UNIT_LITRE[ITEM_UNIT_SIZE];
extern const char UNIT_DECILITRE[ITEM_UNIT_SIZE];
extern const char UNIT_MILLILITRE[ITEM_UNIT_SIZE];
extern const char UNIT_CUBIC_INCH[ITEM_UNIT_SIZE];
extern const char UNIT_GALLON_DRY_US[ITEM_UNIT_SIZE];
extern const char UNIT_GALLON_UK[ITEM_UNIT_SIZE];
extern const char UNIT_OUNCE_LIQUID_UK[ITEM_UNIT_SIZE];
extern const char UNIT_UNKNOWN[ITEM_UNIT_SIZE];

typedef enum {
    IN,
    IN_OUT,
} feed_type_t;

typedef enum {
    NUMERIC,
    BOOLEAN,
    STRING,
    LOCATION,
    ENUM,
    VECTOR,
} data_type_t;

char* feed_type_to_string(feed_type_t feed_type);

extern const char** FEED_TYPE[ITEM_FEED_TYPE_SIZE];

extern const char DATA_TYPE_STRING[ITEM_DATA_TYPE_SIZE];
extern const char DATA_TYPE_NUMERIC[ITEM_DATA_TYPE_SIZE];
extern const char DATA_TYPE_BOOLEAN[ITEM_DATA_TYPE_SIZE];
extern const char DATA_TYPE_HEXADECIMAL[ITEM_DATA_TYPE_SIZE];
extern const char DATA_TYPE_LOCATION[ITEM_DATA_TYPE_SIZE];
extern const char DATA_TYPE_ENUM[ITEM_DATA_TYPE_SIZE];
extern const char DATA_TYPE_VECTOR[ITEM_DATA_TYPE_SIZE];

typedef enum {
    PUSH,
    PULL,
} outbound_mode_t;

#ifdef __cplusplus
}
#endif

#endif // WOLK_TYPES_H
