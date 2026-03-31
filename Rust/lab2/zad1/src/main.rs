fn trim_me(input: &str) -> String {
    let mut j = 0;
    for (_,&item) in input.as_bytes().iter().enumerate(){
        if item == b' '{
            j+=1;
        }else{
            break;
        }
    }
    for (i,&item) in input.as_bytes().iter().enumerate().rev(){
        if item != b' '{
            return input[j..i+1].to_string();
        }
    }
    input.to_string()
    
}
 
fn compose_me(input: &str) -> String {
    let mut out = input.to_string();
   out.push_str(" world!");
   out
 
}
 
fn replace_me(input: &str) -> String {
    let mut j = 0;
    for (i,&item) in input.as_bytes().iter().enumerate(){
        if item == b' '{
            if &input[j..i] == "cars"{
                let mut s = input[..j].to_string();
                s.push_str("balloons");
                s.push_str(&input[i..]);
                return s;
            }
            j = i+1;
        }
    }
    if &input[j..] == "cars"{
        let mut s = input[..j].to_string();
        s.push_str("balloons");
        return s;
    }
    input.to_string()
}
 
fn main() {
    assert_eq!(trim_me("Hello!     "), "Hello!");
    assert_eq!(trim_me("  What's up!"), "What's up!");
    assert_eq!(trim_me("   Hola!  "), "Hola!");
 
 
    assert_eq!(compose_me("Hello"), "Hello world!");
    assert_eq!(compose_me("Goodbye"), "Goodbye world!");
 
 
    assert_eq!(replace_me("I think cars are cool"), "I think balloons are cool");
    assert_eq!(replace_me("I love to look at cars"), "I love to look at balloons");
}