#!/usr/bin/env lua
--  Copyright (c) 2016 Francois Perrad

local f = arg[1] and assert(io.open(arg[1], 'r')) or io.stdin
local src = string.gsub(f:read'*a', '[^><+%-%.,%[%]]', '')
f:close()

print[[
#!/usr/bin/env lua

io.stdout:setvbuf'no'
local buffer = setmetatable({}, {
    __index = function ()
        return 0    -- default value
    end
})
local ptr = 1
-- end of prologue
]]

print((string.gsub(src, '.', {
    ['+'] = [[
buffer[ptr] = buffer[ptr] + 1
]],
    ['-'] = [[
buffer[ptr] = buffer[ptr] - 1
]],
    ['>'] = [[
ptr = ptr + 1
]],
    ['<'] = [[
ptr = ptr - 1
]],
    ['.'] = [[
io.stdout:write(string.char(buffer[ptr]))
]],
    [','] = [[
buffer[ptr] = string.byte(io.stdin:read(1) or '\0')
]],
    ['['] = [[
while buffer[ptr] ~= 0 do
]],
    [']'] = [[
end
]]
})))
