#include "sandDunes.h"

#include <GU/GU_Detail.h>
#include <GA/GA_Handle.h>
#include <OP/OP_AutoLockInputs.h>
#include <OP/OP_Director.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <PRM/PRM_Include.h>
#include <UT/UT_DSOVersion.h>
#include <SYS/SYS_Math.h>
#include <GEO/GEO_PointTree.h>
#include <UT/UT_ValArray.h>
#include <vector>


void
newSopOperator(OP_OperatorTable *table)
{
     table->addOperator(new OP_Operator(
        "sandunes",
        "sandDunes ",
        sandDunes::myConstructor,
        sandDunes::myTemplateList,
        1,
        1,
        0));
}


static PRM_Name iterName("iters", "Iterations");
static PRM_Name DName("dist", "Dist");
static PRM_Name massName("mass", "Mass");
static PRM_Name thrName("threshold", "Threshold");
static PRM_Name nptName("nptCount", "Near Points Count");



static PRM_Default iterDefault(1);
static PRM_Default DDefault(5);
static PRM_Default massDefault(0.05);
static PRM_Default trhDefault(0.09);
static PRM_Default nptDefault(6);



PRM_Template
sandDunes::myTemplateList[] = {
	PRM_Template(PRM_INT_J,  1, &iterName, &iterDefault, 0),
	PRM_Template(PRM_FLT_J,  1, &DName, &DDefault, 0),
	PRM_Template(PRM_FLT_J,  1, &massName, &massDefault, 0),
	PRM_Template(PRM_FLT_J,  1, &thrName, &trhDefault, 0),
	PRM_Template(PRM_INT_J,  1, &nptName, &nptDefault, 0),
    PRM_Template(),
};


OP_Node *
sandDunes::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new sandDunes(net, name, op);
}

sandDunes::sandDunes(OP_Network *net, const char *name, OP_Operator *op)
    : SOP_Node(net, name, op)
{
}

sandDunes::~sandDunes()
{
}

OP_ERROR
sandDunes::cookMySop(OP_Context &context)
{
	fpreal now = context.getTime();

    OP_AutoLockInputs inputs(this);
    if (inputs.lock(context) >= UT_ERROR_ABORT)
        return error();

    duplicateSource(0, context);

	int iterations = ITERATIONS(now);
	fpreal D = GETDIST(now);
    float mass = GETMASS(now);
	float trhs = GETTHRS(now);
	int nptc = GETNPT(now);

	for (int i = 0; i < iterations; i++)
	{
		std::vector<float> h = findH();								   // get current points h
		std::vector<int> npts = findNearestPoints(gdp, D, nptc);	   // get nearest number
		std::vector<float> nptsH = findNptH(npts);					   // get nearest h
		updateData(npts, h, nptsH, trhs, mass);						   // update ptData with new h values
		setH(h);													   // set new h

		/*
		GA_Offset ptoff;
		GA_FOR_ALL_PTOFF(gdp, ptoff)
		{
			//std::cout << "pt" << ptoff << " UPD:h: " << h[ptoff] << " npts " << npts[ptoff] << " UPD:nptH " << nptsH[ptoff] << std::endl;
		}
		*/
	}

    return error();
}

// get h
std::vector<float>
sandDunes::findH()
{
	std::vector<float> h_ls; // points h

	GA_RWHandleF h_h(gdp, GA_ATTRIB_POINT, "h");
	if (h_h.isValid())
	{
		float h;
		for (GA_Iterator it(gdp->getPointRange()); !it.atEnd(); ++it)
		{
			GA_Offset offset = *it;
			float h = h_h.get(offset);
			h_ls.push_back(h);
		}
	}

	return h_ls;
}

// get nearest number
std::vector<int>
sandDunes::findNearestPoints(const GU_Detail* pointsGDP, fpreal maxdist, int nptc)
{
	GEO_PointTreeGAOffset tree;
	tree.build(pointsGDP);
	UT_FloatArray far;

	UT_ValArray<GA_Index> nptListTemp; // near points indexes array temp
	std::vector<int> nptList; // near points indexes array output

	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(pointsGDP, ptoff)
	{
		GA_Index ptidx = pointsGDP->pointIndex(ptoff); // current point index
		UT_Vector3 origPos = pointsGDP->getPos3(ptidx); // current point position

		tree.findNearestGroupIdx(origPos, maxdist, nptc, nptListTemp, far); // find nearest points

		int nptIndex = -1;
		if (nptListTemp.size() > 1)
		{
			int randIdx = 1 + rand() % ((nptListTemp.size()) - 1); // rand index
			nptIndex = nptListTemp[randIdx]; // pick from the list of nearest h

		}
		nptList.push_back(nptIndex);
	}

	return nptList;
}

// get nearest h
std::vector<float>
sandDunes::findNptH(std::vector<int> npts)
{
	std::vector<float> h_ls; // near points h

	GA_RWHandleF h_h(gdp, GA_ATTRIB_POINT, "h");
	if (h_h.isValid())
	{
		float h = -1;
		for (int i = 0; i < npts.size(); i++)
		{
			if (npts[i] != -1)
			{
				h = h_h.get(npts[i]);
				
			}
			h_ls.push_back(h);
		}
	}
	return h_ls;
}

// update h, set new h for current points. If difference between points more than threshold
void
sandDunes::updateData(std::vector<int> npts,
					std::vector<float> &h, 
					std::vector<float> nptsH,
					float trhs, 
					float mass)
{
	GA_RWHandleF h_h(gdp, GA_ATTRIB_POINT, "h");
	std::vector<float> data(npts.size());
	int c = 0;
	for (auto &i : data)
	{
		if (npts[c] != -1)
		{
			if ((h[c] - nptsH[c]) >= trhs)
			{
				h[c] -= mass;
				h[npts[c]] += mass;
			}
		}
		c++;
	}



}

// set H for npt points
void
sandDunes::setH(std::vector<float> h)
{
	GA_RWHandleF h_h(gdp, GA_ATTRIB_POINT, "h");
	if (h_h.isValid())
	{
		for (GA_Iterator it(gdp->getPointRange()); !it.atEnd(); ++it)
		{
			GA_Offset offset = *it;
			h_h.set(offset, h[offset]);
		}
	}

}