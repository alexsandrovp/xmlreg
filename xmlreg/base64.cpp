/*
Copyright (c) 2016 Alex Vargas

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

/*
* Modifications:
*	1. added stl wrappers
*/

#include "base64.h"

static const unsigned char encodelookup[] = {
//  'A'   'B'   'C'   'D'   'E'   'F'   'G'   'H'   'I'   'J'   'K'   'L'   'M'   'N'   'O'   'P'   'Q'   'R'   'S'   'T'   'U'   'V'   'W'   'X'   'Y'   'Z'
0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A,
//  65    66    67    68    69    70    71    72    73    74    75    76    77    78    79    80    81    82    83    84    85    86    87    88    89    90
//  00    01    02    03    04    05    06    07    08    09    10    11    12    13    14    15    16    17    18    19    20    21    22    23    24    25
//  00    01    02    03    04    05    06    07    08    09    0A    0B    0C    0D    0E    0F    10    11    12    13    14    15    16    17    18    19

//  'a'   'b'   'c'   'd'   'e'   'f'   'g'   'h'   'i'   'j'   'k'   'l'   'm'   'n'   'o'   'p'   'q'   'r'   's'   't'   'u'   'v'   'w'   'x'   'y'   'z'
0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A,
//  97    98    99    100   101   102   103   104   105   106   107   108   109   110   111   112   113   114   115   116   117   118   119   120   121   122
//  26    27    28    29    30    31    32    33    34    35    36    37    38    39    40    41    42    43    44    45    46    47    48    49    50    51
//  1A    1B    1C    1D    1E    1F    20    21    22    23    24    25    26    27    28    29    2A    2B    2C    2D    2E    2F    30    31    32    33

//  '0'   '1'   '2'   '3'   '4'   '5'   '6'   '7'   '8'   '9'   '+'   '/'
0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x2B, 0x2F
//  48    49    50    51    52    53    54    55    56    57    43    47
//  52    53    54    55    56    57    58    59    60    61    62    63
//  34    35    36    37    38    39    3A    3B    3C    3D    3E    3F
};

//use full matrix, don't truncate (safe decode) / 0xFF means invalid character in encoded string
static const unsigned char decodelookup[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	//0-15
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	//16-31
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xFF, 0xFF, 0x3F,	//32-47
	0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	//48-63
	0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, //64-79
	0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	//80-95
	0xFF, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,	//96-111
	0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	//112-127
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	//128-143
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, //144-159
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	//160-175
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	//176-191
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	//192-207
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	//208-223
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	//224-239
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF	//240-255
};

/*
size_t get_buffer_size_for_encoding(size_t byte_length)
{
	size_t len = 4 * byte_length / 3;
	size_t rem = len % 4;
	if (rem) return len - rem + 4;
	return len;
}

size_t get_buffer_size_for_decoding(size_t encoded_length)
{
	size_t len = 3 * encoded_length / 4;
	size_t rem = len % 3;
	if (rem) return len - rem + 3;
	return len;
}
*/

std::string b64encode(const char* data, size_t data_length)
{
	char* buffer = nullptr;
	size_t len = base64_encode(data, data_length, &buffer, 1);
	std::string ret(buffer, len);
	delete[] buffer;
	return ret;
}

std::string b64encode(std::string data)
{
	char* buffer = nullptr;
	size_t len = base64_encode(data.c_str(), data.length(), &buffer, 1);
	std::string ret(buffer, len);
	delete[] buffer;
	return ret;
}

std::string b64decode(std::string data)
{
	char* buffer = nullptr;
	size_t len = base64_decode(data.c_str(), data.length(), &buffer);
	std::string ret(buffer, len);
	delete[] buffer;
	return ret;
}

size_t base64_encode(const char* plain, size_t plain_length, char** encoded, int addPad)
{
	//3 octets (bytes) match 4 sextets (bitcount -> 3*8 == 4*6)
	size_t bitcount = plain_length * 4;
	size_t remainder = plain_length % 3;
	size_t limit = (plain_length <= remainder + 1) ? 0 : plain_length - remainder - 1;
	const size_t encoded_length = (bitcount % 3) > 0 ? 1 + bitcount / 3 : bitcount / 3;
	const int padcount = addPad ? 4 - encoded_length % 4 : 0;
	const size_t paddedlen = encoded_length + padcount;

	(*encoded) = (char*)malloc(paddedlen + 1);
	(*encoded)[paddedlen] = 0;

	size_t i = 0, j = 0;
	for (; i < limit; i += 3, j += 4)
	{
		(*encoded)[j] = encodelookup[(plain[i] & 0xFC) >> 2];
		(*encoded)[j + 1] = encodelookup[((plain[i] & 0x03) << 4) | ((plain[i + 1] & 0xF0) >> 4)];
		(*encoded)[j + 2] = encodelookup[((plain[i + 1] & 0x0F) << 2) | ((plain[i + 2] & 0xC0) >> 6)];
		(*encoded)[j + 3] = encodelookup[plain[i + 2] & 0x3F];
	}

	if (remainder-- > 0)
	{
		(*encoded)[j] = encodelookup[(plain[i] & 0xFC) >> 2];
		if (remainder > 0)
		{
			(*encoded)[j + 1] = encodelookup[((plain[i] & 0x03) << 4) | ((plain[i + 1] & 0xF0) >> 4)];
			(*encoded)[j + 2] = encodelookup[((plain[i + 1] & 0x0F) << 2)];
		}
		else
		{
			(*encoded)[j + 1] = encodelookup[((plain[i] & 0x03) << 4)];
		}
	}

	if (padcount > 0 && padcount < 4)
	{
		for (i = encoded_length; i < paddedlen; ++i)
			(*encoded)[i] = '=';
		return paddedlen;
	}

	return encoded_length;
}

size_t base64_decode(const char* encoded, size_t encoded_length, char** plain)
{
	while (encoded_length > 0)
	{
		if (encoded[--encoded_length] == '=') continue;
		++encoded_length;
		break;
	}

	const unsigned char* uencoded = (const unsigned char*)encoded;

	size_t error_index = -1;
	const size_t bitcount = encoded_length * 3;
	const size_t plain_length = bitcount / 4;
	(*plain) = (char*)malloc(plain_length + 1);
	(*plain)[plain_length] = 0;

	size_t remainder = plain_length % 3;
	size_t limit = (plain_length <= remainder + 1) ? 0 : plain_length - remainder - 1;

	size_t i = 0, j = 0;
	for (; i < limit; i += 3, j += 4)
	{
		if (decodelookup[uencoded[j]] == 0xFF)
		{
			error_index = j;
			goto error;
		}
		if (decodelookup[uencoded[j + 1]] == 0xFF)
		{
			error_index = j + 1;
			goto error;
		}
		if (decodelookup[uencoded[j + 2]] == 0xFF)
		{
			error_index = j + 2;
			goto error;
		}
		if (decodelookup[uencoded[j + 3]] == 0xFF)
		{
			error_index = j + 3;
			goto error;
		}

		(*plain)[i] = (char)(((decodelookup[uencoded[j]] & 0x3F) << 2) | ((decodelookup[uencoded[j + 1]] & 0x30) >> 4));
		(*plain)[i + 1] = (char)(((decodelookup[uencoded[j + 1]] & 0x0F) << 4) | ((decodelookup[uencoded[j + 2]] & 0x3C) >> 2));
		(*plain)[i + 2] = (char)(((decodelookup[uencoded[j + 2]] & 0x03) << 6) | (decodelookup[uencoded[j + 3]] & 0x3F));
	}

	if (remainder-- > 0)
	{
		if (decodelookup[uencoded[j]] == 0xFF)
		{
			error_index = j;
			goto error;
		}
		if (decodelookup[uencoded[j + 1]] == 0xFF)
		{
			error_index = j + 1;
			goto error;
		}

		(*plain)[i] = (char)(((decodelookup[uencoded[j]] & 0x3F) << 2) | ((decodelookup[uencoded[j + 1]] & 0x30) >> 4));

		if (remainder > 0)
		{
			if (decodelookup[uencoded[j + 2]] == 0xFF)
			{
				error_index = j + 2;
				goto error;
			}

			(*plain)[i + 1] = (char)(((decodelookup[uencoded[j + 1]] & 0x0F) << 4) | ((decodelookup[uencoded[j + 2]] & 0x3C) >> 2));
		}
	}

	return plain_length;

error:
	free(*plain);
	*plain = 0;
	return error_index;
}
