-module(myList).

-export([contains/2, duplicate_elements/1, sumFloats/1, sumFloatsTail/1]).

contains([L | _Lth], L) ->
    true;
contains([L | Lth], Value) ->
    contains(Lth, Value);
contains([], _) ->
    false.

duplicate_elements([L | List]) when is_list(List) ->
    [L, L | duplicate_elements(List)];
duplicate_elements([]) ->
    [].

sumFloats([F | List]) when is_float(F) and is_list(List) ->
    F + sumFloats(List);
sumFloats([]) ->
    0.

sumFloatsTail(List) when is_list(List) ->
    sumFloatsTail(List, 0).

sumFloatsTail([F | List], Sum) when is_float(F) and is_list(List) ->
    sumFloatsTail(List, Sum + F);
sumFloatsTail([], Sum) ->
    Sum.
