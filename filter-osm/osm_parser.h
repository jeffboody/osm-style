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

#ifndef osm_parser_H
#define osm_parser_H

#include <stdio.h>
#include "libexpat/expat/lib/expat.h"
#include "osm_element.h"

typedef struct osm_parser_s
{
	int indent;
	int error;
	FILE* in;
	FILE* out;
	XML_Parser xml;
	osm_element_t* current;
} osm_parser_t;

osm_parser_t* osm_parser_new(const char* fname_in,
                             const char* fname_out);
int           osm_parser_parse(osm_parser_t* self);
void          osm_parser_delete(osm_parser_t** _self);
void          osm_parser_printElemBegin(osm_parser_t* self,
                                        const XML_Char* name,
                                        int indent);
void          osm_parser_printElemAttsSet(osm_parser_t* self,
                                          const XML_Char** atts);
void          osm_parser_printElemAttsPair(osm_parser_t* self,
                                           const XML_Char** atts);
void          osm_parser_printElemEnd(osm_parser_t* self, int close);
void          osm_parser_printElemClose(osm_parser_t* self,
                                        const XML_Char* name,
                                        int indent);

#endif
