#include "clar_libgit2.h"

#include "git2/clone.h"
#include "repository.h"

#define LIVE_REPO_URL "git://github.com/libgit2/TestGitRepository"

static git_repository *g_repo;

void test_clone_nonetwork__initialize(void)
{
	g_repo = NULL;
}

static void cleanup_repository(void *path)
{
	if (g_repo) {
		git_repository_free(g_repo);
		g_repo = NULL;
	}

	cl_fixture_cleanup((const char *)path);
}

void test_clone_nonetwork__bad_url(void)
{
	/* Clone should clean up the mess if the URL isn't a git repository */
	cl_git_fail(git_clone(&g_repo, "not_a_repo", "./foo", NULL, NULL, NULL));
	cl_assert(!git_path_exists("./foo"));
	cl_git_fail(git_clone_bare(&g_repo, "not_a_repo", "./foo.git", NULL, NULL));
	cl_assert(!git_path_exists("./foo.git"));
}

void test_clone_nonetwork__local(void)
{
	const char *src = cl_git_fixture_url("testrepo.git");
	cl_set_cleanup(&cleanup_repository, "./local");

	cl_git_pass(git_clone(&g_repo, src, "./local", NULL, NULL, NULL));
}

void test_clone_nonetwork__local_bare(void)
{
	const char *src = cl_git_fixture_url("testrepo.git");
	cl_set_cleanup(&cleanup_repository, "./local.git");

	cl_git_pass(git_clone_bare(&g_repo, src, "./local.git", NULL, NULL));
}

void test_clone_nonetwork__fail_when_the_target_is_a_file(void)
{
	cl_set_cleanup(&cleanup_repository, "./foo");

	cl_git_mkfile("./foo", "Bar!");
	cl_git_fail(git_clone(&g_repo, LIVE_REPO_URL, "./foo", NULL, NULL, NULL));
}

void test_clone_nonetwork__fail_with_already_existing_but_non_empty_directory(void)
{
	cl_set_cleanup(&cleanup_repository, "./foo");

	p_mkdir("./foo", GIT_DIR_MODE);
	cl_git_mkfile("./foo/bar", "Baz!");
	cl_git_fail(git_clone(&g_repo, LIVE_REPO_URL, "./foo", NULL, NULL, NULL));
}
