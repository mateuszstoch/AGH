roots (a, b, c) =
  let d = sqrt (b * b - 4 * a * c)
      e = 2 * a
   in ((-b - d) / e, (-b + d) / e)

unitVec2D :: (Double, Double) -> (Double, Double)
unitVec2D
  (x, y) =
    let v = sqrt (x ^ 2 + y ^ 2)
     in (x / v, y / v)