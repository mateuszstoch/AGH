class Node:
    def __init__(self, idx):
        self.idx = idx
        self.out = set()

    def connect_to(self, v):
        self.out.add(v)


def load_graph(V, L):
    G = [None] + [Node(i) for i in range(1, V + 1)]
    for (u, v, _) in L:
        G[u].connect_to(v)
        G[v].connect_to(u)
    return G


def lexBFS(G):
    n = len(G) - 1
    # List of sets of vertices. Initially one set with all vertices {1..n}
    sigma = [set(range(1, n + 1))]
    res = []

    while sigma:
        # 1. Pop a vertex v from the last set in sigma
        curr_set = sigma[-1]
        v = curr_set.pop()
        if not curr_set:
            sigma.pop()

        res.append(v)

        # 2. Update sigma
        # For each set S in sigma, split it into (S \ N(v)) and (S \cap N(v))
        # Replace S with these two sets in order
        new_sigma = []
        neighbors = G[v].out

        for S in sigma:
            intersection = S & neighbors
            difference = S - neighbors

            if difference:
                new_sigma.append(difference)
            if intersection:
                new_sigma.append(intersection)

        sigma = new_sigma

    return res


def checkLexBFS(G, vs):
    n = len(G) - 1
    pi = [0] * (n + 1)
    for i, v in enumerate(vs):
        pi[v] = i

    for i in range(n - 1):
        for j in range(i + 1, n - 1):
            Ni = G[vs[i]].out
            Nj = G[vs[j]].out

            verts = [pi[v] for v in Nj - Ni if pi[v] < i]
            if verts:
                viable = [pi[v] for v in Ni - Nj]
                if not viable or min(verts) <= min(viable):
                    return False
    return True


def get_peo_data(G, vs):
    n = len(G) - 1
    rn = [set() for _ in range(n + 1)]
    parent = [0] * (n + 1)

    # Map vertex to its position in PEO
    pos = [0] * (n + 1)
    for i, v in enumerate(vs):
        pos[v] = i

    for v in vs:
        # RN(v) = neighbors of v that appear BEFORE v in PEO
        for u in G[v].out:
            if pos[u] < pos[v]:
                rn[v].add(u)

        # parent(v) = neighbor in RN(v) that appears LAST in PEO (closest to v)
        if rn[v]:
            # Find u in rn[v] with max pos[u]
            max_p = -1
            best_u = 0
            for u in rn[v]:
                if pos[u] > max_p:
                    max_p = pos[u]
                    best_u = u
            parent[v] = best_u

    return rn, parent


def isChordal(V, L):
    G = load_graph(V, L)
    vs = lexBFS(G)

    # Check if LexBFS output is valid PEO
    rn, parent = get_peo_data(G, vs)

    for v in vs:
        if rn[v] and parent[v] != 0:
            # Check if RN(v) \ {parent(v)} is subset of RN(parent(v))
            if not (rn[v] - {parent[v]}).issubset(rn[parent[v]]):
                return False  # Not a chordal graph

    return True


def maxClique(V, L):
    G = load_graph(V, L)
    vs = lexBFS(G)
    rn, _ = get_peo_data(G, vs)

    max_clique_size = 0
    for v in vs:
        # Size of clique ending at v is |RN(v)| + 1
        size = len(rn[v]) + 1
        if size > max_clique_size:
            max_clique_size = size

    return max_clique_size


def graphColoring(V, L):
    G = load_graph(V, L)
    vs = lexBFS(G)  # PEO order

    n = len(G) - 1
    color = [0] * (n + 1)

    for v in vs:
        neighbors = G[v].out
        used_colors = {color[u] for u in neighbors if color[u] != 0}

        c = 1
        while c in used_colors:
            c += 1
        color[v] = c

    return max(color)


def vertexCover(V, L):
    G = load_graph(V, L)
    vs = lexBFS(G)
    # Process in reverse PEO order for Independent Set
    vs_reversed = vs[::-1]

    I = set()
    for v in vs_reversed:
        neighbors = G[v].out
        # Check if I and neighbors are disjoint
        if I.isdisjoint(neighbors):
            I.add(v)

    # Vertex Cover is V - I
    return V - len(I)
