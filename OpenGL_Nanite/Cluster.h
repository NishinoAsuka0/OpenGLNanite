#pragma once
#include "Bounds.h"
#include <span>
#include "Partitioner.h"
#include "MeshSimplify.h"
#include <unordered_map>
#include <metis.h>
struct Cluster {
    //固定一个Cluster128个三角形
    static const u32 clusterSize = 128;

    //顶点、索引、颜色和外部边
    vector<Vertex> verts;
    vector<u32> indexes;
    vector<vec3> color;
    vector<u32> externalEdges;

    //LOD相关参数
    Bounds boxBounds;
    Sphere sphereBounds;
    Sphere lodBounds;
    f32 lodError;
    u32 mipLevel;
    u32 groupId;
};

struct GPUCluster {
    vec4 lodBounds;
    vec4 parentLodBounds;
    float lodError;
    float maxParentLodError;
    u32 vertOffset;
    u32 indexOffset;
    u32 groupId;
    u32 mipLevel;
    u32 visible;
    u32 indexCount;
    u32 textureID;
    // 显式填充到 80 字节
    u32 _padding[3];
};

struct PackedCluster {
    vector<Vertex> verts;
    vector<u32> indexes;
    vec4 lodBounds;
    vec4 parentLodBounds;
    f32 lodError;
    f32 maxParentLodError;
    u32 mipLevel;
    u32 groupId;

};

struct DrawElementsIndirectCommand {
    unsigned int count;
    unsigned int instanceCount;
    unsigned int firstIndex;
    unsigned int baseVertex;
    unsigned int baseInstance;
};

static_assert(sizeof(GPUCluster) == 80, "GPUCluster必须80 Byte");

struct ClusterGroup {
    //一组32个三角形
    static const u32 groupSize = 32;

    Sphere bounds;
    Sphere lodBounds;
    f32 minLodError;
    f32 maxParentLodError;
    u32 mipLevel;
    vector<u32> clusters; //对cluster数组的下标
    vector<pair<u32, u32>> externalEdges; //first: cluster id, second: edge id
};

//将三角形划分为Cluster
void ClusterTriangles(
    const vector<Vertex>& verts,
    const vector<u32>& indexes,
    vector<Cluster>& clusters
);

//将Cluster划分为Cluster Group
void GroupCluster(
    vector<Cluster>& clusters,
    u32 offset,
    u32 numCluster,
    vector<ClusterGroup>& clusterGroups,
    u32 mipLevel
);


//构建DAG中的父子关系
void BuildParentClusters(
    ClusterGroup& clusterGroup,
    vector<Cluster>& clusters
);;