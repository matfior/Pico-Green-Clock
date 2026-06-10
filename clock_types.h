/* ======================================================================== *\
   clock_types.h
   Shared structure definitions used by both Pico-Green-Clock.c and
   picow_ntp_client.c.

   These types were previously defined twice (once in each file) and had
   already drifted apart (field naming and comments). Keep the single
   authoritative definition here so the two compilation units can never
   disagree on layout again.

   NOTE: struct flash_config is persisted to flash and validated with a
         CRC16. Changing its layout invalidates every user's saved
         configuration - only extend it by carving space out of the
         Reserved1 / Reserved2 fields.
\* ======================================================================== */

#ifndef CLOCK_TYPES_H
#define CLOCK_TYPES_H

#include <stdint.h>
#include <time.h>



/* Alarm definitions. */
struct alarm
{
  uint8_t FlagStatus;
  uint8_t Second;
  uint8_t Minute;
  uint8_t Hour;
  uint8_t Day;
  unsigned char Text[40];
};



/* Summer Time / Winter Time parameters definitions. */
struct dst_parameters
{
  uint8_t  StartMonth;
  uint8_t  StartDayOfWeek;
  int8_t   StartDayOfMonthLow;
  int8_t   StartDayOfMonthHigh;
  uint8_t  StartHour;
  uint16_t StartDayOfYear;
  uint8_t  EndMonth;
  uint8_t  EndDayOfWeek;
  int8_t   EndDayOfMonthLow;
  int8_t   EndDayOfMonthHigh;
  uint8_t  EndHour;
  uint16_t EndDayOfYear;
  uint8_t  ShiftMinutes;
};



/* NTP data structure. */
struct ntp_data
{
  /* Time-related data. */
  uint8_t  CurrentDayOfWeek;
  uint8_t  CurrentDayOfMonth;
  uint8_t  CurrentMonth;
  uint16_t CurrentYear;
  uint8_t  CurrentYearLowPart;
  uint8_t  CurrentHour;
  uint8_t  CurrentMinute;
  uint8_t  CurrentSecond;

  /* Generic data. */
  time_t   Epoch;
  uint8_t  FlagNTPResync;   // flag set to On if there is a specific reason to request an NTP update without delay.
  uint8_t  FlagNTPSuccess;  // flag indicating that NTP date and time request has succeeded.
  uint64_t NTPDelta;
  uint32_t NTPErrors;       // cumulative number of errors while trying to re-sync with NTP.
  uint64_t NTPGetTime;
  uint64_t NTPLastUpdate;
  uint32_t NTPReadCycles;   // total number of re-sync cycles through NTP.
};



/* Structure containing the Green Clock configuration being saved to flash memory.
   Those variables will be restored after a reboot and / or power failure. */
/* IMPORTANT: Version must always be the first element of the structure and
              CRC16   must always be the  last element of the structure. */
struct flash_config
{
  unsigned char Version[6];   // firmware version number (format: "06.00" - including end-of-string).
  uint8_t  CurrentYearCentile; // assume we are in years 20xx on power-up but is adjusted when configuration is read (will your clock live long enough for a "21" ?!).
  uint8_t  Language;          // language used for data display (including date scrolling).
  unsigned char DSTCountry;   // specifies how to handle the daylight saving time (see User Guide and / or clock options above).
  uint8_t  TemperatureUnit;   // CELSIUS or FAHRENHEIT default value (see clock options above).
  uint8_t  TimeDisplayMode;   // H24 or H12 default value (see clock options above).
  uint8_t  ChimeMode;         // chime mode (Off / On / Day).
  uint8_t  ChimeTimeOn;       // hourly chime will begin at this hour.
  uint8_t  ChimeTimeOff;      // hourly chime will stop after this hour.
  uint8_t  NightLightMode;    // night light mode (On / Off / Auto / Night).
  uint8_t  NightLightTimeOn;  // default night light time On.
  uint8_t  NightLightTimeOff; // default night light time Off.
  uint8_t  FlagAutoBrightness; // flag indicating we are in "Auto Brightness" mode.
  uint8_t  FlagKeyclick;      // flag for keyclick ("button-press" tone)
  uint8_t  FlagScrollEnable;  // flag indicating the clock will scroll the date and temperature at regular intervals on the display.
  uint8_t  FlagSummerTime;    // flag indicating the current status (On or Off) of Daylight Saving Time / Summer Time.
  int8_t   Timezone;          // (in hours) value to add to UTC time (Universal Time Coordinate) to get the local time.
  uint8_t  Reserved1[48];     // reserved for future use.
  struct alarm Alarm[9];      // alarms 0 to 8 parameters (numbered 1 to 9 for clock users). Day is a bit mask.
  unsigned char SSID[40];     // SSID for Wi-Fi network. Note: SSID begins at position 5 of the variable string, so that a "footprint" can be confirmed prior to writing to flash.
  unsigned char Password[70]; // password for Wi-Fi network. Note: password begins at position 5 of the variable string, for the same reason as SSID above.
  unsigned char Reserved2[48]; // reserved for future use.
  uint16_t Crc16;             // crc16 of all data above to validate configuration.
};

#endif  // CLOCK_TYPES_H
