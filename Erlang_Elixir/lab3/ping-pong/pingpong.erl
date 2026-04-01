-module(pingpong).

-export([ping_loop/1, pong_loop/0, start/1]).

start(N) ->
    register(ping, spawn(fun() -> ping_loop(0) end)),
    register(pong, spawn(fun() -> pong_loop() end)),
    ping ! N.

ping_loop(Sum) ->
    receive
        0 ->
            io:format("Ping: ~B~n", [0]),
            pong ! stop,
            ok;
        stop ->
            ok;
        N ->
            New_sum = Sum + N,
            io:format("Ping: ~B, Sum = ~B~n", [N, New_sum]),
            timer:sleep(250),
            pong ! N - 1,
            ping_loop(New_sum + N - 1)
    after 20000 ->
        ok
    end.

pong_loop() ->
    receive
        0 ->
            io:format("Pong: ~B~n", [0]),
            ping ! stop,
            ok;
        stop ->
            ok;
        N ->
            io:format("Pong: ~B~n", [N]),
            timer:sleep(250),
            ping ! N - 1,
            pong_loop()
    after 20000 ->
        ok
    end.
