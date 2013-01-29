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

#include "reference.h"

#include "repository.h"
#include "common.h"
#include "error.h"


using v8u::Int;
using v8u::Symbol;
using v8u::Bool;
using v8u::Func;
using v8::Local;
using v8::Persistent;
using v8::Function;

namespace gitteh {

Reference::Reference(git_reference* ptr): ref(ptr), invalid(false) {}
Reference::~Reference() {
  if (invalid) return;
  git_reference_free(ref);
}

V8_ESCTOR(Reference) { V8_CTOR_NO_JS }

// TODO: methods go here

// STATIC / FACTORY METHODS

//// Reference.lookup(...)

GITTEH_WORK_PRE(ref_lookupres) {
  v8::String::Utf8Value* name;
  Repository* repo;
  git_reference* out;
  error_info err;

  Persistent<Function> cb;
  uv_work_t req;
};

V8_SCB(Reference::LookupResolved) {
  v8::Local<v8::Object> repo_obj;
  if (!(args[0]->IsObject() && Repository::HasInstance(repo_obj = v8u::Obj(args[0]))))
    V8_STHROW(v8u::TypeErr("Repository needed as first argument."));
  if (!args[2]->IsFunction()) V8_STHROW(v8u::TypeErr("A Function is needed as callback!"));

  ref_lookupres_req* r = new ref_lookupres_req;
  r->repo = node::ObjectWrap::Unwrap<Repository>(repo_obj);
  r->name = new v8::String::Utf8Value(args[1]);

  r->cb = v8u::Persist<Function>(v8u::Cast<Function>(args[2]));
  GITTEH_WORK_QUEUE(ref_lookupres);
} GITTEH_WORK(ref_lookupres) {
  //
} GITTEH_WORK_AFTER(ref_lookupres) {
  v8::Handle<v8::Value> argv [2];
  if (r->out) {
    argv[0] = v8::Null();
    argv[1] = (new Reference(r->out))->Wrapped();
  } else {
    argv[0] = composeErr(r->err);//FIXME: test
    argv[1] = v8::Null();
  }
  GITTEH_WORK_CALL(2);
} GITTEH_END



NODE_ETYPE(Reference, "Reference") {
  //TODO
  
  Local<Function> func = templ->GetFunction();
  
//  func->Set(Symbol("lookup"), Func(Lookup)->GetFunction());
//  func->Set(Symbol("lookupSync"), Func(LookupSync)->GetFunction());
  
  func->Set(Symbol("lookupResolved"), Func(LookupResolved)->GetFunction());
//  func->Set(Symbol("lookupResolvedSync"), Func(LookupResolvedSync)->GetFunction());
} NODE_TYPE_END()

V8_POST_TYPE(Reference)

};

