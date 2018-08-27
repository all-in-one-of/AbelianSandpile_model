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
	fpreal GETDIST(fpreal t) { return evalFloat("dist", 0, t); }
	float GETMASS(fpreal t) { return evalFloat("mass", 0, t); }
	float GETTHRS(fpreal t) { return evalFloat("threshold", 0, t); }
	int GETNPT(fpreal t) { return evalInt("nptCount", 0, t); }

	std::vector<float> findH();
	std::vector<int> findNearestPoints(const GU_Detail*, fpreal, int);
	std::vector<float> findNptH(std::vector<int>);
	void updateData(std::vector<int>, std::vector<float>&, std::vector<float>, float, float);
	void setH(std::vector<float>);
};

#endif
