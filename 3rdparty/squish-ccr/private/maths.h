/* -----------------------------------------------------------------------------

  Copyright (c) 2006 Simon Brown                          si@sjbrown.co.uk
  Copyright (c) 2012 Niels FrÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂ¶hling              niels@paradice-insight.us

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

#ifndef SQUISH_MATHS_H
#define SQUISH_MATHS_H

#include "squish.h"
#include "maths_sym2x2.h"
#include "maths_sym3x3.h"
#include "maths_sym4x4.h"
#include "simd.h"

/* -------------------------------------------------------------------------- */

namespace squish
{
class Sym2x2;
class Sym3x3;
class Sym4x4;
class Vec3;
class Vec4;

void ComputeWeightedCovariance3(Sym3x3& covariance, Vec3& centroid, int n,
                                Vec3 const* points, const Vec3& metric);
void ComputeWeightedCovariance2(Sym2x2& covariance, Vec4& centroid, int n,
                                Vec4 const* points, Vec4 const& metric);
void ComputeWeightedCovariance3(Sym3x3& covariance, Vec4& centroid, int n,
                                Vec4 const* points, Vec4 const& metric);
void ComputeWeightedCovariance4(Sym4x4& covariance, Vec4& centroid, int n,
                                Vec4 const* points, Vec4 const& metric);
void ComputeWeightedCovariance3(Sym3x3& covariance, Vec3& centroid, int n,
                                Vec3 const* points, const Vec3& metric,
                                Scr3 const* weights);
void ComputeWeightedCovariance2(Sym2x2& covariance, Vec4& centroid, int n,
                                Vec4 const* points, Vec4 const& metric,
                                Vec4 const* weights);
void ComputeWeightedCovariance3(Sym3x3& covariance, Vec4& centroid, int n,
                                Vec4 const* points, Vec4 const& metric,
                                Vec4 const* weights);
void ComputeWeightedCovariance4(Sym4x4& covariance, Vec4& centroid, int n,
                                Vec4 const* points, Vec4 const& metric,
                                Vec4 const* weights);
void ComputePrincipleComponent(Sym3x3 const& smatrix, Vec3& out);
void ComputePrincipleComponent(Sym2x2 const& smatrix, Vec4& out);
void ComputePrincipleComponent(Sym3x3 const& smatrix, Vec4& out);
void ComputePrincipleComponent(Sym4x4 const& smatrix, Vec4& out);
void EstimatePrincipleComponent(Sym3x3 const& smatrix, Vec3& out);
void EstimatePrincipleComponent(Sym2x2 const& smatrix, Vec4& out);
void EstimatePrincipleComponent(Sym3x3 const& smatrix, Vec4& out);
void EstimatePrincipleComponent(Sym4x4 const& smatrix, Vec4& out);
void GetPrincipleProjection(Vec3& enter, Vec3& leave, const Vec3& principle,
                            const Vec3& centroid, int n, Vec3 const* points);
void GetPrincipleProjection(Vec4& enter, Vec4& leave, Vec4 const& principle,
                            Vec4 const& centroid, int n, Vec4 const* points);

#ifdef FEATURE_POWERESTIMATE
#define GetPrincipleComponent(covariance, m_principle) \
  EstimatePrincipleComponent(covariance, m_principle)
#else
#define GetPrincipleComponent(covariance, m_principle) \
  ComputePrincipleComponent(covariance, m_principle)
#endif

const float* ComputeGammaLUT(bool sRGB);

#if defined(SQUISH_USE_AMP) || defined(SQUISH_USE_COMPUTE)
#if !defined(SQUISH_USE_COMPUTE)
typedef Sym3x3& Sym3x3r;
#else
typedef float4 Sym3x3[2];
typedef Sym3x3 Sym3x3r;
#endif

Sym3x3 ComputeWeightedCovariance3(tile_barrier barrier, const int thread, int n,
                                  point16 points,
                                  weight16 weights) amp_restricted;
float3 ComputePrincipleComponent(tile_barrier barrier, const int thread,
                                 Sym3x3r smatrix) amp_restricted;
#endif

/* #################################################################################
 */

extern const float qLUT_1all[2], qLUT_1clr[2], qLUT_1set[2];
extern const float qLUT_2all[4], qLUT_2clr[4], qLUT_2set[4];
extern const float qLUT_3all[8], qLUT_3clr[8], qLUT_3set[8];
extern const float qLUT_4all[16], qLUT_4clr[16], qLUT_4set[16];
extern const float qLUT_5all[32], qLUT_5clr[32], qLUT_5set[32];
extern const float qLUT_6all[64], qLUT_6clr[64], qLUT_6set[64];
extern const float qLUT_7all[128], qLUT_7clr[128], qLUT_7set[128];
extern const float qLUT_8all[256], qLUT_8clr[256], qLUT_8set[256];

static const float* qLUT_a[9] = {qLUT_8all, qLUT_1all, qLUT_2all,
                                 qLUT_3all, qLUT_4all, qLUT_5all,
                                 qLUT_6all, qLUT_7all, qLUT_8all};
static const float* qLUT_c[9] = {qLUT_8all, qLUT_1clr, qLUT_2clr,
                                 qLUT_3clr, qLUT_4clr, qLUT_5clr,
                                 qLUT_6clr, qLUT_7clr, qLUT_8clr};
static const float* qLUT_s[9] = {qLUT_8all, qLUT_1set, qLUT_2set,
                                 qLUT_3set, qLUT_4set, qLUT_5set,
                                 qLUT_6set, qLUT_7set, qLUT_8set};
}

#endif
