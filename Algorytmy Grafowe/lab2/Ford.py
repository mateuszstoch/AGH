from collections import deque


def dfs(G, s, t):
    stack = [s]
    path = []
    parent = [-1 for _ in range(len(G))]
    visited = [False for _ in range(len(G))]
    visited[s] = True
    while stack:
        v = stack.pop()
        visited[v] = True
        if v == t:
            break
        for i in range(len(G)):
            if G[v][i] != 0 and not visited[i]:
                visited[i] = True
                parent[i] = v
                stack.append(i)

    if not visited[t]:
        return None

    path = []
    cur = t
    while cur != -1:
        path.append(cur)
        if cur == s:
            break
        cur = parent[cur]
    path.reverse()
    return path


def bfs(G, s, t):

    queue = deque([s])

    path = []
    parent = [-1 for _ in range(len(G))]
    visited = [False for _ in range(len(G))]

    visited[s] = True

    while queue:

        v = queue.popleft()

        if v == t:
            break

        for i in range(len(G)):
            if G[v][i] != 0 and not visited[i]:
                visited[i] = True
                parent[i] = v
                queue.append(i)

    if not visited[t]:
        return None

    path = []
    cur = t
    while cur != -1:
        path.append(cur)
        if cur == s:
            break
        cur = parent[cur]
    path.reverse()

    return path


def fordFulkerson(V, L):
    G = [[0 for _ in range(L)]for _ in range(L)]
    s = 0
    t = len(G)-1
    for v in V:
        G[v[0]-1][v[1]-1] = v[2]

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
