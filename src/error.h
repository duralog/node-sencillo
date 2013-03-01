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

#ifndef SENCILLO_ERROR_H
#define	SENCILLO_ERROR_H

#include "git2.h"

#include "v8u.hpp"

namespace sencillo {

//  class GitObject : public node::ObjectWrap {
//  public:
//
//  protected:
//    git_object obj;
//  };

/*
 * Info about a LibGit2 error.
 * See `collectErr` for details.
 */
struct error_info {
  int status;
  git_error error;
};

/*
 * Check the status returned by a libgit2 function,
 * throw if error.
 */
void check(int status);

/*
 * Collect information about a status error (which is *known* to be != OK).
 * Use for composing the JS error object later with `composeErr`.
 *
 * This is needed because you can't call V8 functions outside JS threads,
 * (i.e. inside uv_queue_work) which is where the errors occur.
 */
void collectErr(int status, error_info& info);

/*
 * Create the JS error object provided a previous info
 * captured with `collectErr`.
 */
v8::Local<v8::Value> composeErr(error_info& info);

};

#endif	/* SENCILLO_ERROR_H */

