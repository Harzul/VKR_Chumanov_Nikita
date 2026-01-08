package x32bits

type Xoshiro128_P struct {
	S [4]uint32
}

func (x Xoshiro128_P) rotl(a uint32, b int) uint32 {
	return (a << b) | (a >> (32 - b))
}

func (x *Xoshiro128_P) Next() uint32 {
	var result uint32 = x.S[0] + x.S[3]

	var t uint32 = x.S[1] << 9

	(*x).S[2] ^= x.S[0]
	(*x).S[3] ^= x.S[1]
	(*x).S[1] ^= x.S[2]
	(*x).S[0] ^= x.S[3]

	(*x).S[2] ^= t

	(*x).S[3] = x.rotl(x.S[3], 11)

	return result
}
func (x *Xoshiro128_P) Jump() {
	var JUMP [4]uint32 = [4]uint32{0x8764000b, 0xf542d2d3, 0x6fa035c3, 0x77f2db5b}
	var (
		s0 uint32 = 0
		s1 uint32 = 0
		s2 uint32 = 0
		s3 uint32 = 0
	)
	for i := 0; i < 4; i++ {
		for b := 0; b < 32; b++ {
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

func (x *Xoshiro128_P) Long_jump() {
	var LONG_JUMP [4]uint32 = [4]uint32{0xb523952e, 0x0b6f099f, 0xccf5a0ef, 0x1c580662}

	var (
		s0 uint32 = 0
		s1 uint32 = 0
		s2 uint32 = 0
		s3 uint32 = 0
	)
	for i := 0; i < 4; i++ {
		for b := 0; b < 32; b++ {
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
