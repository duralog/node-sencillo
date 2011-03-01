#ifndef GITTEH_INDEX_H
#define GITTEH_INDEX_H

#include "gitteh.h"
#include "index_entry.h"
#include "object_store.h"

class Index : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);
	IndexEntry *wrapIndexEntry(git_index_entry*);

protected:
	static Handle<Value> New(const Arguments&);
	static Handle<Value> EntriesGetter(uint32_t, const AccessorInfo&);

	git_index *index_;
	ObjectStore<IndexEntry, git_index_entry> entryStore_;
	unsigned int entryCount_;
};


#endif // GITTEH_INDEX_H
