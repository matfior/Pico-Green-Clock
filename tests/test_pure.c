/* Host-side unit tests for pure logic extracted from Pico-Green-Clock.c.
 *
 * Build:  gcc -O2 -Wall -std=c11 test_pure.c -o test_pure
 * Run:    ./test_pure    (exits 0 on success, non-zero on failure)
 *
 * The functions under test are copied (not linked) so this harness can run
 * on any host without the Pico SDK or ARM toolchain. When the main file is
 * eventually split into modules, these copies should be replaced with real
 * linkage.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t  UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint8_t  UCHAR;

#define CRC16_POLYNOM 0x1021

/* Copied verbatim from Pico-Green-Clock.c (with hardware-only debug calls stripped). */
static UINT8 MonthDays[2][12] = {
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},  /* leap year */
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},  /* normal year */
};

static UINT16 crc16(UINT8 *Data, UINT16 DataSize)
{
    UINT16 CrcValue = 0;
    UINT8 Loop1UInt8;

    if (Data == NULL) return 0;

    while (DataSize-- > 0)
    {
        CrcValue = CrcValue ^ (UINT8)*Data++ << 8;
        for (Loop1UInt8 = 0; Loop1UInt8 < 8; ++Loop1UInt8)
        {
            if (CrcValue & 0x8000)
                CrcValue = CrcValue << 1 ^ CRC16_POLYNOM;
            else
                CrcValue = CrcValue << 1;
        }
    }
    return (CrcValue & 0xFFFF);
}

static UINT8 get_day_of_week(UINT16 Year, UINT8 Month, UINT8 DayOfMonth)
{
    UINT8 DayOfWeek;

    if (Month == 1 || Month == 2)
    {
        Month += 12;
        --Year;
    }
    DayOfWeek = (DayOfMonth + 1 + 2 * Month + 3 * (Month + 1) / 5
                 + Year + Year / 4 - Year / 100 + Year / 400) % 7;
    return DayOfWeek + 1;
}

static UINT8 get_month_days(UINT16 CurrentYear, UINT8 MonthNumber)
{
    int leap = ((CurrentYear % 4 == 0) && (CurrentYear % 100 != 0))
               || (CurrentYear % 400 == 0);
    if (MonthNumber < 1 || MonthNumber > 12) return 0;
    return MonthDays[leap ? 0 : 1][MonthNumber - 1];
}

static UINT16 get_day_of_year(UINT16 YearNumber, UINT8 MonthNumber, UINT8 DayNumber)
{
    UINT8 Loop1UInt8;
    UINT16 TargetDayNumber = 0;

    for (Loop1UInt8 = 1; Loop1UInt8 < MonthNumber; ++Loop1UInt8)
        TargetDayNumber += get_month_days(YearNumber, Loop1UInt8);
    TargetDayNumber += DayNumber;
    return TargetDayNumber;
}

/* ---------------- Test harness ---------------- */

static int failures = 0;
static int total = 0;

#define CHECK_EQ(actual, expected, label) do {                                    \
    total++;                                                                      \
    long _a = (long)(actual), _e = (long)(expected);                              \
    if (_a != _e) {                                                               \
        failures++;                                                               \
        fprintf(stderr, "FAIL %s:%d  %s: got %ld, want %ld\n",                    \
                __FILE__, __LINE__, (label), _a, _e);                             \
    }                                                                             \
} while (0)

/* Day-of-week: SUN=1, MON=2, TUE=3, WED=4, THU=5, FRI=6, SAT=7 */
static void test_get_day_of_week(void)
{
    CHECK_EQ(get_day_of_week(2024, 1, 1),  2, "2024-01-01 (Mon)");
    CHECK_EQ(get_day_of_week(2023, 9, 9),  7, "2023-09-09 (Sat, 9.03 release date)");
    CHECK_EQ(get_day_of_week(2000, 2, 29), 3, "2000-02-29 (Tue, leap century)");
    CHECK_EQ(get_day_of_week(2024, 2, 29), 5, "2024-02-29 (Thu, leap year)");
    CHECK_EQ(get_day_of_week(1970, 1, 1),  5, "1970-01-01 (Thu, Unix epoch)");
    CHECK_EQ(get_day_of_week(2100, 3, 1),  2, "2100-03-01 (Mon, non-leap century)");
    CHECK_EQ(get_day_of_week(2026, 4, 22), 4, "2026-04-22 (Wed)");
}

static void test_get_month_days(void)
{
    CHECK_EQ(get_month_days(2024, 2), 29, "2024-02 leap");
    CHECK_EQ(get_month_days(2023, 2), 28, "2023-02 non-leap");
    CHECK_EQ(get_month_days(2000, 2), 29, "2000-02 leap (div by 400)");
    CHECK_EQ(get_month_days(1900, 2), 28, "1900-02 non-leap (div by 100 not 400)");
    CHECK_EQ(get_month_days(2024, 1),  31, "2024-01");
    CHECK_EQ(get_month_days(2024, 4),  30, "2024-04");
    CHECK_EQ(get_month_days(2024, 7),  31, "2024-07");
    CHECK_EQ(get_month_days(2024, 12), 31, "2024-12");
    CHECK_EQ(get_month_days(2024, 0),   0, "invalid month 0");
    CHECK_EQ(get_month_days(2024, 13),  0, "invalid month 13");
}

static void test_get_day_of_year(void)
{
    CHECK_EQ(get_day_of_year(2024, 1, 1),   1,   "2024-01-01");
    CHECK_EQ(get_day_of_year(2024, 12, 31), 366, "2024-12-31 leap");
    CHECK_EQ(get_day_of_year(2023, 12, 31), 365, "2023-12-31 non-leap");
    CHECK_EQ(get_day_of_year(2024, 3, 1),   61,  "2024-03-01 (post-leap-Feb)");
    CHECK_EQ(get_day_of_year(2023, 3, 1),   60,  "2023-03-01 (post-non-leap-Feb)");
    CHECK_EQ(get_day_of_year(2024, 7, 4),   186, "2024-07-04");
}

static void test_crc16(void)
{
    UINT8 one = 0x01;
    CHECK_EQ(crc16(NULL, 10),    0, "null data returns 0");
    CHECK_EQ(crc16(&one, 0),     0, "zero length returns 0");
    CHECK_EQ(crc16(&one, 1),     0x1021, "crc16({0x01}) == polynomial");

    UINT8 abc[3] = {'A', 'B', 'C'};
    UINT16 first = crc16(abc, 3);
    UINT16 second = crc16(abc, 3);
    CHECK_EQ(first, second, "crc16 is deterministic");

    /* Stability anchor: if this ever changes, flash CRC check will reject
     * every existing user's saved config — the algorithm must not drift. */
    UINT8 msg[9] = {'1','2','3','4','5','6','7','8','9'};
    CHECK_EQ(crc16(msg, 9), 0x31C3, "crc16('123456789') == 0x31C3 (XMODEM vector)");
}

int main(void)
{
    test_get_day_of_week();
    test_get_month_days();
    test_get_day_of_year();
    test_crc16();

    printf("%d/%d tests passed\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}
