alt - A Little Template Code Generator for C++
==============================================

This is a little program that turns a file "inside out", so that literal runs
of characters turn into function calls printing those characters. You can then
drop in control structures and dynamic to print by using special escape block
comments. This is more maintainable than writing a bunch of `printf()` or
`std::cout` calls and handle escaping and all kinds of nonsense.

The block comments recognized are:

- `/* <arbitrary C++> */` - Control structure, no whitespace trimming
l `/* <arbitrary C++> -*/` - Control structure, whitespace up to and including following newline is removed
- `/*- <C++ expression> */` - Emit expression, no whitespace trimming
- `/*- <C++ expression> -*/` - Emit expression, whitespace up to and including following newline is removed

Ways to use alt
---------------

Generating a function:

    /* void print_stuff(bool bar, int k) { -*/
    Here's some text.
    /* if (bar) {-*/
    This is optionally printed. k = /*- k */
    /* } -*/
    Here's some more text.
    /* } -*/

This generates the following C++ program (with `#line` directives removed for readability):

     void print_stuff(bool bar, int k) {
      emit("Here's some text.\n", 18);
      if (bar) {
        emit("This is optionally printed. k = ", 32);
        emit_expr( k );
        emit("\n", 1);
      }
      emit("Here's some more text.\n", 23);
     }

You can see two things going on here. Runs of literal text are converted to
`emit()` function calls. Expressions turn into `emit_expr()` calls. The host
program must define these two functions.

Alternatively the host program might want to drop in an alt template inline. In
that case, an `#include` statement does the trick.

File `includeme.alt`:

    Here's some text.
    /* if (bar) {-*/
    This is optionally printed.
    /* } -*/
    Here's some more text.

Run alt:

    alt includeme.alt includeme.inl

Host program:

    void print_stuff(bool bar)
    {
    #include "includeme.inl"
    }
  

LICENSE
-------

Alt is licensed under the liberal 2-clause BSD license, as follows.

    Copyright (c) 2016, Andreas Fredriksson
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

