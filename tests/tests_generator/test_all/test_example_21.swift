func fib (_ n: Int) -> Int {
    if n == 0 || n == 1 {
        return n
    } else {
        return fib(n - 1) + fib(n - 2)
    }
}

write(fib(10))