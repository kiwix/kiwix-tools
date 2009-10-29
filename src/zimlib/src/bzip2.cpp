/*
 * Copyright (C) 2008 Tommi Maekitalo
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * is provided AS IS, WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, and
 * NON-INFRINGEMENT.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#include <zim/bzip2.h>
#include <bzlib.h>

namespace zim
{
  const char* Bzip2Error::getErrorString(int ret)
  {
    switch (ret)
    {
      case BZ_OK: return "BZ_OK";
      case BZ_RUN_OK: return "BZ_RUN_OK";
      case BZ_FLUSH_OK: return "BZ_FLUSH_OK";
      case BZ_FINISH_OK: return "BZ_FINISH_OK";
      case BZ_STREAM_END: return "BZ_STREAM_END";
      case BZ_CONFIG_ERROR: return "BZ_CONFIG_ERROR";
      case BZ_SEQUENCE_ERROR: return "BZ_SEQUENCE_ERROR";
      case BZ_PARAM_ERROR: return "BZ_PARAM_ERROR";
      case BZ_MEM_ERROR: return "BZ_MEM_ERROR";
      case BZ_DATA_ERROR: return "BZ_DATA_ERROR";
      case BZ_DATA_ERROR_MAGIC: return "BZ_DATA_ERROR_MAGIC";
      case BZ_IO_ERROR: return "BZ_IO_ERROR";
      case BZ_UNEXPECTED_EOF: return "BZ_UNEXPECTED_EOF";
      case BZ_OUTBUFF_FULL: return "BZ_OUTBUFF_FULL";
      default: return "unknown error";
    }
  }
}
