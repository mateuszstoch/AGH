#[derive(Debug)]
enum Chessman {
    Pawn { position: Position, color: Color },
    Knight { position: Position, color: Color },
    Bishop { position: Position, color: Color },
    Rook { position: Position, color: Color },
    Queen { position: Position, color: Color },
    King { position: Position, color: Color },
}
#[derive(Debug)]
enum Color {
    White,
    Black,
}
#[derive(Debug)]
struct Position {
    x: u8,
    y: u8,
}
impl Chessman {
    fn move_to(&mut self, new_position: Position) -> bool {
        use Chessman::*;
        use Color::*;
        match self {
            Pawn { position, color } => match color {
                White => {
                    if position.y < new_position.y
                        && position.x == new_position.y
                        && new_position.y < 8
                    {
                        position.y = new_position.y;
                        return true;
                    }
                    return false;
                }
                Black => {
                    if position.y > new_position.y && position.x == new_position.y {
                        position.y = new_position.y;
                        return true;
                    }
                    return false;
                }
            },
            _ => false,
        }
    }
}

fn main() {
    println!("Hello, world!");
}
