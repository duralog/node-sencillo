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

#include "oid.h"

#include <node_buffer.h>

#include "error.h"


namespace gitteh {

Oid::Oid(const git_oid& other) {
  git_oid_cpy(&oid, &other);
}
Oid::Oid()  {}
Oid::~Oid() {}

V8_ECTOR(Oid) {
  if (!node::Buffer::HasInstance(args[0]))
    V8_THROW(v8u::TypeErr("Data must be a Buffer"));
  v8::Local<v8::Object> buffer = v8u::Obj(args[0]);
  if (node::Buffer::Length(buffer) < GIT_OID_RAWSZ)
    V8_THROW(v8u::RangeErr("Need more data"));
  char* data = node::Buffer::Data(buffer);
  Oid* inst = new Oid; V8_WRAP(inst);
  git_oid_fromraw(&inst->oid, (unsigned char*)data);
} V8_CTOR_END()

V8_CB(Oid::ToString) {
  Oid* inst = Unwrap(args.This());
  char hex [GIT_OID_HEXSZ];
  git_oid_fmt(hex, &inst->oid);
  V8_RET(v8u::Str(hex, GIT_OID_HEXSZ));
} V8_CB_END()

//V8_CB(Oid::ToRaw) {//FIXME: do we need two-step copy?
//  Oid* inst = Unwrap(args.This());
//  node::Buffer* buf = node::Buffer::New(GIT_OID_RAWSZ);
//  memcpy(node::Buffer::Data(buf), inst->oid.id, GIT_OID_RAWSZ);
//  //TODO: make fast buffer
//} V8_CB_END()
V8_CB(Oid::ToPath) {
  Oid* inst = Unwrap(args.This());
  char path [GIT_OID_HEXSZ+1];
  git_oid_pathfmt(path, &inst->oid);
  V8_RET(v8u::Str(path, GIT_OID_HEXSZ+1));
} V8_CB_END()
//TODO: Shortener

V8_CB(Oid::IsEmpty) {
  Oid* inst = Unwrap(args.This());
  V8_RET( v8u::Bool(git_oid_iszero(&inst->oid)) );
} V8_CB_END()
V8_CB(Oid::Compare) {
  Oid* inst = Unwrap(args.This());
  if (!(args[0]->IsObject() && HasInstance(v8u::Obj(args[0]))))
    V8_THROW(v8u::TypeErr("Another OID required as first argument."));
  git_oid* other = &(Oid::Unwrap(v8u::Obj(args[0]))->oid);
  int32_t n = v8u::Int(args[1]);
  if (n) V8_RET(v8u::Bool(!git_oid_ncmp(&inst->oid, other, n)));
  V8_RET( v8u::Int(git_oid_cmp(&inst->oid, other)) );
} V8_CB_END()
V8_CB(Oid::Equals) {
  Oid* inst = Unwrap(args.This());
  if (!(args[0]->IsObject() && HasInstance(v8u::Obj(args[0]))))
    V8_THROW(v8u::TypeErr("Another OID required as first argument."));
  git_oid* other = &(Oid::Unwrap(v8u::Obj(args[0]))->oid);
  V8_RET(v8u::Bool(git_oid_equal(&inst->oid, other)));
} V8_CB_END()

V8_CB(Oid::Inspect) {
  Oid* inst = Unwrap(args.This());
  char str [GITTEH_OID_REPR_LEN];
  char hex [GIT_OID_HEXSZ];
  git_oid_fmt(hex, &inst->oid);
  hex[GITTEH_OID_REPR_HEX_LEN] = 0;
  V8_RET(v8u::Str(str, sprintf(str,GITTEH_OID_REPR,hex)));
} V8_CB_END()

V8_CB(Oid::Parse) {
  v8::String::Utf8Value str (args[0]);

  Oid* inst = new Oid;
  v8::Local<v8::Object> ret = inst->Wrapped();

  if (*str == NULL) check(git_oid_fromstrn(&inst->oid, "", 0));
  else check(git_oid_fromstrn(&inst->oid, *str, str.length()));
  V8_RET(ret);
} V8_CB_END()

NODE_ETYPE(Oid, "Oid") {
  V8_DEF_CB("toString", ToString);
  //V8_DEF_CB("toRaw", ToRaw);
  V8_DEF_CB("toPath", ToPath);

  V8_DEF_CB("compare", Compare);
  V8_DEF_CB("equals", Equals);
  V8_DEF_CB("isEmpty", IsEmpty);

  V8_DEF_CB("inspect", Inspect);

  v8::Local<v8::Function> func = templ->GetFunction();
  func->Set(v8u::Symbol("parse"), v8u::Func(Parse)->GetFunction());
} NODE_TYPE_END()
V8_POST_TYPE(Oid)

};

