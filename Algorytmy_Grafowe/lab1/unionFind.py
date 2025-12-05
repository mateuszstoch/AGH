from typing import List, Tuple, Optional


class UnionFind:
    def __init__(self, n: int):
        self.parent = list(range(n + 1))
        self.rank = [0] * (n + 1)

    def find(self, x: int) -> int:
        if self.parent[x] != x:
            self.parent[x] = self.find(self.parent[x])
        return self.parent[x]

    def union(self, x: int, y: int) -> bool:
        rx = self.find(x)
        ry = self.find(y)
        if rx == ry:
            return False
        if self.rank[rx] < self.rank[ry]:
            self.parent[rx] = ry
        elif self.rank[rx] > self.rank[ry]:
            self.parent[ry] = rx
        else:
            self.parent[ry] = rx
            self.rank[rx] += 1
        return True


def widest_path_union_find(
        V: int,
        edges: List[Tuple[int, int, int]],
        s: int = 1,
        t: int = 2) -> Optional[int]:
    edges_sorted = sorted(edges, key=lambda e: e[2], reverse=True)
    uf = UnionFind(V)
    for u, v, w in edges_sorted:
        uf.union(u, v)
        if uf.find(s) == uf.find(t):
            return w
    return None
