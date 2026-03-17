use std::io;
 
fn main() {
  let mut board = [[0;3]; 3];
  let mut move_count = 0;
  while move_count < 9{
    let is_player_one_turn = move_count%2==0;
    let player_move : u32 = get_user_input(is_player_one_turn) - 1;
    let player_move = parse_player_move(player_move);
    if !check_move_validity(player_move,&board) {draw_board(&board);continue};
    board[player_move.0 as usize][player_move.1 as usize] = if is_player_one_turn {1} else {2};
    draw_board(&board);
    if check_victory(is_player_one_turn,player_move,&board) {println!("Wygrał gracz {}!!!",if is_player_one_turn {"1 (X)"} else {"2 (O)"} ); return;}
    move_count+=1;
  }
  println!("Remis!!!");
}

fn get_user_input(is_player_one_turn : bool) -> u32{
    let mut user_input = String::new();
    println!("Gracz {}, Twój ruch (wprowadź numer pola od 1 do 9):", if is_player_one_turn {"1 (X)"} else {"2 (O)"} );
    let _ = io::stdin().read_line(&mut user_input); 
    user_input.chars().nth(0).unwrap().to_digit(10).unwrap()
}

fn check_move_validity(player_move : (usize,usize), board:&[[u32;3];3]) -> bool{
     if player_move.0 > 3 || player_move.1 > 3 {
        println!("Nieprawidłowy ruch, Wybrano pole spoza zakresu");
        return false;
    }
    if board[player_move.0][player_move.1] != 0 {
        println!("Nieprawidłowy ruch, Wybrano zajęte pole");
        return false;
    }
    true
}

fn check_victory(is_player_one_turn:bool, player_move : (usize,usize), board:&[[u32;3];3]) -> bool{
    let mut victory : bool = true;
    let player =  if is_player_one_turn  {1} else {2};
    for i in 0..3{
        if board[player_move.0][i] != player{
            victory = false;
        }
    }
    if victory {return true;} else {victory = true;}

    for i in 0..3{
        if board[i][player_move.1] != player{
            victory = false;
        }
    }

    victory || (board[1][1] == player && board[0][0] == board[1][1] && board[1][1] == board[2][2]) || (board[1][1] == player && board[0][2] == board[1][1] && board[1][1] == board[2][0])
}

fn parse_player_move(player_move: u32) -> (usize,usize){
    ((player_move / 3) as usize, (player_move % 3)as usize)
}

fn draw_board(board:&[[u32;3];3]){
    println!("=========");
    for i in 0..9{
        if board[i/3][i%3] == 1{
            print!("X");
        }else if board[i/3][i%3] == 2{
            print!("O");
        }else{
            print!("{}",i+1);
        }
        if (i+1) % 3 == 0 {println!("\n=========")} else {print!(" | ")}
    }
}