#ifndef __sandDunes__
#define __sandDunes__

#include <SOP/SOP_Node.h>

class sandDunes : public SOP_Node
{
public:
	sandDunes(OP_Network *net, const char *name, OP_Operator *op);
	virtual ~sandDunes();

	static PRM_Template myTemplateList[];
	static OP_Node *myConstructor(OP_Network*, const char *, OP_Operator *);

protected:
	virtual OP_ERROR cookMySop(OP_Context &context);
private:
	int ITERATIONS(fpreal t) { return evalInt("iters", 0, t); }
	float GETMASS(fpreal t) { return evalFloat("mass", 0, t); }
	float GETTHRS(fpreal t) { return evalFloat("threshold", 0, t); }
};

#endif