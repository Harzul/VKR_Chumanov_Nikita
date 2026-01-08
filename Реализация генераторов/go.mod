module main

go 1.24.2

require (
	extraPRNG.local v0.0.0-00010101000000-000000000000
	x32bits.local v0.0.0-00010101000000-000000000000
	x64bits.local v0.0.0-00010101000000-000000000000
	x64bits_extra.local v0.0.0-00010101000000-000000000000
)

replace x32bits.local => ./X_32bits

replace x64bits.local => ./X_64bits

replace x64bits_extra.local => ./X_64bits_extra

replace extraPRNG.local => ./extra_PRNG
