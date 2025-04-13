#pragma once
#include "OpenGLwin.h"
#include<map>
#include<vector>

class Graph {
private:
	vector<map<u32, i32>> graph;
public:
	Graph(u32 size);
	void Init(u32 size);
	void AddNode();
	void AddEdge(u32 from, u32 to, i32 cost);
	void IncreaseEdgeCost(u32 from, u32 to, i32 increaseCost);
	vector<map<u32, i32>>& GetGraph();
};

struct MetisGraph {
	idx_t nvtxs;
	vector<idx_t> xadj;
	vector<idx_t> adjncy;	//ѹ��ͼ��ʾ
	vector<idx_t> adjwgt; //��Ȩ��
};

class Partitioner {
private:
	u32 BisectGraph(MetisGraph* graphData, MetisGraph* childGraph[2], u32 start, u32 end);
	void RecursiveBisectGraph(MetisGraph* graphData, u32 start, u32 end);

	vector<u32> nodeID;	//�ڵ㰴���ֱ������
	vector<pair<u32, u32>>ranges; //�ֿ��������Χ����Χ������ͬ����
	vector<u32>sortTo;
public:
	u32 minSize;
	u32 maxSize;
	Partitioner(u32 numNode);
	void Init(u32 numNode);
	void Partition(Graph*& graph, u32 minSize, u32 maxSize);
	vector<u32> &GetNodeID();
	vector<pair<u32, u32>> &GetRanges();
	vector<u32> &GetSortTo();
};
