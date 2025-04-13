#include "Partitioner.h"


Graph::Graph(u32 size)
{
	graph = vector<map<u32, i32>>(size);
}

void Graph::Init(u32 size)
{
	graph.resize(size);
}

void Graph::AddNode()
{
	graph.push_back({});
}

void Graph::AddEdge(u32 from, u32 to, i32 cost)
{
	graph[from][to] = cost;
}

void Graph::IncreaseEdgeCost(u32 from, u32 to, i32 increaseCost)
{
	graph[from][to] += increaseCost;
}

vector<map<u32, i32>>& Graph::GetGraph()
{
	return graph;
}


//转换为CSR方式存储
MetisGraph* ToMetisData(Graph* graph) {
	MetisGraph* metisGraph = new MetisGraph;
	metisGraph->nvtxs = graph->GetGraph().size();
	for (auto& map : graph->GetGraph()) {
		metisGraph->xadj.push_back(metisGraph->adjncy.size());
		for (auto [to, cost] : map) {
			metisGraph->adjncy.push_back(to);
			metisGraph->adjwgt.push_back(cost);
		}
	}
	metisGraph->xadj.push_back(metisGraph->adjncy.size());
	return metisGraph;
}

vector<u32>& Partitioner::GetNodeID()
{
	return nodeID;
}

vector<pair<u32, u32>>& Partitioner::GetRanges()
{
	return ranges;
}

vector<u32>& Partitioner::GetSortTo()
{
	return sortTo;
}



u32 Partitioner::BisectGraph(
	MetisGraph* graphData, 
	MetisGraph* childGraph[2], 
	u32 start, 
	u32 end
)
{
	assert(end - start == graphData->nvtxs);

	if (graphData->nvtxs <= maxSize) {
		ranges.push_back({ start, end });
		return end;
	}

	const u32 expPartSize = (minSize + maxSize) / 2;
	const u32 expNumParts = max(2u, (graphData->nvtxs + expPartSize - 1) / expPartSize);

	vector<idx_t> swapTo(graphData->nvtxs);
	vector<idx_t> part(graphData->nvtxs);

	idx_t nodeCut = 0;
	idx_t nodeWeight = 1;
	idx_t nodePart = 2;

	real_t partWeight[] = {
		float(expNumParts >> 1) / expNumParts,
		1.0f - float(expNumParts >> 1) / expNumParts
	};

	i32 res = METIS_PartGraphRecursive(
		&graphData->nvtxs,
		&nodeWeight,
		graphData->xadj.data(),
		graphData->adjncy.data(),
		nullptr,
		nullptr,
		graphData->adjwgt.data(),
		&nodePart,
		partWeight,
		nullptr,
		nullptr,
		&nodeCut,
		part.data()
	);

	assert(res == METIS_OK);

	i32 l = 0;
	i32 r = graphData->nvtxs - 1;
	while (l <= r) {
		while (l <= r && part[l] == 0) swapTo[l] = l, l++;
		while (l <= r && part[r] == 1) swapTo[r] = r, r--;
		if (l < r) {
			swap(nodeID[start + l], nodeID[start + r]);
			swapTo[l] = r;
			swapTo[r] = l;
			l++;
			r--;
		}
	}
	i32 split = l;

	i32 size[2] = { split, graphData->nvtxs - split };
	assert(size[0] >= 1 && size[1] >= 1);

	//划分的两部分均满足大小则加入范围
	if (size[0] <= maxSize && size[1] <= maxSize) {
		ranges.push_back({ start, start + split });
		ranges.push_back({ start + split, end });
	}
	//否则将两部分作为子图继续划分
	else {
		for (u32 i = 0; i < 2; ++i) {
			childGraph[i] = new MetisGraph;
			childGraph[i]->adjncy.reserve(graphData->adjncy.size() >> 1);
			childGraph[i]->adjwgt.reserve(graphData->adjwgt.size() >> 1);
			childGraph[i]->xadj.reserve(size[i] + 1);
			childGraph[i]->nvtxs = size[i];
		}
		for (u32 i = 0; i < graphData->nvtxs; ++i) {
			u32 needReserve = (i >= childGraph[0]->nvtxs);
			u32 u = swapTo[i];
			MetisGraph* childMeTisGraph = childGraph[needReserve];
			childMeTisGraph->xadj.push_back(childMeTisGraph->adjncy.size());
			for (u32 j = graphData->xadj[u]; j < graphData->xadj[u + 1]; ++j) {
				idx_t v = graphData->adjncy[j];
				idx_t w = graphData->adjwgt[j];
				v = swapTo[v] - (needReserve ? size[0] : 0);
				if (v >= 0 && v < size[needReserve]) {
					childMeTisGraph->adjncy.push_back(v);
					childMeTisGraph->adjwgt.push_back(w);
				}
			}
		}
		childGraph[0]->xadj.push_back(childGraph[0]->adjncy.size());
		childGraph[1]->xadj.push_back(childGraph[1]->adjncy.size());
	}
	
	return start + split;
}

//递归划分
void Partitioner::RecursiveBisectGraph(
	MetisGraph* graphData,
	u32 start, 
	u32 end
)
{
	MetisGraph* childGraphs[2] = { 0 };
	u32 split = BisectGraph(graphData, childGraphs, start, end);
	delete graphData;

	//如果够分为2个继续递归
	if (childGraphs[0] && childGraphs[1]) {
		RecursiveBisectGraph(childGraphs[0], start, split);
		RecursiveBisectGraph(childGraphs[1], split, end);
	}
	else {
		assert(!childGraphs[0] && !childGraphs[1]);
	}
}

Partitioner::Partitioner(u32 numNode)
{
	nodeID = vector<u32>(numNode);
	sortTo = vector<u32>(numNode);
	for (int i = 0; i < numNode; ++i) {
		nodeID[i] = i;
		sortTo[i] = i;
	}
	minSize = 0;
	maxSize = 0;
}

void Partitioner::Init(u32 numNode)
{
	nodeID.resize(numNode);
	sortTo.resize(numNode);
	u32 i = 0;
	for (u32& x : nodeID) {
		x = i, i++;
	}

	i = 0;
	for (u32& x : sortTo) {
		x = i, i++;
	}
}

void Partitioner::Partition(Graph*& graph, u32 minSize, u32 maxSize)
{
	Init(graph->GetGraph().size());
	this->minSize = minSize;
	this->maxSize = maxSize;

	MetisGraph* graphData = ToMetisData(graph);

	RecursiveBisectGraph(graphData, 0, graphData->nvtxs);

	sort(ranges.begin(), ranges.end());

	for (u32 i = 0; i < nodeID.size(); ++i) {
		sortTo[nodeID[i]] = i;
	}
}
