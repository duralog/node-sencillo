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

#include "git2.h"

#include "v8u.hpp"
#include "version.hpp"

#include "error.h"
#include "oid.h"
#include "object.h"
#include "reference.h"
#include "message.h"
#include "repository.h"

#define GITTEH_VERSION 0,1,0
#define SENCILLO_VERSION 0,1,1

using v8u::Symbol;
using v8u::Version;
using v8u::Int;
using v8u::Func;
using v8::Local;

namespace sencillo {

inline Local<v8::Object> libgit2Version() {
  int major, minor, revision;
  git_libgit2_version(&major, &minor, &revision);
  Version* v = new Version(major, minor, revision);
  return v->Wrapped();
}

NODE_DEF_MAIN() {
  // Version class & hash
  Version::init(target);
  Local<v8::Object> versions = v8u::Obj();
  versions->Set(Symbol("gitteh"), (new Version(GITTEH_VERSION))->Wrapped());
  versions->Set(Symbol("sencillo"), (new Version(SENCILLO_VERSION))->Wrapped());
  versions->Set(Symbol("libgit2"), libgit2Version());
  target->Set(Symbol("versions"), versions);

  // Other LibGit2 info
  target->Set(Symbol("capabilities"), Int(git_libgit2_capabilities()));

  //FLAG: capabilities -- CAP
  Local<v8::Object> capHash = v8u::Obj();
  capHash->Set(Symbol("THREADS"), Int(GIT_CAP_THREADS));
  capHash->Set(Symbol("HTTPS"), Int(GIT_CAP_HTTPS));
  target->Set(Symbol("Capability"), capHash);

  // Message utilities
  target->Set(Symbol("prettify"), Func(Prettify)->GetFunction());

  // Classes initialization
  Oid::init(target);
  GitObject::init(target);
  Repository::init(target);
  Reference::init(target);
} NODE_DEF_MAIN_END(sencillo)

};
