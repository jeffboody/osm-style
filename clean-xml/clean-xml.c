/*
 * Copyright (c) 2015 Jeff Boody
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
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define LOG_TAG "clean-xml"
#include "nedgz/nedgz_log.h"

static void abreviate(char* word, char* separator, char* out)
{
	// separator may be NULL
	assert(word);
	assert(out);

	// abreviations based loosely on
	// https://github.com/nvkelso/map-label-style-manual
	if(strncmp(word, "North", 256) == 0)
	{
		strncat(out, "N", 256);
	}
	else if(strncmp(word, "East", 256) == 0)
	{
		strncat(out, "E", 256);
	}
	else if(strncmp(word, "South", 256) == 0)
	{
		strncat(out, "S", 256);
	}
	else if(strncmp(word, "West", 256) == 0)
	{
		strncat(out, "W", 256);
	}
	else if(strncmp(word, "Avenue", 256) == 0)
	{
		strncat(out, "Ave", 256);
	}
	else if(strncmp(word, "Boulevard", 256) == 0)
	{
		strncat(out, "Blvd", 256);
	}
	else if(strncmp(word, "Court", 256) == 0)
	{
		strncat(out, "Ct", 256);
	}
	else if(strncmp(word, "Drive", 256) == 0)
	{
		strncat(out, "Dr", 256);
	}
	else if(strncmp(word, "Expressway", 256) == 0)
	{
		strncat(out, "Expwy", 256);
	}
	else if(strncmp(word, "Freeway", 256) == 0)
	{
		strncat(out, "Fwy", 256);
	}
	else if(strncmp(word, "Highway", 256) == 0)
	{
		strncat(out, "Hwy", 256);
	}
	else if(strncmp(word, "Lane", 256) == 0)
	{
		strncat(out, "Ln", 256);
	}
	else if(strncmp(word, "Parkway", 256) == 0)
	{
		strncat(out, "Pkwy", 256);
	}
	else if(strncmp(word, "Place", 256) == 0)
	{
		strncat(out, "Pl", 256);
	}
	else if(strncmp(word, "Road", 256) == 0)
	{
		strncat(out, "Rd", 256);
	}
	else if(strncmp(word, "Street", 256) == 0)
	{
		strncat(out, "St", 256);
	}
	else if(strncmp(word, "Terrace", 256) == 0)
	{
		strncat(out, "Ter", 256);
	}
	else if(strncmp(word, "Trail", 256) == 0)
	{
		strncat(out, "Tr", 256);
	}
	else if(strncmp(word, "Way", 256) == 0)
	{
		strncat(out, "Wy", 256);
	}
	else if(strncmp(word, "Mount", 256) == 0)
	{
		strncat(out, "Mt", 256);
	}
	else if(strncmp(word, "Mountain", 256) == 0)
	{
		strncat(out, "Mtn", 256);
	}
	else
	{
		strncat(out, word, 256);
	}
	out[255] = '\0';

	if(separator)
	{
		strncat(out, separator, 256);
		out[255] = '\0';
	}
}

static void parseAbreviate(char* val, char* out)
{
	assert(val);
	assert(out);

	// TODO - some names probably shouldn't be abreviated
	// e.g. North Street => North St rather than N St

	// initialize output string
	out[0] = '\0';

	// separate words
	char* s = val;
	char* e = val;
	char* n;
	while(1)
	{
		n = e + 1;

		// abreviate when end-of-word is found
		if(e[0] == '\0')
		{
			if(s != e)
			{
				abreviate(s, NULL, out);
			}
			break;
		}
		else if(e[0] == ' ')
		{
			e[0] = '\0';
			abreviate(s, " ", out);
			s = n;
		}
		else if(e[0] == ';')
		{
			e[0] = '\0';
			abreviate(s, ";", out);
			s = n;
		}

		e = n;
	}
}

static int parseElevation(char* val, char* out)
{
	assert(val);
	assert(out);

	// fix elevations that are improperly formatted
	// and convert units to feet
	float ele = 0.0f;
	char* s   = strstr(val, " ft");
	if(s == NULL)
	{
		// read meters and convert to ft
		ele  = strtof(val, NULL);
		ele *= 3937.0f/1200.0f;
	}
	else
	{
		// read ft
		s[0] = '\0';
		ele = strtof(val, NULL);
	}

	// round to the nearest ft
	snprintf(out, 256, "%i", (int) (ele + 0.5f));
	out[255] = '\0';

	return 1;
}

static int parseEnd(char* line)
{
	assert(line);

	char* s = strstr(line, "/>");
	if(s == NULL)
	{
		return 0;
	}

	return 1;
}

static int parseVal(char** line, char* val)
{
	assert(line);
	assert(val);

	// find the val start
	char* s = strstr(*line, "v=\"");
	if(s == NULL)
	{
		return 0;
	}

	// find the end quote
	char* e = strstr(&s[3], "\"");
	if(e == NULL)
	{
		return 0;
	}
	e[0] = '\0';
	*line = &e[1];

	// extract the val
	strncpy(val, &s[3], 256);
	val[255] = '\0';

	return 1;
}

static int parseKey(char** line, char* key)
{
	assert(line);
	assert(key);

	// find the key start
	char* s = strstr(*line, "k=\"");
	if(s == NULL)
	{
		return 0;
	}

	// find the end quote
	char* e = strstr(&s[3], "\"");
	if(e == NULL)
	{
		return 0;
	}
	e[0] = '\0';
	*line = &e[1];

	// extract the key
	strncpy(key, &s[3], 256);
	key[255] = '\0';

	return 1;
}

static int parseTag(char** line)
{
	assert(line);

	char* s  = strstr(*line, "<tag");
	if(s == NULL)
	{
		return 0;
	}

	*line = &s[4];
	return 1;
}

static void parseLine(FILE* f, char* line)
{
	assert(f);
	assert(line);

	char copy[256];
	strncpy(copy, line, 256);
	copy[255] = '\0';

	char* s = copy;

	char key[256];
	char val[256];
	char out[256];
	if(parseTag(&s)      &&
	   parseKey(&s, key) &&
	   parseVal(&s, val) &&
	   parseEnd(s))
	{
		if(strncmp(key, "ele", 256) == 0)
		{
			// parse elevation tag
			parseElevation(val, out);
			fprintf(f, "  <tag k=\"%s\" v=\"%s\"/>\n",
			        key, out);
			return;
		}
		else if((strncmp(key, "addr:street", 256) == 0) ||
		        (strncmp(key, "name", 256) == 0)        ||
		        (strncmp(key, "destination", 256) == 0) ||
		        (strncmp(key, "exit_to", 256) == 0))
		{
			// parst name tag(s)
			parseAbreviate(val, out);
			fprintf(f, "  <tag k=\"%s\" v=\"%s\"/>\n",
			        key, out);
			return;
		}
	}

	// skip unsupported keys
	fprintf(f, "%s", line);
}

int main(int argc, char** argv)
{
	if(argc != 3)
	{
		LOGE("usage: %s [in.osm] [out.osm]", argv[0]);
		return EXIT_FAILURE;
	}

	// example tags
	// <tag k="addr:street" v="West Bowles Avenue"/>
	// <tag k="name" v="West Coal Mine Avenue"/>
	// <tag k="destination" v="Wilcox Street;Plum Creek Parkway"/>
	// <tag k="exit_to" v="Rose Avenue"/>
	// <tag k="ele" v="3635"/>
	// <tag k="ele" v="11539 ft"/>
	// <tag k="ele" v="2243.345215"/>

	const char* in  = argv[1];
	const char* out = argv[2];

	FILE* fin = fopen(in, "r");
	if(fin == NULL)
	{
		LOGE("fopen failed for %s", in);
		return EXIT_FAILURE;
	}

	FILE* fout = fopen(out, "w");
	if(fout == NULL)
	{
		LOGE("fopen failed for %s", out);
		goto fail_out;
	}

	// parse lines until EOF
	char*  line = NULL;
	size_t n    = 0;
	while(getline(&line, &n, fin) > 0)
	{
		parseLine(fout, line);
	}
	free(line);

	fclose(fout);
	fclose(fin);

	// success
	return EXIT_SUCCESS;

	// failure
	fail_out:
		fclose(fin);
	return EXIT_FAILURE;
}
