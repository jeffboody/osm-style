/*
 * Copyright (c) 2016 Jeff Boody
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include "osm_parser.h"

#define LOG_TAG "osm"
#include "osm_log.h"

int main(int argc, const char** argv)
{
	if(argc != 4)
	{
		LOGE("%s in.xml out.xml db.xml", argv[0]);
		return EXIT_FAILURE;
	}

	osm_parser_t* parser = osm_parser_new(argv[1],
	                                      argv[2],
	                                      argv[3]);
	if(parser == NULL)
	{
		return EXIT_FAILURE;
	}

	if(osm_parser_parse(parser) == 0)
	{
		goto fail_parse;
	}

	osm_parser_delete(&parser);

	// success
	return EXIT_SUCCESS;

	// failure
	fail_parse:
		osm_parser_delete(&parser);
	return EXIT_FAILURE;
}
