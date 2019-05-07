/* -----------------------------------------------------------------------------

  Copyright (c) 2006 Simon Brown                          si@sjbrown.co.uk
  Copyright (c) 2012 Niels Fröhling              niels@paradice-insight.us
  Copyright (c) 2019 Christian Spanier                     github@boxie.eu

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to  deal in the Software without restriction, including
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

#include "paletterangefit.h"
#include "paletteset.h"
#include "paletteblock.h"

#include "inlineables.inl"

namespace squish {

/* *****************************************************************************
 */
palette_range_fit::palette_range_fit(palette_set const* palette, int flags, int swap, int shared)
  : palette_single_match(palette, flags, swap, shared)
  ,    palette_index_fit(palette, flags, swap, shared)
  ,         palette_fit(palette, flags, swap, shared)
{
  // the alpha-set (in theory we can do separate alpha + separate partitioning, but's not codeable)
  int const isets = m_palette->GetSets();
  int const asets = m_palette->IsSeperateAlpha() ? isets : 0;
  bool const trns = m_palette->IsMergedAlpha() && m_palette->IsTransparent();

  assume((isets >  0) && (isets <= 3));
  assume((asets >= 0) && (asets <= 3));
  assume(((isets    +    asets) <= 3));

  for (int s = 0; s < isets; s++) {
    // cache some values
    bool const unweighted = m_palette->IsUnweighted(s);
    int const count = m_palette->GetCount(s);
    Vec4 const* values = m_palette->GetPoints(s);
    Scr4 const* weights = m_palette->GetWeights(s);
    
    // the codebook endpoints
    Vec4 start(0.0f);
    Vec4 end(0.0f);

    // we don't do this for sparse sets
    if (count > 2) {
      Vec4 centroid;
      Vec4 principle;

      // combined alpha
      if (trns) {
        Sym4x4 covariance;

        // get the covariance matrix
        if (unweighted)
    ComputeWeightedCovariance4(covariance, centroid, count, values, m_metric[s]);
        else
    ComputeWeightedCovariance4(covariance, centroid, count, values, m_metric[s], weights);

  // compute the principle component
  GetPrincipleComponent(covariance, principle);
      }
      // no or separate alpha
      else {
        Sym3x3 covariance;

        // get the covariance matrix
        if (unweighted)
    ComputeWeightedCovariance3(covariance, centroid, count, values, m_metric[s]);
        else
    ComputeWeightedCovariance3(covariance, centroid, count, values, m_metric[s], weights);

  // compute the principle component
  GetPrincipleComponent(covariance, principle);
      }

      // get the min and max range as the codebook endpoints

#ifdef  FEATURE_RANGEFIT_PROJECT
      // compute the projection
      GetPrincipleProjection(start, end, principle, centroid, count, values);
#else
      Scr4 min, max;

      // compute the range
      start = end = values[0];
      min = max = Dot(values[0], principle);

      for (int i = 1; i < count; ++i) {
  Scr4 val = Dot(values[i], principle);

  if (min > val) {
    start = values[i];
    min = val;
  }
  else if (max < val) {
    end = values[i];
    max = val;
  }
      }
#endif
    }

    // clamp the output to [0, 1]
    m_start[s] = start;
    m_end  [s] = end;
  }

  // the alpha-set (in theory we can do separate alpha + separate partitioning, but's not codeable)
  for (int a = isets; a < (isets + asets); a++) {
    // cache some values
    int const count = m_palette->GetCount(a);
    Vec4 const* values = m_palette->GetPoints(a);
    
    // the codebook endpoints
    Vec4 start(0.0f);
    Vec4 end(0.0f);
    
    // this is a sparse set
    if (count == 2) {
      start = values[0];
      end   = values[1];
    }
    // we don't do this for sparse sets
    else if (count > 2) {
      // get the min and max range as the codebook endpoints
      start = end = values[0];

      // compute the range
      for (int i = 1; i < count; ++i) {
  start = Min(start, values[i]);
  end   = Max(end  , values[i]);
      }
    }

    // clamp the output to [0, 1]
    m_start[a] = start;
    m_end  [a] = end;
  }

#ifdef  FEATURE_ELIMINATE_FLATBOOKS
  // backup original values, we don't now the precision yet
  for (int x = 0; x < (isets + asets); x++) {
    m_start_candidate[x] = m_start[x];
    m_end_candidate[x] = m_end[x];
  }
#endif
}

void palette_range_fit::Compress(void* block, vQuantizer &q, int mode)
{
  int ib = GetIndexBits(mode);
  int jb = ib >> 16; ib = ib & 0xFF;
  int cb = GetPrecisionBits(mode);
  int ab = cb >> 16; cb = cb & 0xFF;
  int zb = GetSharedField();

  q.ChangeShared(cb, cb, cb, ab, zb);

  // match each point to the closest code
  Scr4 error = Scr4(DISTANCE_BASE);
  SQUISH_ALIGNED std::uint8_t closest[4][16];

  // the alpha-set (in theory we can do separate alpha + separate partitioning, but's not codeable)
  int const isets = m_palette->GetSets();
  int const asets = m_palette->IsSeperateAlpha() ? isets : 0;
  bool const trns = m_palette->IsMergedAlpha() && m_palette->IsTransparent();
  std::uint8_t  const tmask = trns ? 0xFF : 0x00;

  assume((isets >  0) && (isets <= 3));
  assume((asets >= 0) && (asets <= 3));
  assume(((isets    +    asets) <= 3));

  // create a codebook
  Vec4 codes[1 << 4];

  // loop over all sets
  for (int s = 0, sb = zb; s < (isets + asets); s++, sb >>= 1) {
    // how big is the codebook for the current set
    // swap the code-book when the swap-index bit is set
    int kb = ((s < isets) ^ (!!m_swapindex)) ? ib : jb;

    // cache some values
    int const count = m_palette->GetCount(s);
    Vec4 const* values = m_palette->GetPoints(s);
    Scr4 const* freq = m_palette->GetWeights(s);

    // in case of separate alpha the colors of the alpha-set have all been set to alpha
    Vec4 metric = m_metric[s < isets ? 0 : 1];

    // we do single entry fit for sparse sets
    if (count == 1) {
      // clear alpha-weight if alpha is disabled
      // in case of separate alpha the colors of the alpha-set have all been set to alpha
      std::uint8_t mask = ((s < isets) ? 0x7 : 0x8) | tmask;

      // find the closest code
      Scr4 dist = ComputeEndPoints(s, metric, cb, ab, sb, kb, mask);

      // save the index (it's just a single one)
      closest[s][0] = GetIndex();
      
      // accumulate the error
      error += dist * freq[0];
    }
    // we do dual entry fit for sparse sets
    else if (count == 2) {
      // find the closest codes (it's just two)
      Scr4 dist = StretchEndPoints(s, metric, q, sb, kb, closest[s]);
      
      // accumulate the error
      error += dist;
    }
    else {
#ifdef  FEATURE_ELIMINATE_FLATBOOKS
      // read original values and modify them
      m_start[s] = m_start_candidate[s];
      m_end[s] = m_end_candidate[s];

      Vec4 mad = MaximumAbsoluteDifference(m_start[s], m_end[s]);
      Vec4 rng = Vec4((1 << kb) / 255.0f);
      if (CompareFirstLessThan(mad, rng)) {
  Vec4 stretch = rng * Reciprocal(mad);
  Vec4 middle = (m_start[s] + m_end[s]) * 0.5f;

  m_start[s] = middle + ((m_start[s] - middle) * stretch);
  m_end  [s] = middle + ((m_end  [s] - middle) * stretch);
      }
#endif

#if  !defined(FEATURE_SHAREDBITS_TRIALS) || (FEATURE_SHAREDBITS_TRIALS < SHAREDBITS_TRIAL_PERMUTE)
      // snap floating-point-values to the integer-lattice
      Vec4 start = q.SnapToLattice(m_start[s], sb, 1 << SBSTART);
      Vec4 end   = q.SnapToLattice(m_end  [s], sb, 1 << SBEND);
      
      // resolve "metric * (value - code)" to "metric * value - metric * code"
      int ccs = CodebookP(codes, kb, metric * start, metric * end);

      for (int i = 0; i < count; ++i) {
  int idx = 0;

  // find the closest code
  Vec4 value = metric * values[i];
  Scr4 dist = Scr4(FLT_MAX);
  for (int j = 0; j < ccs; j += 0)
    MinDistance4<true>(dist, idx, value, codes, j);

  // accumulate the error
  AddDistance(dist, error, freq[i]);

  // save the index
  closest[s][i] = (std::uint8_t)idx;
      }

#elif  defined(FEATURE_SHAREDBITS_TRIALS) && (FEATURE_SHAREDBITS_TRIALS == SHAREDBITS_TRIAL_PERMUTE)
      // if we have a down-forced bit we need to check 2 versions, the +2bt as well
      // if we have a up-forced bit we need to check 2 versions, the -2bt as well
      // this goes for all component permutations (r+-2,g+-2,...)
      Scr4 gerror = Scr4(FLT_MAX);
      int bestom = 0;
      Vec4 start;
      Vec4 end;

      // try all sharedbits-opposing bit-combinations (64/256)
      // try all of the components separately (rrggbbaa)
      for (int om = 0x00; om <= (ab ? 0xFF : 0x3F); om++) {
  // snap floating-point-values to the integer-lattice
  start = q.SnapToLattice(m_start[s], sb, 1 << SBSTART, om >> 0);
  end   = q.SnapToLattice(m_end  [s], sb, 1 << SBEND  , om >> 1);
  
  // resolve "metric * (value - code)" to "metric * value - metric * code"
  int ccs = CodebookP(codes, kb, metric * start, metric * end);

  Scr4 lerror = Scr4(DISTANCE_BASE);
  for (int i = 0; i < count; ++i) {
    Vec4 value = metric * values[i];
    Scr4 dist = Scr4(FLT_MAX);
    for (int j = 0; j < ccs; j += 4)
      MinDistance4<false>(dist, i, value, codes, j);

    // accumulate the error
    AddDistance(dist, lerror, freq[i]);
  }

  if (gerror > lerror) {
    gerror = lerror;
    bestom = om;
  }
      }

      // snap floating-point-values to the integer-lattice with up/down skew
      start = q.SnapToLattice(m_start[s], sb, 1 << SBSTART, bestom >> 0);
      end   = q.SnapToLattice(m_end  [s], sb, 1 << SBEND  , bestom >> 1);
      
      // resolve "metric * (value - code)" to "metric * value - metric * code"
      int ccs = CodebookP(codes, kb, metric * start, metric * end);

      for (int i = 0; i < count; ++i) {
  int idx = 0;

  // find the closest code
  Vec4 value = metric * values[i];
  Scr4 dist = Scr4(FLT_MAX);
  for (int j = 0; j < ccs; ++j)
    MinDistance4<true>(dist, idx, value, codes, j);

  // accumulate the error
  AddDistance(dist, error, freq[i]);

  // save the index
  closest[s][i] = (std::uint8_t)idx;
      }
#endif
    }

#ifdef NDEBUG
    // kill early if this scheme looses
    if (!(error < m_besterror))
      return;
#endif // NDEBUG
  }

  // because the original alpha-channel's weight was killed it is completely random and need to be set to 1.0f
  if (!m_palette->IsTransparent()) {
    switch (m_palette->GetRotation()) {
      default: for (int a = 0; a < isets + asets; a++) m_start[a].Set<3>(1.0f), m_end[a].Set<3>(1.0f); break;
      case  1: for (int a = 0; a <         isets; a++) m_start[a].Set<0>(1.0f), m_end[a].Set<0>(1.0f); break;
      case  2: for (int a = 0; a <         isets; a++) m_start[a].Set<1>(1.0f), m_end[a].Set<1>(1.0f); break;
      case  3: for (int a = 0; a <         isets; a++) m_start[a].Set<2>(1.0f), m_end[a].Set<2>(1.0f); break;
    }
  }
  
#ifndef NDEBUG
  // kill late if this scheme looses
  Scr4 verify_error = Scr4(0.0f); SumError(closest, q, mode, verify_error);
  Scr4 one_error = Scr4(1.0f / (1 << cb)); one_error *= one_error;
  // the error coming back from the palettesinglefit is not entirely exact with OLD_QUANTIZERR
  if (verify_error > (error + one_error)) {
    Scr4 verify_error2 = Scr4(0.0f); SumError(closest, q, mode, verify_error2);
    abort();
  }

#if defined(DEBUG_DETAILS)
#if defined(DEBUG_DETAILS) && (DEBUG_DETAILS == 2)
  fprintf(stderr, "m: %1d, s: %1d+%1d, n:", mode, isets, asets);
  fprintf(stderr, " %2d", 0 < (isets + asets) ? m_palette->GetCount(0) : 0);
  fprintf(stderr, " %2d", 1 < (isets + asets) ? m_palette->GetCount(1) : 0);
  fprintf(stderr, " %2d", 2 < (isets + asets) ? m_palette->GetCount(2) : 0);
  fprintf(stderr, ", c: %1d, a: %1d", cb, ab);
  if (GetPartitionBits(mode) > 0)
    fprintf(stderr, ", p: %2d",  m_palette->GetPartition());
  else if (GetRotationBits(mode) > 0)
    fprintf(stderr, ", r: %2d", m_palette->GetRotation());
  else
    fprintf(stderr, ", r: --");
  if (GetSelectionBits(mode) > 0)
    fprintf(stderr, ", i: %1d,%1d", !m_swapindex ? ib : jb, !m_swapindex ? jb : ib);
  else
    fprintf(stderr, ", i:  %1d ", ib);
  
#if 0
  // TODO: this is a unit test, put into unit-tests
  for (int s = 0, sb = zb; s < (isets + asets); s++, sb >>= 1) {
    Vec4 fstart = q.SnapToLattice(m_start[s], sb, 1 << SBSTART);
    Vec4 fend   = q.SnapToLattice(m_end  [s], sb, 1 << SBEND);
    
    if (~sb) {
      if (mode == 6) {
  Vec4 _s[1] = {m_start[s]};
  Vec4 _e[1] = {m_end  [s]};
  Col4 _a[1][FIELDN];
  Col4 _b[1][FIELDN];

  FloatTo<7,7,7,7,1,0>(_s, _a, sb >> SBSTART);
  FloatTo<7,7,7,7,1,0>(_e, _b, sb >> SBEND);
  UnpackFrom<7,7,7,7,1,0>(_a);
  UnpackFrom<7,7,7,7,1,0>(_b);
  
  assert(_a[0][0].R() == (int)(fstart.X() * 255.0f));
  assert(_a[0][0].G() == (int)(fstart.Y() * 255.0f));
  assert(_a[0][0].B() == (int)(fstart.Z() * 255.0f));
  assert(_a[0][0].A() == (int)(fstart.W() * 255.0f));
      }
      else if (mode == 7) {
  Vec4 _s[1] = {m_start[s]};
  Vec4 _e[1] = {m_end  [s]};
  Col4 _a[1][FIELDN];
  Col4 _b[1][FIELDN];

  FloatTo<5,5,5,5,1,0>(_s, _a, sb >> SBSTART);
  FloatTo<5,5,5,5,1,0>(_e, _b, sb >> SBEND);
  UnpackFrom<5,5,5,5,1,0>(_a);
  UnpackFrom<5,5,5,5,1,0>(_b);
  
  assert(_a[0][0].R() == (int)(fstart.X() * 255.0f));
  assert(_a[0][0].G() == (int)(fstart.Y() * 255.0f));
  assert(_a[0][0].B() == (int)(fstart.Z() * 255.0f));
  assert(_a[0][0].A() == (int)(fstart.W() * 255.0f));
      }
    }
  }
#endif

  if (!(error < m_besterror)) {
    fprintf(stderr, ", e: %.8f (> %.8f)\n", error.X(), m_besterror.X());
    return;
  }

  fprintf(stderr, ", e: %.8f (< %.8f)\n", error.X(), m_besterror.X());
#endif // DEBUG_DETAILS == 2
#endif // DEBUG_DETAILS

  if (!(error < m_besterror))
    return;

#if defined(DEBUG_DETAILS)
  fprintf(stderr, "RangeFit m: %1d, s: %1d+%1d, n:", mode, isets, asets);
  fprintf(stderr, " %2d", 0 < (isets + asets) ? m_palette->GetCount(0) : 0);
  fprintf(stderr, " %2d", 1 < (isets + asets) ? m_palette->GetCount(1) : 0);
  fprintf(stderr, " %2d", 2 < (isets + asets) ? m_palette->GetCount(2) : 0);
  fprintf(stderr, ", c: %1d, a: %1d", cb, ab);
  if (GetPartitionBits(mode) > 0)
    fprintf(stderr, ", p: %2d",  m_palette->GetPartition());
  else if (GetRotationBits(mode) > 0)
    fprintf(stderr, ", r: %2d", m_palette->GetRotation());
  else
    fprintf(stderr, ", r: --");
  if (GetSelectionBits(mode) > 0)
    fprintf(stderr, ", i: %1d,%1d", !m_swapindex ? ib : jb, !m_swapindex ? jb : ib);
  else
    fprintf(stderr, ", i:  %1d ", ib);
  
  fprintf(stderr, ", e: %.8f (< %.8f)\n", error.X(), m_besterror.X() == FLT_MAX ? -1.0f : m_besterror.X());
#endif // DEBUG_DETAILS
#endif // NDEBUG

  // remap the indices
  for (int s = 0;     s <  isets         ; s++)
    m_palette->RemapIndices(closest[s], m_indices[0], s);
  for (int a = isets; a < (isets + asets); a++) {
    m_palette->RemapIndices(closest[a], m_indices[1], a);

    // copy alpha into the common start/end definition
    m_start[a - isets] = TransferW(m_start[a - isets], m_start[a]);
    m_end  [a - isets] = TransferW(m_end  [a - isets], m_end  [a]);
  }

  typedef std::uint8_t (&Itwo)[2][16];
  typedef std::uint8_t (&Ione)[1][16];

  typedef Vec4 (&V4thr)[3];
  typedef Vec4 (&V4two)[2];
  typedef Vec4 (&V4one)[1];

  // save the block
  int partition = m_palette->GetPartition();
  int rot = m_palette->GetRotation(), sel = m_swapindex;
  switch (mode) {
    case 0: WritePaletteBlock3_m1(partition, (V4thr)m_start, (V4thr)m_end, m_sharedbits, (Ione)m_indices, block); break;
    case 1: WritePaletteBlock3_m2(partition, (V4two)m_start, (V4two)m_end, m_sharedbits, (Ione)m_indices, block); break;
    case 2: WritePaletteBlock3_m3(partition, (V4thr)m_start, (V4thr)m_end, m_sharedbits, (Ione)m_indices, block); break;
    case 3: WritePaletteBlock3_m4(partition, (V4two)m_start, (V4two)m_end, m_sharedbits, (Ione)m_indices, block); break;
    case 4: WritePaletteBlock4_m5(rot, sel , (V4one)m_start, (V4one)m_end, m_sharedbits, (Itwo)m_indices, block); break;
    case 5: WritePaletteBlock4_m6(rot,       (V4one)m_start, (V4one)m_end, m_sharedbits, (Itwo)m_indices, block); break;
    case 6: WritePaletteBlock4_m7(partition, (V4one)m_start, (V4one)m_end, m_sharedbits, (Ione)m_indices, block); break;
    case 7: WritePaletteBlock4_m8(partition, (V4two)m_start, (V4two)m_end, m_sharedbits, (Ione)m_indices, block); break;
  }

  // save the error
  m_besterror = error;
  m_best = true;
}
} // namespace squish
