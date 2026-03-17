-module(qsort).

-export([qs/1, random_elems/3, compare_speeds/3, test_sorted/0, test_unsorted/0]).

less_then(List, Arg) ->
    [X || X <- List, X < Arg].

grt_eq_then(List, Arg) ->
    [X || X <- List, X >= Arg].

qs([]) ->
    [];
qs([Pivot | Tail]) ->
    qs(less_then(Tail, Pivot)) ++ [Pivot] ++ qs(grt_eq_then(Tail, Pivot)).

random_elems(N, Min, Max) ->
    [rand:uniform(Max - Min + 1) + Min - 1 || _ <- lists:seq(1, N)].

compare_speeds(List, Fun1, Fun2) ->
    {Time1, _} = timer:tc(Fun1, [List]),
    {Time2, _} = timer:tc(Fun2, [List]),
    io:format("Time of first func: ~p~nTime of second func: ~p~n", [Time1, Time2]).

test_unsorted() ->
    qsort:compare_speeds(
        qsort:random_elems(20000, 1, 1000), fun qsort:qs/1, fun lists:sort/1).

test_sorted() ->
    qsort:compare_speeds(
        lists:sort(
            qsort:random_elems(20000, 1, 1000)),
        fun qsort:qs/1,
        fun lists:sort/1).
