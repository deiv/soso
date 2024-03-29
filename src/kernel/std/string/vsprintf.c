
/*
 * Partially based on code in linux/lib/vsprintf.c
 *
 * All the pointer related code was removed, and '%p' always print the
 *   real address.
 */

#include "kernel/std/stdarg.h"
#include "kernel/std/stddef.h"
#include "kernel/std/types.h"
#include "kernel/std/limits.h"
#include "kernel/std/string.h"
#include "kernel/std/minmax.h"
#include "kernel/std/math64.h"
#include "kernel/std/ctype.h"
#include "kernel/std/errno.h"
#include "kernel/bug/build_bug.h"
#include "kernel/bug/runtime_bug.h"
#include "kernel/compiler/compiler_types.h"

#include "asm/byteorder.h"


// XXX: duplicated in itoa
const char hex_asc_upper[] = "0123456789ABCDEF";

static noinline_for_stack
int skip_atoi(const char **s)
{
    int i = 0;

    do {
        i = i*10 + *((*s)++) - '0';
    } while (isdigit(**s));

    return i;
}

#define SIGN	1		/* unsigned/signed, must be 1 */
#define LEFT	2		/* left justified */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define ZEROPAD	16		/* pad with zero, must be 16 == '0' - ' ' */
#define SMALL	32		/* use lowercase in hex (must be 32 == 0x20) */
#define SPECIAL	64		/* prefix hex with "0x", octal with "0" */

#define FIELD_WIDTH_MAX ((1 << 23) - 1)
#define PRECISION_MAX ((1 << 15) - 1)

static_assert(SIGN == 1);
static_assert(ZEROPAD == ('0' - ' '));
static_assert(SMALL == ('a' ^ 'A'));

enum format_type {
    FORMAT_TYPE_NONE, /* Just a string part */
    FORMAT_TYPE_WIDTH,
    FORMAT_TYPE_PRECISION,
    FORMAT_TYPE_CHAR,
    FORMAT_TYPE_STR,
    FORMAT_TYPE_PTR,
    FORMAT_TYPE_PERCENT_CHAR,
    FORMAT_TYPE_INVALID,
    FORMAT_TYPE_LONG_LONG,
    FORMAT_TYPE_ULONG,
    FORMAT_TYPE_LONG,
    FORMAT_TYPE_UBYTE,
    FORMAT_TYPE_BYTE,
    FORMAT_TYPE_USHORT,
    FORMAT_TYPE_SHORT,
    FORMAT_TYPE_UINT,
    FORMAT_TYPE_INT,
    FORMAT_TYPE_SIZE_T,
    FORMAT_TYPE_PTRDIFF
};

struct printf_spec {
    unsigned int	type:8;		/* format_type enum */
    signed int	field_width:24;	/* width of output field */
    unsigned int	flags:8;	/* flags to number() */
    unsigned int	base:8;		/* number base, 8, 10 or 16 only */
    signed int	precision:16;	/* # of digits/chars */
} __packed;
static_assert(sizeof(struct printf_spec) == 8);

/*
 * Helper function to decode printf style format.
 * Each call decode a token from the format and return the
 * number of characters read (or likely the delta where it wants
 * to go on the next call).
 * The decoded token is returned through the parameters
 *
 * 'h', 'l', or 'L' for integer fields
 * 'z' support added 23/7/1999 S.H.
 * 'z' changed to 'Z' --davidm 1/25/99
 * 'Z' changed to 'z' --adobriyan 2017-01-25
 * 't' added for ptrdiff_t
 *
 * @fmt: the format string
 * @type of the token returned
 * @flags: various flags such as +, -, # tokens..
 * @field_width: overwritten width
 * @base: base of the number (octal, hex, ...)
 * @precision: precision of a number
 * @qualifier: qualifier of a number (long, size_t, ...)
 */
static noinline_for_stack int format_decode(const char *fmt, struct printf_spec *spec)
{
    const char *start = fmt;
    char qualifier;

    /* we finished early by reading the field width */
    if (spec->type == FORMAT_TYPE_WIDTH) {
        if (spec->field_width < 0) {
            spec->field_width = -spec->field_width;
            spec->flags |= LEFT;
        }
        spec->type = FORMAT_TYPE_NONE;
        goto precision;
    }

    /* we finished early by reading the precision */
    if (spec->type == FORMAT_TYPE_PRECISION) {
        if (spec->precision < 0)
            spec->precision = 0;

        spec->type = FORMAT_TYPE_NONE;
        goto qualifier;
    }

    /* By default */
    spec->type = FORMAT_TYPE_NONE;

    for (; *fmt ; ++fmt) {
        if (*fmt == '%')
            break;
    }

    /* Return the current non-format string */
    if (fmt != start || !*fmt)
        return fmt - start;

    /* Process flags */
    spec->flags = 0;

    while (1) { /* this also skips first '%' */
        bool found = true;

        ++fmt;

        switch (*fmt) {
            case '-': spec->flags |= LEFT;    break;
            case '+': spec->flags |= PLUS;    break;
            case ' ': spec->flags |= SPACE;   break;
            case '#': spec->flags |= SPECIAL; break;
            case '0': spec->flags |= ZEROPAD; break;
            default:  found = false;
        }

        if (!found)
            break;
    }

    /* get field width */
    spec->field_width = -1;

    if (isdigit(*fmt))
        spec->field_width = skip_atoi(&fmt);
    else if (*fmt == '*') {
        /* it's the next argument */
        spec->type = FORMAT_TYPE_WIDTH;
        return ++fmt - start;
    }

    precision:
    /* get the precision */
    spec->precision = -1;
    if (*fmt == '.') {
        ++fmt;
        if (isdigit(*fmt)) {
            spec->precision = skip_atoi(&fmt);
            if (spec->precision < 0)
                spec->precision = 0;
        } else if (*fmt == '*') {
            /* it's the next argument */
            spec->type = FORMAT_TYPE_PRECISION;
            return ++fmt - start;
        }
    }

    qualifier:
    /* get the conversion qualifier */
    qualifier = 0;
    if (*fmt == 'h' || _tolower(*fmt) == 'l' ||
        *fmt == 'z' || *fmt == 't') {
        qualifier = *fmt++;
        if (unlikely(qualifier == *fmt)) {
            if (qualifier == 'l') {
                qualifier = 'L';
                ++fmt;
            } else if (qualifier == 'h') {
                qualifier = 'H';
                ++fmt;
            }
        }
    }

    /* default base */
    spec->base = 10;
    switch (*fmt) {
        case 'c':
            spec->type = FORMAT_TYPE_CHAR;
            return ++fmt - start;

        case 's':
            spec->type = FORMAT_TYPE_STR;
            return ++fmt - start;

        case 'p':
            spec->type = FORMAT_TYPE_PTR;
            return ++fmt - start;

        case '%':
            spec->type = FORMAT_TYPE_PERCENT_CHAR;
            return ++fmt - start;

            /* integer number formats - set up the flags and "break" */
        case 'o':
            spec->base = 8;
            break;

        case 'x':
            spec->flags |= SMALL;
            fallthrough;

        case 'X':
            spec->base = 16;
            break;

        case 'd':
        case 'i':
            spec->flags |= SIGN;
            break;
        case 'u':
            break;

        case 'n':
            /*
             * Since %n poses a greater security risk than
             * utility, treat it as any other invalid or
             * unsupported format specifier.
             */
            fallthrough;

        default:
            WARN("Please remove unsupported %%%c in format string\n", *fmt);
            spec->type = FORMAT_TYPE_INVALID;
            return fmt - start;
    }

    if (qualifier == 'L')
        spec->type = FORMAT_TYPE_LONG_LONG;
    else if (qualifier == 'l') {
        //BUILD_BUG_ON(FORMAT_TYPE_ULONG + SIGN != FORMAT_TYPE_LONG);
        spec->type = FORMAT_TYPE_ULONG + (spec->flags & SIGN);
    } else if (qualifier == 'z') {
        spec->type = FORMAT_TYPE_SIZE_T;
    } else if (qualifier == 't') {
        spec->type = FORMAT_TYPE_PTRDIFF;
    } else if (qualifier == 'H') {
        //BUILD_BUG_ON(FORMAT_TYPE_UBYTE + SIGN != FORMAT_TYPE_BYTE);
        spec->type = FORMAT_TYPE_UBYTE + (spec->flags & SIGN);
    } else if (qualifier == 'h') {
        //BUILD_BUG_ON(FORMAT_TYPE_USHORT + SIGN != FORMAT_TYPE_SHORT);
        spec->type = FORMAT_TYPE_USHORT + (spec->flags & SIGN);
    } else {
        //BUILD_BUG_ON(FORMAT_TYPE_UINT + SIGN != FORMAT_TYPE_INT);
        spec->type = FORMAT_TYPE_UINT + (spec->flags & SIGN);
    }

    return ++fmt - start;
}

static void
set_field_width(struct printf_spec *spec, int width)
{
    spec->field_width = width;
    if (WARN_ON_TRUE(spec->field_width != width, "field width %d too large", width)) {
        spec->field_width = clamp(width, -FIELD_WIDTH_MAX, FIELD_WIDTH_MAX);
    }
}

static void
set_precision(struct printf_spec *spec, int prec)
{
    spec->precision = prec;
    if (WARN_ON_TRUE(spec->precision != prec, "precision %d too large", prec)) {
        spec->precision = clamp(prec, 0, PRECISION_MAX);
    }
}

/*
 * Decimal conversion is by far the most typical, and is used for
 * /proc and /sys data. This directly impacts e.g. top performance
 * with many processes running. We optimize it for speed by emitting
 * two characters at a time, using a 200 byte lookup table. This
 * roughly halves the number of multiplications compared to computing
 * the digits one at a time. Implementation strongly inspired by the
 * previous version, which in turn used ideas described at
 * <http://www.cs.uiowa.edu/~jones/bcd/divide.html> (with permission
 * from the author, Douglas W. Jones).
 *
 * It turns out there is precisely one 26 bit fixed-point
 * approximation a of 64/100 for which x/100 == (x * (u64)a) >> 32
 * holds for all x in [0, 10^8-1], namely a = 0x28f5c29. The actual
 * range happens to be somewhat larger (x <= 1073741898), but that's
 * irrelevant for our purpose.
 *
 * For dividing a number in the range [10^4, 10^6-1] by 100, we still
 * need a 32x32->64 bit multiply, so we simply use the same constant.
 *
 * For dividing a number in the range [100, 10^4-1] by 100, there are
 * several options. The simplest is (x * 0x147b) >> 19, which is valid
 * for all x <= 43698.
 */

static const u16 decpair[100] = {
#define _(x) (u16) cpu_to_le16(((x % 10) | ((x / 10) << 8)) + 0x3030)
        _( 0), _( 1), _( 2), _( 3), _( 4), _( 5), _( 6), _( 7), _( 8), _( 9),
        _(10), _(11), _(12), _(13), _(14), _(15), _(16), _(17), _(18), _(19),
        _(20), _(21), _(22), _(23), _(24), _(25), _(26), _(27), _(28), _(29),
        _(30), _(31), _(32), _(33), _(34), _(35), _(36), _(37), _(38), _(39),
        _(40), _(41), _(42), _(43), _(44), _(45), _(46), _(47), _(48), _(49),
        _(50), _(51), _(52), _(53), _(54), _(55), _(56), _(57), _(58), _(59),
        _(60), _(61), _(62), _(63), _(64), _(65), _(66), _(67), _(68), _(69),
        _(70), _(71), _(72), _(73), _(74), _(75), _(76), _(77), _(78), _(79),
        _(80), _(81), _(82), _(83), _(84), _(85), _(86), _(87), _(88), _(89),
        _(90), _(91), _(92), _(93), _(94), _(95), _(96), _(97), _(98), _(99),
#undef _
};

/*
 * This will print a single '0' even if r == 0, since we would
 * immediately jump to out_r where two 0s would be written but only
 * one of them accounted for in buf. This is needed by ip4_string
 * below. All other callers pass a non-zero value of r.
*/
static noinline_for_stack
char *put_dec_trunc8(char *buf, unsigned r)
{
    unsigned q;

    /* 1 <= r < 10^8 */
    if (r < 100)
        goto out_r;

    /* 100 <= r < 10^8 */
    q = (r * (u64)0x28f5c29) >> 32;
    *((u16 *)buf) = decpair[r - 100*q];
    buf += 2;

    /* 1 <= q < 10^6 */
    if (q < 100)
        goto out_q;

    /*  100 <= q < 10^6 */
    r = (q * (u64)0x28f5c29) >> 32;
    *((u16 *)buf) = decpair[q - 100*r];
    buf += 2;

    /* 1 <= r < 10^4 */
    if (r < 100)
        goto out_r;

    /* 100 <= r < 10^4 */
    q = (r * 0x147b) >> 19;
    *((u16 *)buf) = decpair[r - 100*q];
    buf += 2;
    out_q:
    /* 1 <= q < 100 */
    r = q;
    out_r:
    /* 1 <= r < 100 */
    *((u16 *)buf) = decpair[r];
    buf += r < 10 ? 1 : 2;
    return buf;
}

static noinline_for_stack
char *put_dec_full8(char *buf, unsigned r)
{
	unsigned q;

	/* 0 <= r < 10^8 */
	q = (r * (u64)0x28f5c29) >> 32;
	*((u16 *)buf) = decpair[r - 100*q];
	buf += 2;

	/* 0 <= q < 10^6 */
	r = (q * (u64)0x28f5c29) >> 32;
	*((u16 *)buf) = decpair[q - 100*r];
	buf += 2;

	/* 0 <= r < 10^4 */
	q = (r * 0x147b) >> 19;
	*((u16 *)buf) = decpair[r - 100*q];
	buf += 2;

	/* 0 <= q < 100 */
	*((u16 *)buf) = decpair[q];
	buf += 2;
	return buf;
}

static noinline_for_stack
char *put_dec(char *buf, unsigned long long n)
{
	if (n >= 100*1000*1000)
		buf = put_dec_full8(buf, do_div(n, 100*1000*1000));
	/* 1 <= n <= 1.6e11 */
	if (n >= 100*1000*1000)
		buf = put_dec_full8(buf, do_div(n, 100*1000*1000));
	/* 1 <= n < 1e8 */
	return put_dec_trunc8(buf, n);
}

static noinline_for_stack
char *number(char *buf, char *end, unsigned long long num,
             struct printf_spec spec)
{
    /* put_dec requires 2-byte alignment of the buffer. */
    char tmp[3 * sizeof(num)] __aligned(2);
    char sign;
    char locase;
    int need_pfx = ((spec.flags & SPECIAL) && spec.base != 10);
    int i;
    bool is_zero = num == 0LL;
    int field_width = spec.field_width;
    int precision = spec.precision;

    /* locase = 0 or 0x20. ORing digits or letters with 'locase'
     * produces same digits or (maybe lowercased) letters */
    locase = (spec.flags & SMALL);
    if (spec.flags & LEFT)
        spec.flags &= ~ZEROPAD;
    sign = 0;
    if (spec.flags & SIGN) {
        if ((signed long long)num < 0) {
            sign = '-';
            num = -(signed long long)num;
            field_width--;
        } else if (spec.flags & PLUS) {
            sign = '+';
            field_width--;
        } else if (spec.flags & SPACE) {
            sign = ' ';
            field_width--;
        }
    }
    if (need_pfx) {
        if (spec.base == 16)
            field_width -= 2;
        else if (!is_zero)
            field_width--;
    }

    /* generate full string in tmp[], in reverse order */
    i = 0;
    if (num < spec.base)
        tmp[i++] = hex_asc_upper[num] | locase;
    else if (spec.base != 10) { /* 8 or 16 */
        int mask = spec.base - 1;
        int shift = 3;

        if (spec.base == 16)
            shift = 4;
        do {
            tmp[i++] = (hex_asc_upper[((unsigned char)num) & mask] | locase);
            num >>= shift;
        } while (num);
    } else { /* base 10 */
        i = put_dec(tmp, num) - tmp;
    }

    /* printing 100 using %2d gives "100", not "00" */
    if (i > precision)
        precision = i;
    /* leading space padding */
    field_width -= precision;
    if (!(spec.flags & (ZEROPAD | LEFT))) {
        while (--field_width >= 0) {
            if (buf < end)
                *buf = ' ';
            ++buf;
        }
    }
    /* sign */
    if (sign) {
        if (buf < end)
            *buf = sign;
        ++buf;
    }
    /* "0x" / "0" prefix */
    if (need_pfx) {
        if (spec.base == 16 || !is_zero) {
            if (buf < end)
                *buf = '0';
            ++buf;
        }
        if (spec.base == 16) {
            if (buf < end)
                *buf = ('X' | locase);
            ++buf;
        }
    }
    /* zero or space padding */
    if (!(spec.flags & LEFT)) {
        char c = ' ' + (spec.flags & ZEROPAD);

        while (--field_width >= 0) {
            if (buf < end)
                *buf = c;
            ++buf;
        }
    }
    /* hmm even more zero padding? */
    while (i <= --precision) {
        if (buf < end)
            *buf = '0';
        ++buf;
    }
    /* actual digits of result */
    while (--i >= 0) {
        if (buf < end)
            *buf = tmp[i];
        ++buf;
    }
    /* trailing space padding */
    while (--field_width >= 0) {
        if (buf < end)
            *buf = ' ';
        ++buf;
    }

    return buf;
}

static void move_right(char *buf, char *end, unsigned len, unsigned spaces)
{
    size_t size;
    if (buf >= end)	/* nowhere to put anything */
        return;
    size = end - buf;
    if (size <= spaces) {
        memset(buf, ' ', size);
        return;
    }
    if (len) {
        if (len > size - spaces)
            len = size - spaces;
        memmove(buf + spaces, buf, len);
    }
    memset(buf, ' ', spaces);
}

/*
 * Handle field width padding for a string.
 * @buf: current buffer position
 * @n: length of string
 * @end: end of output buffer
 * @spec: for field width and flags
 * Returns: new buffer position after padding.
 */
static noinline_for_stack
char *widen_string(char *buf, int n, char *end, struct printf_spec spec)
{
    unsigned spaces;

    if (likely(n >= spec.field_width))
        return buf;
    /* we want to pad the sucker */
    spaces = spec.field_width - n;
    if (!(spec.flags & LEFT)) {
        move_right(buf - n, end, n, spaces);
        return buf + spaces;
    }
    while (spaces--) {
        if (buf < end)
            *buf = ' ';
        ++buf;
    }
    return buf;
}

/* Handle string from a well known address. */
static char *string_nocheck(char *buf, char *end, const char *s,
                            struct printf_spec spec)
{
    int len = 0;
    int lim = spec.precision;

    while (lim--) {
        char c = *s++;
        if (!c)
            break;
        if (buf < end)
            *buf = c;
        ++buf;
        ++len;
    }
    return widen_string(buf, len, end, spec);
}

/* Be careful: error messages must fit into the given buffer. */
static char *error_string(char *buf, char *end, const char *s,
                          struct printf_spec spec)
{
    /*
     * Hard limit to avoid a completely insane messages. It actually
     * works pretty well because most error messages are in
     * the many pointer format modifiers.
     */
    if (spec.precision == -1)
        spec.precision = 2 * sizeof(void *);

    return string_nocheck(buf, end, s, spec);
}

/*
 * Do not call any complex external code here. Nested printk()/vsprintf()
 * might cause infinite loops. Failures might break printk() and would
 * be hard to debug.
 */
static const char *check_pointer_msg(const void *ptr)
{
    if (!ptr)
        return "(null)";

    // XXX:  ???
    /*if ((unsigned long)ptr < PAGE_SIZE || IS_ERR_VALUE(ptr))
        return "(efault)";*/

    return NULL;
}

static int check_pointer(char **buf, char *end, const void *ptr,
                         struct printf_spec spec)
{
    const char *err_msg;

    err_msg = check_pointer_msg(ptr);
    if (err_msg) {
        *buf = error_string(*buf, end, err_msg, spec);
        return -EFAULT;
    }

    return 0;
}

static noinline_for_stack
char *string(char *buf, char *end, const char *s,
             struct printf_spec spec)
{
    if (check_pointer(&buf, end, s, spec))
        return buf;

    return string_nocheck(buf, end, s, spec);
}


/*
 * Show a '%p' thing.  A kernel extension is that the '%p' is followed
 * by an extra set of alphanumeric characters that are extended format
 * specifiers.
 *
 * Please update scripts/checkpatch.pl when adding/removing conversion
 * characters.  (Search for "check for vsprintf extension").
 *
 * Right now we handle:
 *
 * - 'S' For symbolic direct pointers (or function descriptors) with offset
 * - 's' For symbolic direct pointers (or function descriptors) without offset
 * - '[Ss]R' as above with __builtin_extract_return_addr() translation
 * - 'S[R]b' as above with module build ID (for use in backtraces)
 * - '[Ff]' %pf and %pF were obsoleted and later removed in favor of
 *	    %ps and %pS. Be careful when re-using these specifiers.
 * - 'B' For backtraced symbolic direct pointers with offset
 * - 'Bb' as above with module build ID (for use in backtraces)
 * - 'R' For decoded struct resource, e.g., [mem 0x0-0x1f 64bit pref]
 * - 'r' For raw struct resource, e.g., [mem 0x0-0x1f flags 0x201]
 * - 'b[l]' For a bitmap, the number of bits is determined by the field
 *       width which must be explicitly specified either as part of the
 *       format string '%32b[l]' or through '%*b[l]', [l] selects
 *       range-list format instead of hex format
 * - 'M' For a 6-byte MAC address, it prints the address in the
 *       usual colon-separated hex notation
 * - 'm' For a 6-byte MAC address, it prints the hex address without colons
 * - 'MF' For a 6-byte MAC FDDI address, it prints the address
 *       with a dash-separated hex notation
 * - '[mM]R' For a 6-byte MAC address, Reverse order (Bluetooth)
 * - 'I' [46] for IPv4/IPv6 addresses printed in the usual way
 *       IPv4 uses dot-separated decimal without leading 0's (1.2.3.4)
 *       IPv6 uses colon separated network-order 16 bit hex with leading 0's
 *       [S][pfs]
 *       Generic IPv4/IPv6 address (struct sockaddr *) that falls back to
 *       [4] or [6] and is able to print port [p], flowinfo [f], scope [s]
 * - 'i' [46] for 'raw' IPv4/IPv6 addresses
 *       IPv6 omits the colons (01020304...0f)
 *       IPv4 uses dot-separated decimal with leading 0's (010.123.045.006)
 *       [S][pfs]
 *       Generic IPv4/IPv6 address (struct sockaddr *) that falls back to
 *       [4] or [6] and is able to print port [p], flowinfo [f], scope [s]
 * - '[Ii][4S][hnbl]' IPv4 addresses in host, network, big or little endian order
 * - 'I[6S]c' for IPv6 addresses printed as specified by
 *       https://tools.ietf.org/html/rfc5952
 * - 'E[achnops]' For an escaped buffer, where rules are defined by combination
 *                of the following flags (see string_escape_mem() for the
 *                details):
 *                  a - ESCAPE_ANY
 *                  c - ESCAPE_SPECIAL
 *                  h - ESCAPE_HEX
 *                  n - ESCAPE_NULL
 *                  o - ESCAPE_OCTAL
 *                  p - ESCAPE_NP
 *                  s - ESCAPE_SPACE
 *                By default ESCAPE_ANY_NP is used.
 * - 'U' For a 16 byte UUID/GUID, it prints the UUID/GUID in the form
 *       "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
 *       Options for %pU are:
 *         b big endian lower case hex (default)
 *         B big endian UPPER case hex
 *         l little endian lower case hex
 *         L little endian UPPER case hex
 *           big endian output byte order is:
 *             [0][1][2][3]-[4][5]-[6][7]-[8][9]-[10][11][12][13][14][15]
 *           little endian output byte order is:
 *             [3][2][1][0]-[5][4]-[7][6]-[8][9]-[10][11][12][13][14][15]
 * - 'V' For a struct va_format which contains a format string * and va_list *,
 *       call vsnprintf(->format, *->va_list).
 *       Implements a "recursive vsnprintf".
 *       Do not use this feature without some mechanism to verify the
 *       correctness of the format string and va_list arguments.
 * - 'K' For a kernel pointer that should be hidden from unprivileged users.
 *       Use only for procfs, sysfs and similar files, not printk(); please
 *       read the documentation (path below) first.
 * - 'NF' For a netdev_features_t
 * - '4cc' V4L2 or DRM FourCC code, with endianness and raw numerical value.
 * - 'h[CDN]' For a variable-length buffer, it prints it as a hex string with
 *            a certain separator (' ' by default):
 *              C colon
 *              D dash
 *              N no separator
 *            The maximum supported length is 64 bytes of the input. Consider
 *            to use print_hex_dump() for the larger input.
 * - 'a[pd]' For address types [p] phys_addr_t, [d] dma_addr_t and derivatives
 *           (default assumed to be phys_addr_t, passed by reference)
 * - 'd[234]' For a dentry name (optionally 2-4 last components)
 * - 'D[234]' Same as 'd' but for a struct file
 * - 'g' For block_device name (gendisk + partition number)
 * - 't[RT][dt][r][s]' For time and date as represented by:
 *      R    struct rtc_time
 *      T    time64_t
 * - 'C' For a clock, it prints the name (Common Clock Framework) or address
 *       (legacy clock framework) of the clock
 * - 'Cn' For a clock, it prints the name (Common Clock Framework) or address
 *        (legacy clock framework) of the clock
 * - 'G' For flags to be printed as a collection of symbolic strings that would
 *       construct the specific value. Supported flags given by option:
 *       p page flags (see struct page) given as pointer to unsigned long
 *       g gfp flags (GFP_* and __GFP_*) given as pointer to gfp_t
 *       v vma flags (VM_*) given as pointer to unsigned long
 * - 'OF[fnpPcCF]'  For a device tree object
 *                  Without any optional arguments prints the full_name
 *                  f device node full_name
 *                  n device node name
 *                  p device node phandle
 *                  P device node path spec (name + @unit)
 *                  F device node flags
 *                  c major compatible string
 *                  C full compatible string
 * - 'fw[fP]'	For a firmware node (struct fwnode_handle) pointer
 *		Without an option prints the full name of the node
 *		f full name
 *		P node name, including a possible unit address
 * - 'x' For printing the address unmodified. Equivalent to "%lx".
 *       Please read the documentation (path below) before using!
 * - '[ku]s' For a BPF/tracing related format specifier, e.g. used out of
 *           bpf_trace_printk() where [ku] prefix specifies either kernel (k)
 *           or user (u) memory to probe, and:
 *              s a string, equivalent to "%s" on direct vsnprintf() use
 *
 * ** When making changes please also update:
 *	Documentation/core-api/printk-formats.rst
 *
 * Note: The default behaviour (unadorned %p) is to hash the address,
 * rendering it useful as a unique identifier.
 */
# if 0
static noinline_for_stack
char *pointer(const char *fmt, char *buf, char *end, void *ptr,
              struct printf_spec spec)
{
    switch (*fmt) {
        case 'S':
        case 's':
            ptr = dereference_symbol_descriptor(ptr);
            fallthrough;
        case 'B':
            return symbol_string(buf, end, ptr, spec, fmt);
        case 'R':
        case 'r':
            return resource_string(buf, end, ptr, spec, fmt);
        case 'h':
            return hex_string(buf, end, ptr, spec, fmt);
        case 'b':
            switch (fmt[1]) {
                case 'l':
                    return bitmap_list_string(buf, end, ptr, spec, fmt);
                default:
                    return bitmap_string(buf, end, ptr, spec, fmt);
            }
        case 'M':			/* Colon separated: 00:01:02:03:04:05 */
        case 'm':			/* Contiguous: 000102030405 */
            /* [mM]F (FDDI) */
            /* [mM]R (Reverse order; Bluetooth) */
            return mac_address_string(buf, end, ptr, spec, fmt);
        case 'I':			/* Formatted IP supported
					 * 4:	1.2.3.4
					 * 6:	0001:0203:...:0708
					 * 6c:	1::708 or 1::1.2.3.4
					 */
        case 'i':			/* Contiguous:
					 * 4:	001.002.003.004
					 * 6:   000102...0f
					 */
            return ip_addr_string(buf, end, ptr, spec, fmt);
        case 'E':
            return escaped_string(buf, end, ptr, spec, fmt);
        case 'U':
            return uuid_string(buf, end, ptr, spec, fmt);
        case 'V':
            return va_format(buf, end, ptr, spec, fmt);
        case 'K':
            return restricted_pointer(buf, end, ptr, spec);
        case 'N':
            return netdev_bits(buf, end, ptr, spec, fmt);
        case '4':
            return fourcc_string(buf, end, ptr, spec, fmt);
        case 'a':
            return address_val(buf, end, ptr, spec, fmt);
        case 'd':
            return dentry_name(buf, end, ptr, spec, fmt);
        case 't':
            return time_and_date(buf, end, ptr, spec, fmt);
        case 'C':
            return clock(buf, end, ptr, spec, fmt);
        case 'D':
            return file_dentry_name(buf, end, ptr, spec, fmt);
#ifdef CONFIG_BLOCK
            case 'g':
		return bdev_name(buf, end, ptr, spec, fmt);
#endif

        case 'G':
            return flags_string(buf, end, ptr, spec, fmt);
        case 'O':
            return device_node_string(buf, end, ptr, spec, fmt + 1);
        case 'f':
            return fwnode_string(buf, end, ptr, spec, fmt + 1);
        case 'x':
            return pointer_string(buf, end, ptr, spec);
        case 'e':
            /* %pe with a non-ERR_PTR gets treated as plain %p */
            if (!IS_ERR(ptr))
                return default_pointer(buf, end, ptr, spec);
            return err_ptr(buf, end, ptr, spec);
        case 'u':
        case 'k':
            switch (fmt[1]) {
                case 's':
                    return string(buf, end, ptr, spec);
                default:
                    return error_string(buf, end, "(einval)", spec);
            }
        default:
            return default_pointer(buf, end, ptr, spec);
    }
}

#endif

static char *pointer_string(char *buf, char *end,
                            const void *ptr,
                            struct printf_spec spec)
{
    spec.base = 16;
    spec.flags |= SMALL;
    if (spec.field_width == -1) {
        spec.field_width = 2 * sizeof(ptr);
        spec.flags |= ZEROPAD;
    }

    return number(buf, end, (unsigned long int)ptr, spec);
}

static char *default_pointer(char *buf, char *end, const void *ptr,
                             struct printf_spec spec)
{
    /*
     * default is to _not_ leak addresses, so hash before printing,
     * unless no_hash_pointers is specified on the command line.
     */
    //if (unlikely(no_hash_pointers))
        return pointer_string(buf, end, ptr, spec);

    //return ptr_to_id(buf, end, ptr, spec);
}

static noinline_for_stack
char *pointer(const char *fmt, char *buf, char *end, void *ptr,
              struct printf_spec spec)
{
    return default_pointer(buf, end, ptr, spec);
}

/**
 * vsnprintf - Format a string and place it in a buffer
 * @buf: The buffer to place the result into
 * @size: The size of the buffer, including the trailing null space
 * @fmt: The format string to use
 * @args: Arguments for the format string
 *
 * This function generally follows C99 vsnprintf, but has some
 * extensions and a few limitations:
 *
 *  - ``%n`` is unsupported
 *  - ``%p*`` is handled by pointer()
 *
 * See pointer() or Documentation/core-api/printk-formats.rst for more
 * extensive description.
 *
 * **Please update the documentation in both places when making changes**
 *
 * The return value is the number of characters which would
 * be generated for the given input, excluding the trailing
 * '\0', as per ISO C99. If you want to have the exact
 * number of characters written into @buf as return value
 * (not including the trailing '\0'), use vscnprintf(). If the
 * return is greater than or equal to @size, the resulting
 * string is truncated.
 *
 * If you're not already dealing with a va_list consider using snprintf().
 */
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
    unsigned long long num;
    char *str, *end;
    struct printf_spec spec = {0};

    /* Reject out-of-range values early.  Large positive sizes are
       used for unknown buffer sizes. */
    if (WARN_ON_TRUE(size > INT_MAX, "vsnprintf: size parameter exceeds INT_MAX, size = %i", size))
        return 0;

    str = buf;
    end = buf + size;

    /* Make sure end is always >= buf */
    if (end < buf) {
        end = ((void *)-1);
        size = end - buf;
    }

    while (*fmt) {
        const char *old_fmt = fmt;
        int read = format_decode(fmt, &spec);

        fmt += read;

        switch (spec.type) {
            case FORMAT_TYPE_NONE: {
                int copy = read;
                if (str < end) {
                    if (copy > end - str)
                        copy = end - str;
                    memcpy(str, old_fmt, copy);
                }
                str += read;
                break;
            }

            case FORMAT_TYPE_WIDTH:
                set_field_width(&spec, va_arg(args, int));
                break;

            case FORMAT_TYPE_PRECISION:
                set_precision(&spec, va_arg(args, int));
                break;

            case FORMAT_TYPE_CHAR: {
                char c;

                if (!(spec.flags & LEFT)) {
                    while (--spec.field_width > 0) {
                        if (str < end)
                            *str = ' ';
                        ++str;

                    }
                }
                c = (unsigned char) va_arg(args, int);
                if (str < end)
                    *str = c;
                ++str;
                while (--spec.field_width > 0) {
                    if (str < end)
                        *str = ' ';
                    ++str;
                }
                break;
            }

            case FORMAT_TYPE_STR:
                str = string(str, end, va_arg(args, char *), spec);
                break;

            case FORMAT_TYPE_PTR:
                str = pointer(fmt, str, end, va_arg(args, void *),
                spec);
                while (isalnum(*fmt))
                    fmt++;
                break;

            case FORMAT_TYPE_PERCENT_CHAR:
                if (str < end)
                    *str = '%';
                ++str;
                break;

            case FORMAT_TYPE_INVALID:
                /*
                 * Presumably the arguments passed gcc's type
                 * checking, but there is no safe or sane way
                 * for us to continue parsing the format and
                 * fetching from the va_list; the remaining
                 * specifiers and arguments would be out of
                 * sync.
                 */
                goto out;

            default:
                switch (spec.type) {
                    case FORMAT_TYPE_LONG_LONG:
                        num = va_arg(args, long long);
                        break;
                    case FORMAT_TYPE_ULONG:
                        num = va_arg(args, unsigned long);
                        break;
                    case FORMAT_TYPE_LONG:
                        num = va_arg(args, long);
                        break;
                    case FORMAT_TYPE_SIZE_T:
                        if (spec.flags & SIGN)
                            num = va_arg(args, ssize_t);
                        else
                            num = va_arg(args, size_t);
                        break;
                    case FORMAT_TYPE_PTRDIFF:
                        num = va_arg(args, ptrdiff_t);
                        break;
                    case FORMAT_TYPE_UBYTE:
                        num = (unsigned char) va_arg(args, int);
                        break;
                    case FORMAT_TYPE_BYTE:
                        num = (signed char) va_arg(args, int);
                        break;
                    case FORMAT_TYPE_USHORT:
                        num = (unsigned short) va_arg(args, int);
                        break;
                    case FORMAT_TYPE_SHORT:
                        num = (short) va_arg(args, int);
                        break;
                    case FORMAT_TYPE_INT:
                        num = (int) va_arg(args, int);
                        break;
                    default:
                        num = va_arg(args, unsigned int);
                }

                str = number(str, end, num, spec);
        }
    }

    out:
    if (size > 0) {
        if (str < end)
            *str = '\0';
        else
            end[-1] = '\0';
    }

    /* the trailing null byte doesn't count towards the total */
    return str-buf;
}


/**
 * snprintf - Format a string and place it in a buffer
 * @buf: The buffer to place the result into
 * @size: The size of the buffer, including the trailing null space
 * @fmt: The format string to use
 * @...: Arguments for the format string
 *
 * The return value is the number of characters which would be
 * generated for the given input, excluding the trailing null,
 * as per ISO C99.  If the return is greater than or equal to
 * @size, the resulting string is truncated.
 *
 * See the vsnprintf() documentation for format string extensions over C99.
 */
int snprintf(char *buf, size_t size, const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i = vsnprintf(buf, size, fmt, args);
    va_end(args);

    return i;
}
