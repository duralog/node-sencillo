#include "clar_libgit2.h"
#include "buffer.h"
#include "refspec.h"
#include "remote.h"

static git_remote *_remote;
static git_repository *_repo;
static const git_refspec *_refspec;

void test_network_remotes__initialize(void)
{
	_repo = cl_git_sandbox_init("testrepo.git");

	cl_git_pass(git_remote_load(&_remote, _repo, "test"));

	_refspec = git_remote_fetchspec(_remote);
	cl_assert(_refspec != NULL);
}

void test_network_remotes__cleanup(void)
{
	git_remote_free(_remote);
	_remote = NULL;

	cl_git_sandbox_cleanup();
}

void test_network_remotes__parsing(void)
{
	git_remote *_remote2 = NULL;

	cl_assert_equal_s(git_remote_name(_remote), "test");
	cl_assert_equal_s(git_remote_url(_remote), "git://github.com/libgit2/libgit2");
	cl_assert(git_remote_pushurl(_remote) == NULL);

	cl_assert_equal_s(git_remote__urlfordirection(_remote, GIT_DIRECTION_FETCH),
					  "git://github.com/libgit2/libgit2");
	cl_assert_equal_s(git_remote__urlfordirection(_remote, GIT_DIRECTION_PUSH),
					  "git://github.com/libgit2/libgit2");

	cl_git_pass(git_remote_load(&_remote2, _repo, "test_with_pushurl"));
	cl_assert_equal_s(git_remote_name(_remote2), "test_with_pushurl");
	cl_assert_equal_s(git_remote_url(_remote2), "git://github.com/libgit2/fetchlibgit2");
	cl_assert_equal_s(git_remote_pushurl(_remote2), "git://github.com/libgit2/pushlibgit2");

	cl_assert_equal_s(git_remote__urlfordirection(_remote2, GIT_DIRECTION_FETCH),
					  "git://github.com/libgit2/fetchlibgit2");
	cl_assert_equal_s(git_remote__urlfordirection(_remote2, GIT_DIRECTION_PUSH),
					  "git://github.com/libgit2/pushlibgit2");

	git_remote_free(_remote2);
}

void test_network_remotes__pushurl(void)
{
	cl_git_pass(git_remote_set_pushurl(_remote, "git://github.com/libgit2/notlibgit2"));
	cl_assert_equal_s(git_remote_pushurl(_remote), "git://github.com/libgit2/notlibgit2");

	cl_git_pass(git_remote_set_pushurl(_remote, NULL));
	cl_assert(git_remote_pushurl(_remote) == NULL);
}

void test_network_remotes__parsing_ssh_remote(void)
{
	cl_assert( git_remote_valid_url("git@github.com:libgit2/libgit2.git") );
}

void test_network_remotes__parsing_local_path_fails_if_path_not_found(void)
{
	cl_assert( !git_remote_valid_url("/home/git/repos/libgit2.git") );
}

void test_network_remotes__supported_transport_methods_are_supported(void)
{
	cl_assert( git_remote_supported_url("git://github.com/libgit2/libgit2") );
}

void test_network_remotes__unsupported_transport_methods_are_unsupported(void)
{
	cl_assert( !git_remote_supported_url("git@github.com:libgit2/libgit2.git") );
}

void test_network_remotes__refspec_parsing(void)
{
	cl_assert_equal_s(git_refspec_src(_refspec), "refs/heads/*");
	cl_assert_equal_s(git_refspec_dst(_refspec), "refs/remotes/test/*");
}

void test_network_remotes__set_fetchspec(void)
{
	cl_git_pass(git_remote_set_fetchspec(_remote, "refs/*:refs/*"));
	_refspec = git_remote_fetchspec(_remote);
	cl_assert_equal_s(git_refspec_src(_refspec), "refs/*");
	cl_assert_equal_s(git_refspec_dst(_refspec), "refs/*");
}

void test_network_remotes__set_pushspec(void)
{
	cl_git_pass(git_remote_set_pushspec(_remote, "refs/*:refs/*"));
	_refspec = git_remote_pushspec(_remote);
	cl_assert_equal_s(git_refspec_src(_refspec), "refs/*");
	cl_assert_equal_s(git_refspec_dst(_refspec), "refs/*");
}

void test_network_remotes__save(void)
{
	git_remote_free(_remote);
	_remote = NULL;

	/* Set up the remote and save it to config */
	cl_git_pass(git_remote_new(&_remote, _repo, "upstream", "git://github.com/libgit2/libgit2", NULL));
	cl_git_pass(git_remote_set_fetchspec(_remote, "refs/heads/*:refs/remotes/upstream/*"));
	cl_git_pass(git_remote_set_pushspec(_remote, "refs/heads/*:refs/heads/*"));
	cl_git_pass(git_remote_set_pushurl(_remote, "git://github.com/libgit2/libgit2_push"));
	cl_git_pass(git_remote_save(_remote));
	git_remote_free(_remote);
	_remote = NULL;

	/* Load it from config and make sure everything matches */
	cl_git_pass(git_remote_load(&_remote, _repo, "upstream"));

	_refspec = git_remote_fetchspec(_remote);
	cl_assert(_refspec != NULL);
	cl_assert_equal_s(git_refspec_src(_refspec), "refs/heads/*");
	cl_assert_equal_s(git_refspec_dst(_refspec), "refs/remotes/upstream/*");
	cl_assert(git_refspec_force(_refspec) == 0);

	_refspec = git_remote_pushspec(_remote);
	cl_assert(_refspec != NULL);
	cl_assert_equal_s(git_refspec_src(_refspec), "refs/heads/*");
	cl_assert_equal_s(git_refspec_dst(_refspec), "refs/heads/*");

	cl_assert_equal_s(git_remote_url(_remote), "git://github.com/libgit2/libgit2");
	cl_assert_equal_s(git_remote_pushurl(_remote), "git://github.com/libgit2/libgit2_push");

	/* remove the pushurl again and see if we can save that too */
	cl_git_pass(git_remote_set_pushurl(_remote, NULL));
	cl_git_pass(git_remote_save(_remote));
	git_remote_free(_remote);
	_remote = NULL;

	cl_git_pass(git_remote_load(&_remote, _repo, "upstream"));
	cl_assert(git_remote_pushurl(_remote) == NULL);
}

void test_network_remotes__fnmatch(void)
{
	cl_assert(git_refspec_src_matches(_refspec, "refs/heads/master"));
	cl_assert(git_refspec_src_matches(_refspec, "refs/heads/multi/level/branch"));
}

void test_network_remotes__transform(void)
{
	char ref[1024];

	memset(ref, 0x0, sizeof(ref));
	cl_git_pass(git_refspec_transform(ref, sizeof(ref), _refspec, "refs/heads/master"));
	cl_assert_equal_s(ref, "refs/remotes/test/master");
}

void test_network_remotes__transform_r(void)
{
	git_buf buf = GIT_BUF_INIT;

	cl_git_pass(git_refspec_transform_r(&buf,  _refspec, "refs/heads/master"));
	cl_assert_equal_s(git_buf_cstr(&buf), "refs/remotes/test/master");
	git_buf_free(&buf);
}

void test_network_remotes__missing_refspecs(void)
{
	git_config *cfg;

	git_remote_free(_remote);
	_remote = NULL;

	cl_git_pass(git_repository_config(&cfg, _repo));
	cl_git_pass(git_config_set_string(cfg, "remote.specless.url", "http://example.com"));
	cl_git_pass(git_remote_load(&_remote, _repo, "specless"));

	git_config_free(cfg);
}

void test_network_remotes__list(void)
{
	git_strarray list;
	git_config *cfg;

	cl_git_pass(git_remote_list(&list, _repo));
	cl_assert(list.count == 4);
	git_strarray_free(&list);

	cl_git_pass(git_repository_config(&cfg, _repo));
	cl_git_pass(git_config_set_string(cfg, "remote.specless.url", "http://example.com"));
	cl_git_pass(git_remote_list(&list, _repo));
	cl_assert(list.count == 5);
	git_strarray_free(&list);

	git_config_free(cfg);
}

void test_network_remotes__loading_a_missing_remote_returns_ENOTFOUND(void)
{
	git_remote_free(_remote);
	_remote = NULL;

	cl_assert_equal_i(GIT_ENOTFOUND, git_remote_load(&_remote, _repo, "just-left-few-minutes-ago"));
}

void test_network_remotes__loading_with_an_invalid_name_returns_EINVALIDSPEC(void)
{
	git_remote_free(_remote);
	_remote = NULL;

	cl_assert_equal_i(GIT_EINVALIDSPEC, git_remote_load(&_remote, _repo, "Inv@{id"));
}

/*
 * $ git remote add addtest http://github.com/libgit2/libgit2
 *
 * $ cat .git/config
 * [...]
 * [remote "addtest"]
 *         url = http://github.com/libgit2/libgit2
 *         fetch = +refs/heads/\*:refs/remotes/addtest/\*
 */
void test_network_remotes__add(void)
{
	git_remote_free(_remote);
	_remote = NULL;

	cl_git_pass(git_remote_add(&_remote, _repo, "addtest", "http://github.com/libgit2/libgit2"));

	git_remote_free(_remote);
	_remote = NULL;

	cl_git_pass(git_remote_load(&_remote, _repo, "addtest"));
	_refspec = git_remote_fetchspec(_remote);
	cl_assert(!strcmp(git_refspec_src(_refspec), "refs/heads/*"));
	cl_assert(git_refspec_force(_refspec) == 1);
	cl_assert(!strcmp(git_refspec_dst(_refspec), "refs/remotes/addtest/*"));
	cl_assert_equal_s(git_remote_url(_remote), "http://github.com/libgit2/libgit2");
}

void test_network_remotes__cannot_add_a_nameless_remote(void)
{
	git_remote *remote;

	cl_assert_equal_i(
		GIT_EINVALIDSPEC,
		git_remote_add(&remote, _repo, NULL, "git://github.com/libgit2/libgit2"));
}

void test_network_remotes__cannot_save_a_nameless_remote(void)
{
	git_remote *remote;

	cl_git_pass(git_remote_new(&remote, _repo, NULL, "git://github.com/libgit2/libgit2", NULL));

	cl_assert_equal_i(GIT_EINVALIDSPEC, git_remote_save(remote));
	git_remote_free(remote);
}

void test_network_remotes__cannot_add_a_remote_with_an_invalid_name(void)
{
	git_remote *remote = NULL;

	cl_assert_equal_i(
		GIT_EINVALIDSPEC,
		git_remote_add(&remote, _repo, "Inv@{id", "git://github.com/libgit2/libgit2"));
	cl_assert_equal_p(remote, NULL);

	cl_assert_equal_i(
		GIT_EINVALIDSPEC,
		git_remote_add(&remote, _repo, "", "git://github.com/libgit2/libgit2"));
	cl_assert_equal_p(remote, NULL);
}

void test_network_remotes__cannot_initialize_a_remote_with_an_invalid_name(void)
{
	git_remote *remote = NULL;

	cl_assert_equal_i(
		GIT_EINVALIDSPEC,
		git_remote_new(&remote, _repo, "Inv@{id", "git://github.com/libgit2/libgit2", NULL));
	cl_assert_equal_p(remote, NULL);

	cl_assert_equal_i(
		GIT_EINVALIDSPEC,
		git_remote_new(&remote, _repo, "", "git://github.com/libgit2/libgit2", NULL));
	cl_assert_equal_p(remote, NULL);
}

void test_network_remotes__tagopt(void)
{
	const char *opt;
	git_config *cfg;

	cl_git_pass(git_repository_config(&cfg, _repo));

	git_remote_set_autotag(_remote, GIT_REMOTE_DOWNLOAD_TAGS_ALL);
	cl_git_pass(git_remote_save(_remote));
	cl_git_pass(git_config_get_string(&opt, cfg, "remote.test.tagopt"));
	cl_assert(!strcmp(opt, "--tags"));

	git_remote_set_autotag(_remote, GIT_REMOTE_DOWNLOAD_TAGS_NONE);
	cl_git_pass(git_remote_save(_remote));
	cl_git_pass(git_config_get_string(&opt, cfg, "remote.test.tagopt"));
	cl_assert(!strcmp(opt, "--no-tags"));

	git_remote_set_autotag(_remote, GIT_REMOTE_DOWNLOAD_TAGS_AUTO);
	cl_git_pass(git_remote_save(_remote));
	cl_assert(git_config_get_string(&opt, cfg, "remote.test.tagopt") == GIT_ENOTFOUND);

	git_config_free(cfg);
}

void test_network_remotes__cannot_load_with_an_empty_url(void)
{
	git_remote *remote = NULL;

	cl_git_fail(git_remote_load(&remote, _repo, "empty-remote-url"));
	cl_assert(giterr_last()->klass == GITERR_INVALID);
	cl_assert_equal_p(remote, NULL);
}

void test_network_remotes__check_structure_version(void)
{
	git_transport transport = GIT_TRANSPORT_INIT;
	const git_error *err;

	git_remote_free(_remote);
	_remote = NULL;
	cl_git_pass(git_remote_new(&_remote, _repo, NULL, "test-protocol://localhost", NULL));

	transport.version = 0;
	cl_git_fail(git_remote_set_transport(_remote, &transport));
	err = giterr_last();
	cl_assert_equal_i(GITERR_INVALID, err->klass);

	giterr_clear();
	transport.version = 1024;
	cl_git_fail(git_remote_set_transport(_remote, &transport));
	err = giterr_last();
	cl_assert_equal_i(GITERR_INVALID, err->klass);
}

void test_network_remotes__dangling(void)
{
	git_remote_free(_remote);
	_remote = NULL;

	cl_git_pass(git_remote_new(&_remote, NULL, "upstream", "git://github.com/libgit2/libgit2", NULL));

	cl_git_pass(git_remote_rename(_remote, "newname", NULL, NULL));
	cl_assert_equal_s(git_remote_name(_remote), "newname");

	cl_git_fail(git_remote_save(_remote));
	cl_git_fail(git_remote_update_tips(_remote));

	cl_git_pass(git_remote_set_repository(_remote, _repo));
	cl_git_pass(git_remote_save(_remote));
}
