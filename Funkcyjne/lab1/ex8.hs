not' :: Bool -> Bool
not' b = case b of
  True -> False
  False -> True

absInt n =
  case (n >= 0) of
    True -> n
    _ -> -n

isItAnswer n =
  case n of
    "Love" -> True
    _ -> False

or' (x, y) =
  case (x, y) of
    (False, False) -> False
    _ -> True

and' (x, y) =
  case (x, y) of
    (True, True) -> True
    _ -> False

nand' (x, y) =
  case (x, y) of
    (True, True) -> False
    _ -> True

xor' (x, y) =
  case (x, y) of
    (True, True) -> False
    (False, False) -> False
    _ -> True