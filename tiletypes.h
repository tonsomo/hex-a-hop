#undef X_DEFAULT
#ifndef X
	#ifndef TILETYPES_H
		#define TILETYPES_H
		#define X_DEFAULT
		#define X(id, col, solid) id,
		enum TileTypes {
	#else
		#define X(id, col, solid)
	#endif
#endif

X(EMPTY,		0x202060, -1)
X(NORMAL,		0x506070, 0)
X(COLLAPSABLE,	0x408040, 0)
X(COLLAPSE_DOOR,0xa0f0a0, 1)
X(TRAMPOLINE,	0x603060, 0)
X(SPINNER,		0x784040, 0)
X(WALL,			0x000080, 1)
X(COLLAPSABLE2,	0x408080, 0)
X(COLLAPSE_DOOR2,0xa0f0f0, 1)
X(GUN,			0xb0a040, 0)
X(TRAP,			0x000000, 0)
X(COLLAPSABLE3,	0x202020, 0)
X(BUILDER,		0x009000, 0)
X(SWITCH,		0x004000, 0)
X(FLOATING_BALL,0xa00050, 0)
X(LIFT_DOWN,	0x7850a0, 0)
X(LIFT_UP,		0x7850a0, 1)

#undef X

#ifdef X_DEFAULT
			NumTileTypes
		};
#undef X_DEFAULT
#endif

