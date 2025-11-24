def dfs(G_residual, s, t):
    L = len(G_residual)
    stack = [s]
    parent = [-1] * L
    visited = [False] * L
    visited[s] = True

    while stack:
        v = stack.pop()
        for i in range(L):
            if G_residual[v][i] > 0 and not visited[i]:
                visited[i] = True
                parent[i] = v
                stack.append(i)

                if i == t:
                    path = []
                    cur = t
                    while cur != -1:
                        path.append(cur)
                        if cur == s:
                            break
                        cur = parent[cur]
                    path.reverse()
                    return path
    return None


def fordFulkerson(V, L, t):
    G = [[0 for _ in range(L)]for _ in range(L)]
    s = 0
    for v in V:
        G[v[0]-1][v[1]-1] = 1
        G[v[1]-1][v[0]-1] = 1

    flow = 0
    path = dfs(G, s, t)
    while path is not None:
        bottleneck = float("inf")
        for i in range(1, len(path)):
            bottleneck = min(bottleneck, G[path[i-1]][path[i]])
        for i in range(1, len(path)):
            G[path[i-1]][path[i]] -= bottleneck
            G[path[i]][path[i-1]] += bottleneck
        flow += bottleneck
        path = dfs(G, s, t)
    return flow


def findGraphConnectivity(V, L):
    mininum = float("inf")
    for i in range(1, L):
        output = fordFulkerson(V, L, i)
        if output < mininum:
            mininum = output
    if mininum != float("inf"):
        return mininum
    return 0
