+    set #0 to 1
[    while #0
    >    point to #1
    ,    read char in #1
    >    move to #2
    ++++ set #2 to 4
    [    while #2
        <    go back to #1
        ++++
        ++++ add 8 to #1
        >    go to #2
        -    dec #2
    ]    end while #2
    <    # go to 1
    .    # output 2
    <    # go to 0
] repeat
