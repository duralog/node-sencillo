#ifndef GITTEH_REF_H
#define GITTEH_REF_H

#include "gitteh.h"

namespace gitteh {

class Repository;

class Reference : public ObjectWrap {
public:
	Reference();

	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);

	Repository *repository_;

protected:
	static Handle<Value> New(const Arguments&);

	static Handle<Value> Rename(const Arguments&);
	static Handle<Value> Delete(const Arguments&);
	static Handle<Value> Resolve(const Arguments&);
	static Handle<Value> SetTarget(const Arguments&);

	git_reference *ref_;
	git_rtype type_;
	bool deleted_;
};

} // namespace gitteh

#endif //GITTEH_REF_H
