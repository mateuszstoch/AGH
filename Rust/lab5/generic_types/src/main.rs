fn max<T: PartialOrd + Copy>(arr: &[T]) -> Option<T> {
    if arr.len() == 0 {
        return None;
    }
    let mut v_max: T = arr[0];
    for n in 1..arr.len() {
        if v_max < arr[n] {
            v_max = arr[n];
        }
    }
    Some(v_max)
}

fn mean<T: Into<f64> + Copy>(arr: &[T]) -> f64 {
    let mut sum = 0.0;
    for n in arr {
        sum += (*n).into();
    }
    sum / (arr.len() as f64)
}

fn main() {
    let arr: [i32; 7] = [1, 2, 3, 4, 9, 2, 3];
    println!("Max {:?}: {:?}", arr, max(&arr));
    let arr_empty: [i32; 0] = [];
    println!("Max {:?}: {:?}", arr_empty, max(&arr_empty));
    println!("Mean {:?}: {:?}", arr, mean(&arr));
}
