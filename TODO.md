TODO:

- Hey, re-implement the binding!
- Document.
- Update tests.
- Move the following to the wiki.

---

TODO: detail updating submodules, better bullets format
After updating submodules:

- Remove the previous bundle from `deps/`:

  ```bash
  $ rm -r deps/libgit2
  ```

- Copy recursively the submodule folder into `deps/`:

  ```bash
  $ cp -r libgit2 deps
  ```

- Remove `.git`:

  ```bash
  $ cd deps/libgit2
  $ rm -r .git
  ```

- Stage the changes for commit using `-A`

 ```bash
 $ git add -A libgit2 deps/libgit2
 ```

- Git doesn't ignore already-tracked files, NPM does.  
  Remove all entries in `.gitignore` that point to existing files, especially CMake.
