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

namespace gitteh {

Repository::Repository(git_repository* ptr): repo(ptr) {}
Repository::~Repository() {
  git_repository_free(repo);
}

V8_ESCTOR(Repository) { V8_CTOR_NO_JS }

NODE_ETYPE(Repository, "Repository") {
  //TODO
  
  v8::Local<v8::Function> func = templ->GetFunction();
  
  //ENUM: repository states -- STATE
  func->Set(Symbol("STATE_NONE"), Int(GIT_REPOSITORY_STATE_NONE));
  func->Set(Symbol("STATE_MERGE"), Int(GIT_REPOSITORY_STATE_MERGE));
  func->Set(Symbol("STATE_REVERT"), Int(GIT_REPOSITORY_STATE_REVERT));
  func->Set(Symbol("STATE_CHERRY_PICK"), Int(GIT_REPOSITORY_STATE_CHERRY_PICK));
  func->Set(Symbol("STATE_BISECT"), Int(GIT_REPOSITORY_STATE_BISECT));
  func->Set(Symbol("STATE_REBASE"), Int(GIT_REPOSITORY_STATE_REBASE));
  func->Set(Symbol("STATE_REBASE_INTERACTIVE"), Int(GIT_REPOSITORY_STATE_REBASE_INTERACTIVE));
  func->Set(Symbol("STATE_REBASE_MERGE"), Int(GIT_REPOSITORY_STATE_REBASE_MERGE));
  func->Set(Symbol("STATE_APPLY_MAILBOX"), Int(GIT_REPOSITORY_STATE_APPLY_MAILBOX));
  func->Set(Symbol("STATE_APPLY_MAILBOX_OR_REBASE"), Int(GIT_REPOSITORY_STATE_APPLY_MAILBOX_OR_REBASE));

  //FLAG: init() options -- INIT
  func->Set(Symbol("INIT_BARE"), Int(GIT_REPOSITORY_INIT_BARE));
  func->Set(Symbol("INIT_NO_REINIT"), Int(GIT_REPOSITORY_INIT_NO_REINIT));
  func->Set(Symbol("INIT_NO_DOTGIT_DIR"), Int(GIT_REPOSITORY_INIT_NO_DOTGIT_DIR));
  func->Set(Symbol("INIT_MKDIR"), Int(GIT_REPOSITORY_INIT_MKDIR));
  func->Set(Symbol("INIT_MKPATH"), Int(GIT_REPOSITORY_INIT_MKPATH));
  func->Set(Symbol("INIT_EXTERNAL_TEMPLATE"), Int(GIT_REPOSITORY_INIT_EXTERNAL_TEMPLATE));

  //FLAG: init() mode options -- INIT_SHARED
  func->Set(Symbol("INIT_SHARED_UMASK"), Int(GIT_REPOSITORY_INIT_SHARED_UMASK));
  func->Set(Symbol("INIT_SHARED_GROUP"), Int(GIT_REPOSITORY_INIT_SHARED_GROUP));
  func->Set(Symbol("INIT_SHARED_ALL"), Int(GIT_REPOSITORY_INIT_SHARED_ALL));

  //FLAG: open() options -- OPEN
  func->Set(Symbol("OPEN_NO_SEARCH"), Int(GIT_REPOSITORY_OPEN_NO_SEARCH));
  func->Set(Symbol("OPEN_CROSS_FS"), Int(GIT_REPOSITORY_OPEN_CROSS_FS));

} NODE_TYPE_END()
V8_POST_TYPE(Repository)

};

