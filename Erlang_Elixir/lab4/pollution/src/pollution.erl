-module(pollution).

-export([create_monitor/0, add_station/3, add_value/5, remove_value/4, get_one_value/4,
         get_station_min/3, get_station_mean/3, get_minimum_polution_station/3, get_daily_mean/3]).

%% ===================================================================
%% Struktura danych:
%%
%% Rekord stacji:
%%   #station{ name   :: string()
%%           , coords :: {number(), number()}
%%           , readings :: map()   %% kluczem jest {DateTime, Type}
%%           }
%%
%% Monitor to mapa:  #{ {name, Name} => #station{}, {coords, Coords} => #station{} }
%% ===================================================================

-record(station, {name, coords, readings = #{}}).

create_monitor() ->
    #{}.

add_station(Name, Coords, Monitor) ->
    case maps:is_key({name, Name}, Monitor) orelse maps:is_key({coords, Coords}, Monitor) of
        true ->
            {error, "Stacja o podanej nazwie lub wspolrzednych juz istnieje"};
        false ->
            Station = #station{name = Name, coords = Coords},
            Monitor#{{name, Name} => Station, {coords, Coords} => Station}
    end.

add_value(Identifier, DateTime, Type, Value, Monitor) ->
    Key = identifier_key(Identifier),
    case maps:find(Key, Monitor) of
        error ->
            {error, "Stacja nie istnieje"};
        {ok,
         Station =
             #station{name = Name,
                      coords = Coords,
                      readings = Readings}} ->
            ReadingKey = {DateTime, Type},
            case maps:is_key(ReadingKey, Readings) of
                true ->
                    {error, "Pomiar o podanej dacie i typie juz istnieje na tej stacji"};
                false ->
                    UpdatedStation = Station#station{readings = Readings#{ReadingKey => Value}},
                    Monitor#{{name, Name} => UpdatedStation, {coords, Coords} => UpdatedStation}
            end
    end.

remove_value(Identifier, DateTime, Type, Monitor) ->
    Key = identifier_key(Identifier),
    case maps:find(Key, Monitor) of
        error ->
            {error, "Stacja nie istnieje"};
        {ok,
         Station =
             #station{name = Name,
                      coords = Coords,
                      readings = Readings}} ->
            ReadingKey = {DateTime, Type},
            case maps:is_key(ReadingKey, Readings) of
                false ->
                    {error, "Pomiar nie istnieje"};
                true ->
                    UpdatedStation = Station#station{readings = maps:remove(ReadingKey, Readings)},
                    Monitor#{{name, Name} => UpdatedStation, {coords, Coords} => UpdatedStation}
            end
    end.

get_one_value(Identifier, DateTime, Type, Monitor) ->
    Key = identifier_key(Identifier),
    case maps:find(Key, Monitor) of
        error ->
            {error, "Stacja nie istnieje"};
        {ok, #station{readings = Readings}} ->
            case maps:find({DateTime, Type}, Readings) of
                error ->
                    {error, "Brak pomiaru dla podanych parametrow"};
                {ok, Value} ->
                    Value
            end
    end.

get_station_min(Identifier, Type, Monitor) ->
    case get_station_values(Identifier, Type, Monitor) of
        {error, _} = Err ->
            Err;
        [] ->
            {error, "Brak pomiarow danego typu na tej stacji"};
        Values ->
            lists:min(Values)
    end.

get_station_mean(Identifier, Type, Monitor) ->
    case get_station_values(Identifier, Type, Monitor) of
        {error, _} = Err ->
            Err;
        [] ->
            {error, "Brak pomiarow danego typu na tej stacji"};
        Values ->
            lists:sum(Values) / length(Values)
    end.

get_minimum_polution_station(DateTime, Type, Monitor) ->
    UniqueStations = [S || {{name, _}, S} <- maps:to_list(Monitor)],
    Values =
        [{Name, V}
         || #station{name = Name, readings = R} <- UniqueStations,
            {{DT, T}, V} <- maps:to_list(R),
            DT =:= DateTime,
            T =:= Type],
    case Values of
        [] ->
            {error, "Brak pomiarow danego dnia i typu"};
        _ ->
            Sorted = lists:keysort(2, Values),
            [{MinName, _} | _] = Sorted,
            MinName
    end.

get_daily_mean(Date, Type, Monitor) ->
    UniqueStations = [S || {{name, _}, S} <- maps:to_list(Monitor)],
    Values =
        [V
         || #station{readings = R} <- UniqueStations,
            {{DT, T}, V} <- maps:to_list(R),
            element(1, DT) =:= Date,
            T =:= Type],
    case Values of
        [] ->
            {error, "Brak pomiarow danego dnia i typu"};
        _ ->
            lists:sum(Values) / length(Values)
    end.

identifier_key(Identifier) when is_list(Identifier) ->
    {name, Identifier};
identifier_key(Identifier) when is_tuple(Identifier) ->
    {coords, Identifier}.

get_station_values(Identifier, Type, Monitor) ->
    Key = identifier_key(Identifier),
    case maps:find(Key, Monitor) of
        error ->
            {error, "Stacja nie istnieje"};
        {ok, #station{readings = Readings}} ->
            [V || {{_, T}, V} <- maps:to_list(Readings), T =:= Type]
    end.
