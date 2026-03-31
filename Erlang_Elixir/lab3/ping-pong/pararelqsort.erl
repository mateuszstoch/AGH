-module(pararelqsort).

-export([generate_lists/2, sort/1, sort_pararel/1, sort_proces/2]).

-import(qsort, [qs/1, random_elems/3]).

generate_lists(Lists_count, Elements_count) ->
    [random_elems(Elements_count, 1, 10000) || _ <- lists:seq(1, Lists_count)].

sort(Lists) ->
    [qs(List) || List <- Lists].

sort_proces(Pid, List) ->
    Pid ! {self(), qs(List)}.

sort_pararel(Lists) ->
    Pids = [spawn(pararelqsort, sort_proces, [self(), List]) || List <- Lists],

    [receive
         {L, SortedList} ->
             SortedList
     after 10000 ->
         error
     end
     || L <- Pids].
