// DataCompressor.h
//
// PASSWORD TECH
// Copyright (c) 2002-2024 by Christian Thoeing <c.thoeing@web.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//---------------------------------------------------------------------------
#ifndef DataCompressorH
#define DataCompressorH
//---------------------------------------------------------------------------
#include "miniz.h"

class CompressorError : public std::runtime_error
{
public:
  CompressorError(const char* pMsg)
    : std::runtime_error(pMsg)
  {}
};

class DataCompressor
{
public:

  virtual ~DataCompressor() {}

  // process (compress or decompress) data
  // -> input buffer
  // -> number of bytes available in input buffer (may be 0)
  // -> output buffer
  // -> size of output buffer
  // -> 'true': finish stream, 'false': keep streaming
  // -> receives number of available bytes in output buffer
  //    0 if output buffer is not yet ready to be accessed
  // <- 'true' if stream is finished
  virtual bool Process(const word8* pIn,
    word32 lInSize,
    word8* pOut,
    word32 lOutSize,
    bool blFinish,
    word32& lAvailOut) = 0;

  // check whether input buffer needs to be refilled
  // <- 'true': input buffer needs to be refilled
  virtual bool CheckRefill(void) = 0;
};

class Deflate : public DataCompressor
{
public:
  Deflate(int nLevel = MZ_DEFAULT_LEVEL)
  {
    memset(&m_stream, 0, sizeof(m_stream));
    if (deflateInit(&m_stream, nLevel) != Z_OK)
      throw CompressorError("deflateInit() failed");
  }

  ~Deflate()
  {
    deflateEnd(&m_stream);
    memset(&m_stream, 0, sizeof(m_stream));
  }

  bool Process(const word8* pIn,
    word32 lInSize,
    word8* pOut,
    word32 lOutSize,
    bool blFinish,
    word32& lAvailOut) override
  {
    if (lInSize || !m_stream.next_in) {
      m_stream.next_in = pIn;
      m_stream.avail_in = lInSize;
    }
    if (!m_stream.next_out) {
      m_stream.next_out = pOut;
      m_stream.avail_out = lOutSize;
    }
    int nStatus = deflate(&m_stream, blFinish ? Z_FINISH : Z_NO_FLUSH);
    if (nStatus == Z_STREAM_END || !m_stream.avail_out) {
      lAvailOut = lOutSize - m_stream.avail_out;
      m_stream.next_out = pOut;
      m_stream.avail_out = lOutSize;
    }
    else
      lAvailOut = 0;
    if (nStatus == Z_STREAM_END)
      return true;
    if (nStatus != Z_OK)
      throw CompressorError("deflate() failed");
    return false;
  }

  bool CheckRefill(void) override
  {
    return m_stream.avail_in == 0;
  }

private:
  z_stream m_stream;
};


class Inflate : public DataCompressor
{
public:
  Inflate()
  {
    memset(&m_stream, 0, sizeof(m_stream));
    if (inflateInit(&m_stream) != Z_OK)
      throw CompressorError("inflateInit() failed");
  }

  ~Inflate()
  {
    inflateEnd(&m_stream);
    memset(&m_stream, 0, sizeof(m_stream));
  }

   bool Process(const word8* pIn,
    word32 lInSize,
    word8* pOut,
    word32 lOutSize,
    bool blFinish,
    word32& lAvailOut) override
  {
    if (lInSize || !m_stream.next_in) {
      m_stream.next_in = pIn;
      m_stream.avail_in = lInSize;
    }
    if (!m_stream.next_out) {
      m_stream.next_out = pOut;
      m_stream.avail_out = lOutSize;
    }
    int nStatus = inflate(&m_stream, Z_SYNC_FLUSH);
    if (nStatus == Z_STREAM_END || !m_stream.avail_out) {
      lAvailOut = lOutSize - m_stream.avail_out;
      m_stream.next_out = pOut;
      m_stream.avail_out = lOutSize;
    }
    else
      lAvailOut = 0;
    if (nStatus == Z_STREAM_END)
      return true;
    if (nStatus != Z_OK)
      throw CompressorError("inflate() failed");
    return false;
  }

  bool CheckRefill(void) override
  {
    return m_stream.avail_in == 0;
  }

private:
  z_stream m_stream;
};

#endif