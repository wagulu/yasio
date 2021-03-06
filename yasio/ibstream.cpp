//////////////////////////////////////////////////////////////////////////////////////////
// A cross platform socket APIs, support ios & android & wp8 & window store
// universal app
//////////////////////////////////////////////////////////////////////////////////////////
/*
The MIT License (MIT)

Copyright (c) 2012-2019 halx99

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "obstream.h"
#include "ibstream.h"

#ifdef _WIN32
#  pragma comment(lib, "ws2_32.lib")
#endif

namespace yasio
{

ibstream_view::ibstream_view() { this->assign("", 0); }

ibstream_view::ibstream_view(const void *data, int size) { this->assign(data, size); }

ibstream_view::ibstream_view(const obstream *obs)
{
  auto &buffer = obs->buffer();
  this->assign(!buffer.empty() ? buffer.data() : "", static_cast<int>(buffer.size()));
}

ibstream_view::~ibstream_view() {}

void ibstream_view::assign(const void *data, int size)
{
  first_ = ptr_ = static_cast<const char *>(data);
  last_         = first_ + size;
}

int32_t ibstream_view::read_i24()
{
  int32_t value = 0;
  auto ptr      = consume(3);
  memcpy(&value, ptr, 3);
  value = ntohl(value) >> 8;

  if (value >> 23)
  {
    return -(0x7FFFFF - (value & 0x7FFFFF)) - 1;
  }
  else
  {
    return value & 0x7FFFFF;
  }
}

uint32_t ibstream_view::read_u24()
{
  uint32_t value = 0;
  auto ptr       = consume(3);
  memcpy(&value, ptr, 3);
  return ntohl(value) >> 8;
}

void ibstream_view::read_v(std::string &oav)
{
  auto sv = read_vx<uint32_t>();
  oav.assign(sv.data(), sv.length());
}

void ibstream_view::read_v16(std::string &oav)
{
  auto sv = read_vx<uint16_t>();
  oav.assign(sv.data(), sv.length());
}

void ibstream_view::read_v8(std::string &oav)
{
  auto sv = read_vx<uint8_t>();
  oav.assign(sv.data(), sv.length());
}

yasio::string_view ibstream_view::read_v() { return read_vx<uint32_t>(); }
yasio::string_view ibstream_view::read_v16() { return read_vx<uint16_t>(); }
yasio::string_view ibstream_view::read_v8() { return read_vx<uint8_t>(); }

void ibstream_view::read_v(void *oav, int len) { read_vx<uint32_t>().copy((char *)oav, len); }
void ibstream_view::read_v16(void *oav, int len) { read_vx<uint16_t>().copy((char *)oav, len); }
void ibstream_view::read_v8(void *oav, int len) { read_vx<uint8_t>().copy((char *)oav, len); }

void ibstream_view::read_bytes(std::string &oav, int len)
{
  if (len > 0)
  {
    oav.resize(len);
    read_bytes(&oav.front(), len);
  }
}

void ibstream_view::read_bytes(void *oav, int len)
{
  if (len > 0)
  {
    ::memcpy(oav, consume(len), len);
  }
}

yasio::string_view ibstream_view::read_bytes(int len)
{
  yasio::string_view sv;
  if (len > 0)
  {
    sv = yasio::string_view(consume(len), len);
  }
  return sv;
}

const char *ibstream_view::consume(size_t size)
{
  if (ptr_ >= last_)
    throw std::logic_error("packet error, data insufficiently!");

  auto ptr = ptr_;
  ptr_ += size;

  return ptr;
}

ptrdiff_t ibstream_view::seek(ptrdiff_t offset, int whence)
{
  switch (whence)
  {
    case SEEK_CUR:
      ptr_ += offset;
      if (ptr_ < first_)
        ptr_ = first_;
      else if (ptr_ > last_)
        ptr_ = last_;
      break;
    case SEEK_END:
      ptr_ = first_;
      break;
    case SEEK_SET:
      ptr_ = last_;
      break;
    default:;
  }
  return ptr_ - first_;
}

/// --------------------- CLASS ibstream ---------------------
ibstream::ibstream(std::vector<char> blob) : ibstream_view(), blob_(std::move(blob))
{
  this->assign(blob_.data(), static_cast<int>(blob_.size()));
}
ibstream::ibstream(const obstream *obs) : ibstream_view(), blob_(obs->buffer())
{
  this->assign(blob_.data(), static_cast<int>(blob_.size()));
}

} // namespace yasio
