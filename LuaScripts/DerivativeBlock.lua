last_input_time = 0
last_input = 0
last_out = 0
data_available = false

function apply(input, output, t, data)
    if not data_available then
        output[1] = 0
        return
    end
    if t == last_out then
        output[1] = last_out
    else
        output[1] = (input[1] - last_input) / (t - last_input_time)
    end
    return
end

function update(input, t, data)
    if not data_available then
        data_available = true
        last_input = input[1]
        last_out = 0
        last_input_time = t
        return
    end
    last_out = (input[1] - last_input) / (t - last_input_time)
    last_input = input[1]
    last_input_time = t
    return
end