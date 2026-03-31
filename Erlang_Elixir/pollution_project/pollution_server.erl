-module(pollution_server).

-export([start/0, stop/0, add_station/2, add_value/4, remove_value/3, get_one_value/3,
         get_station_min/2, get_station_mean/2, get_minimum_polution_station/2, get_daily_mean/2]).

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
    init().

stop() ->
    call(stop).

add_station(Name, Coords) ->
    call({add_station, Name, Coords}).

add_value(Identifier, DateTime, Type, Value) ->
    call({add_value, Identifier, DateTime, Type, Value}).

remove_value(Identifier, DateTime, Type) ->
    call({remove_value, Identifier, DateTime, Type}).

get_one_value(Identifier, DateTime, Type) ->
    call({get_one_value, Identifier, DateTime, Type}).

get_station_min(Identifier, Type) ->
    call({get_station_min, Identifier, Type}).

get_station_mean(Identifier, Type) ->
    call({get_station_mean, Identifier, Type}).

get_minimum_polution_station(DateTime, Type) ->
    call({get_minimum_polution_station, DateTime, Type}).

get_daily_mean(Date, Type) ->
    call({get_daily_mean, Date, Type}).

call(Msg) ->
    pollution_monitor ! {Msg, self()},
    receive
        Reply ->
            Reply
    end.

init() ->
    case lists:member(pollution_monitor, registered()) of
        true ->
            {error, "pollution monitor already running"};
        false ->
            register(pollution_monitor, spawn(fun() -> loop(create_monitor()) end))
    end.

loop(Monitor) ->
    receive
        {{add_station, Name, Coords}, Sender} ->
            case add_station(Name, Coords, Monitor) of
                {error, Message} ->
                    Sender ! {error, Message},
                    loop(Monitor);
                M ->
                    Sender ! ok,
                    loop(M)
            end;
        {{add_value, Identifier, DateTime, Type, Value}, Sender} ->
            case add_value(Identifier, DateTime, Type, Value, Monitor) of
                {error, Message} ->
                    Sender ! {error, Message},
                    loop(Monitor);
                M ->
                    Sender ! ok,
                    loop(M)
            end;
        {{remove_value, Identifier, DateTime, Type}, Sender} ->
            case remove_value(Identifier, DateTime, Type, Monitor) of
                {error, Message} ->
                    Sender ! {error, Message},
                    loop(Monitor);
                M ->
                    Sender ! ok,
                    loop(M)
            end;
        {{get_one_value, Identifier, DateTime, Type}, Sender} ->
            Result = get_one_value(Identifier, DateTime, Type, Monitor),
            Sender ! Result,
            loop(Monitor);
        {{get_station_min, Identifier, Type}, Sender} ->
            Result = get_station_min(Identifier, Type, Monitor),
            Sender ! Result,
            loop(Monitor);
        {{get_station_mean, Identifier, Type}, Sender} ->
            Result = get_station_mean(Identifier, Type, Monitor),
            Sender ! Result,
            loop(Monitor);
        {{get_minimum_polution_station, DateTime, Type}, Sender} ->
            Result = get_minimum_polution_station(DateTime, Type, Monitor),
            Sender ! Result,
            loop(Monitor);
        {{get_daily_mean, Date, Type}, Sender} ->
            Result = get_daily_mean(Date, Type, Monitor),
            Sender ! Result,
            loop(Monitor);
        {stop, Sender} ->
            Sender ! ok
    end.
