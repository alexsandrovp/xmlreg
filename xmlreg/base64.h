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

/* modifications:
	1. removed #ifndef/#define/#endif and replaced with #pragma once
	2. added stl wrappers
*/

#pragma once

#include <string>

#include <stdlib.h>

//size_t get_buffer_size_for_encoding(size_t byte_length);
//size_t get_buffer_size_for_decoding(size_t encoded_length);

// wrapper for stl string
std::string b64encode(const char* data, size_t data_length);
// wrapper for stl string
std::string b64encode(std::string data);
// wrapper for stl string
std::string b64decode(std::string data);

/*
encodes 'plain' bytes as base64 into 'encoded', appending a null character at the end
'encoded' must be an unallocated pointer to char, this function will allocate the necessary memory.
it is the caller's responsibility to free the memory allocated in 'encoded' using 'delete[]'

'plain' is treated as an array of bytes of 'plain_length' length,
thus pay attention if your string has a NULL end, it will might be encoded as well

the function returns the length of the array (excluding the null character)
*/
size_t base64_encode(const char* plain, size_t plain_length, char** encoded, int addPad);

/*
decodes 'encoded' bytes as base64 into 'plain', appending a null character at the end
'plain' must be an unallocated pointer to char, this function will allocate the necessary memory.
it is the caller's responsibility to free the memory allocated in 'plain' using 'delete[]'

'encoded_length' must be the length of bytes in the 'encoded' array that must be encoded.
careful not to pass the array size, as it may contain invalid characters that cannot be decoded (NULL terminator, for instance)

the function returns the length of the array (excluding the null character)

if the function fails, the memory allocated to 'plain' is freed and the pointer is set to null.
the function then returns an index in 'encoded' that triggered the error.
*/
size_t base64_decode(const char* encoded, size_t encoded_length, char** plain);
