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

// prepare input for processing
inline void prepare(const char* in, char* out, int& len) {
  int o = 0;
  for (int i=0; i<len; i++)
    if (in[i] != 0) out[o++] = in[i]; // skip any \0
  out[o] = 0;                        // set last trailing \0
}

// another approach would be to call prettify() with no buffer,
// allocate the size and call again, but this is faster
V8_SCB(Prettify) {
  
  // Allocate
  v8::String::Utf8Value msg (args[0]);
  int len = msg.length();
  int out_len = len;
  char* in  = new char [++out_len]; // one more for the trailing \0
  char* out = new char [++out_len]; // another for the trailing \n

  // Prepare input
  prepare(*msg, in, len);

  // Call & return
  int final_len = git_message_prettify(out, out_len, in, v8u::Bool(args[1]));
  delete[] in;
  v8::Local<v8::String> ret = v8u::Str(out, final_len-1);
  delete[] out;
  return ret;

}

};

