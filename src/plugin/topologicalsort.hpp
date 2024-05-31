// Copyright (c) 2024 Manuel Schneider

#pragma once
#include <map>
#include <set>
#include <vector>

template<class T>
struct TopologicalSortResult
{
    std::vector<T> sorted;
    std::map<T, std::set<T>> error_set;
};

template<class T>
TopologicalSortResult<T> topologicalSort(std::map<T, std::set<T>> graph)
{
    // First, find a list of "start nodes" that have no incoming edges
    // and insert them into a set S; at least one such node must exist
    // in a non-empty (finite) acyclic graph. Then:
    // L ← Empty list that will contain the sorted elements
    // S ← Set of all nodes with no incoming edge
    // while S is not empty do
    //     remove a node n from S
    //     add n to L
    //     for each node m with an edge e from n to m do
    //         remove edge e from the graph
    //         if m has no other incoming edges then
    //             insert m into S
    // if graph has edges then
    //     return error   (graph has at least one cycle)
    // else
    //     return L   (a topologically sorted order)

    std::vector<T> degree_0_set;
    for (auto it = begin(graph); it!= end(graph);)
    {
        if (it->second.empty())
        {
            degree_0_set.push_back(it->first);
            it = graph.erase(it);
        }
        else
            ++it;
    }

    std::vector<T> ordered;
    while (!degree_0_set.empty())
    {
        const auto degree_0_node = degree_0_set.back();
        degree_0_set.pop_back();
        ordered.push_back(degree_0_node);

        for (auto it = begin(graph); it!= end(graph);)
        {
            auto &[node, edges] = *it;
            if (edges.erase(degree_0_node) && edges.empty())
            {
                degree_0_set.push_back(node);
                it = graph.erase(it);
            }
            else
                ++it;
        }
    }
    return {.sorted=ordered, .error_set=graph};
}

