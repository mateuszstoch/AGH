newtype CartesianCoord a = MkCartesianCoord (a, a)

newtype PolarCoord a = MkPolarCoord (a, a)

polartoCartesian :: (Floating a) => PolarCoord a -> CartesianCoord a
polartoCartesian (MkPolarCoord (r, phi)) = MkCartesianCoord (r * cos phi, r * sin phi)
