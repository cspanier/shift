//=====================================================================
// Copyright (c) 2007-2014    Advanced Micro Devices, Inc. All rights reserved.
// Copyright (c) 2004-2006    ATI Technologies Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//=====================================================================

#ifndef COMPRESSONATOR_COMMON_H
#define COMPRESSONATOR_COMMON_H

#include <cstdio>
#include <cstring>
#include <cassert>
#include <cmath>
#include <cfloat>
#include <cstdint>
#include <string>

#include "compressonator/Compressonator.h"

#define QT_KEY_SPACE 0x20  // Qt::Key_Space = 0x20
#define QT_KEY_M 0x4d      // Qt::Key_M = 0x4d

#define UNREFERENCED_PARAMETER(P) (P)

//  Send debug info to a results text file
// #define BC7_DEBUG_TO_RESULTS_TXT

// Turn off BC6H optimization of endpoints - useful for debugging quantization
// and mode checking
// #define BC6H_NO_OPTIMIZE_ENDPOINTS

// Generates a Results.txt file on exe working directory; MultiThreading is
// turned off for debuging to text file
// #define BC6H_DEBUG_TO_RESULTS_TXT

// Enables debug info on decoder
// #define BC6H_DECODE_DEBUG

// Model changes
//#define USE_COMPUTE                                 // Enable compute code for
// compression
#define USE_3DVIEWALLAPI  // Enable 3D model view selection list in application
                          // setting
#define USE_MESH_CLI      // CLI Process Mesh (only support glTF and OBJ files)
#define USE_MESH_DRACO_EXTENSION  // Mesh Compression with Draco support in glTF
                                  // and OBJ files only

// Codec options
#define USE_ETCPACK  // Use ETCPack for ETC2 else use CModel code!

// todo: recommended to use default setting for now as the settings for
// different draco level may produce corrupted textures. #define
// USE_MESH_DRACO_SETTING                   // Expose draco settings for draco
// mesh compression, if disabled default setting will be used for mesh
// compression

// #define USE_GLTF2_MIPSET                         // Enable Image Transcode &
// Compression support for GLTF files using TextureIO #define USE_FILEIO // Used
// for debugging code

// New features under devlopement
// #define ENABLE_V3x_CODE                          // Code under development
// for future releases #define USE_GTC_HDR                              //  HDR
// Gradient Texture Compressor patent pending... #define USE_GTC //  LDR
// Gradient Texture Compressor patent pending... #define USE_GTC_TRANSCODE //
// New feature to transcode GTC to other compressed formats

// To Be enabled in future releases
// #define ARGB_32_SUPPORT                           // Enables 32bit Float
// channel formats #define SUPPORT_ETC_ALPHA                         // for
// ATC_RGB output enable A #define SHOW_PROCESS_MEMORY                       //
// display available CPU process memory #define USE_BCN_IMAGE_DEBUG // Enables
// Combobox in Image View for low level BCn based block compression in debug
// mode #define USE_CRN                                   // Enabled .crn file
// output using CRUNCH encoder #define USE_ASSIMP #define USE_3DCONVERT //
// Enable 3D model conversion (glTF<->obj) icon #define
// ENABLE_USER_ETC2S_FORMATS                 // Enable users to set these
// formats in CLI and GUI applications

#endif
