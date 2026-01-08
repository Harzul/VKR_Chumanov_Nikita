package x64bits

type Xoroshiro128_PP struct {
	S [2]uint64
}

func (x Xoroshiro128_PP) rotl(a uint64, b int) uint64 {
	return (a << b) | (a >> (64 - b))
}

func (x *Xoroshiro128_PP) Next() uint64 {
	var (
		s0     uint64 = x.S[0]
		s1     uint64 = x.S[1]
		result uint64 = x.rotl(s0+s1, 17) + s0
	)
	s1 ^= s0
	(*x).S[0] = x.rotl(s0, 49) ^ s1 ^ (s1 << 21) // a, b
	(*x).S[1] = x.rotl(s1, 28)                   // c
	return result
}
func (x *Xoroshiro128_PP) Jump() {
	var JUMP [2]uint64 = [2]uint64{0x2bd7a6a6e99c2ddc, 0x0992ccaf6a6fca05}
	var (
		s0 uint64 = 0
		s1 uint64 = 0
	)
	for i := 0; i < 2; i++ {
		for b := 0; b < 64; b++ {
			if (JUMP[i] & (1 << b)) != 0 {
				s0 ^= x.S[0]
				s1 ^= x.S[1]
			}
			x.Next()
		}
	}

	(*x).S[0] = s0
	(*x).S[1] = s1
}

func (x *Xoroshiro128_PP) Long_jump() {
	var LONG_JUMP [2]uint64 = [2]uint64{0x360fd5f2cf8d5d99, 0x9c6e6877736c46e3}

	var (
		s0 uint64 = 0
		s1 uint64 = 0
	)
	for i := 0; i < 2; i++ {
		for b := 0; b < 64; b++ {
			if (LONG_JUMP[i] & (1 << b)) != 0 {
				s0 ^= x.S[0]
				s1 ^= x.S[1]
			}
			x.Next()
		}
	}

	(*x).S[0] = s0
	(*x).S[1] = s1
}
