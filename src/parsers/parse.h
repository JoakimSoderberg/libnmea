#ifndef INC_NMEA_PARSE_H
#define INC_NMEA_PARSE_H

#define _XOPEN_SOURCE /* glibc2 needs this */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../nmea/nmea.h"

#define NMEA_TIME_FORMAT	"%H%M%S"
#define NMEA_TIME_FORMAT_LEN	6

/**
 * Parse GPS position longitude or latitude
 *
 * s string containing the position. Ex: "4712.55", 47 degrees and
 *   12.55 minutes. Will be modified.
 * pos is a pointer to a nmea_position struct where the result should be stored.
 *
 * Returns 0 on success, otherwise -1.
 */
int nmea_position_parse(char *s, nmea_position *pos);

/**
 * Parse cardinal direction
 *
 * s is a string containing the letter representing the cardinal direction.
 *
 * Returns the cardinal direction (nmea_cardinal_t). On failure,
 *         NMEA_CARDINAL_DIR_UNKNOWN is returned.
 */
nmea_cardinal_t nmea_cardinal_direction_parse(char *s);

/**
 * Parse time from a string
 *
 * s is a string containing the time in format "HHMMSS".
 * time is a pointer to a tm struct where the parser time will be stored.
 *
 * Returns 0 on success, otherwise -1.
 */
int nmea_time_parse(char *s, struct tm *time);

#endif  /* INC_NMEA_PARSE_H */
