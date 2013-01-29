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

#ifndef GITTEH_COMMON_H
#define	GITTEH_COMMON_H

#include "v8u.hpp"

namespace gitteh {

#define GITTEH_ERROR_THROWER(IDENTIFIER, ERR)                                  \
  V8_SCB(IDENTIFIER) {                                                         \
    v8::HandleScope handle;                                                    \
    return v8::ThrowException(ERR);                                            \
  }

// Use on abstract callbacks
V8_SCB(_isAbstract);


// Conversion and escaping goodies
//buildPathList


// Macros for asynchronous work
// --based on node-lame's code

//Unwrapping and starting work statement-macros
#define GITTEH_ASYNC_CSTR(OBJ, VAR)                                            \
  int len = OBJ->length();                                                     \
  char* VAR = new char[len+1];                                                 \
  memcpy(VAR, **OBJ, len);                                                     \
  VAR[len] = 0
#define GITTEH_SYNC_CSTR(OBJ, VAR)                                             \
  int len = OBJ.length();                                                      \
  char* VAR = new char[len+1];                                                 \
  memcpy(VAR, *OBJ, len);                                                      \
  VAR[len] = 0

#define GITTEH_WORK_UNWRAP(IDENTIFIER)                                         \
  IDENTIFIER##_req* r = (IDENTIFIER##_req*)req->data
#define GITTEH_WORK_QUEUE(IDENTIFIER)                                          \
  r->req.data = r;                                                             \
  return v8::Integer::New(uv_queue_work(uv_default_loop(), &r->req,            \
                                        IDENTIFIER##_work, IDENTIFIER##_after))
#define GITTEH_WORK_CALL(ARGC)                                                 \
  v8::TryCatch try_catch;                                                      \
  r->cb->Call(v8::Context::GetCurrent()->Global(), ARGC, argv);                \
  r->cb.Dispose();                                                             \
  delete r;                                                                    \
  if (try_catch.HasCaught()) node::FatalException(try_catch)


//Work callbacks block-macros
#define GITTEH_WORK_PRE(IDENTIFIER)                                            \
  void IDENTIFIER##_work(uv_work_t *req);                                      \
  void IDENTIFIER##_after(uv_work_t *req);                                     \
  struct IDENTIFIER##_req

#define GITTEH_WORK(IDENTIFIER)                                                \
  void IDENTIFIER##_work(uv_work_t *req) {                                     \
    GITTEH_WORK_UNWRAP(IDENTIFIER);

#define GITTEH_WORK_AFTER(IDENTIFIER) }                                        \
  void IDENTIFIER##_after(uv_work_t *req) {                                    \
    v8::HandleScope scope;                                                     \
    GITTEH_WORK_UNWRAP(IDENTIFIER);

#define GITTEH_END }

#define GITTEH_CHECK_CB_ARGS(MIN)                                              \
  if (len < 1) V8_STHROW(v8u::RangeErr("Not enough arguments!"));

};

#endif	/* GITTEH_COMMON_H */

