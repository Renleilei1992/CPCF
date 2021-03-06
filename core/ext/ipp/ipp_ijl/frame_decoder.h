/*
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 1998-2006 Intel Corporation. All Rights Reserved.
//
//  Intel?Integrated Performance Primitives Intel?JPEG Library -
//        Intel?IPP Version for Windows*
//
//  By downloading and installing this sample, you hereby agree that the
//  accompanying Materials are being provided to you under the terms and
//  conditions of the End User License Agreement for the Intel?Integrated
//  Performance Primitives product previously accepted by you. Please refer
//  to the file ippEULA.rtf or ippEULA.txt located in the root directory of your Intel?IPP product
//  installation for more information.
//
//  JPEG is an international standard promoted by ISO/IEC and other organizations.
//  Implementations of these standards, or the standard enabled platforms may
//  require licenses from various entities, including Intel Corporation.
//
*/

#ifndef __FRAME_DECODER_H__
#define __FRAME_DECODER_H__

#ifdef _FULLDIAG
#pragma message("  Frame_Decoder.h")
#endif

#ifndef __OWN_H__
#include "own.h"
#endif
#ifndef __TRACE_H__
#include "trace.h"
#endif
#ifndef __FILEIO_H__
#include "fileio.h"
#endif
#ifndef __DECODE_BUFFER_H__
#include "decode_buffer.h"
#endif
#ifndef __DECODER_PARSER_H__
#include "decoder_parser.h"
#endif
#ifndef __SCAN_DECODER_H__
#include "scan_decoder.h"
#endif
#ifndef __CC_SS_DECODER_H__
#include "cc_ss_decoder.h"
#endif




/* ///////////////////////////////////////////////////////////////////////////
// Function Prototypes
*/

OWNAPI(IJLERR,IJL_Decode,(
  JPEG_PROPERTIES* jprops));


#endif /* __FRAME_DECODER_H__ */


