/* rndw32uwp - W32 entropy gatherer (UWP)
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "types.h"
#include "g10lib.h"

using namespace Platform;
using namespace Windows::Security::Cryptography;
using namespace Windows::Security::Cryptography::Certificates;
using namespace Windows::Storage::Streams;

extern "C" {
int
_gcry_rnduwp_gather_random(void(*add)(const void*, size_t,
                                      enum random_origins),
                           enum random_origins origin,
                           size_t length, int level)
{
	if (!level)
		return 0;

	IBuffer^ data = CryptographicBuffer::GenerateRandom(length);
	Array<unsigned char>^ data2;
	CryptographicBuffer::CopyToByteArray(data, &data2);
	(*add)(data2->Data, length, origin);

	return 0;
}

void
_gcry_rnduwp_gather_random_fast(void(*add)(const void*, size_t,
                                           enum random_origins),
                                enum random_origins origin)
{
	size_t size = 20 * sizeof(ulong) + 2 * sizeof(POINT) + sizeof(MEMORYSTATUS)
				+ 8 * sizeof(FILETIME) + sizeof(LARGE_INTEGER);

	_gcry_rnduwp_gather_random(add, origin, size, 1);
}
}