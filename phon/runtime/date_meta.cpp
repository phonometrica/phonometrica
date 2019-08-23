/**************************************************************************************
 * Copyright (C) 2013-2019, Artifex Software                                          *
 *           (C) 2019, Julien Eychenne <jeychenne@gmail.com>                          *
 *                                                                                    *
 * Permission to use, copy, modify, and/or distribute this software for any purpose   *
 * with or without fee is hereby granted, provided that the above copyright notice    *
 * and this permission notice appear in all copies.                                   *
 *                                                                                    *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH      *
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND    *
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, *
 * OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,     *
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS    *
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS        *
 * SOFTWARE.                                                                          *
 *                                                                                    *
 **************************************************************************************/

#include <math.h>
#include <time.h>
#include <phon/runtime/runtime.hpp>
#include <phon/runtime/object.hpp>
#include <phon/runtime/toplevel.hpp>

#if defined(__unix__) || defined(__APPLE__)

#include <sys/time.h>

#elif defined(_WIN32)
#include <sys/timeb.h>
#endif

#define js_optnumber(J, I, V) (J.is_defined(I) ? J.to_number(I) : V)

namespace phonometrica {


static double Now(void)
{
#if defined(__unix__) || defined(__APPLE__)
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return floor(tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0);
#elif defined(_WIN32)
    struct _timeb tv;
    _ftime(&tv);
    return tv.time * 1000.0 + tv.millitm;
#else
    return time(nullptr) * 1000.0;
#endif
}

static double LocalTZA(void)
{
    static int once = 1;
    static double tza = 0;
    if (once)
    {
        time_t now = time(nullptr);
        time_t utc = mktime(gmtime(&now));
        time_t loc = mktime(localtime(&now));
        tza = (loc - utc) * 1000;
        once = 0;
    }
    return tza;
}

static double DaylightSavingTA(double t)
{
    return 0; /* TODO */
}

/* Helpers from the ECMA 262 specification */

#define HoursPerDay        24.0
#define MinutesPerDay        (HoursPerDay * MinutesPerHour)
#define MinutesPerHour        60.0
#define SecondsPerDay        (MinutesPerDay * SecondsPerMinute)
#define SecondsPerHour        (MinutesPerHour * SecondsPerMinute)
#define SecondsPerMinute    60.0

#define msPerDay    (SecondsPerDay * msPerSecond)
#define msPerHour    (SecondsPerHour * msPerSecond)
#define msPerMinute    (SecondsPerMinute * msPerSecond)
#define msPerSecond    1000.0

static double pmod(double x, double y)
{
    x = fmod(x, y);
    if (x < 0)
        x += y;
    return x;
}

static int Day(double t)
{
    return floor(t / msPerDay);
}

static double TimeWithinDay(double t)
{
    return pmod(t, msPerDay);
}

static int DaysInYear(int y)
{
    return y % 4 == 0 && (y % 100 || (y % 400 == 0)) ? 366 : 365;
}

static int DayFromYear(int y)
{
    return 365 * (y - 1970) +
           floor((y - 1969) / 4.0) -
           floor((y - 1901) / 100.0) +
           floor((y - 1601) / 400.0);
}

static double TimeFromYear(int y)
{
    return DayFromYear(y) * msPerDay;
}

static int YearFromTime(double t)
{
    int y = floor(t / (msPerDay * 365.2425)) + 1970;
    double t2 = TimeFromYear(y);
    if (t2 > t)
        --y;
    else if (t2 + msPerDay * DaysInYear(y) <= t)
        ++y;
    return y;
}

static int InLeapYear(int t)
{
    return DaysInYear(YearFromTime(t)) == 366;
}

static int DayWithinYear(double t)
{
    return Day(t) - DayFromYear(YearFromTime(t));
}

static int MonthFromTime(double t)
{
    int day = DayWithinYear(t);
    int leap = InLeapYear(t);
    if (day < 31) return 0;
    if (day < 59 + leap) return 1;
    if (day < 90 + leap) return 2;
    if (day < 120 + leap) return 3;
    if (day < 151 + leap) return 4;
    if (day < 181 + leap) return 5;
    if (day < 212 + leap) return 6;
    if (day < 243 + leap) return 7;
    if (day < 273 + leap) return 8;
    if (day < 304 + leap) return 9;
    if (day < 334 + leap) return 10;
    return 11;
}

static int DateFromTime(double t)
{
    int day = DayWithinYear(t);
    int leap = InLeapYear(t);
    switch (MonthFromTime(t))
    {
    case 0:
        return day + 1;
    case 1:
        return day - 30;
    case 2:
        return day - 58 - leap;
    case 3:
        return day - 89 - leap;
    case 4:
        return day - 119 - leap;
    case 5:
        return day - 150 - leap;
    case 6:
        return day - 180 - leap;
    case 7:
        return day - 211 - leap;
    case 8:
        return day - 242 - leap;
    case 9:
        return day - 272 - leap;
    case 10:
        return day - 303 - leap;
    default :
        return day - 333 - leap;
    }
}

static int WeekDay(double t)
{
    return pmod(Day(t) + 4, 7);
}

static double LocalTime(double utc)
{
    return utc + LocalTZA() + DaylightSavingTA(utc);
}

static double UTC(double loc)
{
    return loc - LocalTZA() - DaylightSavingTA(loc - LocalTZA());
}

static int HourFromTime(double t)
{
    return pmod(floor(t / msPerHour), HoursPerDay);
}

static int MinFromTime(double t)
{
    return pmod(floor(t / msPerMinute), MinutesPerHour);
}

static int SecFromTime(double t)
{
    return pmod(floor(t / msPerSecond), SecondsPerMinute);
}

static int msFromTime(double t)
{
    return pmod(t, msPerSecond);
}

static double MakeTime(double hour, double min, double sec, double ms)
{
    return ((hour * MinutesPerHour + min) * SecondsPerMinute + sec) * msPerSecond + ms;
}

static double MakeDay(double y, double m, double date)
{
    /*
     * The following array contains the day of year for the first day of
     * each month, where index 0 is January, and day 0 is January 1.
     */
    static const double firstDayOfMonth[2][12] = {
            {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
            {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}
    };

    double yd, md;
    int im;

    y += floor(m / 12);
    m = pmod(m, 12);

    im = (int) m;
    if (im < 0 || im >= 12)
        return NAN;

    yd = floor(TimeFromYear(y) / msPerDay);
    md = firstDayOfMonth[InLeapYear(y)][im];

    return yd + md + date - 1;
}

static double MakeDate(double day, double time)
{
    return day * msPerDay + time;
}

static double TimeClip(double t)
{
    if (!isfinite(t))
        return NAN;
    if (fabs(t) > 8.64e15)
        return NAN;
    return t < 0 ? -floor(-t) : floor(t);
}

static int toint(const char **sp, int w, int *v)
{
    const char *s = *sp;
    *v = 0;
    while (w--)
    {
        if (*s < '0' || *s > '9')
            return 0;
        *v = *v * 10 + (*s++ - '0');
    }
    *sp = s;
    return 1;
}

static double parseDateTime(const String &str)
{
    int y = 1970, m = 1, d = 1, H = 0, M = 0, S = 0, ms = 0;
    int tza = 0;
    double t;
    auto s = str.data();

    /* Parse ISO 8601 formatted date and time: */
    /* YYYY("-"MM("-"DD)?)?("T"HH":"mm(":"ss("."sss)?)?("Z"|[+-]HH(":"mm)?)?)? */

    if (!toint(&s, 4, &y)) return NAN;
    if (*s == '-')
    {
        s += 1;
        if (!toint(&s, 2, &m)) return NAN;
        if (*s == '-')
        {
            s += 1;
            if (!toint(&s, 2, &d)) return NAN;
        }
    }

    if (*s == 'T')
    {
        s += 1;
        if (!toint(&s, 2, &H)) return NAN;
        if (*s != ':') return NAN;
        s += 1;
        if (!toint(&s, 2, &M)) return NAN;
        if (*s == ':')
        {
            s += 1;
            if (!toint(&s, 2, &S)) return NAN;
            if (*s == '.')
            {
                s += 1;
                if (!toint(&s, 3, &ms)) return NAN;
            }
        }
        if (*s == 'Z')
        {
            s += 1;
            tza = 0;
        }
        else if (*s == '+' || *s == '-')
        {
            int tzh = 0, tzm = 0;
            int tzs = *s == '+' ? 1 : -1;
            s += 1;
            if (!toint(&s, 2, &tzh)) return NAN;
            if (*s == ':')
            {
                s += 1;
                if (!toint(&s, 2, &tzm)) return NAN;
            }
            if (tzh > 23 || tzm > 59) return NAN;
            tza = tzs * (tzh * msPerHour + tzm * msPerMinute);
        }
        else
        {
            tza = LocalTZA();
        }
    }

    if (*s) return NAN;

    if (m < 1 || m > 12) return NAN;
    if (d < 1 || d > 31) return NAN;
    if (H < 0 || H > 24) return NAN;
    if (M < 0 || M > 59) return NAN;
    if (S < 0 || S > 59) return NAN;
    if (ms < 0 || ms > 999) return NAN;
    if (H == 24 && (M != 0 || S != 0 || ms != 0)) return NAN;

    /* TODO: DaylightSavingTA on local times */
    t = MakeDate(MakeDay(y, m - 1, d), MakeTime(H, M, S, ms));
    return t - tza;
}

/* date formatting */

static const char *fmtdate(char *buf, double t)
{
    int y = YearFromTime(t);
    int m = MonthFromTime(t);
    int d = DateFromTime(t);
    if (!isfinite(t))
        return "Invalid Date";
    sprintf(buf, "%04d-%02d-%02d", y, m + 1, d);
    return buf;
}

static const char *fmttime(char *buf, double t, double tza)
{
    int H = HourFromTime(t);
    int M = MinFromTime(t);
    int S = SecFromTime(t);
    int ms = msFromTime(t);
    int tzh = HourFromTime(fabs(tza));
    int tzm = MinFromTime(fabs(tza));
    if (!isfinite(t))
        return "Invalid Date";
    if (tza == 0)
        sprintf(buf, "%02d:%02d:%02d.%03dZ", H, M, S, ms);
    else if (tza < 0)
        sprintf(buf, "%02d:%02d:%02d.%03d-%02d:%02d", H, M, S, ms, tzh, tzm);
    else
        sprintf(buf, "%02d:%02d:%02d.%03d+%02d:%02d", H, M, S, ms, tzh, tzm);
    return buf;
}

static const char *fmtdatetime(char *buf, double t, double tza)
{
    char dbuf[20], tbuf[20];
    if (!isfinite(t))
        return "Invalid Date";
    fmtdate(dbuf, t);
    fmttime(tbuf, t, tza);
    sprintf(buf, "%sT%s", dbuf, tbuf);
    return buf;
}

/* Date functions */

static double js_todate(Environment *J, int idx)
{
    Object *self = J->to_object(idx);
    if (self->type != PHON_CDATE)
        throw J->raise("Type error", "not a date");
    return self->as.number;
}

static void js_setdate(Environment *J, int idx, double t)
{
    Object *self = J->to_object(idx);
    if (self->type != PHON_CDATE)
        throw J->raise("Type error", "not a date");
    self->as.number = TimeClip(t);
    J->push(self->as.number);
}

static void D_parse(Environment &env)
{
    double t = parseDateTime(env.to_string(1));
    env.push(t);
}

static void D_UTC(Environment &env)
{
    double y, m, d, H, M, S, ms, t;
    y = env.to_number(1);
    if (y < 100) y += 1900;
    m = env.to_number(2);
    d = js_optnumber(env, 3, 1);
    H = js_optnumber(env, 4, 0);
    M = js_optnumber(env, 5, 0);
    S = js_optnumber(env, 6, 0);
    ms = js_optnumber(env, 7, 0);
    t = MakeDate(MakeDay(y, m, d), MakeTime(H, M, S, ms));
    t = TimeClip(t);
    env.push(t);
}

static void D_now(Environment &env)
{
    env.push(Now());
}

static void jsB_Date(Environment &env)
{
    char buf[64];
    env.push(fmtdatetime(buf, LocalTime(Now()), LocalTZA()));
}

static void jsB_new_Date(Environment &env)
{
    int top = env.top_count();
    Object *obj;
    double t;

    if (top == 1)
        t = Now();
    else if (top == 2)
    {
        var_to_primitive(&env, 1, PHON_HINT_NONE);
        if (env.is_string(1))
            t = parseDateTime(env.to_string(1));
        else
            t = TimeClip(env.to_number(1));
    }
    else
    {
        double y, m, d, H, M, S, ms;
        y = env.to_number(1);
        if (y < 100) y += 1900;
        m = env.to_number(2);
        d = js_optnumber(env, 3, 1);
        H = js_optnumber(env, 4, 0);
        M = js_optnumber(env, 5, 0);
        S = js_optnumber(env, 6, 0);
        ms = js_optnumber(env, 7, 0);
        t = MakeDate(MakeDay(y, m, d), MakeTime(H, M, S, ms));
        t = TimeClip(UTC(t));
    }

    obj = new Object(env, PHON_CDATE, env.date_meta);
    obj->as.number = t;

    env.push(obj);
}

static void Dp_to_value(Environment &env)
{
    double t = js_todate(&env, 0);
    env.push(t);
}

static void Dp_toString(Environment &env)
{
    char buf[64];
    double t = js_todate(&env, 0);
    env.push(fmtdatetime(buf, LocalTime(t), LocalTZA()));
}

static void Dp_to_date_string(Environment &env)
{
    char buf[64];
    double t = js_todate(&env, 0);
    env.push(fmtdate(buf, LocalTime(t)));
}

static void Dp_to_time_string(Environment &env)
{
    char buf[64];
    double t = js_todate(&env, 0);
    env.push(fmttime(buf, LocalTime(t), LocalTZA()));
}

static void Dp_to_utc_string(Environment &env)
{
    char buf[64];
    double t = js_todate(&env, 0);
    env.push(fmtdatetime(buf, t, 0));
}

static void Dp_to_iso_string(Environment &env)
{
    char buf[64];
    double t = js_todate(&env, 0);
    if (!isfinite(t))
        throw env.raise("Range error","invalid date");
    env.push(fmtdatetime(buf, t, 0));
}

static void Dp_get_full_year(Environment &env)
{
    double t = js_todate(&env, 0);
    env.push(YearFromTime(LocalTime(t)));
}

static void Dp_get_month(Environment &env)
{
    double t = js_todate(&env, 0);
    env.push(MonthFromTime(LocalTime(t)));
}

static void Dp_get_date(Environment &env)
{
    double t = js_todate(&env, 0);
    env.push(DateFromTime(LocalTime(t)));
}

static void Dp_get_day(Environment &env)
{
    double t = js_todate(&env, 0);
    env.push(WeekDay(LocalTime(t)));
}

static void Dp_get_hours(Environment &env)
{
    double t = js_todate(&env, 0);
    env.push(HourFromTime(LocalTime(t)));
}

static void Dp_get_minutes(Environment &env)
{
    double t = js_todate(&env, 0);
    env.push(MinFromTime(LocalTime(t)));
}

static void Dp_get_seconds(Environment &env)
{
    double t = js_todate(&env, 0);
    env.push(SecFromTime(LocalTime(t)));
}

static void Dp_get_milliseconds(Environment &env)
{
    double t = js_todate(&env, 0);
    env.push(msFromTime(LocalTime(t)));
}

static void Dp_get_utc_full_year(Environment &env)
{
    double t = js_todate(&env, 0);
    env.push(YearFromTime(t));
}

static void Dp_get_utc_month(Environment &env)
{
    double t = js_todate(&env, 0);
    env.push(MonthFromTime(t));
}

static void Dp_get_utc_date(Environment &env)
{
    double t = js_todate(&env, 0);
    env.push(DateFromTime(t));
}

static void Dp_get_utc_day(Environment &env)
{
    double t = js_todate(&env, 0);
    env.push(WeekDay(t));
}

static void Dp_get_utc_hours(Environment &env)
{
    double t = js_todate(&env, 0);
    env.push(HourFromTime(t));
}

static void Dp_get_utc_minutes(Environment &env)
{
    double t = js_todate(&env, 0);
    env.push(MinFromTime(t));
}

static void Dp_get_utc_seconds(Environment &env)
{
    double t = js_todate(&env, 0);
    env.push(SecFromTime(t));
}

static void Dp_get_utc_milliseconds(Environment &env)
{
    double t = js_todate(&env, 0);
    env.push(msFromTime(t));
}

static void Dp_get_time_zone_offset(Environment &env)
{
    double t = js_todate(&env, 0);
    env.push((t - LocalTime(t)) / msPerMinute);
}

static void Dp_set_time(Environment &env)
{
    js_setdate(&env, 0, env.to_number(1));
}

static void Dp_set_milliseconds(Environment &env)
{
    double t = LocalTime(js_todate(&env, 0));
    double h = HourFromTime(t);
    double m = MinFromTime(t);
    double s = SecFromTime(t);
    double ms = env.to_number(1);
    js_setdate(&env, 0, UTC(MakeDate(Day(t), MakeTime(h, m, s, ms))));
}

static void Dp_set_seconds(Environment &env)
{
    double t = LocalTime(js_todate(&env, 0));
    double h = HourFromTime(t);
    double m = MinFromTime(t);
    double s = env.to_number(1);
    double ms = js_optnumber(env, 2, msFromTime(t));
    js_setdate(&env, 0, UTC(MakeDate(Day(t), MakeTime(h, m, s, ms))));
}

static void Dp_set_minutes(Environment &env)
{
    double t = LocalTime(js_todate(&env, 0));
    double h = HourFromTime(t);
    double m = env.to_number(1);
    double s = js_optnumber(env, 2, SecFromTime(t));
    double ms = js_optnumber(env, 3, msFromTime(t));
    js_setdate(&env, 0, UTC(MakeDate(Day(t), MakeTime(h, m, s, ms))));
}

static void Dp_set_hours(Environment &env)
{
    double t = LocalTime(js_todate(&env, 0));
    double h = env.to_number(1);
    double m = js_optnumber(env, 2, MinFromTime(t));
    double s = js_optnumber(env, 3, SecFromTime(t));
    double ms = js_optnumber(env, 4, msFromTime(t));
    js_setdate(&env, 0, UTC(MakeDate(Day(t), MakeTime(h, m, s, ms))));
}

static void Dp_set_date(Environment &env)
{
    double t = LocalTime(js_todate(&env, 0));
    double y = YearFromTime(t);
    double m = MonthFromTime(t);
    double d = env.to_number(1);
    js_setdate(&env, 0, UTC(MakeDate(MakeDay(y, m, d), TimeWithinDay(t))));
}

static void Dp_set_month(Environment &env)
{
    double t = LocalTime(js_todate(&env, 0));
    double y = YearFromTime(t);
    double m = env.to_number(1);
    double d = js_optnumber(env, 3, DateFromTime(t));
    js_setdate(&env, 0, UTC(MakeDate(MakeDay(y, m, d), TimeWithinDay(t))));
}

static void Dp_set_full_year(Environment &env)
{
    double t = LocalTime(js_todate(&env, 0));
    double y = env.to_number(1);
    double m = js_optnumber(env, 2, MonthFromTime(t));
    double d = js_optnumber(env, 3, DateFromTime(t));
    js_setdate(&env, 0, UTC(MakeDate(MakeDay(y, m, d), TimeWithinDay(t))));
}

static void Dp_set_utc_milliseconds(Environment &env)
{
    double t = js_todate(&env, 0);
    double h = HourFromTime(t);
    double m = MinFromTime(t);
    double s = SecFromTime(t);
    double ms = env.to_number(1);
    js_setdate(&env, 0, MakeDate(Day(t), MakeTime(h, m, s, ms)));
}

static void Dp_set_utc_seconds(Environment &env)
{
    double t = js_todate(&env, 0);
    double h = HourFromTime(t);
    double m = MinFromTime(t);
    double s = env.to_number(1);
    double ms = js_optnumber(env, 2, msFromTime(t));
    js_setdate(&env, 0, MakeDate(Day(t), MakeTime(h, m, s, ms)));
}

static void Dp_set_utc_minutes(Environment &env)
{
    double t = js_todate(&env, 0);
    double h = HourFromTime(t);
    double m = env.to_number(1);
    double s = js_optnumber(env, 2, SecFromTime(t));
    double ms = js_optnumber(env, 3, msFromTime(t));
    js_setdate(&env, 0, MakeDate(Day(t), MakeTime(h, m, s, ms)));
}

static void Dp_set_utc_hours(Environment &env)
{
    double t = js_todate(&env, 0);
    double h = env.to_number(1);
    double m = js_optnumber(env, 2, HourFromTime(t));
    double s = js_optnumber(env, 3, SecFromTime(t));
    double ms = js_optnumber(env, 4, msFromTime(t));
    js_setdate(&env, 0, MakeDate(Day(t), MakeTime(h, m, s, ms)));
}

static void Dp_set_utc_date(Environment &env)
{
    double t = js_todate(&env, 0);
    double y = YearFromTime(t);
    double m = MonthFromTime(t);
    double d = env.to_number(1);
    js_setdate(&env, 0, MakeDate(MakeDay(y, m, d), TimeWithinDay(t)));
}

static void Dp_set_utc_month(Environment &env)
{
    double t = js_todate(&env, 0);
    double y = YearFromTime(t);
    double m = env.to_number(1);
    double d = js_optnumber(env, 3, DateFromTime(t));
    js_setdate(&env, 0, MakeDate(MakeDay(y, m, d), TimeWithinDay(t)));
}

static void Dp_set_utc_full_year(Environment &env)
{
    double t = js_todate(&env, 0);
    double y = env.to_number(1);
    double m = js_optnumber(env, 2, MonthFromTime(t));
    double d = js_optnumber(env, 3, DateFromTime(t));
    js_setdate(&env, 0, MakeDate(MakeDay(y, m, d), TimeWithinDay(t)));
}

static void Dp_to_json(Environment &env)
{
    env.copy(0);
    var_to_primitive(&env, -1, PHON_HINT_NUMBER);
    if (env.is_number(-1) && !isfinite(env.to_number(-1)))
    {
        env.push_null();
        return;
    }
    env.pop(1);

    env.get_field(0, "to_iso_string");
    if (!env.is_callable(-1))
        throw env.raise("Type error", "Date.meta.to_json: this.to_iso_string not a function");
    env.copy(0);
    env.call(0);
}

void Environment::init_date()
{
    date_meta->as.number = 0;

    push(date_meta);
    {
        add_method("Date.meta.to_value", Dp_to_value, 0);
        add_method("Date.meta.to_string", Dp_toString, 0);
        add_method("Date.meta.to_date_string", Dp_to_date_string, 0);
        add_method("Date.meta.to_time_string", Dp_to_time_string, 0);
        add_method("Date.meta.to_utc_string", Dp_to_utc_string, 0);

        add_method("Date.meta.get_time", Dp_to_value, 0);
        add_method("Date.meta.get_full_year", Dp_get_full_year, 0);
        add_method("Date.meta.get_utc_full_year", Dp_get_utc_full_year, 0);
        add_method("Date.meta.get_month", Dp_get_month, 0);
        add_method("Date.meta.get_utc_month", Dp_get_utc_month, 0);
        add_method("Date.meta.get_date", Dp_get_date, 0);
        add_method("Date.meta.get_utc_date", Dp_get_utc_date, 0);
        add_method("Date.meta.get_day", Dp_get_day, 0);
        add_method("Date.meta.get_utc_day", Dp_get_utc_day, 0);
        add_method("Date.meta.get_hours", Dp_get_hours, 0);
        add_method("Date.meta.get_utc_hours", Dp_get_utc_hours, 0);
        add_method("Date.meta.get_minutes", Dp_get_minutes, 0);
        add_method("Date.meta.get_utc_minutes", Dp_get_utc_minutes, 0);
        add_method("Date.meta.get_seconds", Dp_get_seconds, 0);
        add_method("Date.meta.get_utc_seconds", Dp_get_utc_seconds, 0);
        add_method("Date.meta.get_milliseconds", Dp_get_milliseconds, 0);
        add_method("Date.meta.get_utc_milliseconds", Dp_get_utc_milliseconds, 0);
        add_method("Date.meta.get_time_zone_offset", Dp_get_time_zone_offset, 0);

        add_method("Date.meta.set_time", Dp_set_time, 1);
        add_method("Date.meta.set_milliseconds", Dp_set_milliseconds, 1);
        add_method("Date.meta.set_utc_milliseconds", Dp_set_utc_milliseconds, 1);
        add_method("Date.meta.set_seconds", Dp_set_seconds, 2);
        add_method("Date.meta.set_utc_seconds", Dp_set_utc_seconds, 2);
        add_method("Date.meta.set_minutes", Dp_set_minutes, 3);
        add_method("Date.meta.set_utc_minutes", Dp_set_utc_minutes, 3);
        add_method("Date.meta.set_hours", Dp_set_hours, 4);
        add_method("Date.meta.set_utc_hours", Dp_set_utc_hours, 4);
        add_method("Date.meta.set_date", Dp_set_date, 1);
        add_method("Date.meta.set_utc_date", Dp_set_utc_date, 1);
        add_method("Date.meta.set_month", Dp_set_month, 2);
        add_method("Date.meta.set_utc_month", Dp_set_utc_month, 2);
        add_method("Date.meta.set_full_year", Dp_set_full_year, 3);
        add_method("Date.meta.set_utc_full_year", Dp_set_utc_full_year, 3);
        add_method("Date.meta.to_iso_string", Dp_to_iso_string, 0);
        add_method("Date.meta.to_json", Dp_to_json, 1);
    }
    new_native_constructor(jsB_Date, jsB_new_Date, "Date", 0); /* 1 */
    {
        add_method("Date.parse", D_parse, 1);
        add_method("Date.utc", D_UTC, 7);
        add_method("Date.now", D_now, 0);
    }
    def_global("Date", PHON_DONTENUM);
}

} // namespace phonometrica