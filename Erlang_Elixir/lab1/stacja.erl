-module(stacja).

-export([pomiar/4, generate_sample_data/0]).

pomiar(Name, Date, Time, Mesurments)
    when is_list(Name) and is_tuple(Date) and is_tuple(Time) and is_list(Mesurments) ->
    {{name, Name}, {date, Date}, {time, Time}, {mesurment, Mesurments}}.

generate_sample_data() ->
    [pomiar("Krowodrza", {2026, 10, 2}, {10, 20, 0}, [{"P10", 10}, {"P5", 3}]),
     pomiar("Nowa huta", {2026, 10, 3}, {11, 20, 0}, [{"P10", 28}, {"P5", 2}]),
     pomiar("Centrum", {2026, 10, 2}, {10, 50, 0}, [{"CO", 8}, {"NO", 10}]),
     pomiar("Podgorze", {2026, 10, 4}, {9, 40, 0}, [{"P10", 18}, {"P5", 25}])].
