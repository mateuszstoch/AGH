from typing import List, Tuple


def build_adj_list(V: int, edges: List[Tuple[int, int, int]]):
    adj = [[] for _ in range(V + 1)]
    for u, v, w in edges:
        adj[u].append((v, w))
        adj[v].append((u, w))
    return adj


def dfs_path_exists(adj, s, t, min_w, visited):
    if s == t:
        return True
    visited[s] = True
    for v, w in adj[s]:
        if not visited[v] and w >= min_w:
            if dfs_path_exists(adj, v, t, min_w, visited):
                return True
    return False


def widest_path_binary_dfs(
        edges: List[Tuple[int, int, int]],
        V: int,
        s: int = 1,
        t: int = 2) -> int | None:
    adj = build_adj_list(V, edges)
    low, high = 0, max(w for _, _, w in edges)
    ans = None
    while low <= high:
        mid = (low + high) // 2
        visited = [False] * (V + 1)
        if dfs_path_exists(adj, s, t, mid, visited):
            ans = mid
            low = mid + 1
        else:
            high = mid - 1
    return ans
