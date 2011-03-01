#include "commit.h"

Persistent<FunctionTemplate> Commit::constructor_template;

void Commit::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("Commit"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	t->PrototypeTemplate()->SetAccessor(COMMIT_TREE_SYMBOL, TreeGetter);
}

Handle<Value> Commit::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theCommit);
	//REQ_EXT_ARG(1, theRepo);

	Commit *commit = new Commit();
	commit->commit_ = (git_commit*)theCommit->Value();
	//commit->repo_ = (Repository*)theRepo->Value();

	// Setup some basic info about this commit.
	const char* oidStr = git_oid_allocfmt(git_commit_id(commit->commit_));
	args.This()->Set(String::New("id"), String::New(oidStr), ReadOnly);

	const char* message = git_commit_message(commit->commit_);
	args.This()->Set(String::New("message"), String::New(message), ReadOnly);

	const char* shortMessage = git_commit_message_short(commit->commit_);
	args.This()->Set(String::New("shortMessage"), String::New(shortMessage));

	time_t time = git_commit_time(commit->commit_);
	args.This()->Set(String::New("time"), Date::New(static_cast<double>(time)*1000));

	const git_signature *author;
	author = git_commit_author(commit->commit_);
	CREATE_PERSON_OBJ(authorObj, author);
	args.This()->Set(String::New("author"), authorObj);

	const git_signature *committer;
	committer = git_commit_committer(commit->commit_);
	CREATE_PERSON_OBJ(committerObj, committer);
	args.This()->Set(String::New("committer"), committerObj);

	commit->parentCount = git_commit_parentcount(commit->commit_);
	// Setup the parents.
	Handle<ObjectTemplate> parentObjectTemplate = ObjectTemplate::New();
	parentObjectTemplate->SetInternalFieldCount(1);
	parentObjectTemplate->SetIndexedPropertyHandler(IndexedParentGetter);

	Handle<Object> parentsObject = parentObjectTemplate->NewInstance();
	parentsObject->SetInternalField(0, args.This());
	parentsObject->Set(String::New("length"), Integer::New(commit->parentCount));

	args.This()->Set(String::New("parents"), parentsObject, ReadOnly);

	commit->Wrap(args.This());
	return args.This();
}

Handle<Value> Commit::TreeGetter(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;

	Commit *commit = ObjectWrap::Unwrap<Commit>(info.This());

	const git_tree *tree = git_commit_tree(commit->commit_);

	Tree *treeObject = commit->repository_->wrapTree(const_cast<git_tree*>(tree));
	return treeObject->handle_;
}

Handle<Value> Commit::IndexedParentGetter(uint32_t index, const AccessorInfo& info) {
	HandleScope scope;

	Commit *commit = ObjectWrap::Unwrap<Commit>(Local<Object>::Cast(info.This()->GetInternalField(0)));

	if(index >= commit->parentCount) {
		return ThrowException(Exception::Error(String::New("Parent commit index is out of bounds.")));
	}

	git_commit *parent = git_commit_parent(commit->commit_, index);
	Commit *parentObject = commit->repository_->wrapCommit(parent);
	return parentObject->handle_;
}

Commit::~Commit() {
	// TODO: don't think we ever need to free commits as they're handled by the repo, even newly created ones
	// (I think), probably need to look into this.
}
