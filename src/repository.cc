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

Repository::Repository(git_repository* ptr): repo(ptr) {}
Repository::~Repository() {
  git_repository_free(repo);
}

V8_ESCTOR(Repository) { V8_CTOR_NO_JS }

static inline bool FireCallback(v8::Handle<v8::Function> callback, int argc,
  v8::Handle<v8::Value> argv[]) {
  v8::TryCatch tryCatch;
  callback->Call(v8::Context::GetCurrent()->Global(), argc, argv);
  if(tryCatch.HasCaught()) {
    node::FatalException(tryCatch);
    return false;
  }

  return true;
}


// A FEW ACCESSORS

V8_ESGET(Repository, GetWorkdir) {
  V8_M_UNWRAP(Repository, info.Holder());
  return v8u::Str(git_repository_workdir(inst->repo));
}

V8_ESGET(Repository, GetPath) {
  V8_M_UNWRAP(Repository, info.Holder());
  return v8u::Str(git_repository_path(inst->repo));
}

V8_ESGET(Repository, IsBare) {
  V8_M_UNWRAP(Repository, info.Holder());
  return v8u::Bool(git_repository_is_bare(inst->repo));
}

// SYMBOLS

static Persistent<v8::String> stats_bytes_symbol;
static Persistent<v8::String> stats_received_symbol;
static Persistent<v8::String> stats_indexed_symbol;
static Persistent<v8::String> stats_total_symbol;
static Persistent<v8::String> stats_steps_complete_symbol;
static Persistent<v8::String> stats_steps_symbol;
static Persistent<v8::String> stats_path_symbol;
static Persistent<v8::String> stats_bare_symbol;
static Persistent<v8::String> stats_onprogress_symbol;


// STATIC / FACTORY METHODS

//// Repository.discover(...)

GITTEH_WORK_PRE(repo_discover) {
  bool across_fs;
  v8::String::Utf8Value* start;
  char* ceiling_dirs;
  char* out;
  error_info err;

  Persistent<Function> cb;
  uv_work_t req;
};

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
} GITTEH_WORK(repo_discover) { //FIXME: error vs null
  GITTEH_ASYNC_CSTR(r->start, cstart);
  cstart_len += 7; //one for \0, more for "/.git/"
  r->out = new char[cstart_len];

  int status = git_repository_discover(r->out, cstart_len, cstart, r->across_fs, r->ceiling_dirs);
  delete [] cstart;
  if (status==GIT_OK) return;
  collectErr(status, r->err);
  delete [] r->out;
  r->out = NULL;
} GITTEH_WORK_AFTER(repo_discover) {
  v8::Handle<v8::Value> argv [2];
  if (r->out) {
    argv[0] = v8::Null();
    argv[1] = v8u::Str(r->out);
    delete [] r->out;
  } else {
    argv[0] = composeErr(r->err);
    argv[1] = v8::Null();
  }
  GITTEH_WORK_CALL(2);
} GITTEH_END

V8_SCB(Repository::DiscoverSync) {
  v8::String::Utf8Value start (args[0]);
  GITTEH_SYNC_CSTR(start, cstart);
  cstart_len += 7; //one for \0, more for "/.git/"
  char* out = new char[cstart_len];

  error_info info;
  int status = git_repository_discover(out, cstart_len, cstart, v8u::Bool(args[1]), NULL); //FIXME:ceiling

  delete [] out;
  delete [] cstart;
  if (status == GIT_OK) return v8u::Str(out);
  collectErr(status, info);
  V8_STHROW(composeErr(info));
}


//// Repository.open(...)

GITTEH_WORK_PRE(repo_open) {
  git_repository* out;
  v8::String::Utf8Value* path;
  int flags;
  bool ext;
  char* ceiling_dirs;
  error_info err;

  Persistent<Function> cb;
  uv_work_t req;
};

//TODO: make an exists(...) pair
V8_SCB(Repository::Open) {
  int len = args.Length()-1; // don't count the callback
  if (len < 1) V8_STHROW(v8u::RangeErr("Not enough arguments!"));
  if (len > 3) len = 3;
  if (!args[len]->IsFunction()) V8_STHROW(v8u::TypeErr("A Function is needed as callback!"));

  repo_open_req* r = new repo_open_req;
  r->path = new v8::String::Utf8Value(args[0]);
  if ((r->ext = len > 1)) {
    // enter extended mode if not only the path is given
    r->flags = Int(args[1]);
    /**if (len > 2) r->ceiling_dirs = TODO;
    else**/ r->ceiling_dirs = NULL;
  }

  r->cb = v8u::Persist<Function>(v8u::Cast<Function>(args[len]));
  GITTEH_WORK_QUEUE(repo_open);
} GITTEH_WORK(repo_open) {
  GITTEH_ASYNC_CSTR(r->path, cpath);

  int status;
  if (r->ext) status = git_repository_open_ext(&r->out, cpath, r->flags, r->ceiling_dirs);
  else        status = git_repository_open    (&r->out, cpath);

  if (status == GIT_OK) {
    delete [] cpath;
  } else {
    collectErr(status, r->err);
    delete [] cpath;
    r->out = NULL;
  }
} GITTEH_WORK_AFTER(repo_open) {
  v8::Handle<v8::Value> argv [2];
  if (r->out) {
    argv[0] = v8::Null();
    argv[1] = (new Repository(r->out))->Wrapped();
  } else {
    argv[0] = composeErr(r->err);
    argv[1] = v8::Null();
  }
  GITTEH_WORK_CALL(2);
} GITTEH_END

V8_SCB(Repository::OpenSync) {
  v8::String::Utf8Value path (args[0]);
  GITTEH_SYNC_CSTR(path, cpath);

  int status;
  git_repository* out;
  error_info err;
  if (args.Length() > 1) {
    // enter extended mode if not only the path is given
    int flags = Int(args[1]);
    char* ceiling_dirs;
    /**if (len > 2) ceiling_dirs = TODO;
    else**/ ceiling_dirs = NULL;

    status = git_repository_open_ext(&out, cpath, flags, ceiling_dirs);
    //if (ceiling_dirs) delete [] ceiling_dirs;
  } else status = git_repository_open(&out, cpath);

  delete [] cpath;
  if (status == GIT_OK) return (new Repository(out))->Wrapped();
  collectErr(status, err);
  V8_STHROW(composeErr(err));
}

//// Repository.init(path, [flags], callback)

GITTEH_WORK_PRE(repo_init) {
  git_repository* out;
  v8::String::Utf8Value* path;
  int flags;
  error_info err;

  Persistent<Function> cb;
  uv_work_t req;
};

V8_SCB(Repository::Init) {
  int len = args.Length()-1; // don't count the callback
  if (len < 1) V8_STHROW(v8u::RangeErr("Not enough arguments!"));
  if (len > 1) V8_STHROW(v8u::RangeErr("flags are not implemented yet!"));
  if (len > 1) len = 1;
  if (!args[len]->IsFunction()) V8_STHROW(v8u::TypeErr("A Function is needed as callback!"));

  repo_init_req* r = new repo_init_req;
  r->path = new v8::String::Utf8Value(args[0]);
  /*if (len > 1) {
    // enter extended mode if not only the path is given
    r->flags = Int(args[1]);
  } else if(!args[--len]->IsFunction()) {
    V8_STHROW(v8u::TypeErr("A Function is needed as callback!"));
  }*/

  r->cb = v8u::Persist<Function>(v8u::Cast<Function>(args[len]));
  GITTEH_WORK_QUEUE(repo_init);
} GITTEH_WORK(repo_init) {
  GITTEH_ASYNC_CSTR(r->path, cpath);

  int status;
  git_repository_init_options opts = {
    GIT_REPOSITORY_INIT_OPTIONS_VERSION,
    r->flags | GIT_REPOSITORY_INIT_MKPATH,
    GIT_REPOSITORY_INIT_SHARED_UMASK,
    NULL, // workdir_path
    NULL, // description (TODO)
    NULL, // template_path
    NULL, // initial_head
    NULL  // origin_url (TODO)
  };
  status = git_repository_init_ext(&r->out, cpath, &opts);

  if (status == GIT_OK) {
    delete [] cpath;
  } else {
    collectErr(status, r->err);
    delete [] cpath;
    r->out = NULL;
  }
} GITTEH_WORK_AFTER(repo_init) {
  v8::Handle<v8::Value> argv [2];
  if (r->out) {
    argv[0] = v8::Null();
    argv[1] = (new Repository(r->out))->Wrapped();
  } else {
    argv[0] = composeErr(r->err);
    argv[1] = v8::Null();
  }
  GITTEH_WORK_CALL(2);
} GITTEH_END

V8_SCB(Repository::InitSync) {
  v8::String::Utf8Value path (args[0]);
  GITTEH_SYNC_CSTR(path, cpath);

  int status;
  git_repository* out;
  error_info err;
  git_repository_init_options opts = {
    GIT_REPOSITORY_INIT_OPTIONS_VERSION, // default
    // this will surely change, as I will probably make this collect this data from an object
    (args.Length() > 1 ? Int(args[1]) : 0) | GIT_REPOSITORY_INIT_MKPATH,
    GIT_REPOSITORY_INIT_SHARED_UMASK,
    NULL, // workdir_path
    NULL, // description (TODO)
    NULL, // template_path
    NULL, // initial_head
    NULL  // origin_url (TODO)
  };

  status = git_repository_init_ext(&out, cpath, &opts);

  delete [] cpath;
  if (status == GIT_OK) return (new Repository(out))->Wrapped();
  collectErr(status, err);
  V8_STHROW(composeErr(err));
}

//// Repository.clone(url, path, [bare], callback)

typedef struct progress_data {
  git_transfer_progress fetch_progress;
  size_t completed_steps;
  size_t total_steps;
  const char *path;
  Persistent<Function> cb;
} progress_data;

static void progress_callback(const progress_data *pd) {
  if(pd->cb->IsFunction()) {
    v8::Handle<v8::Object> o = v8::Object::New();
    o->Set(stats_bytes_symbol, v8::Number::New(pd->fetch_progress.received_bytes));
    o->Set(stats_received_symbol, v8::Number::New(pd->fetch_progress.received_objects));
    o->Set(stats_indexed_symbol, v8::Number::New(pd->fetch_progress.indexed_objects));
    o->Set(stats_total_symbol, v8::Number::New(pd->fetch_progress.total_objects));
    o->Set(stats_steps_complete_symbol, v8::Number::New(pd->completed_steps));
    o->Set(stats_steps_symbol, v8::Number::New(pd->total_steps));
    o->Set(stats_path_symbol, v8::String::New(pd->path));

    v8::Handle<v8::Value> argv[] = { o };
    gitteh::FireCallback(pd->cb, 1, argv);
  }

  //int network_percent = (100*pd->fetch_progress.received_objects) / pd->fetch_progress.total_objects;
  //int index_percent = (100*pd->fetch_progress.indexed_objects) / pd->fetch_progress.total_objects;
  //int checkout_percent = pd->total_steps > 0
  //  ? (100 * pd->completed_steps) / pd->total_steps
  //  : 0.f;
  //int kbytes = pd->fetch_progress.received_bytes / 1024;

  /*printf("net %3d%% (%4d kb, %5d/%5d)  /  idx %3d%% (%5d/%5d)  /  chk %3d%% (%4" PRIuZ "/%4" PRIuZ ") %s\n",
       network_percent, kbytes,
       pd->fetch_progress.received_objects, pd->fetch_progress.total_objects,
       index_percent, pd->fetch_progress.indexed_objects, pd->fetch_progress.total_objects,
       checkout_percent,
       pd->completed_steps, pd->total_steps,
       pd->path);*/
}

static int fetch_progress(const git_transfer_progress *stats, void *payload) {
  progress_data *pd = (progress_data*)payload;
  pd->fetch_progress = *stats;
  progress_callback(pd);
  return 0;
}

static void checkout_progress(const char *path, size_t cur, size_t tot, void *payload) {
  progress_data *pd = (progress_data*)payload;
  pd->completed_steps = cur;
  pd->total_steps = tot;
  pd->path = path;
  progress_callback(pd);
}

GITTEH_WORK_PRE(repo_clone) {
  git_repository* out;
  v8::String::Utf8Value* path;
  v8::String::Utf8Value* url;
  int bare;
  error_info err;

  Persistent<Function> progress_cb;
  Persistent<Function> creds_cb;
  Persistent<Function> cb;
  progress_data pd;
  uv_work_t req;
};

V8_SCB(Repository::Clone) {
  int len = args.Length()-1; // don't count the callback
  if (len < 2) V8_STHROW(v8u::RangeErr("Not enough arguments!"));
  if (len > 2) V8_STHROW(v8u::RangeErr("flags are not implemented yet!"));
  if (len > 2) len = 2;
  if (!args[len]->IsFunction()) {
    V8_STHROW(v8u::TypeErr("An Function is needed as callback!"));
  }

  repo_clone_req* r = new repo_clone_req;
  r->url = new v8::String::Utf8Value(args[0]);
  r->path = new v8::String::Utf8Value(args[1]);

  Local<v8::Object> opts;
  if (args[len-1]->IsObject()) {
    opts = v8::Object::Cast(*args[len-1]);
    Local<v8::Value>progress_cb = opts->Get(stats_onprogress_symbol);
    if(progress_cb->IsFunction()) {
      r->pd.cb = v8::Function::Cast(*progress_cb);
    }
  }

  GITTEH_WORK_QUEUE(repo_clone);
} GITTEH_WORK(repo_clone) {
  GITTEH_ASYNC_CSTR(r->path, cpath);
  GITTEH_ASYNC_CSTR(r->url, curl);

  int status;
  git_checkout_opts checkout_opts = {};
  checkout_opts.version = GIT_CHECKOUT_OPTS_VERSION;
  checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE_CREATE;
  checkout_opts.progress_cb = checkout_progress;
  checkout_opts.progress_payload = &r->pd;

  git_clone_options opts = {};
  opts.version = GIT_CLONE_OPTIONS_VERSION;
  opts.checkout_opts = checkout_opts;
  opts.fetch_progress_cb = &fetch_progress;
  opts.fetch_progress_payload = &r->pd;
  // TODO: credentials callback
  //opts.cred_acquire_cb = cred_acquire;

  status = git_clone(&r->out, curl, cpath, &opts);

  if (status == GIT_OK) {
    delete [] cpath;
    delete [] curl;
  } else {
    collectErr(status, r->err);
    delete [] cpath;
    delete [] curl;
    r->out = NULL;
  }
} GITTEH_WORK_AFTER(repo_clone) {
  v8::Handle<v8::Value> argv [2];
  if (r->out) {
    argv[0] = v8::Null();
    argv[1] = (new Repository(r->out))->Wrapped();
  } else {
    argv[0] = composeErr(r->err);
    argv[1] = v8::Null();
  }
  GITTEH_WORK_CALL(2);
} GITTEH_END

V8_SCB(Repository::CloneSync) {
  int len = args.Length(); // don't count the callback
  if (len < 2) V8_STHROW(v8u::RangeErr("Not enough arguments!"));
  if (len > 2) V8_STHROW(v8u::RangeErr("flags are not implemented yet!"));
  if (len > 2) len = 2;

  v8::String::Utf8Value url (args[0]);
  GITTEH_SYNC_CSTR(url, curl);
  v8::String::Utf8Value path (args[1]);
  GITTEH_SYNC_CSTR(path, cpath);

  int status;
  git_repository* out;
  error_info err;
  progress_data pd = {};

  git_checkout_opts checkout_opts = {};
  checkout_opts.version = GIT_CHECKOUT_OPTS_VERSION;
  checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE_CREATE;
  checkout_opts.progress_cb = checkout_progress;
  checkout_opts.progress_payload = &pd;

  git_clone_options clone_opts = {};
  clone_opts.version = GIT_CLONE_OPTIONS_VERSION;
  clone_opts.checkout_opts = checkout_opts;
  clone_opts.fetch_progress_cb = &fetch_progress;
  clone_opts.fetch_progress_payload = &pd;
  // TODO: credentials callback
  //opts.cred_acquire_cb = cred_acquire;

  if (args[2]->IsObject()) {
    Local<v8::Object> opts = v8::Object::Cast(*args[2]);
    Local<v8::Value>progress_cb = opts->Get(stats_onprogress_symbol);
    if(progress_cb->IsFunction()) {
      pd.cb = v8::Function::Cast(*progress_cb);
    }
  }

  status = git_clone(&out, curl, cpath, &clone_opts);

  delete [] cpath;
  if (status == GIT_OK) return (new Repository(out))->Wrapped();
  collectErr(status, err);
  V8_STHROW(composeErr(err));
}


NODE_ETYPE(Repository, "Repository") {
  V8_DEF_GET("workdir", GetWorkdir);
  V8_DEF_GET("path", GetPath);
  V8_DEF_GET("bare", IsBare);

  stats_bytes_symbol = NODE_PSYMBOL("bytes");
  stats_received_symbol = NODE_PSYMBOL("received");
  stats_indexed_symbol = NODE_PSYMBOL("indexed");
  stats_total_symbol = NODE_PSYMBOL("total");
  stats_steps_complete_symbol = NODE_PSYMBOL("steps_complete");
  stats_steps_symbol = NODE_PSYMBOL("steps");
  stats_path_symbol = NODE_PSYMBOL("path");
  stats_total_symbol = NODE_PSYMBOL("total");
  stats_path_symbol = NODE_PSYMBOL("path");
  stats_bare_symbol = NODE_PSYMBOL("bare");
  stats_onprogress_symbol = NODE_PSYMBOL("onprogress");

  Local<Function> func = templ->GetFunction();

  func->Set(Symbol("discover"), Func(Discover)->GetFunction());
  func->Set(Symbol("discoverSync"), Func(DiscoverSync)->GetFunction());

  func->Set(Symbol("open"), Func(Open)->GetFunction());
  func->Set(Symbol("openSync"), Func(OpenSync)->GetFunction());

  func->Set(Symbol("init"), Func(Init)->GetFunction());
  func->Set(Symbol("initSync"), Func(InitSync)->GetFunction());

  func->Set(Symbol("clone"), Func(Clone)->GetFunction());
  func->Set(Symbol("cloneSync"), Func(CloneSync)->GetFunction());

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

