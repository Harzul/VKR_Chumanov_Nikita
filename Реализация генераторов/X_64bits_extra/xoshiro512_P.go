package x64bits_extra

type Xoshiro512_P struct {
	S [8]uint64
}

func (x Xoshiro512_P) rotl(a uint64, b int) uint64 {
	return (a << b) | (a >> (64 - b))
}

func (x *Xoshiro512_P) Next() uint64 {
	var result uint64 = x.S[0] + x.S[2]
	var t uint64 = x.S[1] << 11

	(*x).S[2] ^= x.S[0]
	(*x).S[5] ^= x.S[1]
	(*x).S[1] ^= x.S[2]
	(*x).S[7] ^= x.S[3]
	(*x).S[3] ^= x.S[4]
	(*x).S[4] ^= x.S[5]
	(*x).S[0] ^= x.S[6]
	(*x).S[6] ^= x.S[7]

	(*x).S[6] ^= t

	(*x).S[7] = x.rotl(x.S[7], 21)

	return result
}
func (x *Xoshiro512_P) Jump() {
	var JUMP [8]uint64 = [8]uint64{0x33ed89b6e7a353f9, 0x760083d7955323be, 0x2837f2fbb5f22fae, 0x4b8c5674d309511c, 0xb11ac47a7ba28c25, 0xf1be7667092bcc1c, 0x53851efdb6df0aaf, 0x1ebbc8b23eaf25db}
	var t [8]uint64 = [8]uint64{0, 0, 0, 0, 0, 0, 0, 0}
	for i := 0; i < 8; i++ {
		for b := 0; b < 64; b++ {
			if (JUMP[i] & (1 << b)) != 0 {
				for w := 0; w < 8; w++ {
					t[w] ^= x.S[w]
				}

			}
			x.Next()
		}
	}

	(*x).S = t
}

func (x *Xoshiro512_P) Long_jump() {
	var LONG_JUMP [8]uint64 = [8]uint64{0x11467fef8f921d28, 0xa2a819f2e79c8ea8, 0xa8299fc284b3959a, 0xb4d347340ca63ee1, 0x1cb0940bedbff6ce, 0xd956c5c4fa1f8e17, 0x915e38fd4eda93bc, 0x5b3ccdfa5d7daca5}
	var t [8]uint64 = [8]uint64{0, 0, 0, 0, 0, 0, 0, 0}
	for i := 0; i < 8; i++ {
		for b := 0; b < 64; b++ {
			if (LONG_JUMP[i] & (1 << b)) != 0 {
				for w := 0; w < 8; w++ {
					t[w] ^= x.S[w]
				}
			}
			x.Next()
		}
	}

	(*x).S = t
}
