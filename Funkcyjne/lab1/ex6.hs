absInt :: Int -> Int
absInt n
  | n > 0 = n
  | otherwise = -n

min3Int :: (Double, Double, Double) -> Double
min3Int (x, y, z)
  | x < y && x < z = x
  | y < x && y < z = y
  | otherwise = z