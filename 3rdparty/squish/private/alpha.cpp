/* -----------------------------------------------------------------------------

  Copyright (c) 2006 Simon Brown                          si@sjbrown.co.uk

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to	deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   -------------------------------------------------------------------------- */

#include "alpha.h"
#include <climits>
#include <algorithm>

namespace squish
{
static int FloatToInt(float a, int limit)
{
  // use ANSI round-to-zero behaviour to get round-to-nearest
  int i = static_cast<int>(a + 0.5f);

  // clamp to the limit
  if (i < 0)
    i = 0;
  else if (i > limit)
    i = limit;

  // done
  return i;
}

void CompressAlphaDxt3(gsl::span<const std::uint8_t, 64> rgba, int mask,
                       void* block)
{
  std::uint8_t* bytes = reinterpret_cast<std::uint8_t*>(block);

  // quantise and pack the alpha values pairwise
  for (int i = 0; i < 8; ++i)
  {
    // quantise down to 4 bits
    float alpha1 = rgba[8 * i + 3] * (15.0f / 255.0f);
    float alpha2 = rgba[8 * i + 7] * (15.0f / 255.0f);
    int quant1 = FloatToInt(alpha1, 15);
    int quant2 = FloatToInt(alpha2, 15);

    // set alpha to zero where masked
    int bit1 = 1 << (2 * i);
    int bit2 = 1 << (2 * i + 1);
    if ((mask & bit1) == 0)
      quant1 = 0;
    if ((mask & bit2) == 0)
      quant2 = 0;

    // pack into the byte
    bytes[i] = static_cast<std::uint8_t>(quant1 | (quant2 << 4));
  }
}

void DecompressAlphaDxt3(gsl::span<std::uint8_t, 64> rgba, void const* block)
{
  auto* bytes = reinterpret_cast<const std::uint8_t*>(block);

  // unpack the alpha values pairwise
  for (int i = 0; i < 8; ++i)
  {
    // quantise down to 4 bits
    std::uint8_t quant = bytes[i];

    // unpack the values
    std::uint8_t lo = quant & 0x0f;
    std::uint8_t hi = quant & 0xf0;

    // convert back up to bytes
    rgba[8 * i + 3] = static_cast<std::uint8_t>(lo | (lo << 4));
    rgba[8 * i + 7] = static_cast<std::uint8_t>(hi | (hi >> 4));
  }
}

static void FixRange(int& min, int& max, int steps)
{
  if (max - min < steps)
    max = std::min(min + steps, 255);
  if (max - min < steps)
    min = std::max(0, max - steps);
}

static int FitCodes(gsl::span<const std::uint8_t, 64> rgba, int mask,
                    std::uint8_t const* codes, std::uint8_t* indices)
{
  // fit each alpha value to the codebook
  int err = 0;
  for (int i = 0; i < 16; ++i)
  {
    // check this pixel is valid
    int bit = 1 << i;
    if ((mask & bit) == 0)
    {
      // use the first code
      indices[i] = 0;
      continue;
    }

    // find the least error and corresponding index
    int value = rgba[4 * i + 3];
    int least = INT_MAX;
    int index = 0;
    for (int j = 0; j < 8; ++j)
    {
      // get the squared error from this code
      int dist = value - static_cast<int>(codes[j]);
      dist *= dist;

      // compare with the best so far
      if (dist < least)
      {
        least = dist;
        index = j;
      }
    }

    // save this index and accumulate the error
    indices[i] = static_cast<std::uint8_t>(index);
    err += least;
  }

  // return the total error
  return err;
}

static void WriteAlphaBlock(int alpha0, int alpha1, std::uint8_t const* indices,
                            void* block)
{
  std::uint8_t* bytes = reinterpret_cast<std::uint8_t*>(block);

  // write the first two bytes
  bytes[0] = static_cast<std::uint8_t>(alpha0);
  bytes[1] = static_cast<std::uint8_t>(alpha1);

  // pack the indices with 3 bits each
  std::uint8_t* dest = bytes + 2;
  std::uint8_t const* src = indices;
  for (int i = 0; i < 2; ++i)
  {
    // pack 8 3-bit values
    int value = 0;
    for (int j = 0; j < 8; ++j)
    {
      int index = *src++;
      value |= (index << 3 * j);
    }

    // store in 3 bytes
    for (int j = 0; j < 3; ++j)
    {
      int byte = (value >> 8 * j) & 0xff;
      *dest++ = static_cast<std::uint8_t>(byte);
    }
  }
}

static void WriteAlphaBlock5(int alpha0, int alpha1,
                             std::uint8_t const* indices, void* block)
{
  // check the relative values of the endpoints
  if (alpha0 > alpha1)
  {
    // swap the indices
    std::uint8_t swapped[16];
    for (int i = 0; i < 16; ++i)
    {
      std::uint8_t index = indices[i];
      if (index == 0)
        swapped[i] = 1;
      else if (index == 1)
        swapped[i] = 0;
      else if (index <= 5)
        swapped[i] = 7 - index;
      else
        swapped[i] = index;
    }

    // write the block
    WriteAlphaBlock(alpha1, alpha0, swapped, block);
  }
  else
  {
    // write the block
    WriteAlphaBlock(alpha0, alpha1, indices, block);
  }
}

static void WriteAlphaBlock7(int alpha0, int alpha1,
                             std::uint8_t const* indices, void* block)
{
  // check the relative values of the endpoints
  if (alpha0 < alpha1)
  {
    // swap the indices
    std::uint8_t swapped[16];
    for (int i = 0; i < 16; ++i)
    {
      std::uint8_t index = indices[i];
      if (index == 0)
        swapped[i] = 1;
      else if (index == 1)
        swapped[i] = 0;
      else
        swapped[i] = 9 - index;
    }

    // write the block
    WriteAlphaBlock(alpha1, alpha0, swapped, block);
  }
  else
  {
    // write the block
    WriteAlphaBlock(alpha0, alpha1, indices, block);
  }
}

void CompressAlphaDxt5(gsl::span<const std::uint8_t, 64> rgba, int mask,
                       void* block)
{
  // get the range for 5-alpha and 7-alpha interpolation
  int min5 = 255;
  int max5 = 0;
  int min7 = 255;
  int max7 = 0;
  for (int i = 0; i < 16; ++i)
  {
    // check this pixel is valid
    int bit = 1 << i;
    if ((mask & bit) == 0)
      continue;

    // incorporate into the min/max
    int value = rgba[4 * i + 3];
    if (value < min7)
      min7 = value;
    if (value > max7)
      max7 = value;
    if (value != 0 && value < min5)
      min5 = value;
    if (value != 255 && value > max5)
      max5 = value;
  }

  // handle the case that no valid range was found
  if (min5 > max5)
    min5 = max5;
  if (min7 > max7)
    min7 = max7;

  // fix the range to be the minimum in each case
  FixRange(min5, max5, 5);
  FixRange(min7, max7, 7);

  // set up the 5-alpha code book
  std::uint8_t codes5[8];
  codes5[0] = static_cast<std::uint8_t>(min5);
  codes5[1] = static_cast<std::uint8_t>(max5);
  for (int i = 1; i < 5; ++i)
    codes5[1 + i] = static_cast<std::uint8_t>(((5 - i) * min5 + i * max5) / 5);
  codes5[6] = 0;
  codes5[7] = 255;

  // set up the 7-alpha code book
  std::uint8_t codes7[8];
  codes7[0] = static_cast<std::uint8_t>(min7);
  codes7[1] = static_cast<std::uint8_t>(max7);
  for (int i = 1; i < 7; ++i)
    codes7[1 + i] = static_cast<std::uint8_t>(((7 - i) * min7 + i * max7) / 7);

  // fit the data to both code books
  std::uint8_t indices5[16];
  std::uint8_t indices7[16];
  int err5 = FitCodes(rgba, mask, codes5, indices5);
  int err7 = FitCodes(rgba, mask, codes7, indices7);

  // save the block with least error
  if (err5 <= err7)
    WriteAlphaBlock5(min5, max5, indices5, block);
  else
    WriteAlphaBlock7(min7, max7, indices7, block);
}

void DecompressAlphaDxt5(gsl::span<std::uint8_t, 64> rgba, void const* block)
{
  // get the two alpha values
  std::uint8_t const* bytes = reinterpret_cast<std::uint8_t const*>(block);
  int alpha0 = bytes[0];
  int alpha1 = bytes[1];

  // compare the values to build the codebook
  std::uint8_t codes[8];
  codes[0] = static_cast<std::uint8_t>(alpha0);
  codes[1] = static_cast<std::uint8_t>(alpha1);
  if (alpha0 <= alpha1)
  {
    // use 5-alpha codebook
    for (int i = 1; i < 5; ++i)
      codes[1 + i] =
        static_cast<std::uint8_t>(((5 - i) * alpha0 + i * alpha1) / 5);
    codes[6] = 0;
    codes[7] = 255;
  }
  else
  {
    // use 7-alpha codebook
    for (int i = 1; i < 7; ++i)
      codes[1 + i] =
        static_cast<std::uint8_t>(((7 - i) * alpha0 + i * alpha1) / 7);
  }

  // decode the indices
  std::uint8_t indices[16];
  std::uint8_t const* src = bytes + 2;
  std::uint8_t* dest = indices;
  for (int i = 0; i < 2; ++i)
  {
    // grab 3 bytes
    int value = 0;
    for (int j = 0; j < 3; ++j)
    {
      int byte = *src++;
      value |= (byte << 8 * j);
    }

    // unpack 8 3-bit values from it
    for (int j = 0; j < 8; ++j)
    {
      int index = (value >> 3 * j) & 0x7;
      *dest++ = static_cast<std::uint8_t>(index);
    }
  }

  // write out the indexed codebook values
  for (int i = 0; i < 16; ++i)
    rgba[4 * i + 3] = codes[indices[i]];
}
}
