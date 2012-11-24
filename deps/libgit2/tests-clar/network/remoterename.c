#include "clar_libgit2.h"
#include "config/config_helpers.h"

#include "repository.h"

static git_remote *_remote;
static git_repository *_repo;

void test_network_remoterename__initialize(void)
{
	_repo = cl_git_sandbox_init("testrepo.git");

	cl_git_pass(git_remote_load(&_remote, _repo, "test"));
}

void test_network_remoterename__cleanup(void)
{
	git_remote_free(_remote);
	_remote = NULL;

	cl_git_sandbox_cleanup();
}

static int dont_call_me_cb(const char *fetch_refspec, void *payload)
{
	GIT_UNUSED(fetch_refspec);
	GIT_UNUSED(payload);

	cl_assert(false);

	return -1;
}

void test_network_remoterename__renaming_a_remote_moves_related_configuration_section(void)
{
	assert_config_entry_existence(_repo, "remote.test.fetch", true);
	assert_config_entry_existence(_repo, "remote.just/renamed.fetch", false);

	cl_git_pass(git_remote_rename(_remote, "just/renamed", dont_call_me_cb, NULL));

	assert_config_entry_existence(_repo, "remote.test.fetch", false);
	assert_config_entry_existence(_repo, "remote.just/renamed.fetch", true);
}

void test_network_remoterename__renaming_a_remote_updates_branch_related_configuration_entries(void)
{
	assert_config_entry_value(_repo, "branch.master.remote", "test");

	cl_git_pass(git_remote_rename(_remote, "just/renamed", dont_call_me_cb, NULL));

	assert_config_entry_value(_repo, "branch.master.remote", "just/renamed");
}

void test_network_remoterename__renaming_a_remote_updates_default_fetchrefspec(void)
{
	cl_git_pass(git_remote_rename(_remote, "just/renamed", dont_call_me_cb, NULL));

	assert_config_entry_value(_repo, "remote.just/renamed.fetch", "+refs/heads/*:refs/remotes/just/renamed/*");
}

void test_network_remoterename__renaming_a_remote_without_a_fetchrefspec_doesnt_create_one(void)
{
	git_config *config;

	git_remote_free(_remote);
	cl_git_pass(git_repository_config__weakptr(&config, _repo));
	cl_git_pass(git_config_delete(config, "remote.test.fetch"));

	cl_git_pass(git_remote_load(&_remote, _repo, "test"));

	assert_config_entry_existence(_repo, "remote.test.fetch", false);

	cl_git_pass(git_remote_rename(_remote, "just/renamed", dont_call_me_cb, NULL));

	assert_config_entry_existence(_repo, "remote.just/renamed.fetch", false);
}

static int ensure_refspecs(const char* refspec_name, void *payload)
{
	int i = 0;
	bool found = false;
	const char ** exp = (const char **)payload;

	while (exp[i]) {
		if (strcmp(exp[i++], refspec_name))
			continue;

		found = true;
		break;
	}

	cl_assert(found);

	return 0;
}

void test_network_remoterename__renaming_a_remote_notifies_of_non_default_fetchrefspec(void)
{
	git_config *config;

	char *expected_refspecs[] = {
		"+refs/*:refs/*",
		NULL
	};

	git_remote_free(_remote);
	cl_git_pass(git_repository_config__weakptr(&config, _repo));
	cl_git_pass(git_config_set_string(config, "remote.test.fetch", "+refs/*:refs/*"));
	cl_git_pass(git_remote_load(&_remote, _repo, "test"));

	cl_git_pass(git_remote_rename(_remote, "just/renamed", ensure_refspecs, &expected_refspecs));

	assert_config_entry_value(_repo, "remote.just/renamed.fetch", "+refs/*:refs/*");
}

void test_network_remoterename__new_name_can_contain_dots(void)
{
	cl_git_pass(git_remote_rename(_remote, "just.renamed", dont_call_me_cb, NULL));
	cl_assert_equal_s("just.renamed", git_remote_name(_remote));
}

void test_network_remoterename__new_name_must_conform_to_reference_naming_conventions(void)
{
	cl_git_fail(git_remote_rename(_remote, "new@{name", dont_call_me_cb, NULL));
}

void test_network_remoterename__renamed_name_is_persisted(void)
{
	git_remote *renamed;
	git_repository *another_repo;

	cl_git_fail(git_remote_load(&renamed, _repo, "just/renamed"));

	cl_git_pass(git_remote_rename(_remote, "just/renamed", dont_call_me_cb, NULL));

	cl_git_pass(git_repository_open(&another_repo, "testrepo.git"));
	cl_git_pass(git_remote_load(&renamed, _repo, "just/renamed"));

	git_remote_free(renamed);
	git_repository_free(another_repo);
}

void test_network_remoterename__cannot_overwrite_an_existing_remote(void)
{
	cl_assert_equal_i(GIT_EEXISTS, git_remote_rename(_remote, "test", dont_call_me_cb, NULL));
	cl_assert_equal_i(GIT_EEXISTS, git_remote_rename(_remote, "test_with_pushurl", dont_call_me_cb, NULL));
}

void test_network_remoterename__renaming_an_inmemory_remote_persists_it(void)
{
	git_remote *remote;

	assert_config_entry_existence(_repo, "remote.durable.url", false);

	cl_git_pass(git_remote_new(&remote, _repo, NULL, "git://github.com/libgit2/durable.git", NULL));

	assert_config_entry_existence(_repo, "remote.durable.url", false);

	cl_git_pass(git_remote_rename(remote, "durable", dont_call_me_cb, NULL));

	assert_config_entry_value(_repo, "remote.durable.url", "git://github.com/libgit2/durable.git");

	git_remote_free(remote);
}

void test_network_remoterename__renaming_an_inmemory_nameless_remote_notifies_the_inability_to_update_the_fetch_refspec(void)
{
	git_remote *remote;

	char *expected_refspecs[] = {
		"+refs/heads/*:refs/remotes/volatile/*",
		NULL
	};

	assert_config_entry_existence(_repo, "remote.volatile.url", false);

	cl_git_pass(git_remote_new(
		&remote,
		_repo,
		NULL,
		"git://github.com/libgit2/volatile.git",
		"+refs/heads/*:refs/remotes/volatile/*"));

	cl_git_pass(git_remote_rename(remote, "durable", ensure_refspecs, &expected_refspecs));

	git_remote_free(remote);
}

void test_network_remoterename__renaming_a_remote_moves_the_underlying_reference(void)
{
	git_reference *underlying;

	cl_assert_equal_i(GIT_ENOTFOUND, git_reference_lookup(&underlying, _repo, "refs/remotes/just/renamed"));
	cl_git_pass(git_reference_lookup(&underlying, _repo, "refs/remotes/test/master"));
	git_reference_free(underlying);

	cl_git_pass(git_remote_rename(_remote, "just/renamed", dont_call_me_cb, NULL));

	cl_assert_equal_i(GIT_ENOTFOUND, git_reference_lookup(&underlying, _repo, "refs/remotes/test/master"));
	cl_git_pass(git_reference_lookup(&underlying, _repo, "refs/remotes/just/renamed/master"));
	git_reference_free(underlying);
}
