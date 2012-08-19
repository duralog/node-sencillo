/*
 * The MIT License
 *
 * Copyright (c) 2010 Sam Day
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

#include "v8u.hpp"
#include "version.hpp"

//TODO: here we should include EVERY HPP designing an V8 object
//      there's no need to include anything more (here).

#define GITTEH_VERSION 0,1,0

using v8u::Version;
using v8u::initVersion;

namespace gitteh {
  
  NODE_DEF_MAIN() {
    // Classes initialization
    //TODO: here, we should call init<class>(target)
    
    // Version class & hash
    initVersion(target);
    v8::Local<v8::Object> versions = v8u::Obj();
    versions->Set(v8u::Symbol("gitteh"), (new Version(GITTEH_VERSION))->Wrapped());
    target->Set(v8u::Symbol("versions"), versions);
  } NODE_DEF_MAIN_END(gitteh)
  
};
