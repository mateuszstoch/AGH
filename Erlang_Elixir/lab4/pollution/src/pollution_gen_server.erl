-module(pollution_gen_server).
-behaviour(gen_server).

%% API
-export([start_link/0, stop/0, crash/0]).
-export([add_station/2, add_value/4, remove_value/3, get_one_value/3,
         get_station_min/2, get_station_mean/2, get_minimum_polution_station/2, get_daily_mean/2]).

%% gen_server callbacks
-export([init/1, handle_call/3, handle_cast/2, handle_info/2, terminate/2, code_change/3]).

start_link() ->
    gen_server:start_link({local, ?MODULE}, ?MODULE, [], []).

stop() ->
    gen_server:call(?MODULE, stop).

crash() ->
    gen_server:cast(?MODULE, crash).

add_station(Name, Coords) ->
    gen_server:call(?MODULE, {add_station, Name, Coords}).

add_value(Identifier, DateTime, Type, Value) ->
    gen_server:call(?MODULE, {add_value, Identifier, DateTime, Type, Value}).

remove_value(Identifier, DateTime, Type) ->
    gen_server:call(?MODULE, {remove_value, Identifier, DateTime, Type}).

get_one_value(Identifier, DateTime, Type) ->
    gen_server:call(?MODULE, {get_one_value, Identifier, DateTime, Type}).

get_station_min(Identifier, Type) ->
    gen_server:call(?MODULE, {get_station_min, Identifier, Type}).

get_station_mean(Identifier, Type) ->
    gen_server:call(?MODULE, {get_station_mean, Identifier, Type}).

get_minimum_polution_station(DateTime, Type) ->
    gen_server:call(?MODULE, {get_minimum_polution_station, DateTime, Type}).

get_daily_mean(Date, Type) ->
    gen_server:call(?MODULE, {get_daily_mean, Date, Type}).

%% Callbacks
init([]) ->
    {ok, pollution:create_monitor()}.

handle_call({add_station, Name, Coords}, _From, Monitor) ->
    case pollution:add_station(Name, Coords, Monitor) of
        {error, Msg} -> {reply, {error, Msg}, Monitor};
        NewMonitor -> {reply, ok, NewMonitor}
    end;
handle_call({add_value, Identifier, DateTime, Type, Value}, _From, Monitor) ->
    case pollution:add_value(Identifier, DateTime, Type, Value, Monitor) of
        {error, Msg} -> {reply, {error, Msg}, Monitor};
        NewMonitor -> {reply, ok, NewMonitor}
    end;
handle_call({remove_value, Identifier, DateTime, Type}, _From, Monitor) ->
    case pollution:remove_value(Identifier, DateTime, Type, Monitor) of
        {error, Msg} -> {reply, {error, Msg}, Monitor};
        NewMonitor -> {reply, ok, NewMonitor}
    end;
handle_call({get_one_value, Identifier, DateTime, Type}, _From, Monitor) ->
    Reply = pollution:get_one_value(Identifier, DateTime, Type, Monitor),
    {reply, Reply, Monitor};
handle_call({get_station_min, Identifier, Type}, _From, Monitor) ->
    Reply = pollution:get_station_min(Identifier, Type, Monitor),
    {reply, Reply, Monitor};
handle_call({get_station_mean, Identifier, Type}, _From, Monitor) ->
    Reply = pollution:get_station_mean(Identifier, Type, Monitor),
    {reply, Reply, Monitor};
handle_call({get_minimum_polution_station, DateTime, Type}, _From, Monitor) ->
    Reply = pollution:get_minimum_polution_station(DateTime, Type, Monitor),
    {reply, Reply, Monitor};
handle_call({get_daily_mean, Date, Type}, _From, Monitor) ->
    Reply = pollution:get_daily_mean(Date, Type, Monitor),
    {reply, Reply, Monitor};
handle_call(stop, _From, Monitor) ->
    {stop, normal, ok, Monitor}.

handle_cast(crash, Monitor) ->
    1 = 2, %% This will cause a badmatch error and crash the server
    {noreply, Monitor}.

handle_info(_Info, Monitor) ->
    {noreply, Monitor}.

terminate(_Reason, _Monitor) ->
    ok.

code_change(_OldVsn, Monitor, _Extra) ->
    {ok, Monitor}.
