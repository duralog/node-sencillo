/*
 * The MIT License
 *
 * Copyright (c) 2010 Sam Day
 * Copyright (c) 2012 Xavier Mendez
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "message.h"

#include "git2.h"


namespace gitteh {

// another approach would be to call prettify() with no buffer,
// allocate the size and call again, but this is faster
V8_CB(Prettify) {
  
  // Allocate
  v8::String::Utf8Value msg (args[0]);
  int len = msg.length();
  int out_len = len;
  char* in  = new (std::nothrow) char [++out_len]; // one more for the trailing \0
  if (in == NULL) V8_THROW(v8u::Err("Couldn't allocate memory."));
  char* out = new (std::nothrow) char [++out_len]; // another for the trailing \n
  if (out == NULL) {
    delete[] in;
    V8_THROW(v8u::Err("Couldn't allocate memory."));
  }

  // Prepare input
  memcpy(in, *msg, len);
  for (int i=0; i<len; i++)
    if (in[i] == 0) in[i] = 0x20;   // replace \0 with space
  in[len] = 0;                      // set last trailing \0

  // Call & return
  int final_len = git_message_prettify(out, out_len, in, v8u::Bool(args[1]));
  delete[] in;
  v8::Local<v8::String> ret = v8u::Str(out, final_len-1);
  delete[] out;
  V8_RET(ret);

} V8_CB_END()

};

