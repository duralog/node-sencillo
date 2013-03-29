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

#ifndef SENCILLO_COMMON_H
#define	SENCILLO_COMMON_H

#include "v8u.hpp"
#include "cvv8/convert.hpp"

namespace sencillo {

#define SENCILLO_ERROR_THROWER(IDENTIFIER, ERR)                                  \
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
//TODO: DO SOMETHING WITH STRINGS THAT CONTAIN '\0'
#define SENCILLO_ASYNC_CSTR(OBJ, VAR)                                            \
  int VAR##_len = OBJ->length();                                               \
  char* VAR = new char[VAR##_len+1];                                           \
  memcpy(VAR, **OBJ, VAR##_len);                                               \
  delete OBJ;                                                                  \
  VAR[VAR##_len] = 0
#define SENCILLO_SYNC_CSTR(OBJ, VAR)                                             \
  int VAR##_len = OBJ.length();                                                \
  char* VAR = new char[VAR##_len+1];                                           \
  memcpy(VAR, *OBJ, VAR##_len);                                                \
  VAR[VAR##_len] = 0

#define SENCILLO_WORK_UNWRAP(IDENTIFIER)                                         \
  IDENTIFIER##_req* r = (IDENTIFIER##_req*)req->data
#define SENCILLO_WORK_QUEUE(IDENTIFIER)                                          \
  r->req.data = r;                                                             \
  return v8::Integer::New(uv_queue_work(uv_default_loop(), &r->req,            \
                                        IDENTIFIER##_work, (uv_after_work_cb)IDENTIFIER##_after))
#define SENCILLO_WORK_CALL(ARGC)                                                 \
  v8::TryCatch try_catch;                                                      \
  r->cb->Call(v8::Context::GetCurrent()->Global(), ARGC, argv);                \
  r->cb.Dispose();                                                             \
  delete r;                                                                    \
  if (try_catch.HasCaught()) node::FatalException(try_catch)

#define SENCILLO_CB_CALL(CB, ARGC)                                             \
  v8::TryCatch try_catch;                                                      \
  r->cb->Call(v8::Context::GetCurrent()->Global(), ARGC, argv);                \
  r->cb.Dispose();                                                             \
  delete r;                                                                    \
  if (try_catch.HasCaught()) node::FatalException(try_catch)

//Work callbacks block-macros
#define SENCILLO_WORK_PRE(IDENTIFIER)                                            \
  void IDENTIFIER##_work(uv_work_t *req);                                      \
  void IDENTIFIER##_after(uv_work_t *req);                                     \
  struct IDENTIFIER##_req

#define SENCILLO_WORK(IDENTIFIER)                                                \
  void IDENTIFIER##_work(uv_work_t *req) {                                     \
    SENCILLO_WORK_UNWRAP(IDENTIFIER);

#define SENCILLO_WORK_AFTER(IDENTIFIER) }                                        \
  void IDENTIFIER##_after(uv_work_t *req) {                                    \
    v8::HandleScope scope;                                                     \
    SENCILLO_WORK_UNWRAP(IDENTIFIER);

#define SENCILLO_END }

#define SENCILLO_CHECK_CB_ARGS(MIN)                                              \
  if (len < 1) V8_STHROW(v8u::RangeErr("Not enough arguments!"));

};

#endif	/* SENCILLO_COMMON_H */

