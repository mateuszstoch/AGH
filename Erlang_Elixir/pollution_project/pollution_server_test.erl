-module(pollution_server_test).

-include_lib("eunit/include/eunit.hrl").

server_test_() ->
    {foreach,
     fun setup/0,
     fun teardown/1,
     [fun add_station_test/1,
      fun add_station_fail_test/1,
      fun add_value_test/1,
      fun add_value_fail_test/1,
      fun add_value_non_existing_station_test/1,
      fun remove_value_test/1,
      fun remove_value_fail_test/1,
      fun get_one_value_test/1,
      fun get_one_value_fail_test/1,
      fun get_station_min_test/1,
      fun get_station_min_fail_test/1,
      fun get_station_mean_test/1,
      fun get_station_mean_fail_test/1,
      fun get_minimum_polution_station_test/1,
      fun get_minimum_polution_station_fail_test/1,
      fun get_daily_mean_test/1,
      fun get_daily_mean_fail_test/1]}.

setup() ->
    pollution_server:start().

teardown(_) ->
    pollution_server:stop().

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
add_station_test(_) ->
    ?_test(begin
               ?assertMatch(ok, pollution_server:add_station("Stacja 1", {1, 1})),
               ?assertMatch(ok, pollution_server:add_station("Stacja 2", {2, 2}))
           end).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
add_station_fail_test(_) ->
    ?_test(begin
               ok = pollution_server:add_station("Stacja 1", {1, 1}),
               ?assertMatch({error, _}, pollution_server:add_station("Stacja 1", {1, 1})),
               ?assertMatch({error, _}, pollution_server:add_station("Stacja 1", {9, 9})),
               ?assertMatch({error, _}, pollution_server:add_station("Inna", {1, 1}))
           end).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
add_value_test(_) ->
    ?_test(begin
               ok = pollution_server:add_station("Stacja 1", {1, 1}),
               Time = calendar:local_time(),
               ?assertMatch(ok, pollution_server:add_value("Stacja 1", Time, "PM10", 46.3)),
               ?assertMatch(ok, pollution_server:add_value("Stacja 1", Time, "PM1", 20.0)),
               ?assertMatch(ok,
                            pollution_server:add_value({1, 1},
                                                       {{2023, 3, 27}, {10, 0, 0}},
                                                       "PM10",
                                                       10.0))
           end).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
add_value_fail_test(_) ->
    ?_test(begin
               ok = pollution_server:add_station("Stacja 1", {1, 1}),
               Time = calendar:local_time(),
               ok = pollution_server:add_value("Stacja 1", Time, "PM10", 46.3),
               %% Duplikat — ta sama stacja, czas i typ
               ?assertMatch({error, _}, pollution_server:add_value("Stacja 1", Time, "PM10", 46.3)),
               ?assertMatch({error, _}, pollution_server:add_value("Stacja 1", Time, "PM10", 99.9)),
               %% Po coords — ten sam pomiar
               ?assertMatch({error, _}, pollution_server:add_value({1, 1}, Time, "PM10", 46.3))
           end).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
add_value_non_existing_station_test(_) ->
    ?_test(begin
               ok = pollution_server:add_station("Stacja 1", {1, 1}),
               Time = calendar:local_time(),
               ?assertMatch({error, _}, pollution_server:add_value("Stacja 2", Time, "PM10", 10.0)),
               ?assertMatch({error, _}, pollution_server:add_value({9, 9}, Time, "PM10", 10.0))
           end).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
remove_value_test(_) ->
    ?_test(begin
               ok = pollution_server:add_station("Stacja 1", {1, 1}),
               Time = calendar:local_time(),
               ok = pollution_server:add_value("Stacja 1", Time, "PM10", 46.3),
               ok = pollution_server:add_value("Stacja 1", Time, "PM1", 20.0),
               ?assertMatch(ok, pollution_server:remove_value("Stacja 1", Time, "PM10")),
               %% Po usunięciu pobieranie daje błąd
               ?assertMatch({error, _}, pollution_server:get_one_value("Stacja 1", Time, "PM10")),
               %% Drugi pomiar nadal istnieje
               ?assertMatch(20.0, pollution_server:get_one_value("Stacja 1", Time, "PM1"))
           end).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
remove_value_fail_test(_) ->
    ?_test(begin
               ok = pollution_server:add_station("Stacja 1", {1, 1}),
               Time = calendar:local_time(),
               ok = pollution_server:add_value("Stacja 1", Time, "PM10", 46.3),
               %% Zły typ
               ?assertMatch({error, _}, pollution_server:remove_value("Stacja 1", Time, "PM25")),
               %% Zły czas
               ?assertMatch({error, _},
                            pollution_server:remove_value("Stacja 1",
                                                          {{2000, 1, 1}, {0, 0, 0}},
                                                          "PM10")),
               %% Nieistniejąca stacja
               ?assertMatch({error, _}, pollution_server:remove_value("Stacja 2", Time, "PM10")),
               ?assertMatch({error, _}, pollution_server:remove_value({9, 9}, Time, "PM10"))
           end).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
get_one_value_test(_) ->
    ?_test(begin
               ok = pollution_server:add_station("Stacja 1", {1, 1}),
               Time = calendar:local_time(),
               ok = pollution_server:add_value("Stacja 1", Time, "PM10", 46.3),
               ok = pollution_server:add_value("Stacja 1", Time, "PM1", 20.0),
               ?assertMatch(46.3, pollution_server:get_one_value("Stacja 1", Time, "PM10")),
               ?assertMatch(20.0, pollution_server:get_one_value("Stacja 1", Time, "PM1")),
               %% Pobieranie po coords
               ?assertMatch(46.3, pollution_server:get_one_value({1, 1}, Time, "PM10"))
           end).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
get_one_value_fail_test(_) ->
    ?_test(begin
               ok = pollution_server:add_station("Stacja 1", {1, 1}),
               Time = calendar:local_time(),
               ok = pollution_server:add_value("Stacja 1", Time, "PM10", 46.3),
               ?assertMatch({error, _}, pollution_server:get_one_value("Stacja 1", Time, "PM25")),
               ?assertMatch({error, _}, pollution_server:get_one_value("Stacja 2", Time, "PM10")),
               ?assertMatch({error, _}, pollution_server:get_one_value({9, 9}, Time, "PM10"))
           end).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
get_station_min_test(_) ->
    ?_test(begin
               ok = pollution_server:add_station("Stacja 1", {1, 1}),
               ok =
                   pollution_server:add_value("Stacja 1",
                                              {{2023, 3, 27}, {10, 0, 0}},
                                              "PM10",
                                              30.0),
               ok =
                   pollution_server:add_value("Stacja 1",
                                              {{2023, 3, 27}, {11, 0, 0}},
                                              "PM10",
                                              10.0),
               ok =
                   pollution_server:add_value("Stacja 1",
                                              {{2023, 3, 27}, {12, 0, 0}},
                                              "PM10",
                                              20.0),
               ?assertMatch(10.0, pollution_server:get_station_min("Stacja 1", "PM10")),
               ?assertMatch(10.0, pollution_server:get_station_min({1, 1}, "PM10"))
           end).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
get_station_min_fail_test(_) ->
    ?_test(begin
               ok = pollution_server:add_station("Stacja 1", {1, 1}),
               ?assertMatch({error, _}, pollution_server:get_station_min("Stacja 1", "PM10")),
               ok =
                   pollution_server:add_value("Stacja 1",
                                              {{2023, 3, 27}, {10, 0, 0}},
                                              "PM10",
                                              30.0),
               ?assertMatch({error, _}, pollution_server:get_station_min("Stacja 1", "PM25")),
               ?assertMatch({error, _}, pollution_server:get_station_min("Stacja 2", "PM10"))
           end).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
get_station_mean_test(_) ->
    ?_test(begin
               ok = pollution_server:add_station("Stacja 1", {1, 1}),
               ok =
                   pollution_server:add_value("Stacja 1",
                                              {{2023, 3, 27}, {10, 0, 0}},
                                              "PM10",
                                              10.0),
               ok =
                   pollution_server:add_value("Stacja 1",
                                              {{2023, 3, 27}, {11, 0, 0}},
                                              "PM10",
                                              20.0),
               ok =
                   pollution_server:add_value("Stacja 1",
                                              {{2023, 3, 27}, {12, 0, 0}},
                                              "PM10",
                                              30.0),
               ?assertMatch(20.0, pollution_server:get_station_mean("Stacja 1", "PM10")),
               ?assertMatch(20.0, pollution_server:get_station_mean({1, 1}, "PM10"))
           end).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
get_station_mean_fail_test(_) ->
    ?_test(begin
               ok = pollution_server:add_station("Stacja 1", {1, 1}),
               ?assertMatch({error, _}, pollution_server:get_station_mean("Stacja 1", "PM10")),
               ok =
                   pollution_server:add_value("Stacja 1",
                                              {{2023, 3, 27}, {10, 0, 0}},
                                              "PM10",
                                              10.0),
               ?assertMatch({error, _}, pollution_server:get_station_mean("Stacja 1", "PM25")),
               ?assertMatch({error, _}, pollution_server:get_station_mean("Stacja 2", "PM10"))
           end).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
get_minimum_polution_station_test(_) ->
    ?_test(begin
               ok = pollution_server:add_station("Stacja 1", {1, 1}),
               ok = pollution_server:add_station("Stacja 2", {2, 2}),
               ok = pollution_server:add_station("Stacja 3", {3, 3}),
               DT = {{2023, 3, 27}, {12, 0, 0}},
               ok = pollution_server:add_value("Stacja 1", DT, "PM10", 5.0),
               ok = pollution_server:add_value("Stacja 2", DT, "PM10", 50.0),
               ok = pollution_server:add_value("Stacja 3", DT, "PM10", 100.0),
               ?assertMatch("Stacja 1", pollution_server:get_minimum_polution_station(DT, "PM10"))
           end).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
get_minimum_polution_station_fail_test(_) ->
    ?_test(begin
               ok = pollution_server:add_station("Stacja 1", {1, 1}),
               DT = {{2023, 3, 27}, {12, 0, 0}},
               ?assertMatch({error, _}, pollution_server:get_minimum_polution_station(DT, "PM10")),
               ok = pollution_server:add_value("Stacja 1", DT, "PM10", 42.0),
               ?assertMatch({error, _}, pollution_server:get_minimum_polution_station(DT, "PM25")),
               ?assertMatch({error, _},
                            pollution_server:get_minimum_polution_station({{2023, 3, 28},
                                                                           {12, 0, 0}},
                                                                          "PM10"))
           end).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
get_daily_mean_test(_) ->
    ?_test(begin
               ok = pollution_server:add_station("Stacja 1", {1, 1}),
               ok = pollution_server:add_station("Stacja 2", {2, 2}),
               ok =
                   pollution_server:add_value("Stacja 1",
                                              {{2023, 3, 27}, {10, 0, 0}},
                                              "PM10",
                                              10.0),
               ok =
                   pollution_server:add_value("Stacja 2",
                                              {{2023, 3, 27}, {11, 0, 0}},
                                              "PM10",
                                              20.0),
               ok =
                   pollution_server:add_value("Stacja 1",
                                              {{2023, 3, 27}, {12, 0, 0}},
                                              "PM10",
                                              10.0),
               ok =
                   pollution_server:add_value("Stacja 2",
                                              {{2023, 3, 27}, {13, 0, 0}},
                                              "PM10",
                                              20.0),
               ?assertMatch(15.0, pollution_server:get_daily_mean({2023, 3, 27}, "PM10"))
           end).

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
get_daily_mean_fail_test(_) ->
    ?_test(begin
               ok = pollution_server:add_station("Stacja 1", {1, 1}),
               ?assertMatch({error, _}, pollution_server:get_daily_mean({2023, 3, 27}, "PM10")),
               ok =
                   pollution_server:add_value("Stacja 1",
                                              {{2023, 3, 27}, {10, 0, 0}},
                                              "PM10",
                                              10.0),
               ?assertMatch({error, _}, pollution_server:get_daily_mean({2023, 3, 27}, "PM25")),
               ?assertMatch({error, _}, pollution_server:get_daily_mean({2023, 3, 29}, "PM10"))
           end).
