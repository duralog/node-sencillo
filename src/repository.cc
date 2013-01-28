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

#include "repository.h"

#include "error.h"


using v8u::Int;
using v8u::Symbol;
using v8u::Bool;
using v8u::Func;
using v8::Local;
using v8::Persistent;
using v8::Function;

namespace gitteh {

Repository::Repository(git_repository* ptr): repo(ptr) {}
Repository::~Repository() {
  git_repository_free(repo);
}

V8_ESCTOR(Repository) { V8_CTOR_NO_JS }

// TODO: methods go here

// STATIC / FACTORY METHODS

//// Repository.discover(...)

struct repo_discover_req {
  bool across_fs;
  v8::String::Utf8Value* start;
  char* ceiling_dirs;
  char* output;
  error_info err;

  Persistent<Function> cb;
  uv_work_t req;
};

//FIXME: error vs null
GITTEH_WORK(repo_discover)
  // prepare
  int len = r->start->length() + 7; //one for \0, more for "/.git/"
  char* out = new char[len];
  
  int status = git_repository_discover(out, len, **r->start, r->across_fs, r->ceiling_dirs);
  delete r->start;
  if (status!=GIT_OK) {
    collectErr(status, r->err);
    delete [] out;
    r->output = NULL;
    return;
  }
  
  r->output = out;
GITTEH_WORK_AFTER(repo_discover)
  v8::Handle<v8::Value> argv [2];
  if (r->output) {
    argv[0] = v8::Null();
    argv[1] = v8u::Str(r->output);
    delete [] r->output;
  } else {
    argv[0] = composeErr(r->err);
    argv[1] = v8::Null();
  }
GITTEH_WORK_END(2);

V8_SCB(Repository::Discover) {
  int len = args.Length()-1; // don't count the callback
  if (len < 1) V8_STHROW(v8u::RangeErr("Not enough arguments!"));
  if (len > 3) len = 3;
  if (!args[len]->IsFunction()) V8_STHROW(v8u::TypeErr("A Function is needed as callback!"));
  
  repo_discover_req* r = new repo_discover_req;
  r->start = new v8::String::Utf8Value(args[0]);
  
  r->across_fs = len>=2 ? v8u::Bool(args[1]) : false;
  r->ceiling_dirs = NULL; //FIXME:ceiling
  
  r->cb = v8u::Persist<Function>(v8u::Cast<Function>(args[len]));
  GITTEH_WORK_QUEUE(repo_discover);
}

V8_SCB(Repository::DiscoverSync) {
  v8::String::Utf8Value start (args[0]);
  int len = start.length();
  char* cstart = new char[len+1];
  memcpy(cstart, *start, len);
  cstart[len] = 0;
  
  len += 7; //one for \0, more for "/.git/"
  char* out = new char[len];
  
  error_info info;
  int status = git_repository_discover(out, len, cstart, v8u::Bool(args[1]), NULL); //FIXME:ceiling
  if (status == GIT_OK) {
    Local<v8::String> ret = v8u::Str(out);
    delete [] out;
    delete [] cstart;
    return ret;
  }
  collectErr(status, info);
  delete [] out;
  delete [] cstart;
  V8_STHROW(composeErr(info));
}



NODE_ETYPE(Repository, "Repository") {
  //TODO
  
  Local<Function> func = templ->GetFunction();
  
  func->Set(Symbol("discover"), Func(Discover)->GetFunction());
  func->Set(Symbol("discoverSync"), Func(DiscoverSync)->GetFunction());
  
  //ENUM: repository states -- STATE
  Local<v8::Object> stateHash = v8u::Obj();
  stateHash->Set(Symbol("NONE"), Int(GIT_REPOSITORY_STATE_NONE));
  stateHash->Set(Symbol("MERGE"), Int(GIT_REPOSITORY_STATE_MERGE));
  stateHash->Set(Symbol("REVERT"), Int(GIT_REPOSITORY_STATE_REVERT));
  stateHash->Set(Symbol("CHERRY_PICK"), Int(GIT_REPOSITORY_STATE_CHERRY_PICK));
  stateHash->Set(Symbol("BISECT"), Int(GIT_REPOSITORY_STATE_BISECT));
  stateHash->Set(Symbol("REBASE"), Int(GIT_REPOSITORY_STATE_REBASE));
  stateHash->Set(Symbol("REBASE_INTERACTIVE"), Int(GIT_REPOSITORY_STATE_REBASE_INTERACTIVE));
  stateHash->Set(Symbol("REBASE_MERGE"), Int(GIT_REPOSITORY_STATE_REBASE_MERGE));
  stateHash->Set(Symbol("APPLY_MAILBOX"), Int(GIT_REPOSITORY_STATE_APPLY_MAILBOX));
  stateHash->Set(Symbol("APPLY_MAILBOX_OR_REBASE"), Int(GIT_REPOSITORY_STATE_APPLY_MAILBOX_OR_REBASE));
  func->Set(Symbol("State"), stateHash);

  //FLAG: init() options -- INIT
  Local<v8::Object> initFlagHash = v8u::Obj();
  initFlagHash->Set(Symbol("BARE"), Int(GIT_REPOSITORY_INIT_BARE));
  initFlagHash->Set(Symbol("NO_REINIT"), Int(GIT_REPOSITORY_INIT_NO_REINIT));
  initFlagHash->Set(Symbol("NO_DOTGIT_DIR"), Int(GIT_REPOSITORY_INIT_NO_DOTGIT_DIR));
  initFlagHash->Set(Symbol("MKDIR"), Int(GIT_REPOSITORY_INIT_MKDIR));
  initFlagHash->Set(Symbol("MKPATH"), Int(GIT_REPOSITORY_INIT_MKPATH));
  initFlagHash->Set(Symbol("EXTERNAL_TEMPLATE"), Int(GIT_REPOSITORY_INIT_EXTERNAL_TEMPLATE));
  func->Set(Symbol("InitFlag"), initFlagHash);

  //FLAG: init() mode options -- INIT_SHARED
  Local<v8::Object> initModeHash = v8u::Obj();
  initModeHash->Set(Symbol("SHARED_UMASK"), Int(GIT_REPOSITORY_INIT_SHARED_UMASK));
  initModeHash->Set(Symbol("SHARED_GROUP"), Int(GIT_REPOSITORY_INIT_SHARED_GROUP));
  initModeHash->Set(Symbol("SHARED_ALL"), Int(GIT_REPOSITORY_INIT_SHARED_ALL));
  func->Set(Symbol("InitMode"), initModeHash);

  //FLAG: open() options -- OPEN
  Local<v8::Object> openFlagHash = v8u::Obj();
  openFlagHash->Set(Symbol("NO_SEARCH"), Int(GIT_REPOSITORY_OPEN_NO_SEARCH));
  openFlagHash->Set(Symbol("CROSS_FS"), Int(GIT_REPOSITORY_OPEN_CROSS_FS));
  func->Set(Symbol("OpenFlag"), openFlagHash);

} NODE_TYPE_END()
V8_POST_TYPE(Repository)

};

