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


using v8u::Int;
using v8u::Symbol;
using v8u::Bool;
using v8u::Func;
using v8::Local;

namespace gitteh {

Repository::Repository(git_repository* ptr): repo(ptr) {}
Repository::~Repository() {
  git_repository_free(repo);
}

V8_ESCTOR(Repository) { V8_CTOR_NO_JS }

NODE_ETYPE(Repository, "Repository") {
  //TODO
  
  Local<v8::Function> func = templ->GetFunction();
  
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

