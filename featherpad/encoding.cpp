/*
 featherpad/encoding.cpp
 */

#include "encoding.h"

namespace FeatherPad {

bool validateUTF8(const QByteArray byteArray) {
    const unsigned char* p = reinterpret_cast<const unsigned char*>(byteArray.constData());
    const unsigned char* end = p + byteArray.size();

    while (p < end) {
        unsigned char c = *p++;

        // Fast path for ASCII:
        if (c < 0x80) {
            // c is ASCII, fine, move on
            continue;
        }

        // Otherwise, c >= 0x80.
        // Check leading-byte ranges and subsequent bytes:

        // Two-byte form: [C2..DF] [80..BF]
        if (c >= 0xC2 && c <= 0xDF) {
            if (p == end)
                return false;  // need 1 more byte
            if ((p[0] & 0xC0) != 0x80)
                return false;
            p += 1;
            continue;
        }

        //    Three-byte form: [E0..EF] 2 subsequent [80..BF] bytes
        //    We also must watch out for surrogates [0xD800..0xDFFF]
        else if (c >= 0xE0 && c <= 0xEF) {
            if (end - p < 2)
                return false;  // need 2 more bytes
            if ((p[0] & 0xC0) != 0x80 || (p[1] & 0xC0) != 0x80)
                return false;

            // Minimal form checks for E0 and ED:
            // E0 must not be followed by [80..9F], ED must not be followed by [A0..BF]
            if (c == 0xE0 && (unsigned char)p[0] < 0xA0)
                return false;
            if (c == 0xED && (unsigned char)p[0] > 0x9F)
                return false;

            // Surrogate range check:
            unsigned int code = ((c & 0x0F) << 12) | ((p[0] & 0x3F) << 6) | (p[1] & 0x3F);
            // Surrogates [0xD800..0xDFFF] are invalid in UTF-8
            if (code >= 0xD800 && code <= 0xDFFF)
                return false;

            p += 2;
            continue;
        }

        //    Four-byte form: [F0..F4] + 3 subsequent [80..BF] bytes
        //    (UTF-8 up to 0x10FFFF; 0xF5..0xFF are invalid leading bytes)
        else if (c >= 0xF0 && c <= 0xF4) {
            if (end - p < 3)
                return false;  // need 3 more bytes
            if ((p[0] & 0xC0) != 0x80 || (p[1] & 0xC0) != 0x80 || (p[2] & 0xC0) != 0x80)
                return false;

            // Check minimal form for F0, plus top-of-range for F4
            if (c == 0xF0 && (unsigned char)p[0] < 0x90)
                return false;  // F0 90..BF ...
            if (c == 0xF4 && (unsigned char)p[0] > 0x8F)
                return false;  // F4 80..8F

            // Make sure the resulting code point is <= 0x10FFFF
            unsigned int code = ((c & 0x07) << 18) | ((p[0] & 0x3F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
            if (code > 0x10FFFF)
                return false;

            p += 3;
            continue;
        }

        // Anything else in [80..C1], [F5..FF], or missing bytes is invalid:
        return false;
    }

    return true;
}

/*************************/
const QString detectCharset(const QByteArray& byteArray) {
    if (validateUTF8(byteArray))
        return QStringLiteral("UTF-8");
    /* fallback: legacy encodings are no longer supported on Qt6+ by default,
       so we just return ISO-8859-1 here */
    return QStringLiteral("ISO-8859-1");
}

}  // namespace FeatherPad
