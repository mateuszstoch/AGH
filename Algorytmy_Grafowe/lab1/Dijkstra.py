import heapq
from typing import List, Tuple


def widest_path_dijkstra(
        V: int,
        edges: List[Tuple[int, int, int]],
        s: int = 1,
        t: int = 2) -> int | None:
    adj = [[] for _ in range(V + 1)]
    for u, v, w in edges:
        adj[u].append((v, w))
        adj[v].append((u, w))
    dist = [0] * (V + 1)
    dist[s] = float("inf")
    pq = [(-dist[s], s)]
    while pq:
        cap, u = heapq.heappop(pq)
        cap = -cap
        if u == t:
            return cap
        if cap < dist[u]:
            continue
        for v, w in adj[u]:
            new_cap = min(cap, w)
            if new_cap > dist[v]:
                dist[v] = new_cap
                heapq.heappush(pq, (-new_cap, v))
    return None
