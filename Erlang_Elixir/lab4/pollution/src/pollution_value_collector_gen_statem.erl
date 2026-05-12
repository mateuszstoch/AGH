-module(pollution_value_collector_gen_statem).
-behaviour(gen_statem).

%% API
-export([start_link/0, stop/0, set_station/1, add_value/3, store_data/0]).

%% gen_statem callbacks
-export([init/1, callback_mode/0, terminate/3, code_change/4]).
-export([wait_for_station/3, collecting/3]).

start_link() ->
    gen_statem:start_link({local, ?MODULE}, ?MODULE, [], []).

stop() ->
    gen_statem:stop(?MODULE).

set_station(Station) ->
    gen_statem:cast(?MODULE, {set_station, Station}).

add_value(DateTime, Type, Value) ->
    gen_statem:cast(?MODULE, {add_value, DateTime, Type, Value}).

store_data() ->
    gen_statem:cast(?MODULE, store_data).

init([]) ->
    {ok, wait_for_station, #{station => undefined, values => []}}.

callback_mode() ->
    state_functions.

%% State: wait_for_station
wait_for_station(cast, {set_station, Station}, Data) ->
    {next_state, collecting, Data#{station => Station, values => []}};
wait_for_station(_EventType, _Msg, _Data) ->
    keep_state_and_data.

%% State: collecting
collecting(cast, {add_value, DateTime, Type, Value}, Data = #{values := Values}) ->
    {keep_state, Data#{values => [{DateTime, Type, Value} | Values]}};
collecting(cast, store_data, #{station := Station, values := Values}) ->
    lists:foreach(
        fun({DateTime, Type, Value}) ->
            pollution_gen_server:add_value(Station, DateTime, Type, Value)
        end,
        lists:reverse(Values)
    ),
    {next_state, wait_for_station, #{station => undefined, values => []}};
collecting(cast, {set_station, Station}, Data) ->
    {keep_state, Data#{station => Station, values => []}};
collecting(_EventType, _Msg, _Data) ->
    keep_state_and_data.

terminate(_Reason, _State, _Data) ->
    ok.

code_change(_OldVsn, State, Data, _Extra) ->
    {ok, State, Data}.
