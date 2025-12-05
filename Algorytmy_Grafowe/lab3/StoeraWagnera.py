from queue import PriorityQueue
from math import inf


class Node:
    def __init__(self):
        self.edges = {}
        self.active = True

    def addEdge(self, to, weight):
        self.edges[to] = self.edges.get(to, 0) + weight

    def delEdge(self, to):
        if to in self.edges:
            del self.edges[to]


def mergeVertices(G, x, y):
    if y in G[x].edges:
        del G[x].edges[y]
    if x in G[y].edges:
        del G[y].edges[x]

    for neighbor, weight in G[y].edges.items():
        G[x].addEdge(neighbor, weight)
        G[neighbor].addEdge(x, weight)
        G[neighbor].delEdge(y)

    G[y].active = False


def minimumCutPhase(G):
    n = len(G)
    weights = [0] * n

    Q = PriorityQueue()

    start_node = -1
    for i in range(n):
        if G[i].active:
            start_node = i
            break

    if start_node == -1:
        return 0

    added_order = []
    S = set()

    a = start_node
    S.add(a)
    added_order.append(a)

    for neighbor, weight in G[a].edges.items():
        if G[neighbor].active:
            weights[neighbor] += weight
            Q.put((-weights[neighbor], neighbor))

    active_count = sum(1 for node in G if node.active)

    while len(S) < active_count:
        if Q.empty():
            for i in range(n):
                if G[i].active and i not in S:
                    v = i
                    break
        else:
            w, v = Q.get()
            if v in S:
                continue

        S.add(v)
        added_order.append(v)

        for neighbor, weight in G[v].edges.items():
            if G[neighbor].active and neighbor not in S:
                weights[neighbor] += weight
                Q.put((-weights[neighbor], neighbor))

    s = added_order[-2]
    t = added_order[-1]

    cut_value = weights[t]

    mergeVertices(G, s, t)

    return cut_value


def stoerWagner(V, L):
    G = [Node() for _ in range(V)]
    for (u, v, w) in L:
        x, y = u - 1, v - 1
        G[x].addEdge(y, w)
        G[y].addEdge(x, w)

    min_cut = inf
    active_nodes = V

    while active_nodes > 1:
        potential_cut = minimumCutPhase(G)

        if potential_cut < min_cut:
            min_cut = potential_cut

        active_nodes -= 1

    return min_cut
