-module(util).

-export([power/2]).

power(A, N) when is_number(A) and is_number(N) and (N > 1) ->
    A * power(A, N - 1);
power(A, 1) when is_number(A) ->
    A.
