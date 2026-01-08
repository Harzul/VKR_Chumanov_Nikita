import random
import time

import z3
z3.set_param("auto_config", False)
z3.set_param("memory_max_size", 32212254720)
z3.set_param("parallel.enable", True)
z3.set_param("parallel.threads.max", 16)
def xoshiro128p_next(state, p):
    result = (state[0] + state[3]) & ((1 << 32) - 1)

    t = state[1] << 9

    s = list(state)
    s[2] ^= s[0]
    s[3] ^= s[1]
    s[1] ^= s[2]
    s[0] ^= s[3]

    s[2] ^= t
    s[3] = z3.RotateLeft(s[3], 11)

    return result, s, p


def xoshiro256p_next(state, p):
    result = (state[0] + state[3]) & ((1 << 64) - 1)

    t = state[1] << 17

    s = list(state)
    s[2] ^= s[0]
    s[3] ^= s[1]
    s[1] ^= s[2]
    s[0] ^= s[3]

    s[2] ^= t
    s[3] = z3.RotateLeft(s[3], 45)

    return result, s, p

def xoshiro512p_next(state, p):
    result = (state[0] + state[2]) & ((1 << 64) - 1)

    t = state[1] << 11

    s = list(state)
    s[2] ^= s[0]
    s[5] ^= s[1]
    s[1] ^= s[2]
    s[7] ^= s[3]
    s[3] ^= s[4]
    s[4] ^= s[5]
    s[0] ^= s[6]
    s[6] ^= s[7]

    s[6] ^= t
    s[7] = z3.RotateLeft(s[7], 21)

    return result, s, p

def xoroshiro128p_next(state, p):
    result = (state[0] + state[1]) & ((1 << 64) - 1)

    s = list(state)
    s[1] ^= s[0]

    s[0] = z3.RotateLeft(s[0],24) ^ s[1] ^ (s[1] << 16)
    s[1] = z3.RotateLeft(s[1], 37)

    return result, s, p

def xoroshiro64s_next(state, p):
    result = (state[0] * 0x9E3779BB) & ((1 << 32) - 1)

    s = list(state)
    s[1] ^= s[0]

    s[0] = z3.RotateLeft(s[0],26) ^ s[1] ^ (s[1] << 9)
    s[1] = z3.RotateLeft(s[1], 13)

    return result, s, p

def xoroshiro1024s_next(state, p):
    s = list(state)
    q = p
    s0 = s[(p + 1) & 15]
    p = (p + 1) & 15
    s15 = s[q]

    result = (s0 * 0x9e3779b97f4a7c13)& ((1 << 64) - 1)

    s15 = s0 ^ s15
    s[q] = z3.RotateLeft(s0, 25) ^ s15 ^ (s15 << 27)
    s[p] = z3. RotateLeft(s15, 36)

    return result, s, p
def xoshrio128ss_next(state, p):
    result = (z3.RotateLeft(state[1]*5,7)*9) & ((1 << 32) - 1)

    t = state[1] << 9

    s = list(state)
    s[2] ^= s[0]
    s[3] ^= s[1]
    s[1] ^= s[2]
    s[0] ^= s[3]

    s[2] ^= t
    s[3] = z3.RotateLeft(s[3], 11)

    return result, s, p

def xoshrio256ss_next(state, p):
    result = (z3.RotateLeft(state[1]*5,7)*9) & ((1 << 64) - 1)

    t = state[1] << 17

    s = list(state)
    s[2] ^= s[0]
    s[3] ^= s[1]
    s[1] ^= s[2]
    s[0] ^= s[3]

    s[2] ^= t
    s[3] = z3.RotateLeft(s[3], 45)

    return result, s, p

def xoshrio512ss_next(state, p):
    result = (z3.RotateLeft(state[1]*5,7)*9) & ((1 << 64) - 1)

    t = state[1] << 11

    s = list(state)
    s[2] ^= s[0]
    s[5] ^= s[1]
    s[1] ^= s[2]
    s[7] ^= s[3]
    s[3] ^= s[4]
    s[4] ^= s[5]
    s[0] ^= s[6]
    s[6] ^= s[7]

    s[6] ^= t
    s[7] = z3.RotateLeft(s[7], 21)

    return result, s, p

def xoroshiro64ss_next(state, p):
    result = (z3.RotateLeft(state[0]*0x9E3779BB,5)*5) & ((1 << 32) - 1)

    s = list(state)
    s[1] ^= s[0]

    s[0] = z3.RotateLeft(s[0], 26) ^ s[1] ^ (s[1] << 9)
    s[1] = z3.RotateLeft(s[1], 13)

    return result, s, p

def xoroshiro128ss_next(state, p):
    result = (z3.RotateLeft(state[0]*5,7)*9) & ((1 << 64) - 1)

    s = list(state)
    s[1] ^= s[0]

    s[0] = z3.RotateLeft(s[0], 24) ^ s[1] ^ (s[1] << 16)
    s[1] = z3.RotateLeft(s[1], 37)

    return result, s, p

def xoroshiro1024ss_next(state, p):
    s = list(state)
    q = p
    s0 = s[(p + 1) & 15]
    p = (p + 1) & 15
    s15 = s[q]

    result = (z3.RotateLeft(s0*5,7)*9) & ((1 << 64) - 1)

    s15 = s0 ^ s15
    s[q] = z3.RotateLeft(s0, 25) ^ s15 ^ (s15 << 27)
    s[p] = z3.RotateLeft(s15, 36)

    return result, s, p

def build_solver(outputs,state_size,bits, generator):
    g = z3.Goal()

    s = [z3.BitVec(f"s{i}_0", bits) for i in range(state_size)]
    if generator == "xoshiro128+":
        func = xoshiro128p_next
    elif generator == "xoshiro256+":
        func = xoshiro256p_next
    elif generator == "xoshiro512+":
        func = xoshiro512p_next
    elif generator == "xoroshiro128+":
        func = xoroshiro128p_next
    elif generator == "xoroshiro64*":
        func = xoroshiro64s_next
    elif generator == "xoroshiro1024*":
        func = xoroshiro1024s_next
    elif generator == "xoshiro128**":
        func = xoshrio128ss_next
    elif generator == "xoshiro256**":
        func = xoshrio256ss_next
    elif generator == "xoshiro512**":
        func = xoshrio512ss_next
    elif generator == "xoroshiro64**":
        func = xoroshiro64ss_next
    elif generator == "xoroshiro128**":
        func = xoroshiro128ss_next
    elif generator == "xoroshiro1024**":
        func = xoroshiro1024ss_next
    else:
        return
    curr_state = s
    p = 0
    for out in outputs:
        result, curr_state, p = func(curr_state, p)
        g.add(out == result)

    t = z3.Then(
        "solve-eqs",
        "propagate-values",
        "simplify",
        "sat"
    )

    sg = t(g)[0]

    solver = z3.Solver()
    solver.add(sg.as_expr())


    return solver, s


def solve_all_outputs_optimized(outputs,state_size, bits,max_solutions=10, generator=""):
    solver, s = build_solver(outputs,state_size, bits,generator)
    solutions = []
    print("|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||")
    count = 0
    while solver.check() == z3.sat and count < max_solutions:
        model = solver.model()
        state = [model[reg].as_long() if model[reg] is not None else 0 for reg in s]
        solutions.append(state)

        print(f"Решение {count + 1}:")
        for i, val in enumerate(state):
            print(f"s[{i}] = {val}")
        print()
        solver.add(z3.Or([s[i] != state[i] for i in range(state_size)]))
        count += 1

    if not solutions:
        print("Не удалось найти ни одного состояния.")
    else:
        print(f"Всего найдено решений (ограничено max_solutions={max_solutions}): {len(solutions)}")
    return solutions

if __name__ == "__main__":
    start = time.time()

    outputs = []
    step = 4
    for i in range(1000):
        index = random.randint(0, len(outputs) - step - 1)
        tmp = outputs[index:index + step]
        print(tmp)
        solve_all_outputs_optimized(tmp, 4, 32, generator="")
        print(f"Время: {(time.time() - start) / (i + 1)}")

