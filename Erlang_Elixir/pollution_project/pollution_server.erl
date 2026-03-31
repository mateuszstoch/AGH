-module(pollution_server).

-export([start/0, stop/0]).

-import(pollution,
        [create_monitor/0,
         add_station/3,
         add_value/5,
         remove_value/4,
         get_one_value/4,
         get_station_min/3,
         get_station_mean/3,
         get_minimum_polution_station/3,
         get_daily_mean/3]).

start() ->
    register(pollution_monitor, spawn(pollution_server, loop, [create_monitor()])).

stop() ->
    pollution_monitor ! {stop, self()}.

loop(Monitor) ->
    receive
        {add_station, Name, Coords, Sender} ->
            case add_station(Name, Coors, Monitor) of
                {error, Message} ->
                    Sender ! {error, Message},
                    loop(Monitor);
                M ->
                    Sender ! ok,
                    loop(M)
            end;
        {add_value, Identifier, DateTime, Type, Value, Sender} ->
            case add_value(Identifier, DateTime, Type, Value, Monitor) of
                {error, Message} ->
                    Sender ! {error, Message},
                    loop(Monitor);
                M ->
                    Sender ! ok,
                    loop(M)
            end;
        {remove_value, Identifier, DateTime, Type, Sender} ->
            case remove_value(Identifier, DateTime, Type, Monitor) of
                {error, Message} ->
                    Sender ! {error, Message},
                    loop(Monitor);
                M ->
                    Sender ! ok,
                    loop(M)
            end;
        {get_one_value, Identifier, DateTime, Type} ->
            case get_one_value(Identifier, DateTime, Type, Monitor) of
                {error, Message} ->
                    Sender ! {error, Message},
                    loop(Monitor);
                M ->
                    Sender ! ok,
                    loop(M)
            end;
        {get_station_min, Identifier, Type, Monitor} ->
            case get_station_min(Identifier, Type, Monitor) of
                {error, Message} ->
                    Sender ! {error, Message},
                    loop(Monitor);
                M ->
                    Sender ! ok,
                    loop(M)
            end;
        {stop, Sender} ->
            ok
    end.
