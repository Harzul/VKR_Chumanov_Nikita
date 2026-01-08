package x64bits

type Xoshiro256_P struct {
	S [4]uint64
}

func (x Xoshiro256_P) rotl(a uint64, b int) uint64 {
	return (a << b) | (a >> (64 - b))
}

func (x *Xoshiro256_P) Next() uint64 {
	var result uint64 = x.S[0] + x.S[3]

	var t uint64 = x.S[1] << 17

	(*x).S[2] ^= x.S[0]
	(*x).S[3] ^= x.S[1]
	(*x).S[1] ^= x.S[2]
	(*x).S[0] ^= x.S[3]

	(*x).S[2] ^= t

	(*x).S[3] = x.rotl(x.S[3], 45)

	return result
}
func (x *Xoshiro256_P) Jump() {
	var JUMP [4]uint64 = [4]uint64{0x180ec6d33cfd0aba, 0xd5a61266f0c9392c, 0xa9582618e03fc9aa, 0x39abdc4529b1661c}
	var (
		s0 uint64 = 0
		s1 uint64 = 0
		s2 uint64 = 0
		s3 uint64 = 0
	)
	for i := 0; i < 4; i++ {
		for b := 0; b < 64; b++ {
			if (JUMP[i] & (1 << b)) != 0 {
				s0 ^= x.S[0]
				s1 ^= x.S[1]
				s2 ^= x.S[2]
				s3 ^= x.S[3]
			}
			x.Next()
		}
	}

	(*x).S[0] = s0
	(*x).S[1] = s1
	(*x).S[2] = s2
	(*x).S[3] = s3
}

func (x *Xoshiro256_P) Long_jump() {
	var LONG_JUMP [4]uint64 = [4]uint64{0x76e15d3efefdcbbf, 0xc5004e441c522fb3, 0x77710069854ee241, 0x39109bb02acbe635}

	var (
		s0 uint64 = 0
		s1 uint64 = 0
		s2 uint64 = 0
		s3 uint64 = 0
	)
	for i := 0; i < 4; i++ {
		for b := 0; b < 64; b++ {
			if (LONG_JUMP[i] & (1 << b)) != 0 {
				s0 ^= x.S[0]
				s1 ^= x.S[1]
				s2 ^= x.S[2]
				s3 ^= x.S[3]
			}
			x.Next()
		}
	}

	(*x).S[0] = s0
	(*x).S[1] = s1
	(*x).S[2] = s2
	(*x).S[3] = s3
}
