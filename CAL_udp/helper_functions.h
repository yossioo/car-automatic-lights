#pragma once


/// Convert a 32-bit integer to 4 byte sequence
void uint32to4bytes(uint32_t in, unsigned char *out)
{
    for (int i = 0; i < 4; i++)
        out[3 - i] = (in >> (i * 8));
}
