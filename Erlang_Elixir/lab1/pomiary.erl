-module(pomiary).

-export([number_of_readings/2, calculate_min_and_max/2, calculate_mean/2]).

number_of_readings([], _) ->
    0;
number_of_readings(Readings, Date) ->
    [R | List] = Readings,
    D = element(2, element(2, R)),
    case D of
        Date ->
            1 + number_of_readings(List, Date);
        _ ->
            number_of_readings(List, Date)
    end.

calculate_min_and_max([], _Type) ->
    wrongType;
calculate_min_and_max(Readings, Type) ->
    [R | List] = Readings,
    Mesurments = element(2, element(4, R)),
    Value = find_type(Mesurments, Type),
    case Value of
        false ->
            calculate_min_and_max(List, Type);
        V ->
            calculate_min_and_max_tail(List, Type, {V, V})
    end.

calculate_min_and_max_tail([], _Type, MinMax) ->
    MinMax;
calculate_min_and_max_tail([R | List], Type, {Min, Max}) ->
    Mesurments = element(2, element(4, R)),
    Value = find_type(Mesurments, Type),
    case Value of
        false ->
            calculate_min_and_max_tail(List, Type, {Min, Max});
        V ->
            if V < Min ->
                   calculate_min_and_max_tail(List, Type, {V, Max});
               V > Max ->
                   calculate_min_and_max_tail(List, Type, {Min, V});
               true ->
                   calculate_min_and_max_tail(List, Type, {Min, Max})
            end
    end.

find_type([], _Type) ->
    false;
find_type([{Type, Value} | _Mesurments], Type) ->
    Value;
find_type([_ | Mesurments], Type) ->
    find_type(Mesurments, Type).

calculate_mean([], _Type) ->
    wrongType;
calculate_mean(Readings, Type) ->
    [R | List] = Readings,
    Mesurments = element(2, element(4, R)),
    Value = find_type(Mesurments, Type),
    case Value of
        false ->
            calculate_mean(List, Type);
        V ->
            calculate_mean_tail(List, Type, {V, 1})
    end.

calculate_mean_tail([], _Type, {Sum, Count}) ->
    Sum / Count;
calculate_mean_tail([R | List], Type, {Sum, Count}) ->
    Mesurments = element(2, element(4, R)),
    Value = find_type(Mesurments, Type),
    case Value of
        false ->
            calculate_mean_tail(List, Type, {Sum, Count});
        V ->
            calculate_mean_tail(List, Type, {Sum + V, Count + 1})
    end.
