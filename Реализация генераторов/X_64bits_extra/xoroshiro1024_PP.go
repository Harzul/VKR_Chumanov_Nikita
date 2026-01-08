package x64bits_extra

type Xoroshiro1024_PP struct {
	S [16]uint64
	P int
}

func (x Xoroshiro1024_PP) rotl(a uint64, b int) uint64 {
	return (a << b) | (a >> (64 - b))
}

func (x *Xoroshiro1024_PP) Next() uint64 {
	var q = x.P
	var s0 uint64 = x.S[(x.P+1)&15]
	(*x).P = (x.P + 1) & 15
	var s15 uint64 = x.S[q]
	var result uint64 = x.rotl(s0+s15, 23) + s15

	s15 ^= s0
	(*x).S[q] = x.rotl(s0, 25) ^ s15 ^ (s15 << 27)
	(*x).S[x.P] = x.rotl(s15, 36)

	return result
}
func (x *Xoroshiro1024_PP) Jump() {
	var JUMP [16]uint64 = [16]uint64{0x931197d8e3177f17,
		0xb59422e0b9138c5f, 0xf06a6afb49d668bb, 0xacb8a6412c8a1401,
		0x12304ec85f0b3468, 0xb7dfe7079209891e, 0x405b7eec77d9eb14,
		0x34ead68280c44e4a, 0xe0e4ba3e0ac9e366, 0x8f46eda8348905b7,
		0x328bf4dbad90d6ff, 0xc8fd6fb31c9effc3, 0xe899d452d4b67652,
		0x45f387286ade3205, 0x03864f454a8920bd, 0xa68fa28725b1b384}
	var t [16]uint64 = [16]uint64{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	for i := 0; i < 16; i++ {
		for b := 0; b < 64; b++ {
			if (JUMP[i] & (1 << b)) != 0 {
				for j := 0; j < 16; j++ {
					t[j] ^= x.S[(j+x.P)&(16-1)]
				}

			}
			x.Next()
		}
	}

	for i := 0; i < 16; i++ {
		(*x).S[(i+x.P)&(16-1)] = t[i]
	}
}

func (x *Xoroshiro1024_PP) Long_jump() {
	var LONG_JUMP [16]uint64 = [16]uint64{0x7374156360bbf00f,
		0x4630c2efa3b3c1f6, 0x6654183a892786b1, 0x94f7bfcbfb0f1661,
		0x27d8243d3d13eb2d, 0x9701730f3dfb300f, 0x2f293baae6f604ad,
		0xa661831cb60cd8b6, 0x68280c77d9fe008c, 0x50554160f5ba9459,
		0x2fc20b17ec7b2a9a, 0x49189bbdc8ec9f8f, 0x92a65bca41852cc1,
		0xf46820dd0509c12a, 0x52b00c35fbf92185, 0x1e5b3b7f589e03c1}
	var t [16]uint64 = [16]uint64{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	for i := 0; i < 16; i++ {
		for b := 0; b < 64; b++ {
			if (LONG_JUMP[i] & (1 << b)) != 0 {
				for j := 0; j < 16; j++ {
					t[j] ^= x.S[(j+x.P)&(16-1)]
				}

			}
			x.Next()
		}
	}

	for i := 0; i < 16; i++ {
		(*x).S[(i+x.P)&(16-1)] = t[i]
	}
}
