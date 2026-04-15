use std::fmt::{Display, Formatter};
use std::ops::{Add, Sub, Mul};

struct Vec2D{
    x: f32,
    y: f32,
}

impl Vec2D{
 pub fn equal(&self, other: &Vec2D) -> bool{
    if self.x == other.x && self.y == other.y{
        return true;
    }
    false
 }
 pub fn unit_vector(&self) -> Vec2D{
    Vec2D{x: self.x/self.x.abs(), y: self.y / self.y.abs()}
 }
}

impl Display for Vec2D{
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        write!(f, "Vector[x={},y={}]", self.x, self.y)?;
        Ok(())
    }
}

impl Add for Vec2D {
    type Output = Self; 
 
    fn add(self, other: Vec2D) -> Self {
        Self { x:self.x + other.x, y : self.y + other.y}
    }
}
 
impl Sub for Vec2D {
    type Output = Self;
 
    fn sub(self, other: Vec2D) -> Self {
        Self { x:self.x - other.x, y : self.y - other.y}
    }
}

impl Mul<f32> for Vec2D {
    type Output = Self;
 
    fn mul(self, other: f32) -> Self {
        Self { x:self.x * other, y : self.y * other}
    }
}
impl Mul for Vec2D {
    type Output = f32;
 
    fn mul(self, other: Self) -> f32 {
        self.x*other.x + self.y * other.y
    }
}