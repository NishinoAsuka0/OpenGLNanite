#include "Cluster.h"

inline u32 Hash(vec3 v) {
    union { f32 f; u32 u; } x, y, z;
    x.f = (v.x == 0.f ? 0 : v.x);
    y.f = (v.y == 0.f ? 0 : v.y);
    z.f = (v.z == 0.f ? 0 : v.z);
    return MurMurMix(MurMurAdd(MurMurAdd(x.u, y.u), z.u));
}

inline u32 Hash(pair<vec3, vec3> edge) {
    u32 h0 = ::Hash(edge.first);
    u32 h1 = ::Hash(edge.second);
    return MurMurMix(MurMurAdd(h0, h1));
}

//��ԭ������λ��2��0�ָ���10111->1000001001001����������Ī���� 
inline u32 ExpandBits(u32 v) {
    v = (v * 0x00010001u) & 0xFF0000FFu;
    v = (v * 0x00000101u) & 0x0F00F00Fu;
    v = (v * 0x00000011u) & 0xC30C30C3u;
    v = (v * 0x00000005u) & 0x49249249u;
    return v;
}
//Ī���룬Ҫ�� 0<=x,y,z<=1
u32 Morton3D(vec3 p) {
    u32 x = p.x * 1023, y = p.y * 1023, z = p.z * 1023;
    x = ExpandBits(x);
    y = ExpandBits(y);
    z = ExpandBits(z);
    return (x << 2) | (y << 1) | (z << 1);
}

//�߹�ϣ���ҵ����������෴�ıߣ�����������������
void BuildAdjacencyEdgeLink(
    const vector<Vertex>& verts,
    const vector<u32>& indexes,
    Graph* edgeLink
) {
    HashTable edge_ht(indexes.size());
    edgeLink->Init(indexes.size());

    for (u32 i = 0; i < indexes.size(); i++) {
        vec3 p0 = verts[indexes[i]].pos;
        vec3 p1 = verts[indexes[Cycle3(i)]].pos;
        edge_ht.Add(::Hash({ p0,p1 }), i);

        for (u32 j : edge_ht[::Hash({ p1,p0 })]) {
            if (p1 == verts[indexes[j]].pos && p0 == verts[indexes[Cycle3(j)]].pos) {
                edgeLink->IncreaseEdgeCost(i, j, 1);
                edgeLink->IncreaseEdgeCost(j, i, 1);
            }
        }
    }
}


// ���ݱߵ��ڽӹ��������ε��ڽ�ͼ����ȨΪ1������Ҫ����localʱ��Ҫadjacency��Ȩ�㹻��
void BuildAdjacencyGraph(
    Graph* edgeLink,
    Graph* graph
) {
    graph->Init(edgeLink->GetGraph().size() / 3);
    u32 u = 0;
    for (const auto& mp : edgeLink->GetGraph()) {
        for (auto [v, w] : mp) {
            graph->IncreaseEdgeCost(u / 3, v / 3, 1);
        }
        u++;
    }
}

void ClusterTriangles(
    const vector<Vertex>& verts,
    const vector<u32>& indexes,
    vector<Cluster>& clusters
) {
    Graph* edgeLink = new Graph(0);
    Graph* graph = new Graph(0);
    BuildAdjacencyEdgeLink(verts, indexes, edgeLink);
    BuildAdjacencyGraph(edgeLink, graph);

    Partitioner* partitioner = new Partitioner(0);
    partitioner->Partition(graph, Cluster::clusterSize - 4, Cluster::clusterSize);

    // ���ݻ��ֽ������clusters
    for (auto [l, r] : partitioner->GetRanges()) {
        clusters.push_back({});
        Cluster& cluster = clusters.back();

        unordered_map<u32, u32> mp;
        for (u32 i = l; i < r; i++) {
            u32 triIdx = partitioner->GetNodeID()[i];
            for (u32 k = 0; k < 3; k++) {
                u32 edgeIdx = triIdx * 3 + k;
                u32 vertexIdx = indexes[edgeIdx];
                if (mp.find(vertexIdx) == mp.end()) { //��ӳ�䶥���±�
                    mp[vertexIdx] = cluster.verts.size();
                    cluster.verts.push_back(verts[vertexIdx]);
                }
                bool isExternal = false;
                for (auto [adjEdge, _] : edgeLink->GetGraph()[edgeIdx]) {
                    u32 adjTri = partitioner->GetSortTo()[adjEdge / 3];
                    if (adjTri < l || adjTri >= r) { //�����ڲ�ͬ����˵���Ǳ߽�
                        isExternal = true;
                        break;
                    }
                }
                if (isExternal) {
                    cluster.externalEdges.push_back(cluster.indexes.size());
                }
                cluster.indexes.push_back(mp[vertexIdx]);
            }
        }

        cluster.mipLevel = 0;
        cluster.lodError = 0;
        cluster.sphereBounds = Sphere::FromPoints(cluster.verts.data(), cluster.verts.size());
        cluster.lodBounds = cluster.sphereBounds;
        cluster.boxBounds = cluster.verts[0].pos;
        for (auto& p : cluster.verts) cluster.boxBounds = cluster.boxBounds + p.pos;
    }
}

void BuildClustersEdgeLink(
    span<const Cluster> clusters,
    const vector<pair<u32, u32>>& externalEdges,
    Graph* edgeLink
) {
    HashTable edgeHashTable(externalEdges.size());
    edgeLink->Init(externalEdges.size());

    u32 i = 0;
    for (auto [clusterId, edgeId] : externalEdges) {
        auto& verts = clusters[clusterId].verts;
        auto& idx = clusters[clusterId].indexes;
        vec3 p0 = verts[idx[edgeId]].pos;
        vec3 p1 = verts[idx[Cycle3(edgeId)]].pos;
        edgeHashTable.Add(::Hash({ p0,p1 }), i);
        for (u32 j : edgeHashTable[::Hash({ p1,p0 })]) {
            auto [tempClusterId, tempEdgeId] = externalEdges[j];
            auto& newVert = clusters[tempClusterId].verts;
            auto& newIdx = clusters[tempClusterId].indexes;

            if (newVert[newIdx[tempEdgeId]].pos == p1 && newVert[newIdx[Cycle3(tempEdgeId)]].pos == p0) {
                edgeLink->IncreaseEdgeCost(i, j, 1);
                edgeLink->IncreaseEdgeCost(j, i, 1);
            }
        }
        i++;
    }
}

void BuildClustersGraph(
    Graph* edgeLink,
    const vector<u32>& mp,
    u32 numCluster,
    Graph* graph
) {
    graph->Init(numCluster);
    u32 u = 0;
    for (const auto& edgeMap : edgeLink->GetGraph()) {
        for (auto [v, w] : edgeMap) {
            graph->IncreaseEdgeCost(mp[u], mp[v], 1);
        }
        u++;
    }
}

void GroupCluster(
    vector<Cluster>& clusters,
    u32 offset,
    u32 numCluster,
    vector<ClusterGroup>& clusterGroups,
    u32 mipLevel
) {
    span<const Cluster> clustersView(clusters.begin() + offset, numCluster);

    //ȡ��ÿ��cluster�ı߽磬��������id����id��ӳ��
    vector<u32> edgeToClusterMap; //edge_id to cluster_id
    vector<u32> clusterToEdgeMap; //cluster_id to first_edge_id
    vector<pair<u32, u32>> externalEdges;
    u32 i = 0;

    for (auto& cluster : clustersView) {
        assert(cluster.mipLevel == mipLevel);
        clusterToEdgeMap.push_back(edgeToClusterMap.size());
        for (u32 e : cluster.externalEdges) {
            externalEdges.push_back({ i,e });
            edgeToClusterMap.push_back(i);
        }
        i++;
    }

    Graph* edgeLink = new Graph(0);
    Graph* graph = new Graph(0);

    BuildClustersEdgeLink(clustersView, externalEdges, edgeLink);
    BuildClustersGraph(edgeLink, edgeToClusterMap, numCluster, graph);

    Partitioner* partitioner = new Partitioner(0);
    partitioner->Partition(graph, ClusterGroup::groupSize - 4, ClusterGroup::groupSize);

    //todo: ��Χ��
    for (auto [l, r] : partitioner->GetRanges()) {
        clusterGroups.push_back({});
        auto& group = clusterGroups.back();
        group.mipLevel = mipLevel;

        for (u32 i = l; i < r; i++) {
            u32 clusterId = partitioner->GetNodeID()[i];
            clusters[clusterId + offset].groupId = clusterGroups.size() - 1;
            group.clusters.push_back(clusterId + offset);
            for (u32 edgeIdx = clusterToEdgeMap[clusterId]; edgeIdx < edgeToClusterMap.size() && edgeToClusterMap[edgeIdx] == clusterId; edgeIdx++) {
                bool isExternal = false;
                for (auto [adjEdge, _] : edgeLink->GetGraph()[edgeIdx]) {
                    u32 adjCluster = partitioner->GetSortTo()[edgeToClusterMap[adjEdge]];
                    if (adjCluster < l || adjCluster >= r) {
                        isExternal = true;
                        break;
                    }
                }
                if (isExternal) {
                    u32 e = externalEdges[edgeIdx].second;
                    group.externalEdges.push_back({ clusterId + offset,e });
                }
            }
        }
    }
}

void BuildParentClusters(
    ClusterGroup& clusterGroup,
    std::vector<Cluster>& clusters
) {
    vector<Vertex>verts;
    vector<u32> idx;
    vector<Sphere> lodBounds;
    f32 maxParentLodError = 0;
    u32 iOffset = 0;

    for (u32 c : clusterGroup.clusters) {
        auto& cluster = clusters[c];
        for (auto p : cluster.verts) {
            verts.push_back(p);
        }
        for (u32 i : cluster.indexes) idx.push_back(i + iOffset);
        iOffset += cluster.verts.size();
        lodBounds.push_back(cluster.lodBounds);
        maxParentLodError = max(maxParentLodError, cluster.lodError); //ǿ�Ƹ��ڵ��error���ڵ����ӽڵ�
    }

    Sphere parentLodBound = Sphere::FromSpheres(lodBounds.data(), lodBounds.size());

    MeshSimplifier simplifier(verts.data(), verts.size(), idx.data(), idx.size());
    HashTable edgeHashTable(clusterGroup.externalEdges.size());
    u32 i = 0;

    //cout << clusterGroup.externalEdges.size() << " " << pos.size() << endl;

    for (auto [cluster, edge] : clusterGroup.externalEdges) {
        auto& pos = clusters[cluster].verts;
        auto& idx = clusters[cluster].indexes;
        vec3 p0 = pos[idx[edge]].pos, p1 = pos[idx[Cycle3(edge)]].pos;
        edgeHashTable.Add(::Hash({ p0,p1 }), i);
        simplifier.LockPosition(p0);
        simplifier.LockPosition(p1);
        i++;
    }

    simplifier.Simplify((Cluster::clusterSize - 2) * (clusterGroup.clusters.size() / 2));
    //std::cout << pos.size() << "," << idx.size() << " ";
    verts.resize(simplifier.RemainingNumVert());
    idx.resize(simplifier.RemainingNumTri() * 3);
    //std::cout << pos.size() << "," << idx.size() << std::endl;

    maxParentLodError = max(maxParentLodError, sqrt(simplifier.MaxError()));

    Graph* edgeLink = new Graph(0);
    Graph* graph = new Graph(0);
    BuildAdjacencyEdgeLink(verts, idx, edgeLink);
    BuildAdjacencyGraph(edgeLink, graph);

    Partitioner* partitioner = new Partitioner(0);
    partitioner->Partition(graph, Cluster::clusterSize - 4, Cluster::clusterSize);

    for (auto [l, r] : partitioner->GetRanges()) {
        clusters.push_back({});
        Cluster& cluster = clusters.back();

        unordered_map<u32, u32> mp;
        for (u32 i = l; i < r; i++) {
            u32 triIdx = partitioner->GetNodeID()[i];
            for (u32 k = 0; k < 3; k++) {
                u32 edgeIdx = triIdx * 3 + k;
                u32 vertexIdx = idx[edgeIdx];
                if (mp.find(vertexIdx) == mp.end()) { //��ӳ�䶥���±�
                    mp[vertexIdx] = cluster.verts.size();
                    cluster.verts.push_back(verts[vertexIdx]);
                }

                bool isExternal = false;

                for (auto [adjEdge, _] : edgeLink->GetGraph()[edgeIdx]) {
                    u32 adjTri = partitioner->GetSortTo()[adjEdge / 3];
                    if (adjTri < l || adjTri >= r) { //�����ڲ�ͬ����˵���Ǳ߽�
                        isExternal = true;
                        break;
                    }
                }

                vec3 p0 = verts[vertexIdx].pos, p1 = verts[idx[Cycle3(edgeIdx)]].pos;

                if (!isExternal) {
                    for (u32 j : edgeHashTable[::Hash({ p0,p1 })]) {
                        auto [c, e] = clusterGroup.externalEdges[j];
                        auto& pos = clusters[c].verts;
                        auto& idx = clusters[c].indexes;
                        if (p0 == pos[idx[e]].pos && p1 == pos[idx[Cycle3(e)]].pos) {
                            isExternal = true;
                            break;
                        }
                    }
                }

                if (isExternal) {
                    cluster.externalEdges.push_back(cluster.indexes.size());
                }
                cluster.indexes.push_back(mp[vertexIdx]);
            }
        }

        cluster.mipLevel = clusterGroup.mipLevel + 1;
        cluster.sphereBounds = Sphere::FromPoints(cluster.verts.data(), cluster.verts.size());
        //ǿ�Ƹ��ڵ��lod��Χ�и��������ӽڵ�lod��Χ��
        cluster.lodBounds = parentLodBound;
        cluster.lodError = maxParentLodError;
        cluster.boxBounds = cluster.verts[0].pos;
        for (auto p : cluster.verts) cluster.boxBounds = cluster.boxBounds + p.pos;
    }
    clusterGroup.lodBounds = parentLodBound;
    clusterGroup.maxParentLodError = maxParentLodError;
}
