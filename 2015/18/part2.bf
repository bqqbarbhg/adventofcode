# Cell setup

## Cell format: `FNVLRSab`
* F) flag 0/1: edge/cell
* N) newline
* V) value 0/1
* L) left pass (upwards)
* R) right pass (downwards)
* S) sum accumulator
* a) temporary slot 1
* b) temporary slot 2

## Global variables

    >> align to 8

    >++++++++++ temp 10
    [->++++++++++>>>++++++++++<<<<]> Iterations and Width (100)
    > temp2
    > temp1
    >+ Width (101)
    > move to first cell

## Read input

    >>>>>>>> skip null

    ,[ read input to F

        >+ set N
        < select F
        ---------- sub 10 (newline if zero)

        [
            >- clear N
            >+ set V

            << select F
            ------------------------- sub 25 (zero if value)
            [>>-<<[-]] clear V and F if nonzero

            [-] clear F
        ]

        + set F

        >>>>>>>> move next

    ,] read input to F

### Scan back

    <<<<<<<<[<<<<<<<<]

# Simulation

## Force corners (part2)

### Top left
    >>>>>>>> skip null
    >>[-]+ clearset V

### Top right
    >>>>[-]+ clearset a
    [
        [-] clear a
        >>>>>>>> move next
        [-]+ clearset a
        <<<[-] clear L
        << select N
        [
            [->>+<<] mov N to L
            >>>>>[-] clear a
            <<<<< select N
        ]
        >>[-<<+>>] mov L to N
        >>> select a
    ]

    <<<<<<<<<<<< move prev V
    [-]+ clearset V
    << select F

### Bottom right
    [>>>>>>>>] scan forward
    <<<<<<< select N
    [<<<<<<<<] scan backward
    >[-]+ clearset V

### Bottom left
    >>>>[-]+ clearset a
    [
        [-] clear a
        <<<<<<<< move prev
        [-]+ clearset a
        <<<[-] clear L
        << select N
        [
            [->>+<<] mov N to L
            >>>>>[-] clear a
            <<<<< select N
        ]
        >>[-<<+>>] mov L to N
        >>> select a
    ]

    >>>> move next V
    [-]+ clearset V
    << select F

### Scan back

    <<<<<<<<[<<<<<<<<]


<<<< select Iterations
[
- dec Iterations
>>>> select first cell

## First setup pass

    >>>>>>>> skip null

    [ fwd flag scan

        >>>[-] clear L
        >[-] clear R
        >[-] clear S
        >[-] clear a

        <<<< select V
        [->+>+>>+<<<<] add L R a
        >>>> select a
        [-<<<<+>>>>] add V

        >> move next
    ]

### Scan back

    <<<<<<<<[<<<<<<<<]

## Propagation passes

### Loop counter setup

    <<[-] clear temp1
    > select Width
    [-<+>] add temp1

### Propagation loop

    < select temp1
    [ while temp1

        >> zeroth cell

#### Left propagation (forwards)

        >>>>>>>> move first
        [ fwd flag scan
            >>> select L
            [-<<<<<<<<+>>>>>>>>] add prevL

            >>>>> move next
        ]

#### Right propagation (backwards)

        <<<<<<<< move last
        [ bkd flag scan
            >>>> select R
            [->>>>>>>>+<<<<<<<<] add nextR

            <<<<<<<<<<<< move prev
        ]

        <+ add Width
        <- sub temp1
    ]

    >> select first

## Clear newline propagation

    >>>>>>>> move first

    [ fwd flag scan


        > select N
        [
            >[-] clear V
            >[-] clear L
            >[-] clear R
            >>[-]+ set a
            <<<<< select N
            [-]
        ]

        >>>>> select a
        [-<<<<<+>>>>>] add N

        >> move next
    ]

### Scan back

    <<<<<<<<[<<<<<<<<]

## Accumulation loop

    >>>>>>>> move first

    [ fwd flag scan

        >>> select L
        [->>+<<<<<<<<+>>>>>>>>>>>>>>>>+<<<<<<<<<<] add S prevS nextS
        > select R
        [->+<<<<<<<<+>>>>>>>>>>>>>>>>+<<<<<<<<<] add S prevS nextS

        << select V
        [-<<<<<+>>>>>>>>>+<<<<] add prevS a
        >>>> select a
        [->>>>>>>+<<<<<<<<<<<+>>>>] add nextS V

        >> move next
    ]

### Scan back

    <<<<<<<<[<<<<<<<<]

## State update

    >>>>>>>> move first

    [ fwd flag scan

        >> select V
        [->>>+>+<<<<] add S a
        + set V
        >>> select S
        --- sub 3
        [ if not S=3

            > select a
            [
                < select S
                - sub 1
                >[-] clear a
            ]
            <

            [ if not S=4
                <<<- clear V
                >>>[-] clear S
            ]
        ]

        <<[-] clear L
        << select N
        [
            -
            >[-] clear V
            >+ add L
            << select N
        ]
        >>[-<<+>>] mov L to N

        >>>>> move next
    ]

### Scan back

    <<<<<<<<[<<<<<<<<]

## Force corners again (part2 minified see above)
>>>>>>>>>>[-]+>>>>[-]+[[-]>>>>>>>>[-]+<<<[-]<<[[->>+<<]>>>>>
[-]<<<<<]>>[-<<+>>]>>>]<<<<<<<<<<<<[-]+<<[>>>>>>>>]<<<<<<<
[<<<<<<<<]>[-]+>>>>[-]+[[-]<<<<<<<<[-]+<<<[-]<<[[->>+<<]>>>>>
[-]<<<<<]>>[-<<+>>]>>>]>>>>[-]+<<<<<<<<<<[<<<<<<<<]

### Iteration loop
    <<<< select Iterations
    ]

# Cell counting

    >>>> move to first cell
    >>>>>>>> skip null
    [>>>>>>>>] forward scan

## Initialize digit buffer

### Digit format FVab

    >>>>>>>> skip null
    >>>>>>>> leave 8 cell space between the board and digits
    >>>> skip null digit

    > select V
    ++++++++++++++++ 16 digit max
    [
        [->>>>+<<<<] move V to next V
        +++++++++ set V to 9
        >>>> move to next V
        - dec counter
    ]

    <<<<[<<<<]<<<< scan back

## Collect loop init

    >[-]+ set 1 count
    [

    [-] clear count

    <<<<<<<<< select null
    <<<<<<<<<<<<<<<<[-]+ set 1 to prev S
    >>>>>>>>>>[-]+ set b

## Backwards accumulate

    [
        <<<<<<<< move prev
        [-] clear b
        <[-] clear a
        <<<[-] clear L

        <<< select F
        [->>>+<<<] mov F to L

        <<<[-]>>> clear prev S

        >>> select L
        [
            -<<<+ mov L to F
            >> select V
            [->>>+<<<] add V to S
            >>> select S
            [-<<<<<<<<+>>>>>>>>>>+<<] add S to prev S b
            << select L
        ]

        >>>> select b
    ]

## Forwards move

    << select S
    [->>>>>>>>+<<<<<<<<] add S to next S
    >>> move next
    [
        >>>>> select S
        [->>>>>>>>+<<<<<<<<] add S to next S
        >>> move next
    ]

    >>>>> select S
    - dec S
    [->>>>>>>>>+>+<<<<<<<<<<] mov S to count counter

## Digit sum

    >>>>>>>>>> select counter
    [
        >>>>+ set b

        [
            - sub b
            <<[-]+++++++++ clearset a 9
            >>> move next
            [-]+ clearset F
            >>>[-]+ clearset b
            <[-] clear a
            <[->+<] add V to a
            > select a
            [
                >[-] clear b
                <[-<+>] mov a to V
                <- sub V 1
                > select a (0)
            ]
            > select b
        ]

        <<<[<<<<] scan back

        <- dec counter
    ]
    < select count
    ]


## Final digit printing

    >>>>>>[>>>>] scan forward

    <<<< move prev
    [ flag scan back
        >>[-] clear a
        >[-]+++++ clearset b 5
        [-<++++++++++>] add a 10
        <+++++++ add a 7 to 57
        < select V
        [->->+<<] sub a add b
        >.[-] printclear a
        >[-<<+>>] mov b V
        <<<<<<< move prev
    ]
    ++++++++++.[-] print newline

