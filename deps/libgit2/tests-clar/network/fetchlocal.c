#include "clar_libgit2.h"

#include "buffer.h"
#include "path.h"
#include "remote.h"

static void transfer_cb(const git_transfer_progress *stats, void *payload)
{
	int *callcount = (int*)payload;
	GIT_UNUSED(stats);
	(*callcount)++;
}

void test_network_fetchlocal__complete(void)
{
	git_repository *repo;
	git_remote *origin;
	int callcount = 0;
	git_strarray refnames = {0};

	const char *url = cl_git_fixture_url("testrepo.git");
	cl_git_pass(git_repository_init(&repo, "foo", true));

	cl_git_pass(git_remote_add(&origin, repo, GIT_REMOTE_ORIGIN, url));
	cl_git_pass(git_remote_connect(origin, GIT_DIR_FETCH));
	cl_git_pass(git_remote_download(origin, transfer_cb, &callcount));
	cl_git_pass(git_remote_update_tips(origin));

	cl_git_pass(git_reference_list(&refnames, repo, GIT_REF_LISTALL));
	cl_assert_equal_i(18, refnames.count);
	cl_assert(callcount > 0);

	git_strarray_free(&refnames);
	git_remote_free(origin);
	git_repository_free(repo);
}

void test_network_fetchlocal__partial(void)
{
	git_repository *repo = cl_git_sandbox_init("partial-testrepo");
	git_remote *origin;
	int callcount = 0;
	git_strarray refnames = {0};
	const char *url;

	cl_git_pass(git_reference_list(&refnames, repo, GIT_REF_LISTALL));
	cl_assert_equal_i(1, refnames.count);

	url = cl_git_fixture_url("testrepo.git");
	cl_git_pass(git_remote_add(&origin, repo, GIT_REMOTE_ORIGIN, url));
	cl_git_pass(git_remote_connect(origin, GIT_DIR_FETCH));
	cl_git_pass(git_remote_download(origin, transfer_cb, &callcount));
	cl_git_pass(git_remote_update_tips(origin));

	git_strarray_free(&refnames);

	cl_git_pass(git_reference_list(&refnames, repo, GIT_REF_LISTALL));
	cl_assert_equal_i(19, refnames.count); /* 18 remote + 1 local */
	cl_assert(callcount > 0);

	git_strarray_free(&refnames);
	git_remote_free(origin);

	cl_git_sandbox_cleanup();
}
