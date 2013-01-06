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

#ifndef GITTEH_OID_H
#define	GITTEH_OID_H

#include "git2.h"

#include "v8u.hpp"

namespace gitteh {

#define GITTEH_OID_REPR "<Oid %s>"
#define GITTEH_OID_REPR_HEX_LEN 10
#define GITTEH_OID_REPR_LEN 17 //+1 for the terminating NUL

class Oid : public node::ObjectWrap {
public:
  Oid(const git_oid& other);
  Oid();
  ~Oid();
  static v8::Persistent<v8::FunctionTemplate> prim;
  V8_SCTOR();
  
  static V8_SCB(ToString);
//  static V8_SCB(ToRaw);
  static V8_SCB(ToPath);
  //TODO: Shortener
  
  static V8_SCB(IsEmpty);
  static V8_SCB(Compare);
  static V8_SCB(Equals);
  
  static V8_SCB(Inspect);
  
  static V8_SCB(Parse);
  
  NODE_STYPE(Oid);
protected:
  git_oid oid;
};

};

#endif	/* GITTEH_OID_H */

