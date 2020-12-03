(module
    (import "wasi_unstable" "fd_write" (func $fd_write (param i32 i32 i32 i32) (result i32)))
    (import "wasi_unstable" "fd_read" (func $fd_read (param i32 i32 i32 i32) (result i32)))
    (import "wasi_unstable" "args_get" (func $args_get (param i32 i32) (result i32)))

    (memory 1)
    (export "memory" (memory 0))

    (func $measure_line (param $ptr i32) (result i32)
        (local $end i32)
        (local.set $end (local.get $ptr))
        (block (loop
            (br_if 1 (i32.eq (i32.load8_u (local.get $end)) (i32.const 10))) ;; *ptr == '\n'
            (local.set $end (i32.add (local.get $end) (i32.const 1)))
            (br 0)
        ))
        (i32.sub (local.get $end) (local.get $ptr))
    )

    (func $parse_number (param $str i32) (result i32)
        (local $value i32)
        (local $ch i32)
        (local $sign i32)
        (local.set $value (i32.const 0))
        (local.set $sign (i32.const 1))
        (if (i32.eq (i32.load8_u (local.get $str)) (i32.const 45)) ;; if (*str == '-')
            (block
                (local.set $str (i32.add (local.get $str) (i32.const 1))) ;; str = str + 1
                (local.set $sign (i32.const -1))
            )
        )
        (block
            (loop
                (local.set $ch (i32.load8_u (local.get $str))) ;; ch = *str
                (br_if 1 (i32.eqz (local.get $ch))) ;; if (ch == 0) break
                (local.set $str (i32.add (local.get $str) (i32.const 1))) ;; str = str + 1
                ;; value = (value * 10) + (ch - '0')
                (local.set $value (i32.add
                    (i32.mul (local.get $value) (i32.const 10))
                    (i32.sub (local.get $ch) (i32.const 48))
                ))
                (br 0) ;; continue
        ))
        (i32.mul (local.get $value) (local.get $sign))
    )

    (func $print_number (param $value i32)
        (local $dst i32)
        (local.set $dst (i32.const 2000))
        (local.set $dst (i32.sub (local.get $dst) (i32.const 1)))
        (i32.store8 (local.get $dst) (i32.const 10)) ;; trailing '\n'
        (block (loop
            (local.set $dst (i32.sub (local.get $dst) (i32.const 1))) ;; dst = dst - 1
            (i32.store8 (local.get $dst) (i32.add (i32.rem_u (local.get $value) (i32.const 10)) (i32.const 48))) ;; *dst = value % 10 + '0'
            (local.set $value (i32.div_u (local.get $value) (i32.const 10))) ;; value = value / 10
            (br_if 1 (i32.eqz (local.get $value))) ;; if (value == 0) break
            (br 0) ;; continue
        ))

        ;; iovec[0] = { dst, 2000 - dst }
        (i32.store (i32.const 0) (local.get $dst))
        (i32.store (i32.const 4) (i32.sub (i32.const 2000) (local.get $dst)))
        (drop (call $fd_write
            (i32.const 1) ;; fd: stdout
            (i32.const 0) ;; iovs.data
            (i32.const 1) ;; iovs.size
            (i32.const 20) ;; nread
        ))
    )

    (func $main (export "_start")
        (local $width i32)
        (local $stride i32)
        (local $height i32)
        (local $x i32)
        (local $y i32)
        (local $dx i32)
        (local $dy i32)
        (local $trees i32)

        ;; Read argv to 10000
        (drop (call $args_get (i32.const 10000) (i32.const 15000)))

        ;; Parse dx and dy
        (local.set $dx (call $parse_number (i32.load (i32.const 10004))))
        (local.set $dy (call $parse_number (i32.load (i32.const 10008))))

        ;; iovec[0] = { 20000, 20000 }
        (i32.store (i32.const 0) (i32.const 20000))
        (i32.store (i32.const 4) (i32.const 20000))

        ;; Read the full file
        (drop (call $fd_read
            (i32.const 0) ;; fd: stdin
            (i32.const 0) ;; iovs.data
            (i32.const 1) ;; iovs.size
            (i32.const 20) ;; nread
        ))

        ;; Setup the active area
        (local.set $width (call $measure_line (i32.const 20000)))
        (local.set $stride (i32.add (local.get $width) (i32.const 1)))
        (local.set $height (i32.div_u (i32.add (i32.load (i32.const 20)) (i32.const 1)) (local.get $stride)))
        (local.set $x (i32.const 0))
        (local.set $y (i32.const 0))
        (local.set $trees (i32.const 0))

        (block (loop
            ;; if (data[y * stride + x % width] == '#')
            (if (i32.eq (i32.const 35) (i32.load8_u (i32.add (i32.const 20000)
                (i32.add
                    (i32.mul (local.get $y) (local.get $stride))
                    (i32.rem_u (local.get $x) (local.get $width))))))
                (block 
                    (local.set $trees (i32.add (local.get $trees) (i32.const 1)))
                )
            )
            (local.set $x (i32.add (local.get $x) (local.get $dx)))
            (local.set $y (i32.add (local.get $y) (local.get $dy)))
            (br_if 1 (i32.ge_u (local.get $y) (local.get $height)))
            (br 0)
        ))

        ;; Measure the width
        (call $print_number (local.get $trees))
    )
)

