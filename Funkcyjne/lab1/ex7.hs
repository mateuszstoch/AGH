not' :: Bool -> Bool
not' True = False
not' False = True

isItAnswer :: String -> Bool
isItAnswer "Love" = True
isItAnswer _ = False

or' :: (Bool, Bool) -> Bool
or' (False, False) = False
or' _ = True

and' :: (Bool, Bool) -> Bool
and' (True, True) = True
and' _ = False

nand' :: (Bool, Bool) -> Bool
nand' (False, False) = True
nand' _ = False

xor' :: (Bool, Bool) -> Bool
xor' (True, True) = False
xor' (False, False) = False
xor' _ = True