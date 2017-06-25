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

#ifndef osm_element_H
#define osm_element_H

#include "libexpat/expat/lib/expat.h"

struct osm_parser_s;

typedef struct osm_element_s
{
	XML_Char*  name;
	XML_Char** atts;

	int line;

	// parent
	struct osm_element_s* parent;

	// siblings
	struct osm_element_s* next;

	// children
	struct osm_element_s* head;
	struct osm_element_s* tail;

	// properties
	int has_lat;
	int has_lon;
	int has_ele;
	int has_peak;
	double lat;
	double lon;
	int    ele;

	// database
	int  db_class;
	char db_name[256];
	char db_state[3];
} osm_element_t;

osm_element_t* osm_element_new(osm_element_t* parent,
                               int line,
                               const XML_Char* name,
                               const XML_Char** atts);
void           osm_element_delete(osm_element_t** _self);
void           osm_element_eval(osm_element_t* self,
                                struct osm_parser_s* parser,
                                int indent);
int            osm_element_toClass(const char* k,
                                   const char* v);
const char*    osm_element_fromClass(int class);

#endif
