#include "Conversion.h"
#include "FormatInternal.h"

SoundFormatConvertFunc Funcs[SoundIoFormatLast][SoundIoFormatLast]{};

#include "Format_C.cpp"

void InitConversionFuncs() {
	//querie cpu capabilities, endianess, etc
	Funcs[SoundIoFormatFloat32FE][SoundIoFormatFloat32NE] = Float32FE_to_Float32NE_C;
	Funcs[SoundIoFormatFloat64NE][SoundIoFormatFloat32NE] = Float64NE_to_Float32NE_C;
	Funcs[SoundIoFormatFloat64FE][SoundIoFormatFloat32NE] = Float64FE_to_Float32NE_C;
	Funcs[SoundIoFormatU32NE][SoundIoFormatFloat32NE] = U32NE_to_Float32NE_C;
	Funcs[SoundIoFormatU32FE][SoundIoFormatFloat32NE] = U32FE_to_Float32NE_C;
	Funcs[SoundIoFormatS32NE][SoundIoFormatFloat32NE] = S32NE_to_Float32NE_C;
	Funcs[SoundIoFormatS32FE][SoundIoFormatFloat32NE] = S32FE_to_Float32NE_C;
	Funcs[SoundIoFormatU24NE][SoundIoFormatFloat32NE] = U24NE_to_Float32NE_C;
	Funcs[SoundIoFormatU24FE][SoundIoFormatFloat32NE] = U24FE_to_Float32NE_C;
	Funcs[SoundIoFormatS24NE][SoundIoFormatFloat32NE] = S24NE_to_Float32NE_C;
	Funcs[SoundIoFormatS24FE][SoundIoFormatFloat32NE] = S24FE_to_Float32NE_C;
	Funcs[SoundIoFormatU16NE][SoundIoFormatFloat32NE] = U16NE_to_Float32NE_C;
	Funcs[SoundIoFormatU16FE][SoundIoFormatFloat32NE] = U16FE_to_Float32NE_C;
	Funcs[SoundIoFormatS16NE][SoundIoFormatFloat32NE] = S16NE_to_Float32NE_C;
	Funcs[SoundIoFormatS16FE][SoundIoFormatFloat32NE] = S16FE_to_Float32NE_C;
	Funcs[SoundIoFormatU8][SoundIoFormatFloat32NE] = U8_to_Float32NE_C;
	Funcs[SoundIoFormatS8][SoundIoFormatFloat32NE] = S8_to_Float32NE_C;
}
