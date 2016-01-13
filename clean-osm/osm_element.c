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
#include <assert.h>
#include <string.h>
#include "osm_parser.h"
#include "osm_element.h"

#define LOG_TAG "osm"
#include "osm_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

typedef struct
{
	int abreviate;

	XML_Char word[4096];
	XML_Char abreviation[4096];
	XML_Char separator[2];
} osm_token_t;

static int osm_abreviateWord(const XML_Char* a,
                             XML_Char* b)
{
	assert(a);
	assert(b);

	int abreviate = 1;

	// abreviations based loosely on
	// https://github.com/nvkelso/map-label-style-manual
	// http://pe.usps.gov/text/pub28/28c1_001.htm
	if(strncmp(a, "North", 4096) == 0)
	{
		strncat(b, "N", 4096);
	}
	else if(strncmp(a, "East", 4096) == 0)
	{
		strncat(b, "E", 4096);
	}
	else if(strncmp(a, "South", 4096) == 0)
	{
		strncat(b, "S", 4096);
	}
	else if(strncmp(a, "West", 4096) == 0)
	{
		strncat(b, "W", 4096);
	}
	else if(strncmp(a, "Northeast", 4096) == 0)
	{
		strncat(b, "NE", 4096);
	}
	else if(strncmp(a, "Northwest", 4096) == 0)
	{
		strncat(b, "NW", 4096);
	}
	else if(strncmp(a, "Southeast", 4096) == 0)
	{
		strncat(b, "SE", 4096);
	}
	else if(strncmp(a, "Southwest", 4096) == 0)
	{
		strncat(b, "SW", 4096);
	}
	else if(strncmp(a, "Avenue", 4096) == 0)
	{
		strncat(b, "Ave", 4096);
	}
	else if(strncmp(a, "Boulevard", 4096) == 0)
	{
		strncat(b, "Blvd", 4096);
	}
	else if(strncmp(a, "Court", 4096) == 0)
	{
		strncat(b, "Ct", 4096);
	}
	else if(strncmp(a, "Circle", 4096) == 0)
	{
		strncat(b, "Cir", 4096);
	}
	else if(strncmp(a, "Drive", 4096) == 0)
	{
		strncat(b, "Dr", 4096);
	}
	else if(strncmp(a, "Expressway", 4096) == 0)
	{
		strncat(b, "Expwy", 4096);
	}
	else if(strncmp(a, "Freeway", 4096) == 0)
	{
		strncat(b, "Fwy", 4096);
	}
	else if(strncmp(a, "Highway", 4096) == 0)
	{
		strncat(b, "Hwy", 4096);
	}
	else if(strncmp(a, "Lane", 4096) == 0)
	{
		strncat(b, "Ln", 4096);
	}
	else if(strncmp(a, "Parkway", 4096) == 0)
	{
		strncat(b, "Pkwy", 4096);
	}
	else if(strncmp(a, "Place", 4096) == 0)
	{
		strncat(b, "Pl", 4096);
	}
	else if(strncmp(a, "Road", 4096) == 0)
	{
		strncat(b, "Rd", 4096);
	}
	else if(strncmp(a, "Street", 4096) == 0)
	{
		strncat(b, "St", 4096);
	}
	else if(strncmp(a, "Terrace", 4096) == 0)
	{
		strncat(b, "Ter", 4096);
	}
	else if(strncmp(a, "Trail", 4096) == 0)
	{
		strncat(b, "Tr", 4096);
	}
	else if((strncmp(a, "Mount", 4096) == 0) ||
	        (strncmp(a, "Mt.",   4096) == 0))
	{
		strncat(b, "Mt", 4096);
	}
	else if(strncmp(a, "Mountain", 4096) == 0)
	{
		strncat(b, "Mtn", 4096);
	}
	else
	{
		strncat(b, a, 4096);
		abreviate = 0;
	}
	b[4095] = '\0';

	return abreviate;
}

static void osm_element_catWord(XML_Char* str, XML_Char* word)
{
	assert(str);
	assert(word);

	strncat(str, word, 4096);
	str[4095] = '\0';
}

static const XML_Char* osm_element_parseWord(osm_element_t* self,
                                             const XML_Char* str,
                                             osm_token_t* tok)
{
	assert(self);
	assert(str);
	assert(tok);

	tok->abreviate      = 0;
	tok->word[0]        = '\0';
	tok->abreviation[0] = '\0';
	tok->separator[0]   = '\0';
	tok->separator[1]   = '\0';

	// eat whitespace
	int i = 0;
	while(1)
	{
		if((str[i] == ' ')  ||
		   (str[i] == '\n') ||
		   (str[i] == '\t') ||
		   (str[i] == '\r'))
		{
			++i;
			continue;
		}

		break;
	}

	// find a word
	int len = 0;
	while(1)
	{
		// eat invalid characters
		if((str[i] == '\n') ||
		   (str[i] == '\t') ||
		   (str[i] == '\r'))
		{
			++i;
			continue;
		}

		// check for word boundary
		if((str[i] == '\0') && (len == 0))
		{
			return NULL;
		}
		else if(len == 4095)
		{
			LOGE("invalid line=%i", self->line);
			return NULL;
		}
		else if(str[i] == '\0')
		{
			tok->abreviate = osm_abreviateWord(tok->word,
			                                   tok->abreviation);
			return &str[i];
		}
		else if(str[i] == ' ')
		{
			tok->abreviate = osm_abreviateWord(tok->word,
			                                   tok->abreviation);
			tok->separator[0] = ' ';
			return &str[i + 1];
		}
		else if(str[i] == ';')
		{
			tok->abreviate = osm_abreviateWord(tok->word,
			                                   tok->abreviation);
			tok->separator[0] = ';';
			return &str[i + 1];
		}

		// append character to word
		tok->word[len]     = str[i];
		tok->word[len + 1] = '\0';
		++len;
		++i;
	}
}

static int osm_element_parseName(osm_element_t* self,
                                 const XML_Char* a,
                                 XML_Char* b)
{
	assert(self);
	assert(a);
	assert(b);

	// initialize output string
	b[0] = '\0';

	osm_token_t w0;
	osm_token_t w1;
	osm_token_t wn;

	const XML_Char* str = a;
	str = osm_element_parseWord(self, str, &w0);
	if(str == NULL)
	{
		// input is null string
		LOGE("invalid line=%i, name=%s", self->line, a);
		return 0;
	}
	
	str = osm_element_parseWord(self, str, &w1);
	if(str == NULL)
	{
		// input is single word
		strncpy(b, a, 4096);
		b[4095] = '\0';
		return 1;
	}

	str = osm_element_parseWord(self, str, &wn);
	if(str == NULL)
	{
		// input is two words
		if(w1.abreviate)
		{
			// don't abreviate first word if second
			// word is also abreviated
			osm_element_catWord(b, w0.word);
			osm_element_catWord(b, w0.separator);
			osm_element_catWord(b, w1.abreviation);
			osm_element_catWord(b, w1.separator);
		}
		else if(w0.abreviate)
		{
			osm_element_catWord(b, w0.abreviation);
			osm_element_catWord(b, w0.separator);
			osm_element_catWord(b, w1.word);
			osm_element_catWord(b, w1.separator);
		}
		else
		{
			osm_element_catWord(b, w0.word);
			osm_element_catWord(b, w0.separator);
			osm_element_catWord(b, w1.word);
			osm_element_catWord(b, w1.separator);
		}
		return 1;
	}

	// three or more words
	// end of special cases
	if(w0.abreviate)
	{
		osm_element_catWord(b, w0.abreviation);
	}
	else
	{
		osm_element_catWord(b, w0.word);
	}
	osm_element_catWord(b, w0.separator);

	if(w1.abreviate)
	{
		osm_element_catWord(b, w1.abreviation);
	}
	else
	{
		osm_element_catWord(b, w1.word);
	}
	osm_element_catWord(b, w1.separator);

	// parse the rest of the line
	while(str)
	{
		if(wn.abreviate)
		{
			osm_element_catWord(b, wn.abreviation);
		}
		else
		{
			osm_element_catWord(b, wn.word);
		}
		osm_element_catWord(b, wn.separator);

		str = osm_element_parseWord(self, str, &wn);
	}

	return 1;
}

static int osm_element_parseEle(osm_element_t* self,
                                int ft,
                                const XML_Char* a,
                                XML_Char* b)
{
	assert(self);
	assert(a);
	assert(b);

	// assume the ele is in meters
	float ele = strtof(a, NULL);
	if(ft == 0)
	{
		// convert meters to ft
		ele *= 3937.0f/1200.0f;
	}
	snprintf(b, 4096, "%i", (int) (ele + 0.5f));

	osm_token_t w0;
	osm_token_t w1;
	osm_token_t wn;

	const XML_Char* str = a;
	str = osm_element_parseWord(self, str, &w0);
	if(str == NULL)
	{
		// input is null string
		LOGW("invalid line=%i, ele=%s", self->line, a);
		return 0;
	}
	
	str = osm_element_parseWord(self, str, &w1);
	if(str == NULL)
	{
		// input is single word
		return 1;
	}

	str = osm_element_parseWord(self, str, &wn);
	if(str == NULL)
	{
		// check if w1 is ft
		if((strcmp(w1.word, "ft")   == 0) ||
		   (strcmp(w1.word, "feet") == 0))
		{
			// assume w0 is in ft
			float ele = strtof(w0.word, NULL);
			snprintf(b, 4096, "%i", (int) (ele + 0.5f));
			return 1;
		}
		else
		{
			LOGW("invalid line=%i, ele=%s", self->line, a);
			return 0;
		}
	}

	LOGW("invalid line=%i, ele=%s", self->line, a);
	return 0;
}

static void osm_element_evalNode(osm_element_t* self,
                                 struct osm_parser_s* parser,
                                 int indent)
{
	assert(self);
	assert(parser);

	// parse lat/lon atts
	const XML_Char** atts = (const XML_Char**) self->atts;
	while(atts[0])
	{
		if(strcmp(atts[0], "lat") == 0)
		{
			self->has_lat = 1;
			self->lat     = strtod(atts[1], NULL);
		}
		else if(strcmp(atts[0], "lon") == 0)
		{
			self->has_lon = 1;
			self->lon     = strtod(atts[1], NULL);
		}
		atts += 2;
	}

	osm_parser_printElemBegin(parser, self->name, indent);
	osm_parser_printElemAttsSet(parser, (const XML_Char**) self->atts);
	if(self->head)
	{
		osm_parser_printElemEnd(parser, 0);

		// print the tags
		osm_element_t* iter = self->head;
		while(iter)
		{
			osm_element_eval(iter, parser, indent + 1);
			iter = iter->next;
		}

		#if 0
		// insert the ele tag
		if(self->has_peak &&
		   self->has_lat && self->has_lon &&
		   (self->has_ele == 0))
		{
			// TODO - lookup height
			XML_Char h[4096];
			snprintf(h, 4096, "%i", 404);

			const XML_Char  k[2] = "k";
			const XML_Char  v[2] = "v";
			const XML_Char  e[8] = "ele";
			const XML_Char* key[2] = { k, e };
			const XML_Char* val[2] = { v, h };

			osm_parser_printElemBegin(parser, "tag", indent + 1);
			osm_parser_printElemAttsPair(parser, key);
			osm_parser_printElemAttsPair(parser, val);
			osm_parser_printElemEnd(parser, 1);
		}
		#endif

		osm_parser_printElemClose(parser, self->name, indent);
	}
	else
	{
		osm_parser_printElemEnd(parser, 1);
	}
}

static void osm_element_evalTag(osm_element_t* self,
                                struct osm_parser_s* parser,
                                int indent)
{
	assert(self);
	assert(parser);

	// count the attributes
	const XML_Char** atts = (const XML_Char**) self->atts;
	int count = 0;
	while(atts[count])
	{
		++count;
	}

	// check for errors
	if(self->head || (count != 4)  ||
	   (strcmp(atts[0], "k") != 0) ||
	   (strcmp(atts[2], "v") != 0))
	{
		LOGE("invalid line=%i", self->line);
		parser->error = 1;
		return;
	}

	// mark peaks so missing ele tags may be
	// inserted in osm_element_evalNode
	if(strcmp(atts[1], "natural") == 0)
	{
	   if((strcmp(atts[3], "peak")    == 0) ||
	      (strcmp(atts[3], "volcano") == 0) ||
	      (strcmp(atts[3], "saddle")  == 0))
		{
			osm_element_t* parent = self->parent;
			if(parent)
			{
				parent->has_peak = 1;
			}
		}
	}

	if((strcmp(atts[1], "addr:street") == 0) ||
	   (strcmp(atts[1], "name") == 0)        ||
	   (strcmp(atts[1], "destination") == 0) ||
	   (strcmp(atts[1], "exit_to") == 0))
	{
		XML_Char name[4096];

		const XML_Char  v[2] = "v";
		const XML_Char* p[2] = { v, name };

		if(osm_element_parseName(self, atts[3], name) == 0)
		{
			parser->error = 1;
			return;
		}
		osm_parser_printElemBegin(parser, self->name, indent);
		osm_parser_printElemAttsPair(parser, &(atts[0]));
		osm_parser_printElemAttsPair(parser, p);
	}
	else if((strcmp(atts[1], "ele") == 0) ||
	        (strcmp(atts[1], "ele:ft") == 0))
	{
		XML_Char ele[4096];

		const XML_Char  v[2] = "v";
		const XML_Char* p[2] = { v, ele };

		int ft = (strcmp(atts[1], "ele:ft") == 0) ? 1 : 0;

		if(osm_element_parseEle(self, ft, atts[3], ele) == 0)
		{
			// ignore
			return;
		}
		osm_parser_printElemBegin(parser, self->name, indent);
		osm_parser_printElemAttsPair(parser, &(atts[0]));
		osm_parser_printElemAttsPair(parser, p);

		osm_element_t* parent = self->parent;
		if(parent)
		{
			parent->has_ele = 1;
			parent->ele     = (int) strtol(ele, NULL, 0);
		}
	}
	else
	{
		osm_parser_printElemBegin(parser, self->name, indent);
		osm_parser_printElemAttsPair(parser, &(atts[0]));
		osm_parser_printElemAttsPair(parser, &(atts[2]));
	}
	osm_parser_printElemEnd(parser, 1);
}

static void osm_element_evalDefault(osm_element_t* self,
                                    struct osm_parser_s* parser,
                                    int indent)
{
	assert(self);
	assert(parser);

	osm_parser_printElemBegin(parser, self->name, indent);
	osm_parser_printElemAttsSet(parser, (const XML_Char**) self->atts);
	if(self->head)
	{
		osm_parser_printElemEnd(parser, 0);
		osm_element_t* iter = self->head;
		while(iter)
		{
			osm_element_eval(iter, parser, indent + 1);
			iter = iter->next;
		}
		osm_parser_printElemClose(parser, self->name, indent);
	}
	else
	{
		osm_parser_printElemEnd(parser, 1);
	}
}

/***********************************************************
* public                                                   *
***********************************************************/

osm_element_t* osm_element_new(osm_element_t* parent,
                               int line,
                               const XML_Char* name,
                               const XML_Char** atts)
{
	// parent may be NULL for root
	assert(name);
	assert(atts);

	osm_element_t* self = (osm_element_t*)
	                      malloc(sizeof(osm_element_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	// copy name
	size_t len = strlen(name) + 1;
	self->name = (XML_Char*)
	             malloc(len*sizeof(XML_Char));
	if(self->name == NULL)
	{
		LOGE("malloc failed");
		goto fail_name;
	}
	strncpy(self->name, name, len);
	self->name[len - 1] = '\0';

	// count/alloc attributes
	int count = 0;
	while(atts[count])
	{
		++count;
	}

	//  alloc attributes
	self->atts = (XML_Char**)
	             calloc(count + 1, sizeof(XML_Char*));
	if(self->atts == NULL)
	{
		LOGE("malloc failed");
		goto fail_atts;
	}

	// copy attributes
	int i;
	for(i = 0; i < count; ++i)
	{
		len = strlen(atts[i]) + 1;
		self->atts[i] = (XML_Char*)
		                malloc(len*sizeof(XML_Char));
		if(self->atts[i] == NULL)
		{
			LOGE("malloc failed");
			goto fail_atts_i;
		}
		strncpy(self->atts[i], atts[i], len);
		self->atts[i][len - 1] = '\0';
	}

	self->line     = line;
	self->parent   = parent;
	self->next     = NULL;
	self->head     = NULL;
	self->tail     = NULL;
	self->has_lat  = 0;
	self->has_lon  = 0;
	self->has_ele  = 0;
	self->has_peak = 0;
	self->lat      = 0.0;
	self->lon      = 0.0;
	self->ele      = 0;

	// add the element to the parent
	if(parent)
	{
		if(parent->head == NULL)
		{
			parent->head = self;
			parent->tail = self;
		}
		else
		{
			parent->tail->next = self;
			parent->tail       = self;
		}
	}

	// success
	return self;

	//failure
	fail_atts_i:
		for(i = 0; i < count; ++i)
		{
			free(self->atts[i]);
		}
		free(self->atts);
	fail_atts:
		free(self->name);
	fail_name:
		free(self);
	return NULL;
}

// called by root or parent
void osm_element_delete(osm_element_t** _self)
{
	assert(_self);

	osm_element_t* self = *_self;
	if(self)
	{
		// parent is a ref
		// next is owned by parent

		// delete the children
		osm_element_t* iter = self->head;
		while(iter)
		{
			self->head = iter->next;
			osm_element_delete(&iter);
			iter = self->head;
		}
		self->tail = NULL;

		// delete the atts
		int i = 0;
		while(self->atts[i])
		{
			free(self->atts[i]);
			self->atts[i] = NULL;
			++i;
		}

		free(self->atts);
		free(self->name);
		free(self);
		*_self = NULL;
	}
}

void osm_element_eval(osm_element_t* self,
                      struct osm_parser_s* parser,
                      int indent)
{
	assert(self);
	assert(parser);

	if(strcmp(self->name, "node") == 0)
	{
		osm_element_evalNode(self, parser, indent);
	}
	else if(strcmp(self->name, "tag") == 0)
	{
		osm_element_evalTag(self, parser, indent);
	}
	else
	{
		osm_element_evalDefault(self, parser, indent);
	}
}
