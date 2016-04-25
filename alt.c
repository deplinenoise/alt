
// alt.c - a little template code generator for C++
//
// (Well, C too, but you'll have limited success without overloading the
// emit_expr() function calls.)
//
// usage: alt <input-file> <output-file>
//
// The input syntax is inspired by EJS for JavaScript, but hardcoded to a
// particular set of escapes:
//
// /*  control structures    */
// /*  control, eat ws      -*/
// /*- eval expr to print    */
// /*- eval + print, eat ws -*/
//
// Two functions signatures are expected to be provided by the host program:
//   emit(const char*, int) - write a literal string of bytes to the output stream
//   emit_expr(T t) - write some computed value output stream
//
// We rely on overloading in the latter case to make it convenient to write the
// templates.

// Copyright (c) 2016, Andreas Fredriksson
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>

// Flush a literal run of characters into an emit() call.
static void flush_litrun(const char* start, int len, FILE* out)
{
  int i = 0;
  fprintf(out, "  emit(\"");
  for (i = 0; i < len; ++i) {
    int cc = start[i];
    // Handle escaping the data to be able to put it in a C++ string literal.
    switch (cc) {
    case '"': fputs("\\\"", out); break;
    case '\n': fputs("\\n", out); break;
    case '\r': fputs("\\r", out); break;
    case '\t': fputs("\\t", out); break;
    case '\\': fputs("\\\\", out); break;
    default:
      if (isprint(cc)) {
        putc(cc, out);
      } else {
        fprintf(out, "\\x%02x", cc);
      }
      break;
    }
  }
  fprintf(out, "\", %d);\n", len);
}

// Called when an escape block has been entered (a special block comment).
// t      - first character inside the block comment marker
// te     - one past end of the onput array
// fn     - file name of the input file
// lineno - pointer to master copy of the line number
static const char* escape(const char* t, const char* te, const char* fn, FILE* out, int* lineno)
{
  int last_out = '\0';
  int is_expr = 0;
  int eat_trailing_ws = 0;

  fprintf(out, "#line %d \"%s\"\n", *lineno, fn);

  // An opening '-' in the comment means we're evaluating an expression and printing that.
  // Otherwise, we're just dumping out some literal code.

  if (t != te && *t == '-') {
    is_expr = 1;
    fprintf(out, "  emit_expr(");
    ++t;
  }

  while (t != te) {
    const int cc = *t;
    const int cn = t != te ? t[1] : '\0';

    if (cc == '\n') {
      ++(*lineno);
    }

    if (cc == '*' && cn == '/') {
      t += 2;
      break;
    } else if (cc == '-' && cn == '*' && (t + 2 == te ? 0 : t[2]) == '/') {
      eat_trailing_ws = 1;
      t += 3;
      break;
    } else {
      putc(cc, out);
      last_out = cc;
    }
    ++t;
  }

  // Eat trailing whitespace up to and including the next newline, if
  // requested.
  if (eat_trailing_ws) {
    while (t != te && isspace(*t)) {
      ++t;
      if ('\n' == *t) {
        ++(*lineno);
        break;
      }
    }
  }

  if (is_expr) {
    fprintf(out, ");\n");
  }
  // Make sure we have some newlines in the output, or the #line directives
  // will be all over.
  else if (last_out != '\n') {
    putc('\n', out);
  }
  return t;
}

// Process a block of text as a template.
// t   - pointer to input text
// len - length of input text
// fn  - file name of input file (for #line directives)
// out - output file
static void process(const char* t, size_t len, const char* fn, FILE* out)
{
  int lineno = 1;
  int litrun = 0;
  const char* litstart = NULL;
  const char* te = t + len;

  while (t != te) {
    const int cc = *t++;
    const int cn = t != te ? t[0] : '\0';

    if (cc == '\n') {
      ++lineno;
    }

    if (cc == '/' && cn == '*') {
      if (litrun > 0) {
        flush_litrun(litstart, litrun, out);
      }
      ++t;
      t = escape(t, te, fn, out, &lineno);
      litrun = 0;
    } else {
      if (litrun++ == 0) {
        litstart = t - 1;
      }
    }
  }

  if (litrun > 0) {
    flush_litrun(litstart, litrun, out);
  }
}

int main(int argc, char* argv[])
{
  FILE* inf = NULL;
  FILE* outf = NULL;
  size_t file_size = 0;
  char* buffer = NULL;
  int rc = 1;

  if (argc < 3) {
    fprintf(stderr, "need two filenames; input and output\n");
    goto leave;
  }

  inf = fopen(argv[1], "rb");
  if (!inf) {
    perror(argv[1]);
    goto leave;
  }

  fseek(inf, 0, SEEK_END);
  file_size = ftell(inf);
  rewind(inf);

  buffer = malloc(file_size);
  fread(buffer, 1, file_size, inf);

  outf = fopen(argv[2], "w");
  if (!outf) {
    perror(argv[2]);
    goto leave;
  }

  process(buffer, file_size, argv[1], outf);

  rc = 0;

leave:
  if (outf) fclose(outf);
  if (inf) fclose(inf);
  free(buffer);

  return rc;
}
