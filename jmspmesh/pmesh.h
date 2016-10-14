// ******************************
// pmesh.h
//
// Progressive mesh class.
// This mesh can be simplified by
// removing edges & triangles, while
// retaining the same shape.
//
// Jeff Somers
// Copyright (c) 2002
//
// jsomers@alumni.williams.edu
// March 27, 2002
// ******************************

#ifndef __PMesh_h
#define __PMesh_h



#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#pragma warning(disable:4710) // function not inlined
#pragma warning(disable:4702) // unreachable code
#pragma warning(disable:4514) // unreferenced inline function has been removed
#pragma warning(disable:4786) /* disable "identifier was truncated to '255' characters in the browser information" warning in Visual C++ 6*/
#endif

#define WIN32_LEAN_AND_MEAN
#include "..\stdafx.h"
#include "..\Option.h"
#include <vector>
#include <list>
#include "vertex.h"
#include "triangle.h"
#include "jmsmesh.h"
#include "..\SparseMatrix.h"
#include "..\CSMatrix.h"
#include "..\MultigridContractionSolver.h"
#include "Matrix.h"
#include <afx.h>
using namespace std;


// The edge collapse structure.  The "from vertex" will
// be collapsed to the "to vertex."  This may flatten some
// triangles, which will be removed, and will affect those
// triangles which contain the "from vertex".  Those triangles
// will be updated with the new vertex.
struct EdgeCollapse
{
	int _vfrom;
	int _vto;
	set<int> _trisRemoved;
	set<int> _trisAffected;

	bool _center;


	// Used for debugging
	void dumpEdgeCollapse()
	{
		std::cout << "**** Edge Collapse Dump ****" << std::endl;

		std::cout << "\tFrom Vert# " << _vfrom << " to Vert# " << _vto << std::endl;
		cout << "\tTris removed:";
		set<int>::iterator pos;
		for (pos = _trisRemoved.begin(); pos != _trisRemoved.end(); ++pos) 
		{
			std::cout << " " << *pos;
		}
		cout << std::endl << "\tTris affected:";
		for (pos = _trisAffected.begin(); pos != _trisAffected.end(); ++pos) 
		{
			std::cout << " " << *pos;
		}
		std::cout  << std::endl << "**** End of Edge Collapse Dump ****" << std::endl;
	}
};

// This is a "pointer" to a vertex in a given mesh
struct vertexPtr
{
	jmsMesh* _meshptr;
	int _index; // ptr to vertex position in mesh
	double _center;

	bool operator<(const vertexPtr& vp) const 
	{
		return (_meshptr->getVertex(_index) < vp._meshptr->getVertex(vp._index));
	}
};


typedef multiset<vertexPtr, less<vertexPtr> > vertexPtrSet;

// �Ǽܵ����ݽṹ [7/16/2011 Han Honglei]
struct VertexRecord
{
	bool center;
	set<int> collapseFrom;			// ����������Ǽܵ��ԭʼģ�Ͷ�����
	int colorIndex;
	double nodeSize;
	vec3 pos;						// �Ǽܵ�λ��
	int pqIndex;
	double radius;
	int vIndex;						// ���Ǽܵ���ԭʼģ���еĶ�����
};

// LOD���ƵĽṹ�壬������LOD����ѡ�� [9/28/2011 Han Honglei]
struct LODController
{
	float startDist, vanishDist;		// �ֱ��ʾ��ʼ�򻯺�ֻʣһ����Ƭ�ľ��룬һ��ģ���п����ǹ�ϸ�ڵģ���startDistΪ0
	int startTri, startVert;			// �ֱ��ʾ���ϸ�ڵ������κͶ������
	float collapseVertPerDist;			// ÿ����һ����λ������Ҫִ�ж���ϲ��Ĵ���
};
// Progressive jmsMesh class.  This class will calculate and keep track
// of which vertices and triangles should be removed from/added to the
// mesh as it's simplified (or restored).
class PMesh
{
public:
	// Type of progress mesh algorithm
	enum EdgeCost {SHORTEST, MELAX, QUADRIC, QUADRICTRI, QUADRIC_SEGMENT, HAN, BLUR, MAX_EDGECOST};	// QUADRIC_SEGMENT��ʾ��ӷָ��Ȩ��
	enum DrawMeshType {NONE, SKEL_MAP, COLL_DIST, SEGMENT};

	EdgeCost _cost; // Type of progressive mesh algorithm
	
	PMesh(jmsMesh* mesh, EdgeCost ec);
	PMesh(jmsMesh* mesh, EdgeCost ec, list<int> selList);

	// New collapse function, calc collapse realtime [9/20/2011 Han Honglei]
	bool collapseEdgeRealtime();
	// Collapse one vertex to another.
	bool collapseEdge();
	//bool collapseEdgeOnSel();
	
	// One vertex will be split into two vertices -- this
	// is the opposite of a collapse
	bool splitVertex();

	// number of edge collapses
	int numCollapses() {return _edgeCollList.size();}
	int numEdgeCollapses() {return _edgeCollList.size();}

	// number of triangles, and visible triangles in mesh
	int numTris() {return _newmesh.getNumTriangles();}
	int numVisTris() {return _nVisTriangles;}
	int numVertex() {return _newmesh.getNumVerts();}


	// Create the list of the edge collapses used
	// to simplify the mesh.
	void createEdgeCollapseList();


	bool getTri(int i, triangle& t) {
		t = _newmesh.getTri(i);
		return true;
	}
	triangle&  getTri(int i) {
		return _newmesh.getTri(i);
	}

	// Return a short text description of the current Edge Cost method
	char* getEdgeCostDesc();

	//  [8/18/2010 admin]
	bool SaveFile(CArchive& ar);
	bool LoadSegFiles(char *fileName);
	bool LoadBlurFiles(char *fileName);	// ����ģ�Ͷ����˶�ģ���ľ�����Ϣ  [7/27/2011 Han Honglei]

	~PMesh();

	FILE* outputFile;							// ���ļ��������������Ϣ [7/14/2011 Han Honglei]

	LONGLONG StartTime();						// ���ڼ�ʱ�ĺ���
	double GetElapseTime(LONGLONG startTime);
	void Normalize() {_newmesh.Normalize();}
	bool AdjustLOD(float fDist);
//////////////////////////////////////////////////////////////////////////
	// �Ǽܳ�ȡ��س�Ա
	int collapseIterNum;						// ģ�������Ĵ���
	bool isCollapsing;							// ��ǰ�����Ƿ���ģ�������׶�
	bool bCollapsed;							// ģ���Ƿ����������
	bool isSkeling;								// ��ǰ�����Ƿ��ڹǼܵ��ȡ�׶Σ���ģ��������ϣ����������ģ�ͽ��м򻯻�ùǼܵ㣩
	bool bSimplified;							// ģ���Ƿ�����˼򻯣����Ƿ��Ѿ���ȡ�˹Ǽܵ�

	vector<VertexRecord> simplifiedVertexRec;	// �Ǽܵ��б�
	vector<set<int>> simplifiedVertexNeighbour;	// �Ǽܵ�֮����ڽӹ�ϵ

	bool ChangeColl(bool bNext);				// ��ģ���л�Ϊÿ�ε���������Ľ��
	void Simplification(CSkeOption *opt);		// ���������ģ�ͽ��м򻯣��õ��Ǽܵ�
	void DrawSimplifiedVertices();				// ���ƾ����򻯺�õ��ĹǼ���Ϣ
	int DrawOriginalMesh(DrawMeshType drawType = NONE, bool bSmooth = true);	// ����ԭʼģ��
	void CalcCollapseDist();					// �������ģ��ÿ������ÿ�������ƶ��ľ��룬�Լ�ģ���ж����������С�ƶ�����
	bool HasSegment() {return _newmesh._bHasSegment;}// ģ���Ƿ��зָ���Ϣ
	bool RestoreMesh();							// ��ģ�ͻָ�Ϊԭ��������
	bool GeometryCollapse(CSkeOption *opt);		// ��ģ�ͽ��еݹ�ʽ���۵���ֱ�����������һ���̶ȣ�����false
//////////////////////////////////////////////////////////////////////////
	// calc motion blur weights of model's vertexes
	bool CalcBlurWeights(/*vec3 eye,vec3 gaze,vec3 up*/);
	void meshTransform( const Matrix & mx );
private:
	// If motion blur weights of vertexes are loaded from file(.blr) [11/2/2011 Han]
	bool bIsBlurWeightsLoaded;
//////////////////////////////////////////////////////////////////////////
	// ���ڳ�ȡ�Ǽܵĳ�Ա����
	 int* adjSegmentVertex;						// δʹ��
	 double* lap[3];
	 double* lapWeight;							// ����������˹����������
	 double* posWeight;							// �����������б�����״���Ե�����
	 MultigridContractionSolver *multigridSolver;// �����
	 double* oldAreaRatio;						// ������������
	 double* originalFaceArea;					// ԭ����ģ�����
	 double originalArea;						// ����ģ�ͱ����ԭʼ���
	 void* solver;								// �����
	 void* symbolicSolver;						// 	 
	 vector<vector<vec3>> allCollPos;			// ��������ÿ��������Ľ�� [7/6/2011 Han Honglei]
	 int currCollPos;							// ��ǰ�������̶�
	 vector<vector<double>> allCollapseDist;	// ����ÿ������ʱ��ģ�Ͷ�����ƶ�����
	 vector<double> minCollapseDist;			// ����ÿ�����������У�ģ���ж����ƶ�����С����
	 vector<double> maxCollapseDist;			// ������
	 vector<set<int>> allCollapseFrom;			// ����������������б��۵���Ľ�� [7/10/2011 Han Honglei]

	 void InitCollapse(const CSkeOption *opt);	// �ڵ�һ������֮ǰ����һЩ��ʼ������
	 void InitSkelValues();						// ����๹���ʱ���ʼ��һЩ��Ա����			
	 SparseMatrix BuildMatrixA(const CSkeOption *otp);// �������������ľ���
	 void Dispose();							// �Զ�̬�����ı�������ɾ��
	 // ���ڹǼܵ���ȡ�ĺ���
	 void calcShapeMatrices(jmsMesh &mesh);		// �������µķ�������ÿ���������״����������Q
	 double quadricCollapseCostSkel(jmsMesh& m, vertex& v, CSkeOption* opt);// �����۵�����
	 void calcEdgeCollapseCostsSkel(vertexPtrSet &vertSet, vector<vertexPtrSet::iterator> &vertSetVec, 
		 int nVerts, jmsMesh &mesh, EdgeCost &cost, CSkeOption* opt);// ����ÿ��������۵����ۣ������մ������򣬽��������vertSet��
	 void buildEdgeCollapseListSkel(jmsMesh &mesh, const EdgeCost &cost, 
		 list<EdgeCollapse> &edgeCollList,
		 vertexPtrSet &vertSet, 
		 vector<vertexPtrSet::iterator> &vertSetVec, CSkeOption* opt);// �����۵������𲽽��б��۵�
	 void insureEdgeCollapseValidSkel(EdgeCollapse &ec, vertex &vc, jmsMesh &mesh,  // ȷ�����۵��Ϸ�
		 const EdgeCost &cost, bool &bBadVertex, CSkeOption *opt);	
	 void updateTrianglesSkel(EdgeCollapse &ec, vertex &vc, set<int> &affectedVerts, jmsMesh &mesh);
	 void MatrixXMatrix(double m1[4][4], double m2[4][4], double result[4][4]);// ���������������
	 void MatrixXVector( double m[4][4],double v[4], double result[4]);		   // ���������������
	 // ����ı�����ԭ�����еı�������ʱ��ʹ�û����Ѿ����滻
	 //double* originalVertexPos;
	 //jmsMesh mesh;
	 //CSkeOption opt;
	 //object myDisplayLock = new object();
	 //int remainingVertexCount;
	 //int FaceCount;
	 //int* faceIndex;
	 //double* collapsedLength;
	 //double* collapsedVertexPos;				// ����������ʹ��������������
	 //CCSMatrix ccsA;
	 //CCSMatrix ccsATA;
	 //bool displayIntermediateMesh;
	 //bool displayNodeSphere;
	 //bool displayOriginalMesh;
	 //bool displaySimplifiedMesh;
	 //int displaySimplifiedMeshIndex;
	 //bool displaySkeleton;
	 //VertexRecord rootNode;
	 //List<VertexRecord> simplifiedVertexRec;
	 //float skeletonNodeSize;
	 //VertexRecord[] vRec;
//////////////////////////////////////////////////////////////////////////
	jmsMesh* _mesh; // original mesh - not changed
	jmsMesh _newmesh; // we change this one

	bool _bSel;		// ��ǰ���Ƿ��ڡ�ѡ����Ƭ�򻯡�ģʽ

	LODController _lod;	// ��ģ�͵�lod������п���

	list<int> _selList;

	// This is a set of vertex pointers, ordered by edge collapse cost.
	vertexPtrSet _vertSet;
	vector<vertexPtrSet::iterator> _vertSetVec;

	list<EdgeCollapse> _edgeCollList; // list of edge collapses
	list<EdgeCollapse>::iterator _edgeCollapseIter;

	// functions used to calculate edge collapse costs.  Different
	// methods can be used, depending on user preference.
	double shortEdgeCollapseCost(jmsMesh& m, vertex& v);
	double melaxCollapseCost(jmsMesh& m, vertex& v);
	double quadricCollapseCost(jmsMesh& m, vertex& v);

	int _nVisTriangles; // # of triangles, after we collapse edges

	// Used in the QEM edge collapse methods.
	void calcAllQMatrices(jmsMesh& mesh, bool bUseTriArea); // used for quadric method
	double calcQuadricError(double Qsum[4][4], vertex& v, double triArea); // used for quadric method

	enum {BOUNDARY_WEIGHT = 1000}; // used to weight border edges so they don't collapse
	void applyBorderPenalties(set<border> &borderSet, jmsMesh &mesh);

	PMesh(const PMesh&); // don't allow copy ctor -- too expensive
	PMesh& operator=(const PMesh&); // don't allow assignment op.
	bool operator==(const PMesh&); // don't allow op==

#ifndef NDEBUG
	// used in debugging
	void assertEveryVertActive(int nVerts, int nTri, jmsMesh &mesh);
#endif
	// helper function for edge collapse costs
	void calcEdgeCollapseCosts(vertexPtrSet &vertSet, vector<vertexPtrSet::iterator> &vertSetVec, 
								  int nVerts, jmsMesh &mesh, EdgeCost &cost);

	// Calculate the QEM matrices used to computer edge
	// collapse costs.
	void calcQuadricMatrices(EdgeCost &cost, jmsMesh &mesh);

	// We can't collapse Vertex1 to Vertex2 if Vertex2 is invalid.
	// This can happen if Vertex2 was previously collapsed to a
	// separate vertex.
	void insureEdgeCollapseValid(EdgeCollapse &ec, vertex &vc, jmsMesh &mesh, 
									const EdgeCost &cost, bool &bBadVertex);

	// Calculate the QEM for the "to vertex" in the edge collapse.
	void setToVertexQuadric(vertex &to, vertex &from, const EdgeCost &cost);

	// At this point, we have an edge collapse.  We're collapsing the "from vertex"
	// to the "to vertex."  For all the surrounding triangles which use this edge, 
	// update "from vertex" to the "to vertex".  Also keep track of the vertices
	// in the surrounding triangles. 
	void updateTriangles(EdgeCollapse &ec, vertex &vc, set<int> &affectedVerts, jmsMesh &mesh);


	// These affected vertices are not in the current collapse, 
	// but are in the triangles which share the collapsed edge.
	void updateAffectedVertNeighbors(vertex &vert, const EdgeCollapse &ec, 
		set<int> &affectedVerts);

	// Reset the edge collapse costs of vertices which were
	// affected by a previous edge collapse.
	void resetAffectedVertCosts(const EdgeCost &cost, jmsMesh &newmesh, vertex &vert);

	// If this vertex has no active triangles (i.e. triangles which have
	// not been removed from the mesh) then set it to inactive.
	void removeVertIfNecessary(vertex &vert, vertexPtrSet &vertSet, 
								  vector<vertexPtrSet::iterator> &vertSetVec, 
								  jmsMesh &mesh, const EdgeCost &cost, 
									set<int> &affectedQuadricVerts);

	// Update the vertices affected by the most recent edge collapse
	void updateAffectedVerts(jmsMesh &_newmesh, vector<vertexPtrSet::iterator> &vertSetVec, 
							vertexPtrSet &vertSet, const EdgeCollapse &ec, 
							set<int> &affectedVerts, const EdgeCost &cost, 
							set<int> &affectedQuadricVerts);

	// Recalculate the QEM matrices (yeah, that's redundant) if we're
	// using the Quadrics to calculate edge collapse costs.
	void recalcQuadricCollapseCosts(set<int> &affectedQuadricVerts, 
								   jmsMesh &mesh, const EdgeCost &cost);

	// Calculate the list of edge collapses.  Each edge collapse
	// consists of two vertices:  a "from vertex" and a "to vertex".
	// The "from vertex" is collapsed to the "to vertex".  The
	// "from vertex" is removed from the mesh.
	void buildEdgeCollapseList(jmsMesh &mesh, const EdgeCost &cost, 
							  list<EdgeCollapse> &_edgeCollList,
								vertexPtrSet &vertSet, 
								vector<vertexPtrSet::iterator> &vertSetVec);

	// Helper function for melaxCollapseCost().  This function
	// will loop through all the triangles to which this vertex
	// belongs.
	void calcMelaxMaxValue(jmsMesh &mesh, set<int> &adjfaces, 
							  vertex &v, set<int> &tneighbors,
								float &retmaxValue, 
								bool &bMaxValueFound);
public:
	void InitOpt(CSkeOption *opt);
};

#endif // __PMesh_h
