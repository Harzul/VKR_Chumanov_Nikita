package x64bits

type Xoroshiro128_P struct {
	S [2]uint64
}

func (x Xoroshiro128_P) rotl(a uint64, b int) uint64 {
	return (a << b) | (a >> (64 - b))
}

func (x *Xoroshiro128_P) Next() uint64 {
	var (
		s0     uint64 = x.S[0]
		s1     uint64 = x.S[1]
		result uint64 = s0 + s1
	)
	s1 ^= s0
	(*x).S[0] = x.rotl(s0, 24) ^ s1 ^ (s1 << 16) // a, b
	(*x).S[1] = x.rotl(s1, 37)                   // c
	return result
}
func (x *Xoroshiro128_P) Jump() {
	var JUMP [2]uint64 = [2]uint64{0xdf900294d8f554a5, 0x170865df4b3201fc}
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

func (x *Xoroshiro128_P) Long_jump() {
	var LONG_JUMP [2]uint64 = [2]uint64{0xd2a98b26625eee7b, 0xdddf9b1090aa7ac1}

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
